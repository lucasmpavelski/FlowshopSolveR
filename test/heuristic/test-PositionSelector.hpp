#include <gtest/gtest.h>
#include <algorithm>

#include "flowshop-solver/aos/random.hpp"
#include "flowshop-solver/position-selector/AdaptivePositionSelector.hpp"
#include "flowshop-solver/position-selector/AdaptiveNoReplacementPositionSelector.hpp"

TEST(PositionSelector, AdaptivePostitionSelector) {
    RNG::seed(65787);
    Random<int> selector({1, 2, 3});
    AdaptivePositionSelector<std::vector<int>> aps(selector);
    std::vector<int> counts(301, 0);
    std::vector<int> vec(301, 0);
    aps.init(vec);
    for (int i = 0; i < 1000000; i++) {
        int selected = aps.select(vec);
        ASSERT_TRUE(selected >= 0);
        ASSERT_TRUE(selected < 301);
        counts[selected]++;
    }
    for (int ct : counts) {
        ASSERT_TRUE(ct > 3000);
    }
}

TEST(PositionSelector, AdaptivePostitionSelectorWithRandom) {
    RNG::seed(65787);
    Random<int> selector({0, 1, 2, 3});
    AdaptivePositionSelector<std::vector<int>> aps(selector);
    std::vector<int> counts(301, 0);
    std::vector<int> vec(301, 0);
    aps.init(vec);
    for (int i = 0; i < 1000000; i++) {
        int selected = aps.select(vec);
        ASSERT_TRUE(selected >= 0);
        ASSERT_TRUE(selected < 301);
        counts[selected]++;
    }
    for (int ct : counts) {
        ASSERT_TRUE(ct > 3000);
    }
}

TEST(PositionSelector, AdaptivePostitionSelectorMaxOptionsWithRandom) {
    RNG::seed(65787);
    std::vector<int> options(301);
    std::iota(options.begin(), options.end(), 0);
    Random<int> selector(options);
    AdaptivePositionSelector<std::vector<int>> aps(selector);
    std::vector<int> counts(300, 0);
    std::vector<int> vec(300);
    std::iota(vec.begin(), vec.end(), 0);
    aps.init(vec);
    for (int i = 0; i < 1000000; i++) {
        int selected = aps.select(vec);
        ASSERT_TRUE(selected >= 0);
        ASSERT_TRUE(selected < 300);

        counts[selected]++;
    }
    for (int ct : counts) {
        ASSERT_TRUE(ct > 3000);
    }
}
TEST(PositionSelector, AdaptivePostitionSelectorMaxOptionsWithoutRandom) {
    RNG::seed(65787);
    std::vector<int> options(300);
    std::iota(options.begin(), options.end(), 1);
    Random<int> selector(options);
    AdaptivePositionSelector<std::vector<int>> aps(selector);
    std::vector<int> counts(300, 0);
    std::vector<int> vec(300);
    std::iota(vec.begin(), vec.end(), 0);
    aps.init(vec);
    for (int i = 0; i < 1000000; i++) {
        int selected = aps.select(vec);
        ASSERT_TRUE(selected >= 0);
        ASSERT_TRUE(selected < 300);

        counts[selected]++;
    }
    for (int ct : counts) {
        ASSERT_TRUE(ct > 3000);
    }
}

TEST(PositionSelector, AdaptivePostitionSelectorOnlyRandomArm) {
    RNG::seed(65787);
    std::vector<int> options(1);
    options[0] = 0;
    Random<int> selector(options);
    AdaptivePositionSelector<std::vector<int>> aps(selector);
    std::vector<int> counts(300, 0);
    std::vector<int> vec(300);
    std::iota(vec.begin(), vec.end(), 0);
    aps.init(vec);
    for (int i = 0; i < 1000000; i++) {
        int selected = aps.select(vec);
        ASSERT_TRUE(selected >= 0);
        ASSERT_TRUE(selected < 300);

        counts[selected]++;
    }
    for (int ct : counts) {
        ASSERT_TRUE(ct > 3000);
    }
}


TEST(PositionSelector, AdaptivePostitionSelectorNoRepeat) {
    Random<int> selector({1, 2, 3});
    AdaptiveNoReplacementPositionSelector<std::vector<int>> aps(selector);
    std::vector<int> counts(300, 0);
    std::vector<int> vec(300);
    std::iota(vec.begin(), vec.end(), 400);
    std::vector<int> selectedVec(300);
    aps.init(vec);
    for (int i = 0; i < 300; i++) {
        selectedVec[i] = vec[aps.select(vec)];
    }
    ASSERT_TRUE(std::is_permutation(vec.begin(), vec.end(), selectedVec.begin()));
}