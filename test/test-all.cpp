#include <gtest/gtest.h>

#include <algorithm>
#include <cassert>
#include <numeric>
#include <vector>
#include "../src/flowshop-solver/heuristics/aco.hpp"
#include "../src/flowshop-solver/heuristics/all.hpp"
#include "../src/flowshop-solver/heuristics/hc.hpp"
#include "../src/flowshop-solver/heuristics/ig.hpp"
#include "../src/flowshop-solver/heuristics/ihc.hpp"
#include "../src/flowshop-solver/heuristics/ils.hpp"
#include "../src/flowshop-solver/heuristics/isa.hpp"
#include "../src/flowshop-solver/heuristics/sa.hpp"
#include "../src/flowshop-solver/heuristics/ts.hpp"

std::vector<int> johnson(const std::vector<int>& a, const std::vector<int>& b) {
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

std::vector<int> johnsonApproximate(const std::vector<int>& a,
                                    const std::vector<int>& b,
                                    const std::vector<int>& c) {
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

#include "../flowshop-solver/problems/FSP.hpp"
#include "../flowshop-solver/problems/FSPData.hpp"
#include "../flowshop-solver/problems/FSPEval.hpp"

int lowerBound1(const FSPData& fspData) {
  int no_jobs = fspData.noJobs(), no_machines = fspData.noMachines();
  PermFSPEval<FSPMin> completionTime(fspData, Objective::MAKESPAN);

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
    for (unsigned j = 0; j < unscheduled.size(); j++) {
      int atSum = fspData.partialSumOnAdjacentMachines(
          unscheduled[j], h + 1, fspData.noMachines() - 1);
      if (atSum < min_j)
        min_j = atSum;
    }
    return min_j;
  };

  int lb1 = 0;
  for (int i = 0; i < no_machines; i++) {
    int s = 0;
    for (unsigned j = 0; j < unscheduled.size(); j++) {
      s += fspData.pt(unscheduled[j], i);
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
// #include "../src/flowshop-solver/heuristics/ilsKickOp.hpp"
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
  FSPProblemFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data");
  MHParamsSpecsFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data/specs", 1);
  std::unordered_map<string, string> prob;
  prob["problem"] = "FSP";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "low";
  prob["instance"] = "binom_rand_30_20_01.dat";
  prob["stopping_criterion"] = "EVALS";

  MHParamsSpecs mhParamsSpecs = MHParamsSpecsFactory::get("HC");
  MHParamsValues values(&mhParamsSpecs);
  values.randomizeValues(RNG::engine);
  // std::cout << solveWithHC(prob, values.toMap());
}

TEST(Solve, SA) {
  using std::string;
  RNG::seed(123l);
  FSPProblemFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data/");
  MHParamsSpecsFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data/specs/", 1);
  std::unordered_map<string, string> prob;
  prob["problem"] = "FSP";
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

#include "fla_methods.hpp"

TEST(FLA, ADAPTIVE) {
  using std::string;
  RNG::seed(123l);
  FSPProblemFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data");
  MHParamsSpecsFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data/specs", 1);
  std::unordered_map<string, string> prob;
  prob["problem"] = "FSP";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "low";
  prob["instance"] = "exp_rand_30_20_01.dat";
  prob["stopping_criterion"] = "EVALS";

  std::unordered_map<string, string> sampling;
  sampling["Init.Strat"] = "RANDOM";
  sampling["Sampling.Strat"] = "HC_LENGTH";

  std::cout << adaptiveWalkLength(prob, sampling, 123u);
}

TEST(FLA, ADAPTIVE_WALK) {
  using std::string;
  RNG::seed(123l);
  FSPProblemFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data");
  MHParamsSpecsFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data/specs", 1);
  std::unordered_map<string, string> prob;
  prob["problem"] = "FSP";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "low";
  prob["instance"] = "exp_rand_30_20_01.dat";
  prob["stopping_criterion"] = "EVALS";

  std::unordered_map<string, string> sampling;
  sampling["Init.Strat"] = "RANDOM";
  sampling["Sampling.Strat"] = "HC";

  std::cout << adaptiveWalk(prob, sampling, 123u).size();
}

TEST(Solve, TS) {
  using std::string;
  RNG::seed(123l);
  FSPProblemFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data");
  MHParamsSpecsFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data/specs", 1);
  std::unordered_map<string, string> prob;
  prob["problem"] = "FSP";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "med";
  prob["instance"] = "binom_rand_30_20_01.dat";
  prob["stopping_criterion"] = "EVALS";

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
  FSPProblemFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data");
  MHParamsSpecsFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data/specs", 1);
  std::unordered_map<string, string> prob;
  prob["problem"] = "FSP";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "low";
  prob["instance"] = "binom_rand_30_20_01.dat";
  prob["stopping_criterion"] = "TIME";

  MHParamsSpecs mhParamsSpecs = MHParamsSpecsFactory::get("ILS");
  MHParamsValues values(&mhParamsSpecs);
  values.randomizeValues(RNG::engine);
  // std::cout << solveWithILS(prob, values.toMap());
}

TEST(Solve, IHC) {
  using std::string;
  RNG::seed(123l);
  FSPProblemFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data");
  MHParamsSpecsFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data/specs", 1);
  std::unordered_map<string, string> prob;
  prob["problem"] = "FSP";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "med";
  prob["instance"] = "binom_rand_30_20_01.dat";
  prob["stopping_criterion"] = "EVALS";

  for (int i = 0; i < 10; i++) {
    MHParamsSpecs mhParamsSpecs = MHParamsSpecsFactory::get("IHC");
    MHParamsValues values(&mhParamsSpecs);
    values.randomizeValues(RNG::engine);
    // std::cout << solveWithIHC(prob, values.toMap());
  }
}

TEST(Solve, IG) {
  using std::string;
  RNG::seed(123l);
  FSPProblemFactory::init(DATA_FOLDER);
  MHParamsSpecsFactory::init(DATA_FOLDER "/specs", 1);
  std::unordered_map<string, string> prob;
  // prob["problem"] = "FSP";
  // prob["type"] = "PERM";
  // prob["objective"] = "MAKESPAN";
  // prob["budget"] = "low";
  // prob["instance"] = "binom_rand_30_20_01.dat";
  // prob["stopping_criterion"] = "EVALS";

  prob["problem"] = "FSP";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "med";
  prob["instance"] = "taill-like_rand_30_20_01.dat";
  prob["stopping_criterion"] = "EVALS";

  MHParamsSpecs mhParamsSpecs = MHParamsSpecsFactory::get("IG");
  MHParamsValues values(&mhParamsSpecs);
  values.randomizeValues(RNG::engine);

  // std::cout << solveWithIG(prob, values.toMap());
}

TEST(Solve, ISA) {
  using std::string;
  RNG::seed(123l);
  FSPProblemFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data");
  MHParamsSpecsFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data/specs", 1);

  std::unordered_map<string, string> prob;
  prob["problem"] = "FSP";
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
  FSPProblemFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data");
  MHParamsSpecsFactory::init(
      "/home/lucasmp/projects/git/evolutionary_tunners/data/specs", 1);
  std::unordered_map<string, string> prob;
  prob["problem"] = "FSP";
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

TEST(Solve, AllParamsInAllMH) {
  MHParamsSpecs allSpecs = MHParamsSpecsFactory::get("all");
  std::vector<ParamSpec> params;
  for (auto param : allSpecs) {
    params.push_back(*param);
  }
  unsigned count = 0;
  for (auto mh : all_mh) {
    MHParamsSpecs specs = MHParamsSpecsFactory::get(mh);
    count += specs.noParams();
    for (auto param : specs) {
      ASSERT_TRUE(std::find(params.begin(), params.end(), *param) !=
                  params.end());
    }
  }
  ASSERT_TRUE(params.size() == count + 1);
}

TEST(Solve, SolveAllMHs) {
  RNG::seed(1234);
  MHParamsSpecs allSpecs = MHParamsSpecsFactory::get("all");
  MHParamsValues params(&allSpecs);
  params.randomizeValues(RNG::engine);
  std::unordered_map<std::string, std::string> prob;
  prob["problem"] = "FSP";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "med";
  prob["instance"] = "binom_rand_30_20_01.dat";
  prob["stopping_criterion"] = "EVALS";
  RNG::seed(1234);

  for (std::string mh : all_mh) {
    params["MH"] = allSpecs.getValue("MH", mh);
    RNG::seed(1234);
    std::cerr << mh << "\n";
    Result mhResult = solveWith(mh, prob, params.toMap());
    RNG::seed(1234);
    // Result allResult = solveWithAll(prob, params.toMap());
    // std::cerr << mh << " " << params["MH"] << " " << allResult.fitness <<
    // "\n"; ASSERT_TRUE(mhResult == allResult);
  }
}
// TEST(Solve, irace) {
//   using std::string;
//   RNG::seed(123l);
//   FSPProblemFactory::init(
//       "/home/lucasmp/projects/git/evolutionary_tunners/data");
//   MHParamsSpecsFactory::init(
//       "/home/lucasmp/projects/git/evolutionary_tunners/data/specs", 1);
//   std::unordered_map<string, string> prob;
//   prob["problem"] = "FSP";
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

int main(int argc, char** argv) {
  argc = 2;
  // char* argvv[] = {"", "--gtest_filter=FLA.*"};
  // testing::InitGoogleTest(&argc, argvv);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
