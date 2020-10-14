#include <gtest/gtest.h>

#include "flowshop-solver/problems/FSPData.hpp"
#include "flowshop-solver/problems/PermFSPNeighborMakespanEval.hpp"
#include "flowshop-solver/heuristics/FSPOrderHeuristics.hpp"
#include "flowshop-solver/heuristics/AppendingNEH.hpp"

TEST(NEH, AppendingNEH) {
  const int no_jobs = 20;
  const int no_machines = 5;
  FSPData dt{no_jobs, no_machines};
  PermFSPNeighborMakespanEval neighborEval{dt};
  auto init1 = buildPriority(dt, "lr_it_aj_ct", false, "incr");
  auto init2 = buildPriority(dt, "sum_pij", false, "incr");
  auto insertion = buildInsertionStrategy("first_best", neighborEval);
  AppendingNEH<FSPNeighbor> lrNeh{*init1, *init2, *insertion, 0.5};
  FSP sol;
  lrNeh(sol);

  FSP ref(no_jobs);
  std::iota(begin(ref), end(ref), 0);
  ASSERT_TRUE(std::is_permutation(begin(sol), end(sol), begin(ref)));

  (*init1)(ref);
  ref.resize(0.5 * no_jobs);
  auto prevJob = std::begin(sol);
  for (auto job : ref) {
    auto nextJob = std::find(begin(sol), end(sol), job);
    ASSERT_TRUE(std::distance(prevJob, nextJob) >= 0);
    prevJob = nextJob;
  }
}