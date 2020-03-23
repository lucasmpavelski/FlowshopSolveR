#include <cassert>
#include <string>
#include <iostream>
#include <algorithm>

#include <gtest/gtest.h>

#include "problems/FSPEvalFunc.hpp"
#include "problems/FSPData.hpp"
#include "problems/NIFSPEvalFunc.hpp"
#include "problems/NWFSPEvalFunc.hpp"

#include "problems/fastfspeval.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

std::string instances_folder = TEST_FIXTURES_FOLDER;

void testLoadData(void)
{
  using std::cout;
  using std::endl;
  FSPData fsp_data{instances_folder + "test.txt"};
  assert(fsp_data.maxCT() == 125);
  int pt[] = {
      5, 9, 9, 4,
      9, 3, 4, 8,
      8, 10, 5, 8,
      10, 1, 8, 7,
      1, 8, 6, 2};
  const auto &pt_ref = fsp_data.procTimesRef();
  assert(std::equal(std::begin(pt_ref), std::end(pt_ref), pt));
  assert(fsp_data.machineProcTimesRef()[0] == 27);
  assert(fsp_data.jobProcTimesRef()[0] == 33);
}

void testEvalMin()
{
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

void testEvalMax()
{
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

void testEvalFlowtime()
{
  using std::cout;
  using std::endl;
  PermFSPEvalFunc<FSPMin> fsp_eval{FSPData{instances_folder + "test.txt"}, Objective::FLOWTIME};
  FSPMin sol(4);
  sol[0] = 4 - 1;
  sol[1] = 3 - 1;
  sol[2] = 1 - 1;
  sol[3] = 2 - 1;
  fsp_eval(sol);
  assert(sol.fitness() == (29 + 41 + 46 + 54));
}

void testPartialEval()
{
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

void testNIEval()
{
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

void testNWEval()
{
  using std::cout;
  using std::endl;
  FSPData dt(instances_folder + "test.txt");
  NWFSPEvalFunc<FSPMin> fsp_eval(dt, Objective::MAKESPAN);
  //nwfspEval<FSP> fsp_eval(instances_folder + "test.txt");
  FSPMin sol(4);
  FSPMin solp(2);
  sol[0] = solp[0] = 4 - 1;
  sol[1] = solp[1] = 3 - 1;
  sol[2] = 1 - 1;
  sol[3] = 2 - 1;
  fsp_eval(sol);
  fsp_eval(solp);
  //cout << sol.fitness() << endl;
  //cout << solp.fitness() << endl;
  assert(sol.fitness() == 59);
  assert(solp.fitness() == 41);
  NWFSPEvalFunc<FSPMin> fsp_evalft(dt, Objective::FLOWTIME);
  //nwfspEval<FSP> fsp_evalft(instances_folder + "test.txt", 1);
  FSPMin solft = sol;
  FSPMin solftp = solp;
  fsp_evalft(solft);
  fsp_evalft(solftp);
  //cout << solft.fitness() << endl;
  //cout << solftp.fitness() << endl;
  assert(solft.fitness() == 180);
  assert(solftp.fitness() == 70);
}

TEST(FSPTaillardAcelleration, Evaluation)
{
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

TEST(FSPTaillardAcelleration, IndexConvert)
{
  int n = 200;
  for (int i = 0; i < (n - 1) * (n - 1); i++)
  {
    auto kp = keyToPositionPair(i, n);
    auto pos = positionPairToKey(kp.first, kp.second, n);
    ASSERT_TRUE(i == pos);
  }
}

TEST(FSPTaillardAcelleration, NeighborhoodEval)
{
  rng.reseed(65465l);
  const int no_jobs = 50;
  const int no_machines = 10;
  FSPData fspData(no_jobs, no_machines, 100);

  FSP sol(no_jobs);
  PermFSPEvalFunc<FSP> fullEval(fspData, Objective::MAKESPAN);

  eoInitPermutation<FSP> randomInit(no_jobs);
  randomInit(sol);

  movedSolutionStat movedSolutionStat;
  movedSolutionStat.init(sol);

  FastFSPNeighborEval ne(fspData, movedSolutionStat);
  moFullEvalByCopy<FSPNeighbor> fullNe(fullEval);

  for (int i = 0; i < (no_jobs - 1) * (no_jobs - 1); i++)
  {
    moShiftNeighbor<FSP> neighbor;
    neighbor.index(i);
    ne(sol, neighbor);

    moShiftNeighbor<FSP> neighborFullEval;
    neighborFullEval.index(i);
    fullNe(sol, neighborFullEval);

    ASSERT_EQ(neighbor.fitness(), neighborFullEval.fitness());
  }
}

int main(int argc, char **argv)
{
  argc = 2;
  // char* argvv[] = {"", "--gtest_filter=FLA.*"};
  // testing::InitGoogleTest(&argc, argvv);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}