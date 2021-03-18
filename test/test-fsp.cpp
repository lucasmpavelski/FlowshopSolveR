#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <numeric>
#include <string>

#include <gtest/gtest.h>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/FSPProblemFactory.hpp"
#include "flowshop-solver/heuristics/InsertionStrategy.hpp"
#include "flowshop-solver/problems/FSPData.hpp"
#include "flowshop-solver/problems/FSPEval.hpp"
#include "flowshop-solver/problems/NoIdleFSPEval.hpp"
#include "flowshop-solver/problems/NoWaitFSPEval.hpp"
#include "flowshop-solver/problems/NoWaitFSPNeighborMakespanEval.hpp"
#include "flowshop-solver/problems/PermFSPEval.hpp"

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

#include "flowshop-solver/heuristics/perturb/RandomDestructionStrategy.hpp"

TEST(TaillardAcceleration, DestructionConstruction) {
  rng.reseed(65465l);
  const int no_jobs = 50;
  const int no_machines = 10;
  auto ds = FixedDestructionSize(3);
  RandomDestructionStrategy<FSP> destruction(ds);

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
  DestructionConstruction<FSPNeighbor> opdc(fbf, destruction);

  rng.reseed(65465l);
  opdc(sol);

  InsertFirstBest<FSPNeighbor> fb(ne);
  DestructionConstruction<FSPNeighbor> dc(fb, destruction);
  rng.reseed(65465l);
  dc(sol2);

  ASSERT_EQ(sol, sol2);
}

TEST(TaillardAcceleration, RecompileNeighbor) {
  rng.reseed(65465l);
  const int no_jobs = 20;
  const int no_machines = 20;
  FSPData dt{no_jobs, no_machines};
  FSPProblem prob(dt, "PERM", "MAKESPAN", "low", "FIXEDTIME");

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

#include "flowshop-solver/heuristics/FSPOrderHeuristics.hpp"



#include "problem/test-FSPNoWait.hpp"
#include "problem/test-FSPNoIdle.hpp"
#include "problem/test-FSPPerm.hpp"
#include "problem/test-FSPOrderHeuristics.hpp"
#include "problem/test-FSPNeighbor.hpp"

#include "heuristic/test-InsertionStrategy.hpp"
#include "heuristic/test-AppendingNEH.hpp"
#include "heuristic/test-NEH.hpp"
#include "heuristic/test-IG.hpp"

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
  FSPProblemFactory::init(DATA_FOLDER);
  MHParamsSpecsFactory::init(DATA_FOLDER "/specs");

  testing::InitGoogleTest(&argc, argv);
  // int c = 2;
  // char** v = new char*[2];
  // char s[] = "--gtest_filter=\"NWFSP.*\"";
  // v[1] = new char[200];
  // std::strcpy(v[1], s);
  // testing::InitGoogleTest(&c, v);

  return RUN_ALL_TESTS();
}