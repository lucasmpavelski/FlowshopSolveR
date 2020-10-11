#include <gtest/gtest.h>
#include <iostream>

#include "flowshop-solver/heuristics/AppendingNEH.hpp"
#include "flowshop-solver/heuristics/FSPOrderHeuristics.hpp"
#include "flowshop-solver/problems/FSPData.hpp"
#include "flowshop-solver/problems/PermFSPNeighborMakespanEval.hpp"

TEST(Heuristic, FSPOrderHeuristics) {
  std::vector<std::string> names = {
      "sum_pij",    "abs_dif",    "ss_sra",      "ss_srs",   "ss_srn_rcn",
      "ss_sra_rcn", "ss_srs_rcn", "ss_sra_2rcn", "ra_c1",    "ra_c2",
      "ra_c3",      "ra_c3",      "lr_it_aj_ct", "lr_it_ct", "lr_it",
      "lr_aj",      "lr_ct",      "kk1"};
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

TEST(Heuristic, LROrderExample) {
  std::vector<int> pts = {
      15, 15, 10, 5,   //
      10, 5,  15, 5,   //
      5,  10, 10, 10,  //
  };
  FSPData dt{pts, 4, true};

  std::vector<int> ref(dt.noJobs());
  std::iota(begin(ref), end(ref), 0);

  FSP sol = ref;
  std::iota(begin(sol), end(sol), 0);
  auto init = buildPriority(dt, "lr_it_aj_ct", false, "incr");
  (*init)(sol);

  ref.assign({3, 1, 2, 0});
  ASSERT_TRUE(std::equal(begin(sol), end(sol), begin(ref), end(ref)));
}

TEST(Heuristic, KK1Example) {
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
  auto init = buildPriority(dt, "kk1", false, "decr");
  (*init)(sol);

  ref.assign({2, 4, 3, 5, 0, 1});
  ASSERT_TRUE(std::equal(begin(sol), end(sol), begin(ref), end(ref)));
}

TEST(Heuristic, NMExample) {
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
  auto init = buildPriority(dt, "nm", false, "decr");
  (*init)(sol);
  
  ref.assign({2, 4, 3, 1, 0, 5});
  ASSERT_TRUE(std::equal(begin(sol), end(sol), begin(ref), end(ref)));
}