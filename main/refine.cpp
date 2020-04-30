#include <array>
#include <iostream>

#include <string>

#include "flowshop-solver/fspproblemfactory.hpp"
#include "flowshop-solver/global.hpp"
#include "flowshop-solver/heuristics/all.hpp"
#include "flowshop-solver/specsdata.hpp"

auto main(int, char* []) -> int {
  long seed = 123;  // std::stol(argv[1]);
  std::cerr << seed << '\n';
  RNG::seed(seed);
  FSPProblemFactory::init(DATA_FOLDER);
  MHParamsSpecsFactory::init(DATA_FOLDER "/specs", true);
  std::unordered_map<std::string, std::string> prob;
  prob["problem"] = "FSP";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "med";
  prob["instance"] = "taillard_rand_50_20_09.dat";

  prob["stopping_criterium"] = "FIXEDTIME";
  std::unordered_map<std::string, std::string> params;
  params["IG.Init"] = "neh";
  params["IG.Init.NEH.Priority"] = "sum_pij";
  params["IG.Init.NEH.PriorityOrder"] = "incr";
  params["IG.Init.NEH.PriorityWeighted"] = "0";
  params["IG.Init.NEH.Insertion"] = "first_best";
  params["IG.Comp.Strat"] = "0";
  params["IG.Neighborhood.Size"] = "9.9999";
  params["IG.Neighborhood.Strat"] = "0";
  params["IG.Local.Search"] = "3";
  params["IG.Accept"] = "temperature";
  params["IG.Accept.Temperature"] = "0.5";
  params["IG.Algo"] = "adaptive";
  params["IG.Destruction.Size"] = "4";
  params["IG.LS.Single.Step"] = "0";
  params["IG.LSPS.Local.Search"] = "3";
  params["IG.LSPS.Single.Step"] = "0";
  params["IG.AOS.Strategy"] = "probability_matching";

  RunOptions ro;
  ro.printBestFitness = true;

  std::cout << solveWith("IG", prob, params, ro);
}
