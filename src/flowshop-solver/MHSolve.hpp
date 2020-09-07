#pragma once

#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>

// paradiseo libs
#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

// problem
#include "flowshop-solver/global.hpp"
#include "flowshop-solver/problems/FSPEvalFunc.hpp"
#include "flowshop-solver/problems/NIFSPEvalFunc.hpp"
#include "flowshop-solver/problems/NWFSPEvalFunc.hpp"

// NEH
#include "flowshop-solver/heuristics/FSPOrderHeuristics.hpp"
#include "flowshop-solver/heuristics/NEHInit.hpp"

// TS
#include "flowshop-solver/heuristics/dummyAspiration.hpp"
#include "flowshop-solver/heuristics/moFirstBestTS.hpp"
#include "flowshop-solver/heuristics/moFirstTS.hpp"

// ILS
#include "flowshop-solver/heuristics/IGexplorer.hpp"
#include "flowshop-solver/heuristics/IGexplorerWithRepl.hpp"
// IG components
#include "flowshop-solver/heuristics/perturb/perturb.hpp"
#include "flowshop-solver/heuristics/acceptCritTemperature.hpp"

#include "flowshop-solver/MHParamsValues.hpp"
#include "flowshop-solver/FSPProblemFactory.hpp"
#include "flowshop-solver/problems/FSPProblem.hpp"
#include "flowshop-solver/MHParamsSpecsFactory.hpp"

template <class Ngh, class EOT = typename Ngh::EOT>
void solveILS(const int N,
              const int M,
              const int max_ct,
              const std::vector<int>& order,
              const int nh_size,
              eoEvalFuncCounter<EOT>& fullEval,
              moEvalCounter<Ngh>& evalN,
              moCheckpoint<Ngh>& checkpoint,
              moCheckpoint<Ngh>& checkpointGlobal,
              const MHParamsValues& params) {
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
  switch (params.categorical("Comp.Strat")) {
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
  NEHInitOrdered<EOT> init1(fullEval, order);
  NEHInitRandom<EOT> init2(fullEval, N);
  eoInit<EOT>* init = nullptr;
  switch (params.categorical("Init.Strat")) {
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
      assert(false);
      break;
  }

  // neighborhood
  moOrderNeighborhood<Ngh> neighborhood0(nh_size);
  moRndWithoutReplNeighborhood<Ngh> neighborhood1(nh_size);
  moNeighborhood<Ngh>* neighborhood = nullptr;
  switch (params.categorical("Neighborhood.Strat")) {
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
  IGexplorer<Ngh> igexplorer(
      fullEval, N,
      *compSS);  // iterative greedy improvement without replacement (IG)
  moLocalSearch<Ngh> algo3(igexplorer, checkpoint, fullEval);
  IGexplorerWithRepl<Ngh> igWithReplexplorer(
      fullEval, N, *compSS);  // iterative greedy improvement with replacement
  moLocalSearch<Ngh> algo4(igWithReplexplorer, checkpoint, fullEval);

  moLocalSearch<Ngh>* algo;
  switch (params.categorical("ILS.Algo")) {
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
    case 4:
      algo = &algo4;
      break;
    default:
      assert(false);
      break;
  }

  moAlwaysAcceptCrit<Ngh> accept0;
  moBetterAcceptCrit<Ngh> accept1(
      compSS0);  // no interest to accept equal solution here !
  // IG accept criterion based on temperature
  const double temperature =
      params.real("ILS.Accept.Temperature") * max_ct / (N * M * 10);
  acceptCritTemperature<Ngh> accept2(temperature);

  moAcceptanceCriterion<Ngh>* accept;
  switch (params.categorical("ILS.Accept")) {
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
  *** Perturb in ILS
  ****/

  eoInit<EOT>* perturbRestartInit = nullptr;
  switch (params.categorical("ILS.Perturb.Restart.Init")) {
    case 0:
      perturbRestartInit = &init0;
      break;
    case 1:
      perturbRestartInit = &init1;
      break;
    case 2:
      perturbRestartInit = &init2;
      break;
    default:
      assert(false);
      break;
  }

  moRestartPerturb<Ngh> perturb0(
      *perturbRestartInit, fullEval,
      params.integer("ILS.Perturb.Restart.Threshold"));
  // IG perturbation
  OpPerturbDestConst<EOT> OpPerturb(
      fullEval, params.integer("ILS.Perturb.Destruction.Size"));
  moMonOpPerturb<Ngh> perturb1(OpPerturb, fullEval);
  // Kick perturbation with Exhange
  // eoSwapMutation<EOT> kickPerturb(params.integer("ILS.Perturb.No.Kick"));
  // moMonOpPerturb<Ngh> perturb2(kickPerturb, fullEval);

  // init -> monOp in order to use it in nilsPerturb framework
  eoInitAdaptor<EOT> initPerturb0(init0);  // init perturb random
  eoInitAdaptor<EOT> initPerturb1(init1);  // init perturb NEH
  eoInitAdaptor<EOT> initPerturb2(init2);  // init perturb NEHrnd
  OpPerturbDestConst<EOT> NILSOpPerturb(
      fullEval, params.integer("ILS.Perturb.NILS.Destruction.Size"));
  // eoSwapMutation<EOT> NILSkickPerturb(
  //    params.integer("ILS.Perturb.NILS.No.Kick"));

  eoMonOp<EOT>* nilsEscape;
  switch (params.categorical("ILS.Perturb.NILS.Escape")) {
    case 0:
      nilsEscape = &initPerturb0;
      break;
    case 1:
      nilsEscape = &initPerturb1;
      break;
    case 2:
      nilsEscape = &initPerturb2;
      break;
    case 3:
      nilsEscape = &NILSOpPerturb;
      break;
    case 4:
      nilsEscape = nullptr;
      break;
    default:
      assert(false);
      break;
  }

  // NILS perturbation : nilsMNS
  randomNeutralWalkExplorer<Ngh> nilsPerturb(
      *neighborhood, evalN, fullEval, compSN0,
      params.integer("ILS.Perturb.NILS.MNS"), *nilsEscape);
  moLocalSearch<Ngh> nilsPerturbLS(nilsPerturb, checkpoint, fullEval);
  moMonOpPerturb<Ngh> perturb3(nilsPerturbLS, fullEval);

  moPerturbation<Ngh>* perturb;
  switch (params.categorical("ILS.Perturb")) {
    case 0:
      perturb = &perturb0;
      break;
    case 1:
      perturb = &perturb1;
      break;
    case 2:
      perturb = nullptr;
      break;
    case 3:
      perturb = &perturb3;
      break;
    default:
      assert(false);
      break;
  }

  /****
  *** ILS
  ****/

  //	moTimeContinuator<Ngh> continuatorGlobal(maxTime, false);
  //	moTrueContinuator<Ngh> continuatorGlobal;
  // fspEvalContinuator<Ngh> continuatorGlobal(fullEval, maxEval, true);
  // checkpoint pour best !!
  // moCheckpoint<Ngh> checkpointGlobal(continuatorGlobal);
  // checkpointGlobal.add(bestFound);

  moILS<Ngh, Ngh> ils(*algo, fullEval, checkpointGlobal, *perturb, *accept);
  iterateUntilMaxEval(*init, ils, fullEval, checkpointGlobal);
}

template <class Ngh, class EOT = typename Ngh::EOT>
void iterateUntilMaxEval(eoInit<EOT>& init,
                         moLocalSearch<Ngh>& algo,
                         eoEvalFunc<EOT>& eval,
                         moCheckpoint<Ngh>& checkpoint) {
  moRestartPerturb<Ngh> perturb(init, eval, 0);
  moAlwaysAcceptCrit<Ngh> accept;
  moILS<Ngh, Ngh> ils(algo, eval, checkpoint, perturb, accept);
  EOT sol;
  init(sol);
  eval(sol);
  ils(sol);
}

template <class Ngh, class EOT = typename Ngh::EOT>
double evaluate(const MHParamsValues& params,
                Problem<Ngh>& prob,
                const std::vector<int>& order) {
  const int N = prob.size(0);
  const int max_nh_size = pow(N - 1, 2);
  const std::string mh = params.mhName();

  // continuator
  moContinuator<Ngh>& continuator = prob.continuator();
  eoEvalFunc<EOT>& fullEval = prob.eval();
  moEval<Ngh>& evalN = prob.neighborEval();

  moCheckpoint<Ngh> checkpoint(continuator);
  moBestSoFarStat<EOT> bestFound(true);
  checkpoint.add(bestFound);
  moCheckpoint<Ngh> checkpointGlobal(continuator);
  moBestSoFarStat<EOT> bestFoundGlobal(false);
  checkpoint.add(bestFoundGlobal);

  // debug
  //  prefixedPrinter print("local_best:", " ");
  //  print.add(bestFound);
  //  prefixedPrinter printg("global_best:", " ");
  //  printg.add(bestFoundGlobal);
  //  checkpoint.add(print);
  //  checkpointGlobal.add(printg);

  // initialization
  eoInitPermutation<EOT> init0(N);
  NEHInitOrdered<EOT> init1(fullEval, order);
  NEHInitRandom<EOT> init2(fullEval, N);
  eoInit<EOT>* init = nullptr;

  switch (params.categorical("Init.Strat")) {
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
      assert(false);
      break;
  }

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
  switch (params.categorical("Comp.Strat")) {
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
  // neighborhood size
  const int min_nh_size = (N >= 20) ? 11 : 2;
  const int nh_interval = (N >= 20) ? 10 : 1;
  const int no_nh_sizes = (max_nh_size - min_nh_size) / nh_interval;
  const int scale =
      int((no_nh_sizes + 1) * params.real("Neighborhood.Size") / 10.0);
  const int nh_size = std::min(max_nh_size, min_nh_size + scale * nh_interval);

  if (mh == "HC") {
    moOrderNeighborhood<Ngh> neighborhood0(nh_size);
    moRndWithoutReplNeighborhood<Ngh> neighborhood1(nh_size);
    moNeighborhood<Ngh>* neighborhood = nullptr;
    switch (params.categorical("Neighborhood.Strat")) {
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
    // HC algorithms
    moFirstImprHC<Ngh> fi(*neighborhood, fullEval, evalN, checkpoint, *compNN,
                          *compSN);
    moSimpleHC<Ngh> best(*neighborhood, fullEval, evalN, checkpoint, *compNN,
                         *compSN);
    moRandomBestHC<Ngh> rand_best(*neighborhood, fullEval, evalN, checkpoint,
                                  *compNN, *compSN);
    moLocalSearch<Ngh>* algo = nullptr;
    switch (params.categorical("HC.Algo")) {
      case 0:
        algo = &fi;
        break;
      case 1:
        algo = &best;
        break;
      case 2:
        algo = &rand_best;
        break;
      default:
        assert(false);
        break;
    }
    iterateUntilMaxEval(*init, *algo, fullEval, checkpointGlobal);
  } else if (mh == "SA") {
    // neighborhood (fixed with strategy = RANDOM)
    moRndWithoutReplNeighborhood<Ngh> neighborhood(nh_size);
    // cooling schedule
    moSimpleCoolingSchedule<EOT> cooling0(
        params.real("Init.Temp"), params.real("Alpha"),
        params.integer("Span.Simple"), params.real("Final.Temp"));
    moDynSpanCoolingSchedule<EOT> cooling1(
        params.real("Init.Temp"), params.real("Alpha"),
        params.integer("Span.Tries.Max"), params.integer("Span.Move.Max"),
        params.integer("Nb.Span.Max"));
    moCoolingSchedule<EOT>* cooling = nullptr;
    switch (params.categorical("SA.Algo")) {
      case 0:
        cooling = &cooling0;
        break;
      case 1:
        cooling = &cooling1;
        break;
      default:
        assert(false);
        break;
    }

    // SA algorithm
    moSA<Ngh> algo(neighborhood, fullEval, evalN, *cooling, *compSN,
                   checkpoint);
    iterateUntilMaxEval(*init, algo, fullEval, checkpointGlobal);

  } else if (mh == "TS") {
    // neighborhood
    moOrderNeighborhood<Ngh> neighborhood0(nh_size);
    moRndWithoutReplNeighborhood<Ngh> neighborhood1(nh_size);
    moNeighborhood<Ngh>* neighborhood = nullptr;
    switch (params.categorical("Neighborhood.Strat")) {
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
    // tabu list
    // Indexed TL: howlongTaboo=nb iterations a move is taboo
    moIndexedVectorTabuList<Ngh> tabuList0(nh_size,
                                           params.integer("How.Long.Taboo"));
    // (+Rnd:uniform value added)
    moRndIndexedVectorTabuList<Ngh> tabuList1(
        nh_size, params.integer("How.Long.Taboo"),
        params.integer("How.Long.Rnd.Taboo"));
    // for tabuList{2,3}, if howlong=0 : tabu while tabu list is not full
    moNeighborVectorTabuList<Ngh> tabuList2(params.integer("Max.Size.TL"),
                                            params.integer("How.Long.Taboo"));
    moSolVectorTabuList<Ngh> tabuList3(params.integer("Max.Size.TL"),
                                       params.integer("How.Long.Taboo"));
    moTabuList<Ngh>* tabuList = nullptr;
    switch (params.categorical("Tabu.List.Type")) {
      case 0:
        tabuList = &tabuList0;
        break;
      case 1:
        tabuList = &tabuList1;
        break;
      case 2:
        tabuList = &tabuList2;
        break;
      case 3:
        tabuList = &tabuList3;
        break;
      default:
        assert(false);
        break;
    }
    // aspiration
    dummyAspiration<Ngh> aspiration0;
    moBestImprAspiration<Ngh> aspiration1;
    moAspiration<Ngh>* aspiration = nullptr;
    switch (params.categorical("Aspiration")) {
      case 0:
        aspiration = &aspiration0;
        break;
      case 1:
        aspiration = &aspiration1;
        break;
      default:
        assert(false);
        break;
    }
    // TS algorithm
    moDummyIntensification<Ngh> dummyIntensification;
    moDummyDiversification<Ngh> dummyDiversification;
    moTS<Ngh> algo0(*neighborhood, fullEval, evalN, *compNN, *compSN,
                    checkpoint, *tabuList, dummyIntensification,
                    dummyDiversification, *aspiration);
    moFirstTS<Ngh> algo1(*neighborhood, fullEval, evalN, *compNN, *compSN,
                         checkpoint, *tabuList, dummyIntensification,
                         dummyDiversification, *aspiration);
    moFirstBestTS<Ngh> algo2(*neighborhood, fullEval, evalN, *compNN, *compSN,
                             checkpoint, *tabuList, dummyIntensification,
                             dummyDiversification, *aspiration);
    moLocalSearch<Ngh>* algo = nullptr;
    switch (params.categorical("TS.Algo")) {
      case 0:
        algo = &algo0;
        break;
      case 1:
        algo = &algo1;
        break;
      case 2:
        algo = &algo2;
        break;
      default:
        assert(false);
        break;
    }
    iterateUntilMaxEval(*init, *algo, fullEval, checkpointGlobal);
  } else if (mh == "ILS") {
    //  solveILS(N, M, max_ct, order, nh_size, fullEval, evalN, checkpoint,
    //  checkpointGlobal, params);
  } else if (mh == "ILS2") {
    // solveILS2(N, M, max_ct, order, fullEval, evalN, checkpoint,
    // checkpointGlobal, params);
  } else {
    assert(false);
  }
  return bestFoundGlobal.value().fitness();
}

template <class Ngh, class EOT = typename Ngh::EOT>
double evaluateMean(const MHParamsValues& params,
                    Problem<Ngh>& prob,
                    const EOT& order,
                    int no_samples) {
  prob.reset();
  double sum = 0.0;
  for (int i = 0; i < no_samples; i++) {
    sum += evaluate(params, prob, order);
    prob.reset();
  }
  return sum / no_samples;
}

void initFactories(const std::string& instances_folder,
                   const std::string& specs_folder,
                   bool quiet = false) {
  FSPProblemFactory::init(instances_folder);
  MHParamsSpecsFactory::init(specs_folder, quiet);
}

float solveWithFactories(
    const std::string& mh_name,
    const std::unordered_map<std::string, std::string>& prob_data,
    const long seed,
    const std::unordered_map<std::string, float>& params) {
  rng.reseed(seed);
  MHParamsSpecs mh_specs = MHParamsSpecsFactory::get(mh_name);
  MHParamsValues values(&mh_specs);
  for (auto ps : mh_specs) {
    if (params.find(ps->name) == params.end())
      throw std::runtime_error("Parameter " + ps->name + " needs a value!");
    values[ps->name] = params.at(ps->name);
  }
  const std::string problem_name = prob_data.at("problem");
  double result = 0.0;
  if (problem_name == "FSP") {
    auto problem = FSPProblemFactory::get(prob_data);
    problem.reset();
  //  result = evaluate(values, problem, totalProcTimes(problem.getData()));
  } else {
    throw std::runtime_error("Unknown problem: " + problem_name);
  }
  return result;
}
