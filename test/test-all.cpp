#include <algorithm>
#include <cassert>
#include <numeric>
#include <vector>

#include <gtest/gtest.h>

#include "flowshop-solver/fla/LocalOptimaNetwork.hpp"
#include "flowshop-solver/heuristics/aco.hpp"
#include "flowshop-solver/heuristics/all.hpp"
#include "flowshop-solver/heuristics/hc.hpp"
#include "flowshop-solver/heuristics/ig.hpp"
#include "flowshop-solver/heuristics/ihc.hpp"
#include "flowshop-solver/heuristics/ils.hpp"
#include "flowshop-solver/heuristics/isa.hpp"
#include "flowshop-solver/heuristics/sa.hpp"
#include "flowshop-solver/heuristics/ts.hpp"

auto johnson(const std::vector<int>& a, const std::vector<int>& b)
    -> std::vector<int> {
  assert(a.size() == b.size());
  std::vector<int> perm(a.size());
  std::iota(perm.begin(), perm.end(), 0);
  auto middle = std::partition(perm.begin(), perm.end(),
                               [&a, &b](unsigned i) { return a[i] <= b[i]; });
  std::sort(perm.begin(), middle,
            [&a](unsigned i, unsigned j) { return a[i] < a[j]; });
  std::sort(middle, perm.end(),
            [&b](unsigned i, unsigned j) { return b[i] > b[j]; });
  return perm;
}

auto johnsonApproximate(const std::vector<int>& a,
                        const std::vector<int>& b,
                        const std::vector<int>& c) -> std::vector<int> {
  assert(a.size() == b.size() && a.size() == c.size());
  std::vector<int> ab(a.size());
  std::vector<int> bc(a.size());
  std::transform(a.begin(), a.end(), b.begin(), ab.begin(), std::plus<>());
  std::transform(b.begin(), b.end(), c.begin(), bc.begin(), std::plus<>());
  return johnson(ab, bc);
}

TEST(LowerBound, JohnsonAlgorithm) {
  auto perm =
      johnson(std::vector<int>{3, 5, 1, 6, 7}, std::vector<int>{6, 2, 2, 6, 5});
  auto resp = {2, 0, 3, 4, 1};
  ASSERT_TRUE(std::equal(perm.begin(), perm.end(), resp.begin(), resp.end()));
}

#include "flowshop-solver/problems/FSP.hpp"
#include "flowshop-solver/problems/FSPData.hpp"
#include "flowshop-solver/problems/FSPEval.hpp"

auto lowerBound1(const FSPData& fspData) -> int {
  int no_jobs = fspData.noJobs(), no_machines = fspData.noMachines();
  PermFSPMakespanEval completionTime(fspData);

  std::vector<int> scheduled(1);
  std::vector<int> unscheduled;
  unscheduled.reserve(no_jobs - 1);
  scheduled[0] = 0;
  for (int i = 0; i < no_jobs; i++) {
    if (std::find(scheduled.begin(), scheduled.end(), i) == scheduled.end())
      unscheduled.push_back(i);
  }

  std::vector<int> completionTimes0(no_machines);
  completionTimes0[0] = fspData.pt(scheduled[0], 0);
  for (int i = 1; i < no_machines; i++) {
    completionTimes0[i] = completionTimes0[i - 1] + fspData.pt(scheduled[0], i);
  }

  auto r_i = [&fspData, &completionTimes0, &unscheduled](int i) {
    int max = 0;
    for (int k = 0; k <= i; k++) {
      int min_j = std::numeric_limits<int>::max();
      for (int j : unscheduled) {
        int atSum = fspData.partialSumOnAdjacentMachines(j, k, i - 1);
        if (atSum < min_j)
          min_j = atSum;
      }
      int r = completionTimes0[k] + min_j;
      if (r > max)
        max = r;
    }
    return max;
  };

  auto q_h = [&fspData, &unscheduled](int h) {
    std::cout << "q_" << h << ": ";
    int min_j = 10000;
    for (int j : unscheduled) {
      int atSum = fspData.partialSumOnAdjacentMachines(
          j, h + 1, fspData.noMachines() - 1);
      if (atSum < min_j)
        min_j = atSum;
    }
    return min_j;
  };

  int lb1 = 0;
  for (int i = 0; i < no_machines; i++) {
    int s = 0;
    for (int j : unscheduled) {
      s += fspData.pt(j, i);
    }
    int val = r_i(i) + s + q_h(i);
    std::cout << "r_" << i << ": " << r_i(i) << '\t';
    std::cout << "psum_" << i << ": " << s << '\t';
    std::cout << "q_" << i << ": " << q_h(i) << '\n';
    if (val > lb1)
      lb1 = val;
  }

  return lb1;
}

// TEST(LowerBound, lowerBound1) {
//  FSPData fspData(4, 4);
//  fspData.pt(0, 0) = 4; fspData.pt(1, 0) = 2; fspData.pt(2, 0) = 3;
//  fspData.pt(3, 0) = 5; fspData.pt(0, 1) = 3; fspData.pt(1, 1) = 8;
//  fspData.pt(2, 1) = 2; fspData.pt(3, 1) = 4; fspData.pt(0, 2) = 7;
//  fspData.pt(1, 2) = 2; fspData.pt(2, 2) = 4; fspData.pt(3, 2) = 3;
//  fspData.pt(0, 3) = 3; fspData.pt(1, 3) = 5; fspData.pt(2, 3) = 1;
//  fspData.pt(3, 3) = 5; ASSERT_EQ(lowerBound1(fspData), 28);
//}
//
// TEST(LowerBound, JohnsonAlgorithmWithTies) {
//  auto perm = johnson(
//    std::vector<int>{4, 2, 4},
//    std::vector<int>{3, 2, 5});
//  auto resp = {1, 2, 0};
//  ASSERT_TRUE(std::equal(perm.begin(), perm.end(), resp.begin(), resp.end()));
//}

// TODO: relocate operator files
// #include "flowshop-solver/heuristics/ilsKickOp.hpp"
// TEST(Operator, ilsKick) {
//   ilsKickOp<std::vector<int>> op(1, 0.0);
//   // for (int i = 0; i < 1000; i++) {
//   std::vector<int> a = {1, 2, 3, 4};
//   op(a);
//   std::cerr << a[0] << a[1] << a[2] << a[3] << '\n';
//   //}
// }

TEST(Solve, HC) {
  using std::string;
  RNG::seed(123l);
  std::unordered_map<string, string> prob;
  prob["problem"] = "flowshop";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "low";
  prob["instance"] = "uniform_machine-correlated_100_60_01.txt";
  prob["stopping_criterion"] = "EVALS";

  MHParamsSpecs mhParamsSpecs = MHParamsSpecsFactory::get("HC");
  MHParamsValues values(&mhParamsSpecs);
  values.randomizeValues(RNG::engine);
  // std::cout << solveWithHC(prob, values.toMap());
}

TEST(Solve, SA) {
  using std::string;
  RNG::seed(123l);
  std::unordered_map<string, string> prob;
  prob["problem"] = "flowshop";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "low";
  prob["instance"] = "taill-like_rand_20_10_01.dat";
  prob["stopping_criterion"] = "FITNESS";

  MHParamsSpecs mhParamsSpecs = MHParamsSpecsFactory::get("SA");
  MHParamsValues values(&mhParamsSpecs);
  values.randomizeValues(RNG::engine);

  // std::cout << solveWithSA(prob, values.toMap());
}

#include "flowshop-solver/fla_methods.hpp"

TEST(FLA, ADAPTIVE) {
  using std::string;
  RNG::seed(123l);
  std::unordered_map<string, string> prob;
  prob["problem"] = "flowshop";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "low";
  prob["instance"] = "uniform_machine-correlated_100_60_01.txt";
  prob["stopping_criterion"] = "FITNESS";

  std::unordered_map<string, string> sampling;
  sampling["Init.Strat"] = "RANDOM";
  sampling["Sampling.Strat"] = "HC_LENGTH";

  std::cout << adaptiveWalkLength(prob, sampling, 123u);
}

TEST(FLA, ADAPTIVE_WALK) {
  using std::string;
  RNG::seed(123l);
  std::unordered_map<string, string> prob;
  prob["problem"] = "flowshop";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "low";
  prob["instance"] = "uniform_machine-correlated_100_60_01.txt";
  prob["stopping_criterion"] = "FITNESS";

  std::unordered_map<string, string> sampling;
  sampling["Init.Strat"] = "RANDOM";
  sampling["Sampling.Strat"] = "HC";

  std::cout << adaptiveWalk(prob, sampling, 123u).size();
}

TEST(Solve, TS) {
  using std::string;
  RNG::seed(123l);
  std::unordered_map<string, string> prob;
  prob["problem"] = "flowshop";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "low";
  prob["instance"] = "y";
  prob["stopping_criterion"] = "FITNESS";

  for (int i = 0; i < 10; i++) {
    MHParamsSpecs mhParamsSpecs = MHParamsSpecsFactory::get("TS");
    MHParamsValues values(&mhParamsSpecs);
    values.randomizeValues(RNG::engine);
    // std::cout << solveWithTS(prob, values.toMap());
  }
}

TEST(Solve, ILS) {
  using std::string;
  RNG::seed(123l);
  std::unordered_map<string, string> prob;
  prob["problem"] = "flowshop";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "low";
  prob["instance"] = "binom_random_30_20_01.dat";
  prob["stopping_criterion"] = "TIME";

  MHParamsSpecs mhParamsSpecs = MHParamsSpecsFactory::get("ILS");
  MHParamsValues values(&mhParamsSpecs);
  values.randomizeValues(RNG::engine);
  // std::cout << solveWithILS(prob, values.toMap());
}

TEST(Solve, IHC) {
  using std::string;
  RNG::seed(123l);
  std::unordered_map<string, string> prob;
  prob["problem"] = "flowshop";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "med";
  prob["instance"] = "binom_rand_30_20_01.dat";
  prob["stopping_criterion"] = "EVALS";

  for (int i = 0; i < 10; i++) {
    MHParamsSpecs mhParamsSpecs = MHParamsSpecsFactory::get("IHC");
    MHParamsValues values(&mhParamsSpecs);
    values.randomizeValues(RNG::engine);
  }
}

TEST(Solve, ISA) {
  using std::string;
  RNG::seed(123l);

  std::unordered_map<string, string> prob;
  prob["problem"] = "flowshop";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "med";
  prob["instance"] = "binom_rand_30_20_01.dat";
  prob["stopping_criterion"] = "TIME";

  MHParamsSpecs mhParamsSpecs = MHParamsSpecsFactory::get("ISA");
  MHParamsValues values(&mhParamsSpecs);
  values.randomizeValues(RNG::engine);

  // std::cout << solveWithISA(prob, values.toMap());
}

TEST(Solve, ACO) {
  using std::string;
  RNG::seed(123l);
  std::unordered_map<string, string> prob;
  prob["problem"] = "flowshop";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "low";
  prob["instance"] = "binom_jcorr_10_10_05.dat";
  prob["stopping_criterion"] = "EVALS";

  MHParamsSpecs mhParamsSpecs = MHParamsSpecsFactory::get("ACO");
  MHParamsValues values(&mhParamsSpecs);
  values.randomizeValues(RNG::engine);
  // std::cout << solveWithACO(prob, values.toMap());  // 10 7 6 0 3 9 4 5 1 2 8
}

std::vector<std::string> all_mh = {"IHC", "ISA", "TS", "ILS", "IG", "ACO"};
auto mh_cat_values = {0, 1, 2,
                      3, 4, 5};  // TODO: permit values for categorical params
//
// TEST(Solve, AllParamsInAllMH) {
//   MHParamsSpecs allSpecs = MHParamsSpecsFactory::get("all");
//   std::vector<ParamSpec> params;
//   for (const auto& param : allSpecs) {
//     params.push_back(*param);
//   }
//   unsigned count = 0;
//   for (const auto& mh : all_mh) {
//     MHParamsSpecs specs = MHParamsSpecsFactory::get(mh);
//     count += specs.noParams();
//     for (const auto& param : specs) {
//       ASSERT_TRUE(std::find(params.begin(), params.end(), *param) !=
//                   params.end());
//     }
//   }
//   ASSERT_TRUE(params.size() == count + 1);
// }

TEST(Solve, SolveAllMHs) {
  RNG::seed(1234);
  MHParamsSpecs allSpecs = MHParamsSpecsFactory::get("all");
  MHParamsValues params(&allSpecs);
  params.randomizeValues(RNG::engine);
  std::unordered_map<std::string, std::string> prob;
  prob["problem"] = "flowshop";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "med";
  prob["instance"] = "exponential_random_30_10_01.txt";
  prob["stopping_criterion"] = "EVALS";
  RNG::seed(1234);

  for (const std::string& mh : all_mh) {
    params["MH"] = allSpecs.getValue("MH", mh);
    RNG::seed(1234);
    std::cerr << mh << "\n";
    Result mhResult = solveWith(mh, prob, params.toMap());
    RNG::seed(1234);
    auto paramMap = params.toMap();
    paramMap["mh"] = mh;
    Result allResult = solveWith("all", prob, paramMap);
    ASSERT_TRUE(mhResult == allResult);
  }
}
// TEST(Solve, irace) {
//   using std::string;
//   RNG::seed(123l);
//   std::unordered_map<string, string> prob;
//   prob["problem"] = "flowshop";
//   prob["type"] = "PERM";
//   prob["objective"] = "MAKESPAN";
//   prob["budget"] = "low";
//   prob["instance"] = "exp_rand_30_20_01.dat";
//   prob["stopping_criterion"] = "EVALS";

//   std::unordered_map<string, double> params;
//   params["Comp.Strat"        ] = 0.0;
//   params["Init.Strat"        ] = 1.0;
//   params["Neighborhood.Size" ] = 10.0;
//   params["Neighborhood.Strat"] = 1.0;
//   params["Local.Search"      ] = 3.0;
//   params["LS.Single.Step"    ] = 0.0;
//   params["MMAS.T.Min.Factor" ] = 0.25;
//   params["MMAS.Rho"          ] = 0.75;
//   params["MMAS.P0"           ] = 0.87;
//   std::cout << solveWithAny(prob, params);
// }

#include "flowshop-solver/fla/SnowballLONSampling.hpp"

TEST(FLA, Snowball) {
  std::unordered_map<std::string, std::string> prob;
  prob["problem"] = "vrf-small";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "med";
  prob["instance"] = "VRF60_10_1_Gap.txt";
  prob["stopping_criterion"] = "EVALS";

  std::unordered_map<std::string, std::string> sample;
  sample[                          "Snowball.Depth"] = "2" ;
  sample[                        "Snowball.NoEdges"] = "60" ;
  sample[                     "Snowball.WalkLength"] = "1" ;
  sample[                           "Snowball.Init"] = "random" ;
  sample[                     "Snowball.Comp.Strat"] = "strict" ;
  sample[              "Snowball.Neighborhood.Size"] = "1" ;
  sample[             "Snowball.Neighborhood.Strat"] = "ordered" ;
  sample[                   "Snowball.Local.Search"] = "best_insertion" ;
  sample[                 "Snowball.LS.Single.Step"] = "0" ;
  sample[                        "Snowball.Perturb"] = "rs" ;
  sample["Snowball.Perturb.DestructionSizeStrategy"] = "fixed" ;
  sample[        "Snowball.Perturb.DestructionSize"] = "4" ;
  sample[              "Snowball.Perturb.Insertion"] = "first_best" ;
  sample[                         "Snowball.Accept"] = "better" ;
  sample[       "Snowball.Accept.Better.Comparison"] = "equal" ;

  long seed = 123;

  SnowballLONSampling sampling;
  auto lon = sampling.sampleLON(prob, sample, seed);
  lon.print(std::cerr);
}

#include "flowshop-solver/fla/MarkovChainLONSampling.hpp"

TEST(FLA, MarkovChainLONSampling) {
  std::unordered_map<std::string, std::string> prob;
  prob["problem"] = "flowshop";
  prob["type"] = "PERM";
  prob["objective"] = "FLOWTIME";
  prob["budget"] = "med";
  prob["instance"] = "uniform_machine-correlated_30_20_01.txt";
  prob["stopping_criterion"] = "EVALS";

  std::unordered_map<std::string, std::string> sample;
  sample["MarkovChainLONSampling.Init"] = "random";
  sample["MarkovChainLONSampling.Comp.Strat"] = "strict";
  sample["MarkovChainLONSampling.Neighborhood.Size"] = "1";
  sample["MarkovChainLONSampling.Neighborhood.Strat"] = "ordered";
  sample["MarkovChainLONSampling.Local.Search"] = "best_insertion";
  sample["MarkovChainLONSampling.LS.Single.Step"] = "0";
  sample["MarkovChainLONSampling.Perturb"] = "rs";
  sample["MarkovChainLONSampling.Perturb.DestructionSizeStrategy"] = "fixed";
  sample["MarkovChainLONSampling.Perturb.DestructionSize"] = "8";
  sample["MarkovChainLONSampling.Perturb.Insertion"] = "first_best";
  sample["MarkovChainLONSampling.Accept"] = "better";
  sample["MarkovChainLONSampling.Accept.Better.Comparison"] = "equal";

  sample["MarkovChainLONSampling.NumberOfIterations"] = "10000";
  sample["MarkovChainLONSampling.NumberOfSamples"] = "200";

  long seed = 123;

  MarkovChainLONSampling sampling;
  sampling.sampleLON(prob, sample, seed);
}

TEST(FLA, MergeGraphContainsAllNodes) {
  LocalOptimaNetwork<FSP> lon1;
  FSP a = {1, 2};
  FSP b = {3, 4};
  lon1.addNode(a, a, 0);
  lon1.addNode(b, b, 0);
  LocalOptimaNetwork<FSP> lon2;
  FSP c = {5, 6};
  lon2.addNode(b, b, 0);
  lon2.addNode(c, c, 0);

  lon1.merge(lon2);

  ASSERT_EQ(3, lon1.size());
  ASSERT_TRUE(lon1.contains(a));
  ASSERT_TRUE(lon1.contains(b));
  ASSERT_TRUE(lon1.contains(c));
}

TEST(FLA, MergeGraphContainsAllEdges) {
  LocalOptimaNetwork<FSP> lon1;
  FSP a = {1, 2};
  FSP b = {3, 4};
  FSP c = {5, 6};
  lon1.addNode(a, a, 0);
  lon1.addNode(b, b, 0);
  lon1.addNode(c, c, 0);
  lon1.addEdge(a, b, 1);
  lon1.addEdge(b, c, 1);
  FSP d = {7, 8};
  LocalOptimaNetwork<FSP> lon2;
  lon2.addNode(b, b, 0);
  lon2.addNode(c, c, 0);
  lon2.addNode(d, d, 0);
  lon2.addEdge(b, c, 1);
  lon2.addEdge(c, d, 1);

  lon1.merge(lon2);

  ASSERT_EQ(3, lon1.noEdges());
  ASSERT_EQ(1, lon1.getEdge(a, b)->weight);
  ASSERT_EQ(2, lon1.getEdge(b, c)->weight);
  ASSERT_EQ(1, lon1.getEdge(c, d)->weight);
}

auto main(int argc, char** argv) -> int {
  FSPProblemFactory::init(DATA_FOLDER);
  MHParamsSpecsFactory::init(DATA_FOLDER "/specs", true);

  argc = 2;
  char* argvv[] = {"", "--gtest_filter=FLA.Snowball"};
  testing::InitGoogleTest(&argc, argvv);
  // testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
