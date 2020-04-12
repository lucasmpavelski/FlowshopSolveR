#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "MHParamsValues.hpp"
#include "NEHInit.hpp"
#include "OpPerturbDestConst.hpp"
#include "acceptCritTemperature.hpp"
#include "adaptive_destruction.hpp"
#include "aos/frrmab.hpp"
#include "aos/lin_ucb.hpp"
#include "aos/probability_matching.hpp"
#include "aos/random.hpp"
#include "aos/thompson_sampling.hpp"
#include "falseContinuator.hpp"
#include "fla/AdaptiveWalkLengthFLA.hpp"
#include "fla/AutocorrelationFLA.hpp"
#include "fla/FitnessDistanceCorrelationFLA.hpp"
#include "fla/FitnessHistory.hpp"
#include "fla/NeutralityFLA.hpp"
#include "fspproblemfactory.hpp"
#include "heuristics.hpp"
#include "heuristics/BestInsertionExplorer.hpp"
#include "heuristics/InsertionStrategy.hpp"
#include "heuristics/fastigexplorer.hpp"
#include "ig_lsps.hpp"
#include "specsdata.hpp"

template <class OpT>
class OperatorSelectionFactory {
 public:
  OperatorSelectionFactory() = default;

  auto create(std::vector<OpT> options,
              const MHParamsValues& params,
              ProblemContext& context)
      -> std::shared_ptr<OperatorSelection<OpT>> {
    switch (params.categorical("IG.AOS.Strategy")) {
      case 0:
        return std::make_unique<ProbabilityMatching<int>>(options);
      case 1:
        return std::make_unique<FRRMAB<int>>(options);
      case 2:
        return std::make_unique<LinUCB<OpT>>(options, context);
      case 3:
        return std::make_unique<ThompsonSampling<OpT>>(options);
      case 4:
        return std::make_unique<Random<OpT>>(options);
      default:
        return {nullptr};
    }
  }
};

auto solveWithIG(
    const std::unordered_map<std::string, std::string>& problem_specs,
    const std::unordered_map<std::string, double>& param_values) -> Result {
  MHParamsSpecs specs = MHParamsSpecsFactory::get("IG");
  MHParamsValues params(&specs);
  params.readValues(param_values);

  using EOT = FSPProblem::EOT;
  using Ngh = FSPProblem::Ngh;
  EOT sol;

  FSPProblem prob = FSPProblemFactory::get(problem_specs);
  const int N = prob.size(0);
  const int M = prob.size(1);
  const int max_nh_size = pow(N - 1, 2);
  const std::string mh = params.mhName();
  const double max_ct = prob.upperBound();

  // continuator
  eoEvalFunc<EOT>& fullEval = prob.eval();
  moEval<Ngh>& evalN = prob.neighborEval();

  moContinuator<Ngh>& continuator = prob.continuator();
  moCheckpoint<Ngh>& checkpoint = prob.checkpoint();
  moCheckpoint<Ngh>& checkpointGlobal = prob.checkpointGlobal();

  // comparator strategy
  moSolComparator<EOT> compSS0;               // comp sol/sol strict
  moSolNeighborComparator<Ngh> compSN0;       // comp sol/Ngh strict
  moNeighborComparator<Ngh> compNN0;          // comp Ngh/Ngh strict
  moEqualSolComparator<EOT> compSS1;          // comp sol/sol with equal
  moEqualSolNeighborComparator<Ngh> compSN1;  // comp sol/Ngh with equal
  moEqualNeighborComparator<Ngh> compNN1;     // comp Ngh/Ngh with equal
  moSolComparator<EOT>* compSS = nullptr;
  moSolNeighborComparator<Ngh>* compSN = nullptr;
  moNeighborComparator<Ngh>* compNN = nullptr;
  switch (params.categorical("IG.Comp.Strat")) {
    case 0:
      compSS = &compSS0;
      compSN = &compSN0;
      compNN = &compNN0;
      break;
    case 1:
      compSS = &compSS1;
      compSN = &compSN1;
      compNN = &compNN1;
      break;
    default:
      assert(false);
      break;
  }

  // initialization
  eoInitPermutation<EOT> init0(N);
  NEHInit<EOT> init1(fullEval, N, *compSS);
  int cycle = 3;
  NEHInitRandom<EOT> init2(fullEval, N, cycle, *compSS);
  // FastNEH fastNeh(prob.getData());
  // FastNEHRandom init2(prob.getData());
  eoInit<EOT>* init = nullptr;

  switch (params.categorical("IG.Init.Strat")) {
    case 0:
      init = &init0;
      break;
    case 1:
      init = &init1;
      break;
    case 2:
      init = &init2;
      break;
    default:
      throw std::runtime_error(
          "Unknonwn IG.Init.Strat value " +
          std::to_string(params.categorical("IG.Init.Strat")));
      break;
  }

  // neighborhood size
  const int min_nh_size = (N >= 20) ? 11 : 2;
  const int nh_interval = (N >= 20) ? 10 : 1;
  const int no_nh_sizes = (max_nh_size - min_nh_size) / nh_interval + 1;
  const int scale =
      static_cast<int>(0.1 * no_nh_sizes * params.real("IG.Neighborhood.Size"));
  const int nh_size = std::min(max_nh_size, min_nh_size + scale * nh_interval);

  moOrderNeighborhood<Ngh> neighborhood0(nh_size);
  moRndWithoutReplNeighborhood<Ngh> neighborhood1(nh_size);
  moNeighborhood<Ngh>* neighborhood = nullptr;
  switch (params.categorical("IG.Neighborhood.Strat")) {
    case 0:
      neighborhood = &neighborhood0;
      break;
    case 1:
      neighborhood = &neighborhood1;
      break;
    default:
      assert(false);
      break;
  }

  // algos xxHC
  moFirstImprHC<Ngh> algo0(*neighborhood, fullEval, evalN, checkpoint, *compNN,
                           *compSN);  // FIHC
  moSimpleHC<Ngh> algo1(*neighborhood, fullEval, evalN, checkpoint, *compNN,
                        *compSN);  // BestHC
  moRandomBestHC<Ngh> algo2(*neighborhood, fullEval, evalN, checkpoint, *compNN,
                            *compSN);  // rndBestHC

  // IG (Ruiz+Stuetzle)
  // iterative greedy improvement without replacement (IG)
  // FastIGexplorer igexplorer(evalN, *compNN, *compSN);
  moTrueContinuator<Ngh> tc;
  NeigborhoodCheckpoint<Ngh> neighborhoodCheckpoint{tc};
  NeutralityFLA<EOT> neutralityFLA{compSS0};

  if (params.categorical("IG.AOS.Strategy") == 2) {
    neighborhoodCheckpoint.add(neutralityFLA);
    checkpoint.add(neighborhoodCheckpoint);
    checkpoint.add(neutralityFLA);
  }

  BestInsertionExplorer<EOT> igexplorer(evalN, neighborhoodCheckpoint, *compNN,
                                        *compSN);
  moLocalSearch<Ngh> algo3(igexplorer, checkpoint, fullEval);
  // IGexplorerWithRepl<Ngh> igWithReplexplorer(fullEval, N, *compSS); //
  // iterative greedy improvement with replacement moLocalSearch<Ngh>
  // algo4(igWithReplexplorer, checkpoint, fullEval);
  moLocalSearch<Ngh>* algo;
  switch (params.categorical("IG.Local.Search")) {
    case 0:
      algo = &algo0;
      break;
    case 1:
      algo = &algo1;
      break;
    case 2:
      algo = &algo2;
      break;
    case 3:
      algo = &algo3;
      break;
    // case 4: algo=&algo4; break;
    default:
      assert(false);
      break;
  }
  moCombinedContinuator<Ngh> singleStepContinuator(checkpoint);
  falseContinuator<Ngh> falseCont;
  singleStepContinuator.add(falseCont);
  if (params.categorical("IG.LS.Single.Step")) {
    algo->setContinuator(singleStepContinuator);
  }

  moAlwaysAcceptCrit<Ngh> accept0;
  // no interest to accept equal solution here !
  moBetterAcceptCrit<Ngh> accept1(compSS0);
  // IG accept criterion based on temperature
  const double temperature =
      params.real("IG.Accept.Temperature") * max_ct / (N * M * 10);
  acceptCritTemperature<Ngh> accept2(temperature);

  moAcceptanceCriterion<Ngh>* accept;
  switch (params.categorical("IG.Accept")) {
    case 0:
      accept = &accept0;
      break;
    case 1:
      accept = &accept1;
      break;
    case 2:
      accept = &accept2;
      break;
    default:
      assert(false);
      break;
  }

  /****
  *** Perturb
  ****/
  const int destruction_size = params.integer("IG.Destruction.Size");
  InsertFirstBest<Ngh> perturbInsertion(evalN);
  DestructionConstruction<Ngh> OpPerturb(perturbInsertion, destruction_size);
  moMonOpPerturb<Ngh> perturb0(OpPerturb, fullEval);

  const int N_lsps = N - destruction_size;
  const int nh_size_lsps =
      getNhSize(N_lsps, params.real("IG.Neighborhood.Size"));

  moOrderNeighborhood<Ngh> neighborhood0_lsps(nh_size_lsps);
  moRndWithoutReplNeighborhood<Ngh> neighborhood1_lsps(nh_size_lsps);
  moNeighborhood<Ngh>* neighborhood_lsps = nullptr;
  switch (params.categorical("IG.Neighborhood.Strat")) {
    case 0:
      neighborhood_lsps = &neighborhood0_lsps;
      break;
    case 1:
      neighborhood_lsps = &neighborhood1_lsps;
      break;
    default:
      assert(false);
      break;
  }

  // algos xxHC_lsps
  moFirstImprHC<Ngh> algo0_lsps(*neighborhood_lsps, fullEval, evalN,
                                continuator, *compNN,
                                *compSN);  // FIHC
  moSimpleHC<Ngh> algo1_lsps(*neighborhood_lsps, fullEval, evalN, continuator,
                             *compNN,
                             *compSN);  // BestHC
  moRandomBestHC<Ngh> algo2_lsps(*neighborhood_lsps, fullEval, evalN,
                                 continuator, *compNN,
                                 *compSN);  // rndBestHC
  BestInsertionExplorer<EOT> igexplorer_lsps(evalN, neighborhoodCheckpoint,
                                             *compNN, *compSN);
  moLocalSearch<Ngh> algo3_lsps(igexplorer_lsps, continuator, fullEval);

  moLocalSearch<Ngh>* igLSPSLocalSearh = nullptr;
  switch (params.categorical("IG.LSPS.Local.Search")) {
    case 0:
      igLSPSLocalSearh = &algo0_lsps;
      break;
    case 1:
      igLSPSLocalSearh = &algo1_lsps;
      break;
    case 2:
      igLSPSLocalSearh = &algo2_lsps;
      break;
    case 3:
      igLSPSLocalSearh = &algo3_lsps;
      break;
    default:
      throw std::runtime_error(
          "Unknown IG LSPS local search: " +
          std::to_string(params.categorical("IG.LSPS.Local.Search")));
      break;
  }

  moTrueContinuator<Ngh> trueCont;
  if (params.categorical("IG.LSPS.Single.Step")) {
    igLSPSLocalSearh->setContinuator(singleStepContinuator);
  }

  InsertFirstBest<Ngh> insert(evalN);
  IGLocalSearchPartialSolution<Ngh> igLSPS(insert, *igLSPSLocalSearh,
                                           destruction_size, *compSS);
  moMonOpPerturb<Ngh> perturb1(igLSPS, fullEval);

  FitnessReward<EOT> fitness_reward;
  FitnessHistory<EOT> fitness_history;
  checkpoint.add(fitness_reward);

  if (params.categorical("IG.AOS.Strategy") == 2) {
    checkpoint.add(fitness_history);
  }

  std::vector<int> destruction_sizes = {2, 4, 8};
  OperatorSelectionFactory<int> osf;

  AdaptiveWalkLengthFLA<EOT> awSize{fitness_history};
  AutocorrelationFLA<EOT> autocorr{fitness_history};
  FitnessDistanceCorrelationFLA<EOT> fdc{fitness_history};
  ProblemContext context;
  context.add(awSize);
  context.add(neutralityFLA);
  context.add(autocorr);
  context.add(fdc);
  auto operator_selection{osf.create(destruction_sizes, params, context)};

  AdaptiveDestruction<EOT> adaptiveDestruction(fullEval, *operator_selection,
                                               fitness_reward);
  moMonOpPerturb<Ngh> perturb2(adaptiveDestruction, fullEval);

  moPerturbation<Ngh>* perturb;
  switch (params.categorical("IG.Algo")) {
    case 0:
      perturb = &perturb0;
      break;
    case 1:
      perturb = &perturb1;
      break;
    case 2:
      perturb = &perturb2;
      break;
    default:
      assert(false);
      break;
  }

  /****
  *** ILS
  ****/
  moILS<Ngh, Ngh> ils(*algo, fullEval, checkpointGlobal, *perturb, *accept);

  return runExperiment(*init, ils, prob);
}