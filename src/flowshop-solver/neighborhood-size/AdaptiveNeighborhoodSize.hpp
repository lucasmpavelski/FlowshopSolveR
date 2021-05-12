#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/neighborhood-size/NeighborhoodSize.hpp"

template <class EOT>
class AdaptiveNeighborhoodSize : public NeighborhoodSize {
  int size;
  OperatorSelection<int>& operatorSelection;
  FitnessRewards<EOT>& rewards;
  int rewardType;

 public:
  AdaptiveNeighborhoodSize(int size,
                           OperatorSelection<int> operatorSelection,
                           FitnessRewards<EOT>& rewards,
                           int rewardType)
      : size(size),
        operatorSelection(operatorSelection),
        rewards(rewards),
        rewardType(rewardType) {}

  auto getSize() -> int override {
    if (rewards.available()) {
      operatorSelection.feedback(rewards.reward(rewardType));
      operatorSelection.update();
    }
    return (1.0 / operatorSelection.selectOperator()) * size;
  }
};
