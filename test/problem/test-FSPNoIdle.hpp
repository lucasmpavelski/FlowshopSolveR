#include <gtest/gtest.h>

#include "flowshop-solver/problems/FSPData.hpp"
#include "flowshop-solver/problems/NoIdleFSPEval.hpp"
#include "flowshop-solver/problems/NoIdleFSPNeighborEval.hpp"


TEST(NoIdleFSP, FastNeighborhoodExample) {
  std::vector<int> pts = {
      //
      3, 3, 2,  //
      4, 1, 3,  //
      2, 3, 3,  //
      2, 2, 3   //
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
  for (int i = 0; i < 100; i++) {
    FSPData dt{no_jobs, no_machines};
    NoIdleFSPMakespanEval fullEval{dt};
    NoIdleFSPNeighborMakespanEval neighborEval{dt};
    eoInitPermutation<FSP> init(no_jobs);
    FSP sol(no_jobs);
    init(sol);
    sol.resize(1 + rand() % (no_jobs - 1));
    for (int j = 0; j < sol.size(); j++) {
      for (int k = 0; k < sol.size(); k++) {
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
  for (int i = 0; i < 100; i++) {
    FSPData dt{no_jobs, no_machines};
    NoIdleFSPFlowtimeEval fullEval{dt};
    NoIdleFSPNeighborFlowtimeEval neighborEval{dt};
    FSP sol(no_jobs);
    eoInitPermutation<FSP> init(no_jobs);
    init(sol);
    sol.resize(1 + rand() % (no_jobs - 1));
    for (int j = 0; j < sol.size(); j++) {
      for (int k = 0; k < sol.size(); k++) {
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