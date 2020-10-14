#include <algorithm>

#include <gtest/gtest.h>

#include "flowshop-solver/problems/FSP.hpp"

TEST(FSPNeighbor, Moves) {
  auto testMove = [](std::initializer_list<int> init, int from, int to,
                     std::initializer_list<int> result) {
    FSP sol;
    sol.assign(init);
    FSPNeighbor ng(from, to, sol.size());
    ng.move(sol);
    return std::equal(sol.begin(), sol.end(), result.begin());
  };

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