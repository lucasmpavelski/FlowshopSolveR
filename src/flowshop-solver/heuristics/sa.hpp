#pragma once

#include <unordered_map>

#include "flowshop-solver/MHParamsValues.hpp"
#include "flowshop-solver/heuristics.hpp"
#include "flowshop-solver/problems/Problem.hpp"
#include "flowshop-solver/heuristics/op_cooling_schedule.hpp"
#include "flowshop-solver/MHParamsSpecsFactory.hpp"

template <class Ngh, class EOT = typename Problem<Ngh>::EOT>
auto solveWithSA(Problem<Ngh>& prob, const MHParamsValues& params) -> Result {
  const int N = prob.size(0);
  const int M = prob.size(1);
  const int max_nh_size = pow(N - 1, 2);

  // continuator
  eoEvalFunc<EOT>& fullEval = prob.eval();
  moEval<Ngh>& evalN = prob.neighborEval();

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
  switch (params.categorical("SA.Comp.Strat")) {
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
  eoInit<EOT>* init = nullptr;
  
  // neighborhood size
  const int min_nh_size = (N >= 20) ? 11 : 2;
  const int nh_interval = (N >= 20) ? 10 : 1;
  const int no_nh_sizes = (max_nh_size - min_nh_size) / nh_interval;
  const int scale =
      int((no_nh_sizes + 1) * params.real("SA.Neighborhood.Size") / 10.0);
  const int nh_size = std::min(max_nh_size, min_nh_size + scale * nh_interval);

  // neighborhood (fixed with strategy = RANDOM)
  moRndWithoutReplNeighborhood<Ngh> neighborhood(nh_size);
  // cooling schedule
  moSimpleCoolingSchedule<EOT> cooling0(
      params.real("SA.Init.Temp"), params.real("SA.Alpha"),
      params.integer("SA.Span.Simple"), params.real("SA.Final.Temp"));
  moDynSpanCoolingSchedule<EOT> cooling1(
      params.real("SA.Init.Temp"), params.real("SA.Alpha"),
      params.integer("SA.Span.Tries.Max"), params.integer("SA.Span.Move.Max"),
      params.integer("SA.Nb.Span.Max"));

  double tempScale = prob.upperBound() / (10.0 * N * M);
  opCoolingSchedule<EOT> cooling2(params.real("SA.T") * tempScale,
                                  params.real("SA.Final.Temp"),
                                  params.real("SA.Beta"));

  moCoolingSchedule<EOT>* cooling = nullptr;
  switch (params.categorical("SA.Algo")) {
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

  moSA<Ngh> algo(neighborhood, fullEval, evalN, *cooling, *compSN,
                 checkpointGlobal);

  return runExperiment(*init, algo, prob);
}
