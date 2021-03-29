#include <gtest/gtest.h>
#include <algorithm>

#include "flowshop-solver/aos/random.hpp"
#include "flowshop-solver/heuristics/AdaptiveBestInsertionExplorer.hpp"

TEST(PositionSelector, AdaptivePostitionSelector) {
    RNG::seed(65787);
    Random<int> selector({1, 2, 3});
    AdaptivePositionSelector aps(selector);
    std::vector<int> counts(301);
    std::vector<int> vec(301);
    aps.init(vec);
    for (int i = 0; i < 1000000; i++) {
        int selected = aps.select(vec);
        counts[selected]++;
    }
    for (int ct : counts) {
        //std::cerr << ct << " ";
        ASSERT_TRUE(ct > 3000);
    }
}


TEST(PositionSelector, AdaptivePostitionSelectorNoRepeat) {
    Random<int> selector({1, 2, 3});
    AdaptiveNoReplacementPositionSelector aps(selector);
    std::vector<int> counts(300);
    std::vector<int> vec(300);
    std::iota(vec.begin(), vec.end(), 400);
    std::vector<int> selectedVec(300);
    aps.init(vec);
    for (int i = 0; i < 300; i++) {
        selectedVec[i] = vec[aps.select(vec)];
    }
    ASSERT_TRUE(std::is_permutation(vec.begin(), vec.end(), selectedVec.begin()));
}