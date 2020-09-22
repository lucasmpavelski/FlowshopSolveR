#pragma once

#include <paradiseo/mo/acceptCrit/moAlwaysAcceptCrit.h>
#include <paradiseo/mo/algo/moILS.h>
#include <paradiseo/mo/perturb/moRestartPerturb.h>

#include <unordered_map>

#include "MHParamsValues.hpp"
#include "NEHInit.hpp"
#include "flowshop-solver/heuristics.hpp"
#include "FSPProblemFactory.hpp"
#include "op_cooling_schedule.hpp"
#include "MHParamsSpecsFactory.hpp"

template <class Ngh, class EOT = typename Problem<Ngh>::EOT>
auto solveWithIHC(Problem<Ngh>& prob, const MHParamsValues& params) -> Result {
  const int N = prob.size(0);
  const int max_nh_size = pow(N - 1, 2);

  // continuator
  eoEvalFunc<EOT>& fullEval = prob.eval();
  moEval<Ngh>& evalN = prob.neighborEval();
  moCheckpoint<Ngh>& checkpoint = prob.checkpoint();
  moCheckpoint<Ngh>& checkpointGlobal = prob.checkpointGlobal();

  // debug
  //  prefixedPrinter print("local_best:", " ");
  //  print.add(bestFound);
  //  prefixedPrinter printg("global_best:", " ");
  //  printg.add(bestFoundGlobal);
  //  checkpoint.add(print);
  //  checkpointGlobal.add(printg);

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
  switch (params.categorical("IHC.Comp.Strat")) {
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

  switch (params.categorical("IHC.Init.Strat")) {
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
          "Unknonwn IHC.Init.Strat value " +
          std::to_string(params.categorical("Init.Strat")));
      break;
  }

  // neighborhood size
  const int min_nh_size = (N >= 20) ? 11 : 2;
  const int nh_interval = (N >= 20) ? 10 : 1;
  const int no_nh_sizes = (max_nh_size - min_nh_size) / nh_interval;
  const int scale =
      int((no_nh_sizes + 1) * params.real("IHC.Neighborhood.Size") / 10.0);
  const int nh_size = std::min(max_nh_size, min_nh_size + scale * nh_interval);

  moOrderNeighborhood<Ngh> neighborhood0(nh_size);
  moRndWithoutReplNeighborhood<Ngh> neighborhood1(nh_size);
  moNeighborhood<Ngh>* neighborhood = nullptr;
  switch (params.categorical("IHC.Neighborhood.Strat")) {
    case 0:
      neighborhood = &neighborhood0;
      break;
    case 1:
      neighborhood = &neighborhood1;
      break;
    default:
      throw std::runtime_error(
          "Unknonwn Neighborhood.Strat value " +
          std::to_string(params.categorical("Neighborhood.Strat")));
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
  switch (params.categorical("IHC.Algo")) {
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
      throw std::runtime_error("Unknonwn IHC.Algo value " +
                               std::to_string(params.categorical("IHC.Algo")));
      break;
  }

  moRestartPerturb<Ngh> perturb(*init, fullEval, 0);
  moAlwaysAcceptCrit<Ngh> accept;
  moILS<Ngh, Ngh> ils(*algo, fullEval, checkpointGlobal, perturb, accept);

  return runExperiment(*init, ils, prob);
}
