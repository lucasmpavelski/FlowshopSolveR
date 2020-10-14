#include <gtest/gtest.h>

#include "flowshop-solver/problems/FSP.hpp"
#include "flowshop-solver/problems/FSPData.hpp"
#include "flowshop-solver/problems/FSPProblem.hpp"
#include "flowshop-solver/heuristics/FSPOrderHeuristics.hpp"
#include "flowshop-solver/heuristics/NEH.hpp"

TEST(NEH, KK2Example) {
  std::vector<int> pts = {
      19, 44, 85, 59, 87, 51,  //
      46, 63, 56, 68, 66, 4,   //
      65, 12, 98, 25, 53, 63   //
  };
  FSPData dt{pts, 6, true};

  std::vector<int> ref(dt.noJobs());
  std::iota(begin(ref), end(ref), 0);

  FSP sol = ref;
  std::iota(begin(sol), end(sol), 0);

  FSPProblem prob(dt, "PERM", "MAKESPAN", "low", "TIME");

  auto init = buildPriority(dt, "kk1", false, "decr");
  auto insert = buildInsertionStrategyFSP("kk2", prob.neighborEval(), dt);
  NEH<FSPNeighbor> neh(*init, *insert);

  neh(sol);

  ref.assign({0, 2, 4, 1, 5, 3});
  ASSERT_TRUE(std::equal(begin(sol), end(sol), begin(ref), end(ref)));
  ASSERT_EQ(sol.fitness(), 438);
}

TEST(Heuristic, NEHKK2l) {
  std::vector<int> pts = {
      19, 44, 85, 59, 87, 51,  //
      46, 63, 56, 68, 66, 4,   //
      65, 12, 98, 25, 53, 63   //
  };
  FSPData dt{pts, 6, true};

  std::vector<int> ref(dt.noJobs());
  std::iota(begin(ref), end(ref), 0);

  FSP sol = ref;
  std::iota(begin(sol), end(sol), 0);

  FSPProblem prob(dt, "PERM", "MAKESPAN", "low", "TIME");

  auto init = buildPriority(dt, "kk2", false, "decr");
  auto insert = buildInsertionStrategyFSP("kk2", prob.neighborEval(), dt);
  NEH<FSPNeighbor> neh(*init, *insert);

  neh(sol);

  ref.assign({0, 2, 4, 1, 5, 3});
  // ASSERT_TRUE(std::equal(begin(sol), end(sol), begin(ref), end(ref)));
  // ASSERT_EQ(sol.fitness(), 438);
}