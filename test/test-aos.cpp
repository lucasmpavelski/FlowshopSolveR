#include <iostream>
#include <array>

#include <gtest/gtest.h>

#include "aos/probability_matching.hpp"
#include "aos/lin_ucb.hpp"
#include "flowshop-solver/fspproblemfactory.hpp"
#include "flowshop-solver/specsdata.hpp"
#include "flowshop-solver/global.hpp"
#include "flowshop-solver/heuristics/all.hpp"

TEST(LinUCB, AOSTests)
{
  RNG::seed(123l);
  FSPProblemFactory::init(DATA_FOLDER);
  MHParamsSpecsFactory::init(DATA_FOLDER "/specs", true);
  std::unordered_map<std::string, std::string> prob;
  prob["problem"] = "FSP";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "high";
  prob["instance"] = "taill-like_rand_50_20_03.dat";
  prob["stopping_criterium"] = "EVALS";
  std::unordered_map<std::string, double> params;
  params["IG.Init.Strat"] = 1;
  params["IG.Comp.Strat"] = 0;
  params["IG.Neighborhood.Size"] = 9.9999;
  params["IG.Neighborhood.Strat"] = 0;
  params["IG.Local.Search"] = 3;
  params["IG.Accept"] = 1;
  params["IG.Accept.Temperature"] = 0.25;
  params["IG.Algo"] = 2;
  params["IG.Destruction.Size"] = 8.0 / 30;
  params["IG.LS.Single.Step"] = 0;
  params["IG.LSPS.Local.Search"] = 0;
  params["IG.LSPS.Single.Step"] = 0;
  std::cout << solveWith("IG", prob, params) << '\n';
  ASSERT_TRUE(1);
}

int main(int argc, char **argv)
{
  argc = 2;
  // char* argvv[] = {"", "--gtest_filter=FLA.*"};
  // testing::InitGoogleTest(&argc, argvv);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
