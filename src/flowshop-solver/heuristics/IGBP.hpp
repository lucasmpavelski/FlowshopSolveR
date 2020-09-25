#pragma once

#include <algorithm>
#include <array>
#include <iostream>

#include <string>

#include "flowshop-solver/heuristics.hpp"
#include "flowshop-solver/MHParamsValues.hpp"
#include "flowshop-solver/RunOptions.hpp"
#include "flowshop-solver/eoFSPFactory.hpp"
#include "flowshop-solver/FSPProblemFactory.hpp"
#include "flowshop-solver/global.hpp"
#include "flowshop-solver/MHParamsSpecsFactory.hpp"

template <class PertT, class LOT, class AccT, class EOT>
void globalGlobalReward(bool                      firstIteration,
                        const std::vector<PertT*> perturbs,
                        LOT*                      localSearch,
                        AccT*                     accept,
                        EOT&                      sol) {
  FSPNeighbor emptyNeighbor;
  const auto  noOps = perturbs.size();

  FSP currentSol = sol;
  // currentSol.fitness(sol.fitness());
  // double initialFitness = _solution.fitness();
  std::vector<FSP> solsP(noOps, sol);

  if (!firstIteration) {
    for (unsigned i = 0; i < noOps; i++)
      (*perturbs[i])(solsP[i]);
  }

  // apply the local search on the copy
  for (unsigned i = 0; i < noOps; i++) {
    (*localSearch)(solsP[i]);

    // if a solution in the neighborhood can be accepted
    if ((*accept)(sol, solsP[i])) {
      for (auto& perturb : perturbs)
        perturb->add(sol, emptyNeighbor);
      accept->add(sol, emptyNeighbor);
    } else {
      solsP[i] = currentSol;
    }

  }
  sol = *std::min_element(begin(solsP), end(solsP), [&](FSP& a, FSP& b) {
    return a.fitness() > b.fitness();
  });
}

template <class PertT, class LOT, class AccT, class EOT>
void globalLocalReward(bool                      firstIteration,
                       const std::vector<PertT*> perturbs,
                       LOT*                      localSearch,
                       AccT*                     accept,
                       EOT&                      sol) {
  FSPNeighbor emptyNeighbor;
  const auto  noOps = perturbs.size();

  FSP currentSol = sol;
  // currentSol.fitness(sol.fitness());
  // double initialFitness = _solution.fitness();
  std::vector<FSP> solsP(noOps, sol);

  if (!firstIteration) {
    for (unsigned i = 0; i < noOps; i++)
      (*perturbs[i])(solsP[i]);
  }

  // apply the local search on the copy
  for (auto& solP : solsP)
    (*localSearch)(solP);

  currentSol = *std::min_element(begin(solsP), end(solsP), [&](FSP& a, FSP& b) {
    return a.fitness() > b.fitness();
  });

  // if a solution in the neighborhood can be accepted
  if ((*accept)(sol, currentSol)) {
    sol = currentSol;
    for (auto& perturb : perturbs)
      perturb->add(sol, emptyNeighbor);
    accept->add(sol, emptyNeighbor);
  }
}

template <class PertT, class LOT, class AccT, class EOT>
void localGlobalReward(bool                      firstIteration,
                       const std::vector<PertT*> perturbs,
                       LOT*                      localSearch,
                       AccT*                     accept,
                       EOT&                      sol) {
  FSPNeighbor emptyNeighbor;
  const auto  noOps = perturbs.size();

  FSP currentSol = sol;
  // currentSol.fitness(sol.fitness());
  // double initialFitness = _solution.fitness();
  std::vector<FSP> solsP(noOps, sol);

  double bestReward = -std::numeric_limits<double>::infinity();
  if (!firstIteration) {
    FSP initialSolution = sol;
    for (unsigned i = 0; i < noOps; i++) {
      (*perturbs[i])(solsP[i]);
      double initialFintess = solsP[i].fitness();
      (*localSearch)(solsP[i]);

      // if a solution in the neighborhood can be accepted
      if ((*accept)(initialSolution, solsP[i])) {
        for (auto& perturb : perturbs)
          perturb->add(solsP[i], emptyNeighbor);
        accept->add(solsP[i], emptyNeighbor);
      } else {
        solsP[i] = initialSolution;
      }

      if (initialFintess - solsP[i].fitness() > bestReward) {
        sol = solsP[i];
        bestReward = initialFintess - solsP[i].fitness();
      }
    }
  } else {
    (*localSearch)(currentSol);
    if ((*accept)(sol, currentSol)) {
        sol = currentSol;
        for (auto& perturb : perturbs)
          perturb->add(sol, emptyNeighbor);
        accept->add(sol, emptyNeighbor);
      }
  }
}

template <class PertT, class LOT, class AccT, class EOT>
auto localLocalReward(bool                      firstIteration,
                      const std::vector<PertT*> perturbs,
                      LOT*                      localSearch,
                      AccT*                     accept,
                      EOT&                      sol) -> unsigned {
  FSPNeighbor emptyNeighbor;
  const auto  noOps = perturbs.size();

  FSP currentSol = sol;
  // currentSol.fitness(sol.fitness());
  // double initialFitness = _solution.fitness();
  std::vector<FSP> solsP(noOps, sol);

  double bestReward = -std::numeric_limits<double>::infinity();
  unsigned bestPerturb = 0;
  if (!firstIteration) {
    for (unsigned i = 0; i < noOps; i++) {
      (*perturbs[i])(solsP[i]);
      double initialFintess = solsP[i].fitness();
      (*localSearch)(solsP[i]);
      if (initialFintess - solsP[i].fitness() > bestReward) {
        currentSol = solsP[i];
        bestReward = initialFintess - solsP[i].fitness();
        bestPerturb = i;
      }
    }
  } else {
    (*localSearch)(currentSol);
  }

  // if a solution in the neighborhood can be accepted
  if ((*accept)(sol, currentSol)) {
    sol = currentSol;
    for (auto& perturb : perturbs)
      perturb->add(sol, emptyNeighbor);
    accept->add(sol, emptyNeighbor);
  }

  return bestPerturb;
}

inline auto solveWithIGBP(FSPProblem& problem, MHParamsValues& paramsValues, const RunOptions& runOptions)
    -> Result {

  FSPNeighbor emptyNeighbor;

  eoFSPFactory factory{paramsValues, problem};

  const std::vector<int>                        destructionSizes = {2, 4, 8};
  const unsigned                                noOps = destructionSizes.size();
  std::vector<moPerturbation<FSPProblem::Ngh>*> perturbs(noOps, nullptr);
  for (unsigned i = 0; i < noOps; i++) {
    paramsValues["IGBP.Perturb.DestructionSize"] = destructionSizes[i];
    perturbs[i]                                  = factory.buildPerturb();
  }

  FitnessRewards<FSP> rewards;
  /*RewardPrinter<FSP>  rewardPrinter{rewards};
  if (runOptions.printFitnessReward) {
    std::cout << rewardPrinter.header();
    problem.checkpoint().add(rewards.localStat());
    problem.checkpointGlobal().add(rewards.globalStat());
    problem.checkpointGlobal().add(rewardPrinter);
  }*/

  myTimeStat<FSP>           timer;
  myTimeFitnessPrinter<FSP> timeFitness{timer};
  if (runOptions.printBestFitness) {
    std::puts("runtime,fitness");
    problem.checkpointGlobal().add(timeFitness);
  }

  auto init   = factory.buildInit();
  auto algo   = factory.buildLocalSearch();
  auto accept = factory.buildAcceptanceCriterion();

  FSP _solution;
  (*init)(_solution);
  problem.checkpoint().init(_solution);
  problem.checkpointGlobal().init(_solution);

  FSP currentSol;

  if (_solution.invalid())
    problem.eval()(_solution);

  // initialization of the parameter of the search (for example fill empty the
  // tabu list)
  for (auto& perturb : perturbs)
    perturb->init(_solution);
  accept->init(_solution);

  // initialization of the external continuator (for example the time, or the
  // number of generations)
  problem.continuator().init(_solution);

  bool firstIteration = true;
  do {
    // perturb solution exept at the first iteration

    unsigned best = 0;
    if (paramsValues.categorical("IGBP.AOS.RewardType") == 0)
      globalGlobalReward(firstIteration, perturbs, algo, accept, _solution);
    else if (paramsValues.categorical("IGBP.AOS.RewardType") == 1)
      globalLocalReward(firstIteration, perturbs, algo, accept, _solution);
    else if (paramsValues.categorical("IGBP.AOS.RewardType") == 2)
      localGlobalReward(firstIteration, perturbs, algo, accept, _solution);
    else if (paramsValues.categorical("IGBP.AOS.RewardType") == 3)
      best = localLocalReward(firstIteration, perturbs, algo, accept, _solution);

    if (runOptions.printDestructionChoices && !firstIteration) {
      timer(_solution);
      std::cout << timer.value() << ' ' << destructionSizes[best] << '\n';
    }

    if (firstIteration)
      firstIteration = false;

    for (auto& perturb : perturbs)
      perturb->update(_solution, emptyNeighbor);
    accept->update(_solution, emptyNeighbor);

  } while (problem.checkpointGlobal()(_solution));

  problem.checkpointGlobal().lastCall(_solution);

  Result result;
  result.fitness = _solution.fitness();
  return result;
}