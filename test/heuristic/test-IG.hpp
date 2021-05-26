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
params["IG.Neighborhood.Strat"] = "adaptive" ;
params["IG.LS.Single.Step"] = "0" ;
params["IG.Accept"] = "temperature" ;
params["IG.Accept.Better.Comparison"] = "strict" ;
params["IG.Accept.Temperature"] = "0.25" ;
params["IG.Perturb.Insertion"] = "random_best" ;
params["IG.Perturb.DestructionSizeStrategy"] = "adaptive" ;
params["IG.Perturb.DestructionSize"] = "4" ;
params["IG.DestructionStrategy"] = "adaptive_position" ;
params["IG.Local.Search"] = "adaptive_with_adaptive_best_insertion" ;
params["IG.LSPS.Local.Search"] = "best_insertion" ;
params["IG.LSPS.Single.Step"] = "0" ;
params["IG.Perturb"] = "adaptive" ;
params["IG.AdaptivePerturb.AOS.WarmUp"] = "0" ;
params["IG.AdaptivePerturb.AOS.WarmUp.Strategy"] = "random" ;
params["IG.AdaptivePerturb.AOS.RewardType"] = "1" ;
params["IG.AdaptivePerturb.AOS.Strategy"] = "frrmab" ;
params["IG.AdaptivePerturb.AOS.FRRMAB.WindowSize"] = "59" ;
params["IG.AdaptivePerturb.AOS.FRRMAB.Scale"] = "72.4176" ;
params["IG.AdaptivePerturb.AOS.FRRMAB.Decay"] = "0.7902" ;
params["IG.AOS.WarmUp"] = "185" ;
params["IG.AOS.WarmUp.Strategy"] = "fixed" ;
params["IG.AOS.RewardType"] = "2" ;
params["IG.AOS.Options"] = "4_8" ;
params["IG.AOS.Strategy"] = "thompson_sampling" ;
params["IG.AOS.TS.Strategy"] = "static" ;
params["IG.AOS.TS.C"] = "NA" ;
params["IG.AdaptiveLocalSearch.AOS.WarmUp"] = "0" ;
params["IG.AdaptiveLocalSearch.AOS.WarmUp.Strategy"] = "random" ;
params["IG.AdaptiveLocalSearch.AOS.RewardType"] = "1" ;
params["IG.AdaptiveLocalSearch.AOS.Strategy"] = "thompson_sampling" ;
params["IG.AdaptiveLocalSearch.AOS.TS.Strategy"] = "dynamic" ;
params["IG.AdaptiveLocalSearch.AOS.TS.C"] = "414" ;
params["IG.AdaptiveNeighborhoodSize.AOS.NoArms"] = "2" ;
params["IG.AdaptiveNeighborhoodSize.AOS.WarmUp"] = "2000" ;
params["IG.AdaptiveNeighborhoodSize.AOS.WarmUp.Strategy"] = "random" ;
params["IG.AdaptiveNeighborhoodSize.AOS.RewardType"] = "0" ;
params["IG.AdaptiveNeighborhoodSize.AOS.Strategy"] = "frrmab" ;
params["IG.AdaptiveNeighborhoodSize.AOS.FRRMAB.WindowSize"] = "32" ;
params["IG.AdaptiveNeighborhoodSize.AOS.FRRMAB.Scale"] = "14.2617" ;
params["IG.AdaptiveNeighborhoodSize.AOS.FRRMAB.Decay"] = "0.9113" ;
params["IG.AdaptivePosition.AOS.WarmUp"] = "2000" ;
params["IG.AdaptivePosition.AOS.WarmUp.Strategy"] = "random" ;
params["IG.AdaptivePosition.Replace"] = "no" ;
params["IG.AdaptivePosition.NoArms"] = "fixed_10" ;
params["IG.AdaptivePosition.RandomArm"] = "yes" ;
params["IG.AdaptivePosition.RewardType"] = "1" ;
params["IG.AdaptivePosition.AOS.Strategy"] = "thompson_sampling" ;
params["IG.AdaptivePosition.AOS.TS.Strategy"] = "static" ;
params["IG.AdaptivePosition.AOS.TS.C"] = "NA" ;
params["IG.AdaptiveBestInsertion.AOS.WarmUp"] = "100000" ;
params["IG.AdaptiveBestInsertion.AOS.WarmUp.Strategy"] = "random" ;
params["IG.AdaptiveBestInsertion.Replace"] = "no" ;
params["IG.AdaptiveBestInsertion.NoArms"] = "fixed_10" ;
params["IG.AdaptiveBestInsertion.RandomArm"] = "no" ;
params["IG.AdaptiveBestInsertion.AOS.Strategy"] = "frrmab" ;
params["IG.AdaptiveBestInsertion.AOS.FRRMAB.WindowSize"] = "271" ;
params["IG.AdaptiveBestInsertion.AOS.FRRMAB.Scale"] = "9.611" ;
params["IG.AdaptiveBestInsertion.AOS.FRRMAB.Decay"] = "0.8757" ;;
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
prob["set_type"] = "test" ;
prob["instance"] = "exponential_random_50_10_06.txt" ;;
RNG::seed(557698556);
;
  RunOptions ro;;
  ro.printBestFitness = false;;
   
   auto result = solveWith("IG", prob, params, ro);;
;
  std::cerr << result;;
;/*
  ASSERT_TRUE(result.fitness > 0);  ;
  ASSERT_TRUE(result.no_evals > 0);  
  ASSERT_TRUE(result.time > 0); */
}