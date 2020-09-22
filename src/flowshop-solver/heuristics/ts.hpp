#pragma once

#include <paradiseo/mo/continuator/moBestNoImproveContinuator.h>
#include <paradiseo/mo/continuator/moCombinedContinuator.h>

#include <unordered_map>

#include "MHParamsValues.hpp"
#include "NEHInit.hpp"
#include "dummyAspiration.hpp"
#include "flowshop-solver/heuristics.hpp"
#include "FSPProblemFactory.hpp"
#include "moFirstBestTS.hpp"
#include "moFirstTS.hpp"
#include "MHParamsSpecsFactory.hpp"

template <class Ngh, class EOT = typename Problem<Ngh>::EOT>
auto solveWithTS(Problem<Ngh>& prob, const MHParamsValues& params) -> Result {
  const int N = prob.size(0);
  const int max_nh_size = pow(N - 1, 2);

  // continuator
  eoEvalFunc<EOT>& fullEval = prob.eval();
  moEval<Ngh>& evalN = prob.neighborEval();

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
  switch (params.categorical("TS.Comp.Strat")) {
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

  switch (params.categorical("TS.Init.Strat")) {
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
          "Unknonwn TS.Init.Strat value " +
          std::to_string(params.categorical("TS.Init.Strat")));
      break;
  }

  // neighborhood size
  const int min_nh_size = (N >= 20) ? 11 : 2;
  const int nh_interval = (N >= 20) ? 10 : 1;
  const int no_nh_sizes = (max_nh_size - min_nh_size) / nh_interval;
  const int scale =
      int((no_nh_sizes + 1) * params.real("TS.Neighborhood.Size") / 10.0);
  const int nh_size = std::min(max_nh_size, min_nh_size + scale * nh_interval);

  moOrderNeighborhood<Ngh> neighborhood0(nh_size);
  moRndWithoutReplNeighborhood<Ngh> neighborhood1(nh_size);
  moNeighborhood<Ngh>* neighborhood = nullptr;
  switch (params.categorical("TS.Neighborhood.Strat")) {
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
                                         params.integer("TS.How.Long.Taboo"));
  // (+Rnd:uniform value added)
  moRndIndexedVectorTabuList<Ngh> tabuList1(
      nh_size, params.integer("TS.How.Long.Taboo"),
      params.integer("TS.How.Long.Rnd.Taboo"));
  // for tabuList{2,3}, if howlong=0 : tabu while tabu list is not full
  moNeighborVectorTabuList<Ngh> tabuList2(params.integer("TS.Max.Size.TL"),
                                          params.integer("TS.How.Long.Taboo"));
  moSolVectorTabuList<Ngh> tabuList3(params.integer("TS.Max.Size.TL"),
                                     params.integer("TS.How.Long.Taboo"));
  moTabuList<Ngh>* tabuList = nullptr;
  switch (params.categorical("TS.Tabu.List.Type")) {
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
  switch (params.categorical("TS.Aspiration")) {
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

  int max_iter_no_improve = params.integer("TS.Max.Stagnation.Window");
  moBestNoImproveContinuator<Ngh> best_no_improve(prob.bestLocalSoFar().value(),
                                                  max_iter_no_improve, false);
  moCombinedContinuator<Ngh> localCont(best_no_improve);
  localCont.add(checkpoint);

  // TS algorithm
  moDummyIntensification<Ngh> dummyIntensification;
  moDummyDiversification<Ngh> dummyDiversification;
  moTS<Ngh> algo0(*neighborhood, fullEval, evalN, *compNN, *compSN, localCont,
                  *tabuList, dummyIntensification, dummyDiversification,
                  *aspiration);
  moFirstTS<Ngh> algo1(*neighborhood, fullEval, evalN, *compNN, *compSN,
                       localCont, *tabuList, dummyIntensification,
                       dummyDiversification, *aspiration);
  moFirstBestTS<Ngh> algo2(*neighborhood, fullEval, evalN, *compNN, *compSN,
                           localCont, *tabuList, dummyIntensification,
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
      throw std::runtime_error("Unknown value for TS.Algo: " +
                               std::to_string(params.categorical("TS.Algo")));
      break;
  }

  moRestartPerturb<Ngh> perturb(*init, fullEval, 0);
  moAlwaysAcceptCrit<Ngh> accept;
  moILS<Ngh, Ngh> ils(*algo, fullEval, checkpointGlobal, perturb, accept);

  return runExperiment(*init, ils, prob);
}
