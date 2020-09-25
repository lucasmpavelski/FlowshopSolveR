#pragma once

#include <unordered_map>

#include "flowshop-solver/heuristics.hpp"
#include "flowshop-solver/MHParamsValues.hpp"
#include "flowshop-solver/heuristics/NEHInit.hpp"
#include "flowshop-solver/FSPProblemFactory.hpp"
#include "flowshop-solver/heuristics/op_cooling_schedule.hpp"
#include "flowshop-solver/MHParamsSpecsFactory.hpp"

template <class Ngh, class EOT = typename Problem<Ngh>::EOT>
auto solveWithISA(Problem<Ngh>& prob, const MHParamsValues& params) -> Result {
  const int N = prob.size(0);
  const int M = prob.size(1);
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
  switch (params.categorical("ISA.Comp.Strat")) {
    case 0:
      compSS = &compSS0;
      compSN = &compSN0;
      break;
    case 1:
      compSS = &compSS1;
      compSN = &compSN1;
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

  switch (params.categorical("ISA.Init.Strat")) {
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
          "Unknonwn ISA.Init.Strat value " +
          std::to_string(params.categorical("ISA.Init.Strat")));
      break;
  }

  // neighborhood size
  const int min_nh_size = (N >= 20) ? 11 : 2;
  const int nh_interval = (N >= 20) ? 10 : 1;
  const int no_nh_sizes = (max_nh_size - min_nh_size) / nh_interval;
  const int scale =
      int((no_nh_sizes + 1) * params.real("ISA.Neighborhood.Size") / 10.0);
  const int nh_size = std::min(max_nh_size, min_nh_size + scale * nh_interval);

  // neighborhood (fixed with strategy = RANDOM)
  moRndWithoutReplNeighborhood<Ngh> neighborhood(nh_size);
  // cooling schedule
  moSimpleCoolingSchedule<EOT> cooling0(
      params.real("ISA.Init.Temp"), params.real("ISA.Alpha"),
      params.integer("ISA.Span.Simple"), params.real("ISA.Final.Temp"));
  moDynSpanCoolingSchedule<EOT> cooling1(
      params.real("ISA.Init.Temp"), params.real("ISA.Alpha"),
      params.integer("ISA.Span.Tries.Max"), params.integer("ISA.Span.Move.Max"),
      params.integer("ISA.Nb.Span.Max"));

  double tempScale = prob.upperBound() / (10.0 * N * M);
  opCoolingSchedule<EOT> cooling2(params.real("ISA.T") * tempScale,
                                  params.real("ISA.Final.Temp"),
                                  params.real("ISA.Beta"));

  moCoolingSchedule<EOT>* cooling = nullptr;
  switch (params.categorical("ISA.Algo")) {
    case 0:
      cooling = &cooling0;
      break;
    case 1:
      cooling = &cooling1;
      break;
    case 2:
      cooling = &cooling2;
      break;
    default:
      assert(false);
      break;
  }

  moSA<Ngh> algo(neighborhood, fullEval, evalN, *cooling, *compSN, checkpoint);

  moRestartPerturb<Ngh> perturb(*init, fullEval, 0);
  moAlwaysAcceptCrit<Ngh> accept;
  moILS<Ngh, Ngh> ils(algo, fullEval, checkpointGlobal, perturb, accept);

  return runExperiment(*init, ils, prob);
}
