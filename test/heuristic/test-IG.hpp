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
  prob["problem"] = "flowshop";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "low";
  prob["instance"] = "uniform_random_100_40_05.txt";
  prob["stopping_criterion"] = "EVALS";

  std::unordered_map<string, string> params;
  params["IG.Init"                           ] = "random"; // c (random,neh)
  params["IG.Init.NEH.Priority"              ] = "sum_pij"; // c (sum_pij,dev_pij,avgdev_pij,abs_dif,ss_sra,ss_srs,ss_srn_rcn,ss_sra_rcn,ss_srs_rcn,ss_sra_2rcn,ra_c1,ra_c2,ra_c3) | IG.Init == 'neh'
  params["IG.Init.NEH.PriorityOrder"         ] = "incr"; // c (incr,decr,hill,valley,hi_hilo,hi_lohi,lo_hilo,lo_lohi) | IG.Init == 'neh'
  params["IG.Init.NEH.PriorityWeighted"      ] = "0"; // i (0,1) | IG.Init == 'neh'
  params["IG.Init.NEH.Insertion"             ] = "first_best"; // c (first_best,last_best,random_best) | IG.Init == 'neh'
  params["IG.Comp.Strat"                     ] = "strict"; // c (strict,equal)
  params["IG.Neighborhood.Size"              ] = "1.0"; // r (0.0,1.0)
  params["IG.Neighborhood.Strat"             ] = "ordered"; // c (ordered,random)
  params["IG.Local.Search"                   ] = "first_improvement"; // c (none,first_improvement,best_improvement,random_best_improvement,best_insertion)
  params["IG.LS.Single.Step"                 ] = "1"; // c (0, 1)
  params["IG.Accept"                         ] = "better"; // c (always,better,temperature)
  params["IG.Accept.Better.Comparison"       ] = "equal"; // c (equal,strict)
  params["IG.Accept.Temperature"             ] = "5.0"; // r (0.0,5.0)    | IG.Accept == 'temperature'
  params["IG.Perturb"                        ] = "lsps"; // c (rs,lsps)
  params["IG.Perturb.DestructionSizeStrategy"] = "adaptive"; // c (fixed,adaptive)
  params["IG.Perturb.DestructionSize"        ] = "4"; // i (2,8) | IG.Perturb.DestructionSizeStrategy == 'fixed'
  params["IG.Perturb.Insertion"              ] = "first_best"; // c (first_best,last_best,random_best)
  params["IG.LSPS.Local.Search"              ] = "best_improvement"; // c (none,first_improvement,best_improvement,random_best_improvement,best_insertion) | IG.Perturb == 'lsps'
  params["IG.LSPS.Single.Step"               ] = "1"; // c (0,1) | IG.Perturb == 'lsps' & IG.LSPS.Local.Search != 'none'
  params["IG.AOS.Strategy"                   ] = "frrmab"; // c (probability_matching,frrmab,linucb,thompson_sampling,random) | IG.Perturb.DestructionSizeStrategy == 'adaptive'
  params["IG.AOS.RewardType"                 ] = "2"; // c (0,1,2,3) | IG.Perturb.DestructionSizeStrategy == 'adaptive'  & IG.AOS.Strategy != 'random'
  params["IG.AOS.Options"                    ] = "1_4_8"; // c (2_4, 1_2, 1_4_8, 2_4_8) | IG.Perturb.DestructionSizeStrategy == 'adaptive' & IG.AOS.Strategy != 'random'
  params["IG.AOS.PM.RewardType"              ] = "avgabs"; // c (avgabs,avgnorm,extabs,extnorm)  | IG.Perturb.DestructionSizeStrategy == 'adaptive' & IG.AOS.Strategy == 'probability_matching'
  params["IG.AOS.PM.Alpha"                   ] = "0.3"; // r (0.1, 0.9) | IG.Perturb.DestructionSizeStrategy == 'adaptive' & IG.AOS.Strategy == 'probability_matching'
  params["IG.AOS.PM.PMin"                    ] = "0.01"; // r (0.05, 0.2) | IG.Perturb.DestructionSizeStrategy == 'adaptive' & IG.AOS.Strategy == 'probability_matching'
  params["IG.AOS.PM.UpdateWindow"            ] = "500"; // i (1,500) | IG.Perturb.DestructionSizeStrategy == 'adaptive' & IG.AOS.Strategy == 'probability_matching'
  params["IG.AOS.FRRMAB.WindowSize"          ] = "450"; // i (10, 500) | IG.Perturb.DestructionSizeStrategy == 'adaptive' & IG.AOS.Strategy == 'frrmab'
  params["IG.AOS.FRRMAB.Scale"               ] = "0.5"; // r (0.01, 100) | IG.Perturb.DestructionSizeStrategy == 'adaptive' & IG.AOS.Strategy == 'frrmab'
  params["IG.AOS.FRRMAB.Decay"               ] = "0.9"; // r (0.25, 1.0) | IG.Perturb.DestructionSizeStrategy == 'adaptive' & IG.AOS.Strategy == 'frrmab'
  params["IG.AOS.LINUCB.Alpha"               ] = "0.75"; // r (0.0, 1.5) | IG.Perturb.DestructionSizeStrategy == 'adaptive' & IG.AOS.Strategy == 'linucb'
  params["IG.AOS.WarmUp.Proportion"          ] = "0.4"; // r (0.0,0.5) | IG.Perturb.DestructionSizeStrategy == 'adaptive'
  params["IG.AOS.WarmUp.Strategy"            ] = "random"; // c (random, fixed) | IG.Perturb.DestructionSizeStrategy == 'adaptive'
  params["IG.AOS.TS.Strategy"                ] = "dynamic"; // c (static, dynamic) | IG.Perturb.DestructionSizeStrategy == 'adaptive' & IG.AOS.Strategy == 'thompson_sampling'
  params["IG.AOS.TS.C"                       ] = "250"; // i (1,500)  | IG.Perturb.DestructionSizeStrategy == 'adaptive' & IG.AOS.Strategy == 'thompson_sampling' & IG.AOS.TS.Strategy == 'dynamic'
  params["IG.DestructionStrategy"            ] = "adaptive_position";
  params["IG.AdaptivePosition.AOS.Strategy"] = "random";
  params["IG.AdaptivePosition.AOS.Options"] = "1_2_3";
  params["IG.AdaptivePosition.AOS.TS.Strategy"] = "dynamic";
  params["IG.AdaptivePosition.AOS.TS.C"] = "250";
  params["IG.AdaptivePosition.AOS.WarmUp.Proportion"] = "0.25";
  params["IG.AdaptivePosition.AOS.WarmUp.Strategy"] = "random";

  auto result = solveWith("IG", prob, params);

  ASSERT_TRUE(result.fitness > 0);  
  ASSERT_TRUE(result.no_evals > 0);  
  ASSERT_TRUE(result.time > 0);  
}