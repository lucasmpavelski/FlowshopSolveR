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
params["IG.Init"] = "neh" ;
params["IG.Init.NEH.Ratio"] = "0" ;
params["IG.Init.NEH.Priority"] = "sum_pij" ;
params["IG.Init.NEH.PriorityOrder"] = "incr" ;
params["IG.Init.NEH.PriorityWeighted"] = "no" ;
params["IG.Init.NEH.Insertion"] = "first_best" ;
params["IG.Comp.Strat"] = "strict" ;
params["IG.Neighborhood.Size"] = "1.0" ;
params["IG.Neighborhood.Strat"] = "ordered" ;
params["IG.LS.Single.Step"] = "0" ;
params["IG.Accept"] = "temperature" ;
params["IG.Accept.Better.Comparison"] = "strict" ;
params["IG.Accept.Temperature"] = "0.25" ;
params["IG.Perturb.Insertion"] = "random_best" ;
params["IG.Perturb"] = "rs" ;
params["IG.Perturb.DestructionSizeStrategy"] = "fixed" ;
params["IG.Perturb.DestructionSize"] = "4" ;
params["IG.DestructionStrategy"] = "random" ;
params["IG.Local.Search"] = "best_insertion" ;
params["IG.LSPS.Local.Search"] = "best_insertion";
params["IG.LSPS.Single.Step"] = "0";
params["IG.AdaptiveNeighborhoodSize.AOS.WarmUp"] = "1000" ;
params["IG.AdaptiveNeighborhoodSize.AOS.WarmUp.Strategy"] = "random" ;
params["IG.AdaptiveNeighborhoodSize.AOS.RewardType"] = "3" ;
params["IG.AdaptiveNeighborhoodSize.AOS.NoArms"] = "10" ;
params["IG.AdaptiveNeighborhoodSize.AOS.Strategy"] = "thompson_sampling" ;
params["IG.AdaptiveNeighborhoodSize.AOS.TS.Strategy"] = "static" ;
params["IG.AdaptiveNeighborhoodSize.AOS.TS.C"] = "NA" ;
prob["dist"] = "exponential" ;
prob["corr"] = "random" ;
prob["no_jobs"] = "50" ;
prob["no_machines"] = "10" ;
prob["problem"] = "flowshop" ;
prob["corv"] = "0" ;
prob["objective"] = "MAKESPAN" ;
prob["type"] = "PERM" ;
prob["stopping_criterion"] = "FIXEDTIME" ;
prob["budget"] = "low" ;
prob["id"] = "791" ;
prob["set_type"] = "train" ;
prob["instance"] = "exponential_random_50_10_01.txt" ;
RNG::seed(557698556);
;
  RunOptions ro;;
  ro.printBestFitness = true;;
   
   auto result = solveWith("IG", prob, params, ro);;
;
  std::cerr << result;;
;/*
  ASSERT_TRUE(result.fitness > 0);  ;
  ASSERT_TRUE(result.no_evals > 0);  
  ASSERT_TRUE(result.time > 0); */
}