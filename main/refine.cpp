#include <array>
#include <iostream>

#include <string>

#include "flowshop-solver/fspproblemfactory.hpp"
#include "flowshop-solver/global.hpp"
#include "flowshop-solver/heuristics/all.hpp"
#include "flowshop-solver/specsdata.hpp"

int main(int, char *[]) {
  long seed = 123; // std::stol(argv[1]);
  std::cerr << seed << '\n';
  RNG::seed(seed);
  FSPProblemFactory::init(DATA_FOLDER);
  MHParamsSpecsFactory::init(DATA_FOLDER "/specs", true);
  std::unordered_map<std::string, std::string> prob;
  prob["problem"] = "FSP";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "med";
  prob["instance"] = "taillard_rand_20_20_02.dat";
  prob["stopping_criterium"] = "FIXEDTIME";
  std::unordered_map<std::string, double> params;
  params["IG.Init.Strat"] = 0;
  params["IG.Comp.Strat"] = 0;
  params["IG.Neighborhood.Size"] = 9.9999;
  params["IG.Neighborhood.Strat"] = 0;
  params["IG.Local.Search"] = 3;
  params["IG.Accept"] = 2;
  params["IG.Accept.Temperature"] = 0.5;
  params["IG.Algo"] = 0;
  params["IG.Destruction.Size"] = 12;
  params["IG.LS.Single.Step"] = 0;
  params["IG.LSPS.Local.Search"] = 0;
  params["IG.LSPS.Single.Step"] = 0;
  params["IG.AOS.Strategy"] = 0;
  std::cout << solveWith("IG", prob, params);
}
