#include <gtest/gtest.h>

#include "flowshop-solver/problems/FSPData.hpp"
#include "flowshop-solver/problems/NoWaitFSPEval.hpp"
#include "flowshop-solver/problems/NoWaitFSPNeighborMakespanEval.hpp"

TEST(NoWaitFSP, NeighborEvaluationExample) {
  std::vector<int> pts = {
      //
      20, 25, 25, 10, 17,  //
      30, 20, 25, 25, 28,  //
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
    sol.resize(1 + rand() % (no_jobs - 1));
    for (int j = 0; j < sol.size(); j++) {
      for (int k = 0; k < sol.size(); k++) {
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