#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/number-of-swaps/NumberOfSwaps.hpp"
#include "flowshop-solver/heuristics/FitnessReward.hpp"
#include "flowshop-solver/aos/adaptive_operator_selection.hpp"

template <class EOT>
class AdaptiveNumberOfSwaps : public NumberOfSwaps {
  OperatorSelection<int>& operatorSelection;
  FitnessRewards<EOT>& rewards;
  int rewardType;

 public:
  AdaptiveNumberOfSwaps(OperatorSelection<int>& operatorSelection,
                        FitnessRewards<EOT>& rewards,
                        int rewardType)
      : operatorSelection(operatorSelection),
        rewards(rewards),
        rewardType(rewardType) {}

  auto get() -> int override {
    if (rewards.available()) {
      operatorSelection.feedback(rewards.reward(rewardType));
      operatorSelection.update();
    }
    int selected = operatorSelection.selectOperator();
    return selected;
  }
};
