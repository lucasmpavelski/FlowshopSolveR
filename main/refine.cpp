#include <algorithm>
#include <array>
#include <iostream>

#include <string>

#include "flowshop-solver/FSPProblemFactory.hpp"
#include "flowshop-solver/global.hpp"
#include "flowshop-solver/heuristics/all.hpp"
#include "flowshop-solver/MHParamsSpecsFactory.hpp"



auto main(int, char*[]) noexcept -> int {
  long seed = 123;  // std::stol(argv[1]);
  std::cerr << seed << '\n';
  RNG::seed(seed);
  FSPProblemFactory::init("/home/lucasmp/dev/ig-aos-flowshop/data");
  MHParamsSpecsFactory::init(
      "/home/lucasmp/dev/ig-aos-flowshop/data"
      "/specs",
      true);

  std::unordered_map<std::string, std::string> prob;
  prob["problem"]            = "FSP";
  prob["type"]               = "PERM";
  prob["objective"]          = "MAKESPAN";
  prob["budget"]             = "med";
  prob["instance"]           = "taillard_rand_50_20_09.dat";
  prob["stopping_criterion"] = "FIXEDTIME";
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


  return 0;
}
