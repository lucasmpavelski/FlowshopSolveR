#pragma once

#include <unordered_map>

#include "heuristics.hpp"
#include "fspproblemfactory.hpp"
#include "MHParamsValues.hpp"
#include "op_cooling_schedule.hpp"
#include "NEHInit.hpp"
#include "specsdata.hpp"

Result solveWithSA(const std::unordered_map<std::string, std::string>& problem_specs,
                   const std::unordered_map<std::string, double>& param_values) {
  using Problem = FSPProblem;
  Problem prob = FSPProblemFactory::get(problem_specs);
  MHParamsSpecs specs = MHParamsSpecsFactory::get("SA");
  MHParamsValues params(&specs);
  params.readValues(param_values);

  using EOT = Problem::EOT;
  using Ngh = Problem::Ngh;
  EOT sol;

  const int N = prob.size(0);
  const int max_nh_size = pow(N - 1, 2);
  const std::string mh = params.mhName();
  
  // continuator
  moContinuator<Ngh> &continuator = prob.continuator();
  eoEvalFunc<EOT> &fullEval = prob.eval();
  moEval<Ngh> &evalN = prob.neighborEval();

  moCheckpoint<Ngh>& checkpoint = prob.checkpoint();
  moCheckpoint<Ngh>& checkpointGlobal = prob.checkpointGlobal();

  // comparator strategy
  moSolComparator<EOT> compSS0;              // comp sol/sol strict
  moSolNeighborComparator<Ngh> compSN0;      // comp sol/Ngh strict
  moNeighborComparator<Ngh> compNN0;         // comp Ngh/Ngh strict
  moEqualSolComparator<EOT> compSS1;         // comp sol/sol with equal
  moEqualSolNeighborComparator<Ngh> compSN1; // comp sol/Ngh with equal
  moEqualNeighborComparator<Ngh> compNN1;    // comp Ngh/Ngh with equal
  moSolComparator<EOT> *compSS = nullptr;
  moSolNeighborComparator<Ngh> *compSN = nullptr;
  moNeighborComparator<Ngh> *compNN = nullptr;
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
  eoInitPermutation<EOT> init0(N);
  NEHInit<EOT> init1(fullEval, N, *compSS);
  int cycle = 3;
  NEHInitRandom<EOT> init2(fullEval, N, cycle, *compSS);
  //FastNEH fastNeh(prob.getData());
  //FastNEHRandom init2(prob.getData());
  eoInit<EOT> *init = nullptr;

  switch (params.categorical("SA.Init.Strat")) {
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
    throw std::runtime_error("Unknonwn SA.Init.Strat value " + 
    std::to_string(params.categorical("SA.Init.Strat")));
    break;
  }

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
    opCoolingSchedule<EOT> cooling2(
          prob.getData(), params.real("SA.T"), params.real("SA.Final.Temp"),
          params.real("SA.Beta"));

    moCoolingSchedule<EOT> *cooling = nullptr;
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

  moSA<Ngh> algo(neighborhood, fullEval, evalN, *cooling, *compSN, checkpointGlobal);

  return runExperiment(*init, algo, prob);
}
