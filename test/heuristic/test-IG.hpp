#pragma once

#include <gtest/gtest.h>
#include <unordered_map>

#include "flowshop-solver/FSPProblemFactory.hpp"
#include "flowshop-solver/MHParamsSpecs.hpp"
#include "flowshop-solver/MHParamsSpecsFactory.hpp"
#include "flowshop-solver/MHParamsValues.hpp"
#include "flowshop-solver/eoFSPFactory.hpp"
#include "flowshop-solver/heuristics/all.hpp"
#include "flowshop-solver/problems/FSPProblem.hpp"

TEST(Solve, IG) {

  using namespace std;
  std::unordered_map<string, string> prob;
  std::unordered_map<string, string> params;
  
prob["dist"] = "uniform";
prob["corr"] = "random";
prob["no_jobs"] = "300";
prob["no_machines"] = "60";
prob["problem"] = "flowshop";
prob["corv"] = "0";
prob["objective"] = "MAKESPAN";
prob["type"] = "PERM";
prob["stopping_criterion"] = "TIME";
prob["budget"] = "high";
prob["id"] = "2699";
prob["instance"] = "uniform_random_200_60_06.txt";
;

params["IG.Init"] = "neh";
params["IG.Init.NEH.Ratio"] = "0";
params["IG.Init.NEH.Priority"] = "sum_pij";
params["IG.Init.NEH.PriorityOrder"] = "incr";
params["IG.Init.NEH.PriorityWeighted"] = "no";
params["IG.Init.NEH.Insertion"] = "last_best";
params["IG.Comp.Strat"] = "strict";
params["IG.Neighborhood.Size"] = "1.0";
params["IG.Neighborhood.Strat"] = "ordered";
params["IG.Local.Search"] = "best_insertion";
params["IG.LS.Single.Step"] = "1";
params["IG.Accept"] = "better";
params["IG.Accept.Better.Comparison"] = "strict";
params["IG.Accept.Temperature"] = "0.25";
params["IG.Perturb"] = "rs";
params["IG.Perturb.Insertion"] = "random_best";
params["IG.Perturb.DestructionSizeStrategy"] = "adaptive";
params["IG.Perturb.DestructionSize"] = "4";
params["IG.DestructionStrategy"] = "random";

params["IG.AOS.WarmUp"           ] = "0"; // i (0,2000) | IG.Perturb.DestructionSizeStrategy == 'adaptive'
params["IG.AOS.WarmUp.Strategy"  ] = "fixed"; // c (random, fixed) | IG.Perturb.DestructionSizeStrategy == 'adaptive'
params["IG.AOS.Strategy"         ] = "thompson_sampling"; // (probability_matching,frrmab,linucb,thompson_sampling,random) | IG.Perturb.DestructionSizeStrategy == 'adaptive'
params["IG.AOS.RewardType"       ] = "2"; // (0,1,2,3) | IG.Perturb.DestructionSizeStrategy == 'adaptive'  & IG.AOS.Strategy != 'random'
params["IG.AOS.Options"          ] = "1_2_4"; // (2_4, 1_2_4, 1_4_8, 2_4_8) | IG.Perturb.DestructionSizeStrategy == 'adaptive' & IG.AOS.Strategy != 'random'
params["IG.AOS.TS.Strategy"      ] = "dynamic"; // c (static, dynamic) | IG.Perturb.DestructionSizeStrategy == 'adaptive' & IG.AOS.Strategy == 'thompson_sampling'
params["IG.AOS.TS.C"             ] = "10"; // i (1,500)  | IG.Perturb.DestructionSizeStrategy == 'adaptive' & IG.AOS.Strategy == 'thompson_sampling' & IG.AOS.TS.Strategy == 'dynamic'


RNG::seed(557698556);
;
  RunOptions ro;;
  ro.printBestFitness = true;;
   
   auto result = solveWith("IG", prob, params, ro);;
;
  std::cerr << result;;
;
  ASSERT_TRUE(result.fitness > 0);  ;
  ASSERT_TRUE(result.no_evals > 0);  
  ASSERT_TRUE(result.time > 0); 
}