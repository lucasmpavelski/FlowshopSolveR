#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>
#include <string>

#include "flowshop-solver/MHParamsValues.hpp"
#include "flowshop-solver/aos/frrmab.hpp"
#include "flowshop-solver/aos/lin_ucb.hpp"
#include "flowshop-solver/aos/probability_matching.hpp"
#include "flowshop-solver/aos/random.hpp"
#include "flowshop-solver/aos/thompson_sampling.hpp"
#include "flowshop-solver/fla/AdaptiveWalkLengthFLA.hpp"
#include "flowshop-solver/fla/AutocorrelationFLA.hpp"
#include "flowshop-solver/fla/FitnessDistanceCorrelationFLA.hpp"
#include "flowshop-solver/fla/FitnessHistory.hpp"
#include "flowshop-solver/fla/NeutralityFLA.hpp"
#include "flowshop-solver/fspproblemfactory.hpp"
#include "flowshop-solver/heuristics.hpp"
#include "flowshop-solver/heuristics/AdaptiveDestructionConstruction.hpp"
#include "flowshop-solver/heuristics/BestInsertionExplorer.hpp"
#include "flowshop-solver/heuristics/DestructionConstruction.hpp"
#include "flowshop-solver/heuristics/InsertionStrategy.hpp"
#include "flowshop-solver/heuristics/NEHInit.hpp"
#include "flowshop-solver/heuristics/OpPerturbDestConst.hpp"
#include "flowshop-solver/heuristics/acceptCritTemperature.hpp"
#include "flowshop-solver/heuristics/falseContinuator.hpp"
#include "flowshop-solver/heuristics/ig_lsps.hpp"
#include "flowshop-solver/specsdata.hpp"

#include "flowshop-solver/RunOptions.hpp"
#include "flowshop-solver/eoFactory.hpp"
#include "heuristics/FitnessReward.hpp"

template <class OpT>
class OperatorSelectionFactory {
 public:
  OperatorSelectionFactory() = default;

  auto create(std::vector<OpT> options,
              const MHParamsValues& params,
              ProblemContext& context)
      -> std::shared_ptr<OperatorSelection<OpT>> {
    std::string name = params.categoricalName("IG.AOS.Strategy");
    if (name == "probability_matching") {
      return std::make_unique<ProbabilityMatching<int>>(options);
    } else if (name == "frrmab") {
      return std::make_unique<FRRMAB<int>>(options);
    } else if (name == "linucb") {
      return std::make_unique<LinUCB<OpT>>(options, context);
    } else if (name == "thompson_sampling") {
      return std::make_unique<ThompsonSampling<OpT>>(options);
    } else if (name == "random") {
      return std::make_unique<Random<OpT>>(options);
    } else {
      return {nullptr};
    }
  }
};

template <class Ngh, class EOT = typename Problem<Ngh>::EOT>
auto solveWithIG(Problem<Ngh>& prob,
                 const MHParamsValues& params,
                 eoFactory<Ngh>& factory,
                 const RunOptions& runOptions) -> Result {
  const int N = prob.size(0);
  const int max_nh_size = pow(N - 1, 2);

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

  eoInit<EOT>* init = factory.buildInit();

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
  const std::string neighborhoodStrat =
      params.categoricalName("IG.Neighborhood.Strat");
  if (neighborhoodStrat == "ordered")
    neighborhood = &neighborhood0;
  else if (neighborhoodStrat == "random")
    neighborhood = &neighborhood1;

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
  NeutralityFLA<Ngh> neutralityFLA;

  if (params.categorical("IG.AOS.Strategy") == 2) {
    neighborhoodCheckpoint.add(neutralityFLA);
    checkpoint.add(neighborhoodCheckpoint);
    checkpoint.add(neutralityFLA);
  }

  BestInsertionExplorer<EOT> igexplorer(evalN, neighborhoodCheckpoint, *compNN,
                                        *compSN, fromString(neighborhoodStrat));
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

  auto accept = factory.buildAcceptanceCriterion();

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

  FitnessHistory<EOT> fitnessHistory;
  FitnessRewards<EOT> rewards;
  checkpoint.add(rewards.localStat());
  checkpointGlobal.add(rewards.globalStat());

  if (params.categorical("IG.AOS.Strategy") == 2) {
    checkpoint.add(fitnessHistory);
  }

  std::vector<int> destruction_sizes = {2, 4, 8};
  OperatorSelectionFactory<int> osf;

  AdaptiveWalkLengthFLA<EOT> awSize{fitnessHistory, 1.0 / N};
  AutocorrelationFLA<EOT> autocorr{fitnessHistory};
  FitnessDistanceCorrelationFLA<EOT> fdc{fitnessHistory};
  ProblemContext context;
  context.add(awSize);
  context.add(neutralityFLA);
  context.add(autocorr);
  context.add(fdc);
  auto operator_selection{osf.create(destruction_sizes, params, context)};

  InsertFirstBest<Ngh> insertDC{evalN};
  AdaptiveDestructionConstruction<Ngh> adaptiveDC(
      insertDC, *operator_selection, rewards,
      params.categorical("IG.AOS.RewardType"),
      runOptions.printFitnessReward,
      runOptions.printDestructionChoices
    );
  moMonOpPerturb<Ngh> perturb2(adaptiveDC, fullEval);

  moPerturbation<Ngh>* perturb;
  const std::string name = params.categoricalName("IG.Algo");
  if (name == "rs") {
    perturb = &perturb0;
  } else if (name == "lsps") {
    perturb = &perturb1;
  } else if (name == "adaptive") {
    perturb = &perturb2;
  } else {
    assert(false);
  }

  /****
  *** ILS
  ****/
  moILS<Ngh, Ngh> ils(*algo, fullEval, checkpointGlobal, *perturb, *accept);

  return runExperiment(*init, ils, prob, runOptions);
}
