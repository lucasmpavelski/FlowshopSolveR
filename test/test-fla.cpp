#include <gtest/gtest.h>

#include "flowshop-solver/fla/SnowballLONSampling.hpp"
#include "flowshop-solver/fla_methods.hpp"


TEST(SamplingLON, CanSampleLON) {
  // data/fla/lons_cache/1/NEH;IG;2;10;15;3;SWAP_FSP;med;binom;mcorr;NOWAIT;MAKESPAN;10;5;3;all;EVALS;binom_mcorr_10_5_03.dat.Rdata 
  FSPProblemFactory::init(DATA_FOLDER);
  MHParamsSpecsFactory::init(DATA_FOLDER "/specs");
  std::unordered_map<std::string, std::string> prob;
  prob["problem"] = "flowshop";
  prob["type"] = "NOWAIT";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "med";
  prob["instance"] = "taill-like_rand_30_20_03.dat";
  prob["stopping_criterion"] = "EVALS";
  std::unordered_map<std::string, std::string> sampling_params;
  sampling_params["Init.Strat"    ] = "NEH";
  sampling_params["Sampling.Strat"] = "FULL_IG";
  sampling_params["Depth"         ] = "2";
  sampling_params["No.Edges"      ] = "10";
  sampling_params["Walk.Lengh"    ] = "15";
  sampling_params["Strength"      ] = "3";
  sampling_params["Perturbation.Operator"  ] = "SWAP";

  sampling_params["IG.Init.Strat"        ] = "1";
  sampling_params["IG.Comp.Strat"        ] = "1";
  sampling_params["IG.Neighborhood.Size" ] = "1";
  sampling_params["IG.Neighborhood.Strat"] = "1";
  sampling_params["IG.Local.Search"      ] = "1";
  sampling_params["IG.Accept"            ] = "1";
  sampling_params["IG.Accept.Temperature"] = "1";
  sampling_params["IG.Algo"              ] = "1";
  sampling_params["IG.Destruction.Size"  ] = "1";
  sampling_params["IG.LS.Single.Step"    ] = "1";
  sampling_params["IG.LSPS.Local.Search" ] = "1";
  sampling_params["IG.LSPS.Single.Step"  ] = "1";


 // sampleLON(prob, sampling_params, 123l);
}

auto main(int argc, char **argv) -> int {
  argc = 2;
  // char* argvv[] = {"", "--gtest_filter=FLA.*"};
  // testing::InitGoogleTest(&argc, argvv);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
