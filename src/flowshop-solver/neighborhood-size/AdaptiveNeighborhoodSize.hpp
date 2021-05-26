#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/neighborhood-size/NeighborhoodSize.hpp"
#include "flowshop-solver/heuristics/FitnessReward.hpp"
#include "flowshop-solver/aos/adaptive_operator_selection.hpp"

template <class EOT>
class AdaptiveNeighborhoodSize : public NeighborhoodSize {
  int size;
  OperatorSelection<int>& operatorSelection;
  FitnessRewards<EOT>& rewards;
  int rewardType;
  int lastSelected;

 public:
  AdaptiveNeighborhoodSize(int size,
                           OperatorSelection<int>& operatorSelection,
                           FitnessRewards<EOT>& rewards,
                           int rewardType)
      : size(size),
        operatorSelection(operatorSelection),
        rewards(rewards),
        rewardType(rewardType) {}

  auto getSize() -> int override {
    if (rewards.available()) {
      operatorSelection.feedback(rewards.reward(rewardType) * lastSelected);
      operatorSelection.update();
    }
    int selected = operatorSelection.selectOperator();
    lastSelected = selected;
    return (1.0 / selected) * size;
  }
};
