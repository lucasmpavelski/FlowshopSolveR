#include <array>
#include <iostream>

#include <string>

#include "flowshop-solver/fspproblemfactory.hpp"
#include "flowshop-solver/global.hpp"
#include "flowshop-solver/heuristics/all.hpp"
#include "flowshop-solver/specsdata.hpp"

auto main(int, char*[]) noexcept -> int {
  long seed = 123;  // std::stol(argv[1]);
  std::cerr << seed << '\n';
  RNG::seed(seed);
  FSPProblemFactory::init(DATA_FOLDER);
  MHParamsSpecsFactory::init(DATA_FOLDER "/specs", true);

  std::unordered_map<std::string, std::string> prob;
  prob["problem"]            = "FSP";
  prob["type"]               = "PERM";
  prob["objective"]          = "MAKESPAN";
  prob["budget"]             = "med";
  prob["instance"]           = "taillard_rand_50_20_09.dat";
  prob["stopping_criterium"] = "FIXEDTIME";

  std::unordered_map<std::string, std::string> params;
  params["IG.Init"]                      = "neh";
  params["IG.Init.NEH.Priority"]         = "sum_pij";
  params["IG.Init.NEH.PriorityOrder"]    = "incr";
  params["IG.Init.NEH.PriorityWeighted"] = "0";
  params["IG.Init.NEH.Insertion"]        = "first_best";
  params["IG.Comp.Strat"]                = "strict";
  params["IG.Neighborhood.Size"]         = "1";
  params["IG.Neighborhood.Strat"]        = "random";
  params["IG.Local.Search"]              = "best_insertion";
  params["IG.LS.Single.Step"]            = "0";
  params["IG.Accept"]                    = "temperature";
  params["IG.Accept.Temperature"]        = "0.5";
  params["IG.Perturb"]                   = "rs";
  params["IG.Perturb.DestructionSize"]   = "4";
  params["IG.Perturb.Insertion"]         = "first_best";
  params["IG.LSPS.Local.Search"]         = "best_insertion";
  params["IG.LSPS.Single.Step"]          = "0";
  params["IG.AOS.Strategy"]              = "frrmab";
  params["IG.AOS.RewardType"]            = "1";

  RunOptions runOptions;
  runOptions.printBestFitness = true;

  FSPProblem     problem = FSPProblemFactory::get(prob);
  MHParamsSpecs  specs   = MHParamsSpecsFactory::get("IG");
  MHParamsValues paramsValues(&specs);
  paramsValues.readValues(params);

  eoFSPFactory factory{paramsValues, problem};

  FitnessRewards<FSP> rewards;
  RewardPrinter<FSP>  rewardPrinter{rewards};
  if (runOptions.printFitnessReward) {
    std::cout << rewardPrinter.header();
    problem.checkpoint().add(rewards.localStat());
    problem.checkpointGlobal().add(rewards.globalStat());
    problem.checkpointGlobal().add(rewardPrinter);
  }

  myTimeStat<FSP>           timer;
  myTimeFitnessPrinter<FSP> timeFitness{timer};
  if (runOptions.printBestFitness) {
    std::puts("runtime,fitness");
    problem.checkpointGlobal().add(timeFitness);
  }

  auto init    = factory.buildInit();
  auto algo    = factory.buildLocalSearch();
  auto accept  = factory.buildAcceptanceCriterion();
  auto perturb = factory.buildPerturb();

  FSP _solution;
  (*init)(_solution);
  problem.checkpoint().init(_solution);
  problem.checkpointGlobal().init(_solution);

  FSP         currentSol;
  FSPNeighbor emptyNeighbor;

  if (_solution.invalid())
    problem.eval()(_solution);

  // initialization of the parameter of the search (for example fill empty the
  // tabu list)
  perturb->init(_solution);
  accept->init(_solution);

  // initialization of the external continuator (for example the time, or the
  // number of generations)
  problem.continuator().init(_solution);
  bool firstIteration = true;
  do {
    // perturb solution exept at the first iteration
    currentSol = _solution;
    if (!firstIteration)
      (*perturb)(currentSol);
    else
      firstIteration = false;

    // apply the local search on the copy
    (*algo)(currentSol);

    // if a solution in the neighborhood can be accepted
    if ((*accept)(_solution, currentSol)) {
      _solution   = currentSol;
      perturb->add(_solution, emptyNeighbor);
      accept->add(_solution, emptyNeighbor);
    }

    perturb->update(_solution, emptyNeighbor);
    accept->update(_solution, emptyNeighbor);

  } while (problem.checkpointGlobal()(_solution));

  problem.checkpointGlobal().lastCall(_solution);

  return 0;
}
