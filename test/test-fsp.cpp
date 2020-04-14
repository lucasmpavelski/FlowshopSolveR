#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include "problems/FSPData.hpp"
#include "problems/FSPEvalFunc.hpp"
#include "problems/NIFSPEvalFunc.hpp"
#include "problems/NWFSPEvalFunc.hpp"

#include "problems/fastfspeval.hpp"

#include "flowshop-solver/heuristics/InsertionStrategy.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

std::string instances_folder = TEST_FIXTURES_FOLDER;

void testLoadData() {
  using std::cout;
  using std::endl;
  FSPData fsp_data{instances_folder + "test.txt"};
  assert(fsp_data.maxCT() == 125);
  std::array<int, 20> pt = {5, 9, 9,  4, 9, 3, 4, 8, 8, 10,
                            5, 8, 10, 1, 8, 7, 1, 8, 6, 2};
  const auto& pt_ref = fsp_data.procTimesRef();
  assert(std::equal(std::begin(pt_ref), std::end(pt_ref), std::begin(pt)));
  assert(fsp_data.machineProcTimesRef()[0] == 27);
  assert(fsp_data.jobProcTimesRef()[0] == 33);
}

void testEvalMin() {
  using std::cout;
  using std::endl;
  PermFSPEvalFunc<FSPMin> fsp_eval{FSPData{instances_folder + "test.txt"}};
  FSPMin sol(4);
  sol[0] = 4 - 1;
  sol[1] = 3 - 1;
  sol[2] = 1 - 1;
  sol[3] = 2 - 1;
  fsp_eval(sol);
  assert(sol.fitness() == 54);
}

void testEvalMax() {
  using std::cout;
  using std::endl;
  PermFSPEvalFunc<FSP> fsp_eval{FSPData{instances_folder + "test.txt"}};
  FSP sol(4);
  sol[0] = 4 - 1;
  sol[1] = 3 - 1;
  sol[2] = 1 - 1;
  sol[3] = 2 - 1;
  fsp_eval(sol);
  assert(sol.fitness() == 125 - 54);
}

void testEvalFlowtime() {
  using std::cout;
  using std::endl;
  PermFSPEvalFunc<FSPMin> fsp_eval{FSPData{instances_folder + "test.txt"},
                                   Objective::FLOWTIME};
  FSPMin sol(4);
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
  PermFSPEvalFunc<FSPMin> fsp_eval{FSPData{instances_folder + "test.txt"}};
  FSPMin sol(2);
  sol[0] = 4 - 1;
  sol[1] = 3 - 1;
  fsp_eval(sol);
  assert(sol.fitness() == 41);
}

void testNIEval() {
  using std::cout;
  using std::endl;
  FSPData dt(instances_folder + "test.txt");
  NIFSPEvalFunc<FSPMin> fsp_eval(dt, Objective::MAKESPAN);
  FSPMin sol(4);
  FSPMin solp(2);
  sol[0] = solp[0] = 4 - 1;
  sol[1] = solp[1] = 3 - 1;
  sol[2] = 1 - 1;
  sol[3] = 2 - 1;
  fsp_eval(sol);
  fsp_eval(solp);
  assert(sol.fitness() == 56);
  assert(solp.fitness() == 42);
  NIFSPEvalFunc<FSPMin> fsp_evalft(dt, Objective::FLOWTIME);
  FSPMin solft = sol;
  FSPMin solftp = solp;
  fsp_evalft(solft);
  fsp_evalft(solftp);
  assert(solft.fitness() == 192);
  assert(solftp.fitness() == 78);
}

void testNWEval() {
  using std::cout;
  using std::endl;
  FSPData dt(instances_folder + "test.txt");
  NWFSPEvalFunc<FSPMin> fsp_eval(dt, Objective::MAKESPAN);
  // nwfspEval<FSP> fsp_eval(instances_folder + "test.txt");
  FSPMin sol(4);
  FSPMin solp(2);
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
  NWFSPEvalFunc<FSPMin> fsp_evalft(dt, Objective::FLOWTIME);
  // nwfspEval<FSP> fsp_evalft(instances_folder + "test.txt", 1);
  FSPMin solft = sol;
  FSPMin solftp = solp;
  fsp_evalft(solft);
  fsp_evalft(solftp);
  // cout << solft.fitness() << endl;
  // cout << solftp.fitness() << endl;
  assert(solft.fitness() == 180);
  assert(solftp.fitness() == 70);
}

TEST(FSPTaillardAcelleration, Evaluation) {
  rng.reseed(65465l);
  const int no_jobs = 4;
  const int no_machines = 3;
  FSPData fspData(no_jobs, no_machines, 8);

  ivec seq = {0, 1, 2, 3};
  CompiledSchedule csd(no_jobs, no_machines);
  csd.compile(fspData, seq);

  PermFSPEvalFunc<FSPMin> fsp_eval{fspData};
  FSPMin sol(4);
  sol[0] = 3;
  sol[1] = 0;
  sol[2] = 1;
  sol[3] = 2;
  fsp_eval(sol);
  ASSERT_EQ(sol.fitness(), csd.makespan[0]);
  sol[0] = 0;
  sol[1] = 3;
  sol[2] = 1;
  sol[3] = 2;
  fsp_eval(sol);
  ASSERT_EQ(sol.fitness(), csd.makespan[1]);
  sol[0] = 0;
  sol[1] = 1;
  sol[2] = 3;
  sol[3] = 2;
  fsp_eval(sol);
  ASSERT_EQ(sol.fitness(), csd.makespan[2]);
  sol[0] = 0;
  sol[1] = 1;
  sol[2] = 2;
  sol[3] = 3;
  fsp_eval(sol);
  ASSERT_EQ(sol.fitness(), csd.makespan[3]);
}

auto auxMoveNeighborCompare(std::initializer_list<int> init,
                            int from,
                            int to,
                            std::initializer_list<int> result) -> bool {
  FSP sol;
  sol.assign(init);
  FSPNeighbor ng(from, to, sol.size());
  ng.move(sol);
  return std::equal(sol.begin(), sol.end(), result.begin());
}

TEST(FSPNeighbor, MoveOperatorEqual) {
  ASSERT_TRUE(
      auxMoveNeighborCompare({1, 2, 3, 4, 5, 6}, 0, 0, {1, 2, 3, 4, 5, 6}));
}

TEST(FSPNeighbor, MoveAhead) {
  ASSERT_TRUE(
      auxMoveNeighborCompare({1, 2, 3, 4, 5, 6}, 1, 5, {1, 3, 4, 5, 2, 6}));
}

TEST(FSPNeighbor, MoveBack) {
  ASSERT_TRUE(
      auxMoveNeighborCompare({1, 2, 3, 4, 5, 6}, 4, 1, {1, 5, 2, 3, 4, 6}));
}

TEST(FSPNeighbor, MoveBegin) {
  ASSERT_TRUE(
      auxMoveNeighborCompare({1, 2, 3, 4, 5, 6}, 4, 0, {5, 1, 2, 3, 4, 6}));
}

TEST(FSPNeighbor, MoveEnd) {
  ASSERT_TRUE(
      auxMoveNeighborCompare({1, 2, 3, 4, 5, 6}, 1, 6, {1, 3, 4, 5, 6, 2}));
}

TEST(FSPTaillardAcelleration, IndexConvert) {
  int n = 200;
  for (int i = 0; i < (n - 1) * (n - 1); i++) {
    auto kp = keyToPositionPair(i, n);
    auto pos = positionPairToKey(kp.first, kp.second, n);
    ASSERT_TRUE(i == pos);
  }
}

#include "flowshop-solver/fspproblemfactory.hpp"

TEST(FSPTaillardAcelleration, Results) {
  auto res = {
      44, 11, 45, 38, 31, 18, 49, 21, 34, 3,  30, 20, 5,  46, 7,  12, 17,
      9,  42, 37, 41, 0,  14, 27, 24, 43, 28, 40, 47, 35, 48, 29, 2,  16,
      23, 8,  6,  4,  1,  26, 19, 25, 13, 15, 39, 36, 32, 33, 22, 10
      // 44, 11, 8, 12, 7, 25, 35, 19, 31, 13, 3, 24, 41, 38, 17, 23, 40, 48,
      // 49, 34, 43, 37, 18, 15, 47, 2, 9, 21, 0, 46, 42, 4, 27, 45, 29, 1, 6,
      // 5, 26, 14, 28, 20, 39, 30, 32, 36, 16, 33, 22, 10 11, 49, 25, 31, 38,
      // 45, 36, 7, 18, 14, 8, 3, 19, 34, 35, 12, 21, 26, 20, 1, 17, 43, 46, 27,
      // 28, 41, 13, 44, 2, 29, 24, 9, 5, 42, 32, 30, 0, 47, 40, 37, 23, 16, 15,
      // 39, 48, 6, 4, 33, 22, 10};
  };
  auto res2 = {
      11, 44, 45, 38, 31, 18, 49, 21, 34, 3,  30, 20, 5,  46, 7,  12, 17,
      9,  42, 37, 41, 0,  14, 27, 24, 43, 28, 40, 47, 35, 48, 29, 2,  16,
      23, 8,  6,  4,  1,  26, 19, 25, 13, 15, 39, 36, 32, 33, 22, 10
      // 44, 11, 8, 12, 7, 25, 35, 19, 31, 13, 3, 24, 41, 38, 17, 23, 40, 48,
      // 49, 34, 43, 37, 18, 15, 47, 2, 9, 21, 0, 46, 42, 4, 27, 45, 29, 1, 6,
      // 5, 26, 14, 28, 20, 39, 30, 32, 36, 16, 33, 22, 10 11, 49, 25, 31, 38,
      // 45, 36, 7, 18, 14, 8, 3, 19, 34, 35, 12, 21, 26, 20, 1, 17, 43, 46, 27,
      // 28, 41, 13, 44, 2, 29, 24, 9, 5, 42, 32, 30, 0, 47, 40, 37, 23, 16, 15,
      // 39, 48, 6, 4, 33, 22, 10};
  };
  FSP sol(50);
  sol.assign(res);
  FSP sol2(50);
  sol.assign(res2);
  FSPProblemFactory::init(DATA_FOLDER);
  std::unordered_map<std::string, std::string> prob;
  prob["problem"] = "FSP";
  prob["type"] = "PERM";
  prob["objective"] = "MAKESPAN";
  prob["budget"] = "med";
  prob["instance"] = "taill-like_rand_50_5_01.dat";
  prob["stopping_criterium"] = "FIXEDTIME";
  // 2720
  FSPProblem problem = FSPProblemFactory::get(prob);
  problem.eval()(sol);

  FSPNeighbor neighbor(1, 1, 50);
  problem.neighborEval()(sol, neighbor);

  ASSERT_EQ(sol.fitness(), neighbor.fitness());
  ASSERT_EQ(sol.fitness(), 2737);
}

TEST(FSPTaillardAcelleration, NeighborhoodEval) {
  rng.reseed(65465l);
  const int no_jobs = 50;
  const int no_machines = 10;
  FSPData fspData(no_jobs, no_machines, 100);

  FSP sol(no_jobs);
  PermFSPEvalFunc<FSP> fullEval(fspData, Objective::MAKESPAN);

  eoInitPermutation<FSP> randomInit(no_jobs);
  randomInit(sol);

  myMovedSolutionStat<FSP> movedSolutionStat;
  movedSolutionStat.init(sol);

  FastFSPNeighborEval ne(fspData, fullEval);
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
#include "flowshop-solver/heuristics/OpPerturbDestConst.hpp"
#include "flowshop-solver/heuristics/fastigexplorer.hpp"
#include "flowshop-solver/heuristics/neighborhood_checkpoint.hpp"

TEST(TaillardAcceleration, BestInsertionNeighborhood) {
  rng.reseed(65465l);
  const int no_jobs = 50;
  const int no_machines = 30;
  FSPData fspData(no_jobs, no_machines, 100);

  FSP sol(no_jobs);
  PermFSPEvalFunc<FSP> fullEval(fspData, Objective::MAKESPAN);

  eoInitPermutation<FSP> randomInit(no_jobs);
  randomInit(sol);
  FSP sol2 = sol;

  myMovedSolutionStat<FSP> movedSolutionStat;
  movedSolutionStat.init(sol);

  FastFSPNeighborEval ne(fspData, fullEval);
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
  PermFSPEvalFunc<FSP> fullEval(fspData, Objective::MAKESPAN);

  eoInitPermutation<FSP> randomInit(no_jobs);
  randomInit(sol);

  fullEval(sol);

  myMovedSolutionStat<FSP> movedSolutionStat;
  movedSolutionStat.init(sol);
  FastFSPNeighborEval ne(fspData, fullEval);

  FSPNeighbor neighbor;
  // 0 neighbor is compiled
  neighbor.set(0, sol.size() - 1, sol.size());
  ne(sol, neighbor);

  for (int newPos = sol.size(); newPos >= 0; newPos--) {
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
  const int ds = 3;

  FSPData fspData(no_jobs, no_machines, 100);

  FSP sol(no_jobs);
  PermFSPEvalFunc<FSP> fullEval(fspData, Objective::MAKESPAN);

  eoInitPermutation<FSP> randomInit(no_jobs);
  randomInit(sol);
  FSP sol2 = sol;

  FastFSPNeighborEval ne(fspData, fullEval);
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
  prob_dt["stopping_criterium"] = "FIXEDTIME";
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

TEST(TaillardAcceleration, NEH) {
  std::vector<int> vec;
  vec.assign({
      16, 4, 4, 14, 12,  //
      14, 3, 4, 14, 10,  //
      18, 5, 5, 15, 13,  //
      4,  2, 2, 12, 3,   //
      4,  2, 2, 12, 2,   //
      3,  1, 2, 12, 1,   //
      2,  2, 2, 11, 2,   //
      5,  3, 4, 12, 4,   //
      6,  4, 3, 12, 3,   //
      7,  2, 4, 14, 4    //
  });
  FSPData fspData(vec, 10, false);
  std::cerr << fspData << '\n';
}

auto main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) -> int {
  testing::InitGoogleTest(&argc, argv);
  // testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}