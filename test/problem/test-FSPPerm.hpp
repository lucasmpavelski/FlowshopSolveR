#include <gtest/gtest.h>

#include "flowshop-solver/problems/FSPData.hpp"
#include "flowshop-solver/problems/PermFSPEval.hpp"
#include "flowshop-solver/problems/PermFSPNeighborMakespanEval.hpp"

TEST(PermFSP, NeighborMakespanEvaluationSamples) {
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
    sol.resize(1 + rand() % (no_jobs - 1));
    for (int j = 0; j < sol.size(); j++) {
      for (int k = 0; k < sol.size(); k++) {
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

TEST(PermFSP, NeighborCachedFlowtimeEvaluationSamples) {
  const int no_jobs = 10;
  const int no_machines = 10;
  for (int i = 0; i < 1000; i++) {
    FSPData dt{no_jobs, no_machines};
    PermFSPFlowtimeEval cacheEval{dt};
    PermFSPNeighborMakespanEval neighborEval{dt};
    FSP sol(no_jobs);
    FSP solMoved(no_jobs);
    eoInitPermutation<FSP> init(no_jobs);
    init(sol);
    sol.resize(1 + rand() % (no_jobs - 1));
    for (int j = 0; j < sol.size(); j++) {
      for (int k = 0; k < sol.size(); k++) {
        if (k != j && k != j - 1) {
          PermFSPFlowtimeEval noCacheEval{dt};
          FSPNeighbor ngh(j, k, sol.size());
          solMoved = sol;
          ngh.move(solMoved);
          noCacheEval(solMoved);
          auto noCacheFitness = solMoved.fitness();
          cacheEval(solMoved);
          auto cacheFitness = solMoved.fitness();
          ASSERT_EQ(noCacheFitness, cacheFitness);
        }
      }
    }
  }
}