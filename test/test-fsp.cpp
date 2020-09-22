#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <numeric>
#include <string>

#include <gtest/gtest.h>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/heuristics/InsertionStrategy.hpp"
#include "flowshop-solver/problems/FSPData.hpp"
#include "flowshop-solver/problems/FSPEval.hpp"
#include "flowshop-solver/problems/PermFSPEval.hpp"
#include "flowshop-solver/problems/NoIdleFSPEval.hpp"
#include "flowshop-solver/problems/NoWaitFSPEval.hpp"
#include "flowshop-solver/problems/NoWaitFSPNeighborMakespanEval.hpp"
#include "flowshop-solver/FSPProblemFactory.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

std::string instances_folder = TEST_FIXTURES_FOLDER;

void testLoadData() {
  using std::cout;
  using std::endl;
  FSPData fspData{instances_folder + "test.txt"};
  assert(fspData.maxCT() == 125);
  std::array<int, 20> pt = {5, 9, 9,  4, 9, 3, 4, 8, 8, 10,
                            5, 8, 10, 1, 8, 7, 1, 8, 6, 2};
  const auto& pt_ref = fspData.procTimesRef();
  assert(std::equal(std::begin(pt_ref), std::end(pt_ref), std::begin(pt)));
  assert(fspData.machineProcTimesRef()[0] == 27);
  assert(fspData.jobProcTimesRef()[0] == 33);
}

void testEvalMin() {
  using std::cout;
  using std::endl;
  FSPData dt{instances_folder + "test.txt"};
  PermFSPMakespanEval fsp_eval{dt};
  FSP sol(4);
  sol[0] = 4 - 1;
  sol[1] = 3 - 1;
  sol[2] = 1 - 1;
  sol[3] = 2 - 1;
  fsp_eval(sol);
  assert(sol.fitness() == 54);
}

void testEvalFlowtime() {
  using std::cout;
  using std::endl;
  FSPData dt{instances_folder + "test.txt"};
  PermFSPFlowtimeEval fsp_eval{dt};
  FSP sol(4);
  sol[0] = 4 - 1;
  sol[1] = 3 - 1;
  sol[2] = 1 - 1;
  sol[3] = 2 - 1;
  fsp_eval(sol);
  assert(sol.fitness() == (29 + 41 + 46 + 54));
}

void testPartialEval() {
  using std::cout;
  using std::endl;
  std::string instances_folder = TEST_FIXTURES_FOLDER;
  FSPData dt{instances_folder + "test.txt"};
  PermFSPMakespanEval fsp_eval{dt};
  FSP sol(2);
  sol[0] = 4 - 1;
  sol[1] = 3 - 1;
  fsp_eval(sol);
  assert(sol.fitness() == 41);
}

void testNIEval() {
  using std::cout;
  using std::endl;
  FSPData dt(instances_folder + "test.txt");
  NoIdleFSPMakespanEval fsp_eval(dt);
  FSP sol(4);
  FSP solp(2);
  sol[0] = solp[0] = 4 - 1;
  sol[1] = solp[1] = 3 - 1;
  sol[2] = 1 - 1;
  sol[3] = 2 - 1;
  fsp_eval(sol);
  fsp_eval(solp);
  assert(sol.fitness() == 56);
  assert(solp.fitness() == 42);
  NoIdleFSPFlowtimeEval fsp_evalft(dt);
  FSP solft = sol;
  FSP solftp = solp;
  fsp_evalft(solft);
  fsp_evalft(solftp);
  assert(solft.fitness() == 192);
  assert(solftp.fitness() == 78);
}

void testNWEval() {
  using std::cout;
  using std::endl;
  FSPData dt(instances_folder + "test.txt");
  NoWaitFSPMakespanEval fsp_eval(dt);
  // nwfspEval<FSP> fsp_eval(instances_folder + "test.txt");
  FSP sol(4);
  FSP solp(2);
  sol[0] = solp[0] = 4 - 1;
  sol[1] = solp[1] = 3 - 1;
  sol[2] = 1 - 1;
  sol[3] = 2 - 1;
  fsp_eval(sol);
  fsp_eval(solp);
  // cout << sol.fitness() << endl;
  // cout << solp.fitness() << endl;
  assert(sol.fitness() == 59);
  assert(solp.fitness() == 41);
  NoWaitFSPFlowtimeEval fsp_evalft(dt);
  // nwfspEval<FSP> fsp_evalft(instances_folder + "test.txt", 1);
  FSP solft = sol;
  FSP solftp = solp;
  fsp_evalft(solft);
  fsp_evalft(solftp);
  // cout << solft.fitness() << endl;
  // cout << solftp.fitness() << endl;
  assert(solft.fitness() == 180);
  assert(solftp.fitness() == 70);
}

// TEST(FSPTaillardAcelleration, Evaluation) {
//   rng.reseed(65465l);
//   const int no_jobs = 4;
//   const int no_machines = 3;
//   FSPData fspData(no_jobs, no_machines, 8);

//   ivec seq = {0, 1, 2, 3};
//   CompiledSchedule csd(no_jobs, no_machines);
//   csd.compile(fspData, seq);

//   PermFSPEval fsp_eval{fspData};
//   FSP sol(4);
//   sol[0] = 3;
//   sol[1] = 0;
//   sol[2] = 1;
//   sol[3] = 2;
//   fsp_eval(sol);
//   ASSERT_EQ(sol.fitness(), csd.makespan[0]);
//   sol[0] = 0;
//   sol[1] = 3;
//   sol[2] = 1;
//   sol[3] = 2;
//   fsp_eval(sol);
//   ASSERT_EQ(sol.fitness(), csd.makespan[1]);
//   sol[0] = 0;
//   sol[1] = 1;
//   sol[2] = 3;
//   sol[3] = 2;
//   fsp_eval(sol);
//   ASSERT_EQ(sol.fitness(), csd.makespan[2]);
//   sol[0] = 0;
//   sol[1] = 1;
//   sol[2] = 2;
//   sol[3] = 3;
//   fsp_eval(sol);
//   ASSERT_EQ(sol.fitness(), csd.makespan[3]);
// }

auto testMove(std::initializer_list<int> init,
              int from,
              int to,
              std::initializer_list<int> result) -> bool {
  FSP sol;
  sol.assign(init);
  FSPNeighbor ng(from, to, sol.size());
  ng.move(sol);
  return std::equal(sol.begin(), sol.end(), result.begin());
}

TEST(FSPNeighbor, Moves) {
  ASSERT_TRUE(testMove({1, 2, 3, 4, 5, 6}, 0, 0, {1, 2, 3, 4, 5, 6}));
  ASSERT_TRUE(testMove({1, 2, 3, 4, 5, 6}, 0, 1, {2, 1, 3, 4, 5, 6}));
  ASSERT_TRUE(testMove({1, 2, 3, 4, 5, 6}, 0, 2, {2, 3, 1, 4, 5, 6}));
  ASSERT_TRUE(testMove({1, 2, 3, 4, 5, 6}, 0, 3, {2, 3, 4, 1, 5, 6}));
  ASSERT_TRUE(testMove({1, 2, 3, 4, 5, 6}, 0, 4, {2, 3, 4, 5, 1, 6}));
  ASSERT_TRUE(testMove({1, 2, 3, 4, 5, 6}, 0, 5, {2, 3, 4, 5, 6, 1}));

  ASSERT_TRUE(testMove({1, 2, 3, 4, 5, 6}, 5, 0, {6, 1, 2, 3, 4, 5}));
  ASSERT_TRUE(testMove({1, 2, 3, 4, 5, 6}, 5, 1, {1, 6, 2, 3, 4, 5}));
  ASSERT_TRUE(testMove({1, 2, 3, 4, 5, 6}, 5, 2, {1, 2, 6, 3, 4, 5}));
  ASSERT_TRUE(testMove({1, 2, 3, 4, 5, 6}, 5, 3, {1, 2, 3, 6, 4, 5}));
  ASSERT_TRUE(testMove({1, 2, 3, 4, 5, 6}, 5, 4, {1, 2, 3, 4, 6, 5}));
  ASSERT_TRUE(testMove({1, 2, 3, 4, 5, 6}, 5, 5, {1, 2, 3, 4, 5, 6}));
}

// TEST(FSPTaillardAcelleration, Results) {
//   auto res = {
//       44, 11, 45, 38, 31, 18, 49, 21, 34, 3,  30, 20, 5,  46, 7,  12, 17,
//       9,  42, 37, 41, 0,  14, 27, 24, 43, 28, 40, 47, 35, 48, 29, 2,  16,
//       23, 8,  6,  4,  1,  26, 19, 25, 13, 15, 39, 36, 32, 33, 22, 10
//       // 44, 11, 8, 12, 7, 25, 35, 19, 31, 13, 3, 24, 41, 38, 17, 23, 40, 48,
//       // 49, 34, 43, 37, 18, 15, 47, 2, 9, 21, 0, 46, 42, 4, 27, 45, 29, 1, 6,
//       // 5, 26, 14, 28, 20, 39, 30, 32, 36, 16, 33, 22, 10 11, 49, 25, 31, 38,
//       // 45, 36, 7, 18, 14, 8, 3, 19, 34, 35, 12, 21, 26, 20, 1, 17, 43, 46, 27,
//       // 28, 41, 13, 44, 2, 29, 24, 9, 5, 42, 32, 30, 0, 47, 40, 37, 23, 16, 15,
//       // 39, 48, 6, 4, 33, 22, 10};
//   };
//   auto res2 = {
//       11, 44, 45, 38, 31, 18, 49, 21, 34, 3,  30, 20, 5,  46, 7,  12, 17,
//       9,  42, 37, 41, 0,  14, 27, 24, 43, 28, 40, 47, 35, 48, 29, 2,  16,
//       23, 8,  6,  4,  1,  26, 19, 25, 13, 15, 39, 36, 32, 33, 22, 10
//       // 44, 11, 8, 12, 7, 25, 35, 19, 31, 13, 3, 24, 41, 38, 17, 23, 40, 48,
//       // 49, 34, 43, 37, 18, 15, 47, 2, 9, 21, 0, 46, 42, 4, 27, 45, 29, 1, 6,
//       // 5, 26, 14, 28, 20, 39, 30, 32, 36, 16, 33, 22, 10 11, 49, 25, 31, 38,
//       // 45, 36, 7, 18, 14, 8, 3, 19, 34, 35, 12, 21, 26, 20, 1, 17, 43, 46, 27,
//       // 28, 41, 13, 44, 2, 29, 24, 9, 5, 42, 32, 30, 0, 47, 40, 37, 23, 16, 15,
//       // 39, 48, 6, 4, 33, 22, 10};
//   };
//   FSP sol(50);
//   sol.assign(res);
//   FSP sol2(50);
//   sol.assign(res2);
//   FSPProblemFactory::init(DATA_FOLDER);
//   std::unordered_map<std::string, std::string> prob;
//   prob["problem"] = "FSP";
//   prob["type"] = "PERM";
//   prob["objective"] = "MAKESPAN";
//   prob["budget"] = "med";
//   prob["instance"] = "taill-like_rand_50_5_01.dat";
//   prob["stopping_criterion"] = "FIXEDTIME";
//   // 2720
//   FSPProblem problem = FSPProblemFactory::get(prob);
//   problem.eval()(sol);

//   FSPNeighbor neighbor(1, 1, 50);
//   problem.neighborEval()(sol, neighbor);

//   ASSERT_EQ(sol.fitness(), neighbor.fitness());
//   ASSERT_EQ(sol.fitness(), 2737);
// }

TEST(FSPTaillardAcelleration, NeighborhoodEval) {
  rng.reseed(65465l);
  const int no_jobs = 50;
  const int no_machines = 10;
  FSPData fspData(no_jobs, no_machines, 100);

  FSP sol(no_jobs);
  PermFSPMakespanEval fullEval(fspData);

  eoInitPermutation<FSP> randomInit(no_jobs);
  randomInit(sol);

  PermFSPNeighborMakespanEval ne(fspData);
  moFullEvalByCopy<FSPNeighbor> fullNe(fullEval);

  for (int i = 0; i < (no_jobs - 1) * (no_jobs - 1); i++) {
    FSPNeighbor neighbor;
    neighbor.index(i);
    ne(sol, neighbor);

    FSPNeighbor neighborFullEval;
    neighborFullEval.index(i);
    fullNe(sol, neighborFullEval);

    ASSERT_EQ(neighbor.fitness(), neighborFullEval.fitness());
  }
}

#include "flowshop-solver/heuristics/BestInsertionExplorer.hpp"
#include "flowshop-solver/heuristics/neighborhood_checkpoint.hpp"
#include "flowshop-solver/heuristics/perturb/DestructionConstruction.hpp"
#include "flowshop-solver/heuristics/perturb/perturb.hpp"

TEST(TaillardAcceleration, BestInsertionNeighborhood) {
  rng.reseed(65465l);
  const int no_jobs = 50;
  const int no_machines = 30;
  FSPData fspData(no_jobs, no_machines, 100);

  FSP sol(no_jobs);
  PermFSPMakespanEval fullEval(fspData);

  eoInitPermutation<FSP> randomInit(no_jobs);
  randomInit(sol);
  FSP sol2 = sol;

  PermFSPNeighborMakespanEval ne(fspData);
  moFullEvalByCopy<FSPNeighbor> fullNe(fullEval);

  moTrueContinuator<FSPNeighbor> tc;
  NeigborhoodCheckpoint<FSPNeighbor> neighborhoodCheckpoint{tc};
  moNeighborComparator<FSPNeighbor> compNN;
  moSolNeighborComparator<FSPNeighbor> compSN;
  BestInsertionExplorer<FSP> igexplorer(ne, neighborhoodCheckpoint, compNN,
                                        compSN);

  rng.reseed(65465l);
  fullEval(sol);
  igexplorer.initParam(sol);
  igexplorer(sol);

  BestInsertionExplorer<FSP> igexplorerFull(fullNe, neighborhoodCheckpoint,
                                            compNN, compSN);

  rng.reseed(65465l);
  fullEval(sol2);
  igexplorerFull.initParam(sol2);
  igexplorerFull(sol2);

  ASSERT_EQ(sol.fitness(), sol2.fitness());
  ASSERT_TRUE(std::equal(sol.begin(), sol.end(), sol2.begin()));
}

#include "paradiseo/mo/algo/moFirstImprHC.h"

TEST(TaillardAcceleration, ReCompileEval) {
  rng.reseed(65465l);
  const int no_jobs = 100;
  const int no_machines = 30;
  FSPData fspData(no_jobs, no_machines, 100);

  FSP sol(no_jobs);
  PermFSPMakespanEval fullEval(fspData);

  eoInitPermutation<FSP> randomInit(no_jobs);
  randomInit(sol);

  fullEval(sol);

  PermFSPNeighborMakespanEval ne(fspData);

  FSPNeighbor neighbor;
  // 0 neighbor is compiled
  neighbor.set(0, sol.size() - 1, sol.size());
  ne(sol, neighbor);

  for (int newPos = sol.size() - 1; newPos >= 0; newPos--) {
    neighbor.set(sol.size() / 2, newPos, sol.size());
    // ne(sol, neighbor);

    // solution is moved
    neighbor.move(sol);

    FSP sol2 = sol;
    fullEval(sol2);

    // evaluate 0 without recompiling the whole solution
    neighbor.set(0, 0, sol.size());
    ne(sol, neighbor);

    ASSERT_EQ(neighbor.fitness(), sol2.fitness());
  }
}

TEST(TaillardAcceleration, DestructionConstruction) {
  rng.reseed(65465l);
  const int no_jobs = 50;
  const int no_machines = 10;
  auto ds = FixedDestructionSize(3);

  FSPData fspData(no_jobs, no_machines, 100);

  FSP sol(no_jobs);
  PermFSPMakespanEval fullEval(fspData);

  eoInitPermutation<FSP> randomInit(no_jobs);
  randomInit(sol);
  fullEval(sol);
  FSP sol2 = sol;

  PermFSPNeighborMakespanEval ne(fspData);
  moFullEvalByCopy<FSPNeighbor> fullNe(fullEval);

  InsertFirstBest<FSPNeighbor> fbf(fullNe);
  DestructionConstruction<FSPNeighbor> opdc(fbf, ds);

  rng.reseed(65465l);
  opdc(sol);

  InsertFirstBest<FSPNeighbor> fb(ne);
  DestructionConstruction<FSPNeighbor> dc(fb, ds);
  rng.reseed(65465l);
  dc(sol2);

  ASSERT_EQ(sol, sol2);
}

TEST(TaillardAcceleration, RecompileNeighbor) {
  rng.reseed(65465l);
  FSPProblemFactory::init(DATA_FOLDER);
  std::unordered_map<std::string, std::string> prob_dt;
  prob_dt["problem"] = "FSP";
  prob_dt["type"] = "PERM";
  prob_dt["objective"] = "MAKESPAN";
  prob_dt["budget"] = "med";
  prob_dt["instance"] = "taillard_rand_20_20_02.dat";
  prob_dt["stopping_criterion"] = "FIXEDTIME";
  FSPProblem prob = FSPProblemFactory::get(prob_dt);

  eoEvalFunc<FSP>& fullEval = prob.eval();
  moEval<FSPNeighbor>& ne = prob.neighborEval();

  FSP sol1;
  sol1.assign(
      {11, 3, 6, 19, 5, 0, 18, 13, 14, 8, 12, 9, 15, 10, 7, 16, 2, 4, 17, 1});
  FSP sol2;
  sol2.assign(
      {11, 3, 6, 19, 0, 18, 14, 8, 12, 13, 9, 15, 10, 7, 5, 16, 2, 4, 17, 1});

  FSPNeighbor ng;
  ng.set(7, 7, sol1.size());

  ng.set(7, 7, sol1.size());
  ne(sol2, ng);

  fullEval(sol2);

  ASSERT_EQ(ng.fitness(), sol2.fitness());
}

// TEST(TaillardAcceleration, NEH) {
//   std::vector<int> vec;
//   vec.assign({
//       16, 4, 4, 14, 12,  //
//       14, 3, 4, 14, 10,  //
//       18, 5, 5, 15, 13,  //
//       4,  2, 2, 12, 3,   //
//       4,  2, 2, 12, 2,   //
//       3,  1, 2, 12, 1,   //
//       2,  2, 2, 11, 2,   //
//       5,  3, 4, 12, 4,   //
//       6,  4, 3, 12, 3,   //
//       7,  2, 4, 14, 4    //
//   });
//   FSPData fspData(vec, 10, false);
// }

#include "flowshop-solver/heuristics/FSPOrderHeuristics.hpp"

TEST(Heuristic, FSPOrderHeuristics) {
  std::vector<std::string> names = {"sum_pij",    "abs_dif",     "ss_sra",
                                    "ss_srs",     "ss_srn_rcn",  "ss_sra_rcn",
                                    "ss_srs_rcn", "ss_sra_2rcn", "ra_c1",
                                    "ra_c2",      "ra_c3",       "ra_c3"};
  std::vector<std::string> orders = {"incr",    "decr",    "valley",
                                     "hill",    "hi_hilo", "hi_lohi",
                                     "lo_hilo", "lo_lohi"};
  FSPData dt(20, 5);
  std::vector<int> ref(20);
  std::iota(begin(ref), end(ref), 0);
  for (const auto& name : names) {
    for (const auto& order : orders) {
      FSP sol = ref;
      auto init = buildPriority(dt, name, false, order);
      (*init)(sol);
      ASSERT_TRUE(std::is_permutation(begin(sol), end(sol), begin(ref)));
      auto initw = buildPriority(dt, name, true, order);
      (*initw)(sol);
      ASSERT_TRUE(std::is_permutation(begin(sol), end(sol), begin(ref)));
    }
  }
}

TEST(PermFSP, NeighborEvaluationSamples) {
  const int no_jobs = 10;
  const int no_machines = 10;
  for (int i = 0; i < 100; i++) {
    FSPData dt{no_jobs, no_machines};
    PermFSPMakespanEval fullEval{dt};
    PermFSPNeighborMakespanEval neighborEval{dt};
    FSP sol(no_jobs);
    FSP solMoved(no_jobs);
    eoInitPermutation<FSP> init(no_jobs);
    init(sol);
    for (int j = 0; j < no_jobs; j++) {
      for (int k = 0; k < no_jobs; k++) {
        if (k != j && k != j - 1) {
          FSPNeighbor ngh(j, k, sol.size());
          neighborEval(sol, ngh);
          solMoved = sol;
          ngh.move(solMoved);
          fullEval(solMoved);
          ASSERT_EQ(solMoved.fitness(), ngh.fitness());
        }
      }
    }
  }
}

TEST(NoWaitFSP, NeighborEvaluationExample) {
  std::vector<int> pts = { //
    20, 25, 25, 10, 17, //
    30, 20, 25, 25, 28, //
  };
  FSPData dt{pts, 5, true};
  NoWaitFSPMakespanEval nwFspEval{dt};
  NoWaitFSPNeighborMakespanEval fastEval{dt, nwFspEval};

  FSP sol;
  sol.assign({0, 1, 2, 3, 4});

  FSPNeighbor ngh1(2, 0, sol.size());
  fastEval(sol, ngh1);
  ASSERT_EQ(153, ngh1.fitness());

  FSPNeighbor ngh2(2, 4, sol.size());
  fastEval(sol, ngh2);
  ASSERT_EQ(148, ngh2.fitness());

  FSPNeighbor ngh3(2, 1, sol.size());
  fastEval(sol, ngh3);
  ASSERT_EQ(148, ngh3.fitness());
}

TEST(NoWaitFSP, NeighborEvaluationSamples) {
  const int no_jobs = 10;
  const int no_machines = 10;
  for (int i = 0; i < 100; i++) {
    FSPData dt{no_jobs, no_machines};
    NoWaitFSPMakespanEval nwFspEval{dt};
    NoWaitFSPNeighborMakespanEval fastEval{dt, nwFspEval};
    FSP sol(no_jobs);
    FSP solMoved(no_jobs);
    eoInitPermutation<FSP> init(no_jobs);
    init(sol);
    for (int j = 0; j < no_jobs; j++) {
      for (int k = 0; k < no_jobs; k++) {
        if (k != j && k != j - 1) {
          FSPNeighbor ngh(j, k, sol.size());
          fastEval(sol, ngh);
          solMoved = sol;
          ngh.move(solMoved);
          nwFspEval(solMoved);
          ASSERT_EQ(solMoved.fitness(), ngh.fitness());
        }
      }
    }
  }
}

TEST(NoIdleFSP, FastNeighborhoodExample) {
  std::vector<int> pts = { //
    3, 3, 2, //
    4, 1, 3, //
    2, 3, 3, //
    2, 2, 3  //
  };
  FSPData dt{pts, 4, false};
  NoIdleFSPNeighborMakespanEval fastEval{dt};

  FSP sol;
  sol.assign({0, 1, 2, 3});

  FSPNeighbor ngh1(0, 1, sol.size());
  fastEval(sol, ngh1);
  ASSERT_EQ(19, ngh1.fitness());

  FSPNeighbor ngh2(0, 2, sol.size());
  fastEval(sol, ngh2);
  ASSERT_EQ(17, ngh2.fitness());

  FSPNeighbor ngh3(0, 3, sol.size());
  fastEval(sol, ngh3);
  ASSERT_EQ(17, ngh3.fitness());
}

TEST(NoIdleFSP, FastCmaxNeighborhoodRandom) {
  const int no_jobs = 10;
  const int no_machines = 10;
  for (int i = 0; i < 1000; i++) {
    FSPData dt{no_jobs, no_machines};
    NoIdleFSPMakespanEval fullEval{dt};
    NoIdleFSPNeighborMakespanEval neighborEval{dt};
    eoInitPermutation<FSP> init(no_jobs);
    FSP sol(no_jobs);
    init(sol);
    for (int j = 0; j < no_jobs; j++) {
      for (int k = 0; k < no_jobs; k++) {
        if (k != j && k != j - 1) {
          FSP solMoved(no_jobs);
          FSPNeighbor ngh(j, k, sol.size());
          neighborEval(sol, ngh);
          solMoved = sol;
          ngh.move(solMoved);
          fullEval(solMoved);
          ASSERT_EQ(solMoved.fitness(), ngh.fitness());
        }
      }
    }
  }
}

TEST(NoIdleFSP, FastTFTNeighborhoodRandom) {
  const int no_jobs = 10;
  const int no_machines = 10;
  for (int i = 0; i < 1000; i++) {
    FSPData dt{no_jobs, no_machines};
    NoIdleFSPFlowtimeEval fullEval{dt};
    NoIdleFSPNeighborFlowtimeEval neighborEval{dt};
    FSP sol(no_jobs);
    eoInitPermutation<FSP> init(no_jobs);
    init(sol);
    for (int j = 0; j < no_jobs; j++) {
      for (int k = 0; k < no_jobs; k++) {
        if (k != j && k != j - 1) {
          FSP solMoved(no_jobs);
          FSPNeighbor ngh(j, k, sol.size());
          neighborEval(sol, ngh);
          solMoved = sol;
          ngh.move(solMoved);
          solMoved.invalidate();
          fullEval(solMoved);
          ASSERT_EQ(solMoved.fitness(), ngh.fitness());
        }
      }
    }
  }
}

// TEST(AllFSP, ScheduleInfo) {
//   std::vector<int> pts = { //
//     15, 15, 10,  5, //
//     10,  5, 15,  5, //
//      5, 10, 10, 10, //
//   };
//   FSPData dt{pts, 4, true};

//   FSP sol(4);
//   sol.assign({0, 1, 2, 3});

//   PermFSPEval<FSP> permCmaxEval(dt, Objective::MAKESPAN);
//   PermFSPEval<FSP> permFTEval(dt, Objective::FLOWTIME);

//   NoWaitFSPEval<FSP> nwCmaxEval(dt, Objective::MAKESPAN);
//   NoWaitFSPEval<FSP> nwFTEval(dt, Objective::FLOWTIME);

//   NoIdleFSPEval<FSP> niCmaxEval(dt, Objective::MAKESPAN);
//   NoIdleFSPEval<FSP> niFTEval(dt, Objective::FLOWTIME);

//   permCmaxEval(sol);
//   std::cerr << "permCmaxEval " << sol.fitness() << '\n';
//   permFTEval(sol);
//   std::cerr << "permFTEval " << sol.fitness() << '\n';

//   nwCmaxEval(sol);
//   std::cerr << "nwCmaxEval " << sol.fitness() << '\n';
//   nwFTEval(sol);
//   std::cerr << "nwFTEval " << sol.fitness() << '\n';

//   niCmaxEval(sol);
//   std::cerr << "niCmaxEval " << sol.fitness() << '\n';
//   niFTEval(sol);
//   std::cerr << "niFTEval " << sol.fitness() << '\n';
// }


auto main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) -> int {
  testing::InitGoogleTest(&argc, argv);
  // int c = 2;
  // char** v = new char*[2];
  // char s[] = "--gtest_filter=\"NWFSP.*\"";
  // v[1] = new char[200];
  // std::strcpy(v[1], s);
  // testing::InitGoogleTest(&c, v);

  return RUN_ALL_TESTS();
}