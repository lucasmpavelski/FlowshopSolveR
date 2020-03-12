#pragma once

#include <unordered_map>

#include "../heuristics.hpp"
#include "MHParamsValues.hpp"
#include "NEHInit.hpp"
#include "fastnehheuristic.hpp"
#include "fspproblemfactory.hpp"
#include "op_cooling_schedule.hpp"
#include "specsdata.hpp"

Result solveWithHC(
    const std::unordered_map<std::string, std::string>& problem_specs,
    const std::unordered_map<std::string, double>& param_values) {
  double result = 0.0;
  MHParamsSpecs specs = MHParamsSpecsFactory::get("HC");
  MHParamsValues params(&specs);
  params.readValues(param_values);

  using ProblemType = FSPProblem;
  using EOT = typename ProblemType::EOT;
  using Ngh = typename ProblemType::Ngh;
  EOT sol;

  ProblemType prob = FSPProblemFactory::get(problem_specs);
  const int N = prob.size(0);
  const int max_nh_size = pow(N - 1, 2);
  const std::string mh = params.mhName();

  // continuator
  moContinuator<Ngh>& continuator = prob.continuator();
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
  switch (params.categorical("HC.Comp.Strat")) {
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
      throw std::runtime_error(
        "Unknown HC.Comp.Strat " + 
        std::to_string(params.categorical("HC.Comp.Strat"))
      );
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

  switch (params.categorical("HC.Init.Strat")) {
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
          "Unknonwn HC.Init.Strat value " +
          std::to_string(params.categorical("HC.Init.Strat")));
      break;
  }

  // neighborhood size
  const int min_nh_size = (N >= 20) ? 11 : 2;
  const int nh_interval = (N >= 20) ? 10 : 1;
  const int no_nh_sizes = (max_nh_size - min_nh_size) / nh_interval;
  const int scale =
      int((no_nh_sizes + 1) * params.real("HC.Neighborhood.Size") / 10.0);
  const int nh_size = std::min(max_nh_size, min_nh_size + scale * nh_interval);

  moOrderNeighborhood<Ngh> neighborhood0(nh_size);
  moRndWithoutReplNeighborhood<Ngh> neighborhood1(nh_size);
  moNeighborhood<Ngh>* neighborhood = nullptr;
  switch (params.categorical("HC.Neighborhood.Strat")) {
    case 0:
      neighborhood = &neighborhood0;
      break;
    case 1:
      neighborhood = &neighborhood1;
      break;
    default:
      throw std::runtime_error(
          "Unknonwn HC.Neighborhood.Strat value " +
          std::to_string(params.categorical("HC.Neighborhood.Strat")));
      break;
  }
  // HC algorithms
  moFirstImprHC<Ngh> fi(*neighborhood, fullEval, evalN, checkpointGlobal,
                        *compNN, *compSN);
  moSimpleHC<Ngh> best(*neighborhood, fullEval, evalN, checkpointGlobal,
                       *compNN, *compSN);
  moRandomBestHC<Ngh> rand_best(*neighborhood, fullEval, evalN,
                                checkpointGlobal, *compNN, *compSN);
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
      throw std::runtime_error(
          "Unknonwn HC.Algo value " +
          std::to_string(params.categorical("HC.Algo")));
      break;
  }

  return runExperiment(*init, *algo, prob);
}
