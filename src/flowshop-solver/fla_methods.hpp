#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/problems/FSPProblem.hpp"
#include "flowshop-solver/heuristics/FSPOrderHeuristics.hpp"
#include "flowshop-solver/heuristics/IGexplorer.hpp"
#include "flowshop-solver/heuristics/InsertionStrategy.hpp"
#include "flowshop-solver/heuristics/NEHInit.hpp"
#include "flowshop-solver/heuristics/perturb/perturb.hpp"
#include "flowshop-solver/FSPProblemFactory.hpp"
#include "flowshop-solver/MHParamsSpecsFactory.hpp"
#include "flowshop-solver/heuristics/acceptCritTemperature.hpp"
#include "flowshop-solver/heuristics/InsertionStrategy.hpp"

#include "flowshop-solver/heuristics/ig.hpp"


std::vector<FSPProblem::EOT> adaptiveWalk(
    std::unordered_map<std::string, std::string> prob_params,
    std::unordered_map<std::string, std::string> sampling_params,
    unsigned seed) {
  rng.reseed(seed);
  FSPProblem problem = FSPProblemFactory::get(prob_params);
  using EOT = FSPProblem::EOT;
  using Ngh = FSPProblem::Ngh;

  // sampling params
  using namespace std::string_literals;
  auto init_strat = getWithDef(sampling_params, "Init.Strat"s, "RANDOM"s);
  auto sampling_strat = getWithDef(sampling_params, "Sampling.Strat"s, "FI"s);

  EOT order;
  SUM_PIJ(problem.getData(), false, "incr")(order);
  auto& eval = problem.eval();
  auto& neighborEval = problem.neighborEval();

  eoInitPermutation<EOT> init0(problem.size());
  NEHInitOrdered<EOT> init1(eval, order);
  NEHInitRandom<EOT> init2(eval, problem.size());
  eoInit<EOT>* init = nullptr;
  if (init_strat == "RANDOM")
    init = &init0;
  else if (init_strat == "NEH")
    init = &init1;
  else if (init_strat == "RANDOM_NEH")
    init = &init2;
  else
    assert(false);

  const int nh_size = std::pow(problem.size() - 1, 2);
  moRndWithoutReplNeighborhood<Ngh> neighborhood(nh_size);

  moTrueContinuator<Ngh> tc;
  moCheckpoint<Ngh> checkpoint(tc);

  moSolutionStat<EOT> solutionStat;
  moVectorMonitor<EOT> solutionMonitor(solutionStat);
  checkpoint.add(solutionStat);
  checkpoint.add(solutionMonitor);

  moFirstImprHC<Ngh> fi(neighborhood, eval, neighborEval, checkpoint);
  moSimpleHC<Ngh> hc(neighborhood, eval, neighborEval, checkpoint);

  moSolComparator<EOT> comparator;
  IGexplorer<Ngh> igExplorer(eval, problem.size(0), comparator);
  moLocalSearch<Ngh> ig(igExplorer, checkpoint, eval);

  moLocalSearch<Ngh>* localSearch = nullptr;
  if (sampling_strat == "FI")
    localSearch = &fi;
  else if (sampling_strat == "HC")
    localSearch = &hc;
  else if (sampling_strat == "IG")
    localSearch = &ig;
  else
    throw std::runtime_error("Unknown sampling strat: " + sampling_strat);

  EOT sol;
  (*init)(sol);
  solutionStat.init(sol);
  (*localSearch)(sol);

  std::cerr << "no_evals" << problem.noEvals() << "\n";

  auto res = solutionMonitor.getSolutions();
  // remove duplicated last solution
  res.pop_back();
  return res;
}

double adaptiveWalkLength(
    const std::unordered_map<std::string, std::string>& prob_params,
    const std::unordered_map<std::string, std::string>& sampling_params,
    unsigned seed) {
  rng.reseed(seed);
  FSPProblem problem = FSPProblemFactory::get(prob_params);
  using EOT = FSPProblem::EOT;
  using Ngh = FSPProblem::Ngh;

  // sampling params
  using namespace std::string_literals;
  auto init_strat = getWithDef(sampling_params, "Init.Strat"s, "RANDOM"s);
  auto sampling_strat = getWithDef(sampling_params, "Sampling.Strat"s, "FI"s);

  EOT order;
  SUM_PIJ(problem.getData(), false, "incr")(order);
  auto& eval = problem.eval();
  auto& neighborEval = problem.neighborEval();

  eoInitPermutation<EOT> init0(problem.size());
  NEHInitOrdered<EOT> init1(eval, order);
  NEHInitRandom<EOT> init2(eval, problem.size());
  eoInit<EOT>* init = nullptr;
  if (init_strat == "RANDOM")
    init = &init0;
  else if (init_strat == "NEH")
    init = &init1;
  else if (init_strat == "RANDOM_NEH")
    init = &init2;
  else
    assert(false);

  const int nh_size = std::pow(problem.size() - 1, 2);
  moRndWithoutReplNeighborhood<Ngh> neighborhood(nh_size);

  moSampling<Ngh>* sampling = nullptr;
  moAdaptiveWalkSampling<Ngh> adaptive(*init, neighborhood, eval, neighborEval,
                                       1);
  moHillClimberSampling<Ngh> hc(*init, neighborhood, eval, neighborEval, 1);

  if (sampling_strat == "FI_LENGTH")
    sampling = &adaptive;
  else if (sampling_strat == "HC_LENGTH")
    sampling = &hc;
  sampling->operator()();
  std::cerr << "no_evals" << problem.noEvals() << "\n";
  sampling->fileExport("teste.txt");
  return sampling->getValues(0).at(0);
}

std::vector<double> enumerateAll(
    const std::unordered_map<std::string, std::string>& prob_params) {
  std::vector<double> res;

  using ProblemTp = FSPProblem;
  ProblemTp problem = FSPProblemFactory::get(prob_params);
  auto& fullEval = problem.eval();
  using EOT = ProblemTp::EOT;

  int n = problem.size();
  EOT sol(n);
  std::iota(sol.begin(), sol.end(), 0);
  long no_solutions = factorial(n);
  res.reserve(no_solutions);

  for (int i = 0; i < no_solutions; i++) {
    fullEval(sol);
    res.emplace_back(sol.fitness());
    std::next_permutation(sol.begin(), sol.end());
    sol.invalidate();
  }

  return res;
}

template <class Ngh, class EOT = typename Ngh::EOT>
std::vector<EOT> enumerateAllSolutions(Problem<Ngh>& problem) {
  const int n = problem.size();
  const long no_solutions = factorial(n);
  auto& fullEval = problem.eval();

  std::vector<EOT> solutions;
  solutions.reserve(no_solutions);

  EOT sol(n);
  std::iota(sol.begin(), sol.end(), 0);
  for (int i = 0; i < no_solutions; i++) {
    fullEval(sol);
    solutions.emplace_back(sol);
    std::next_permutation(sol.begin(), sol.end());
    sol.invalidate();
  }

  return solutions;
}

