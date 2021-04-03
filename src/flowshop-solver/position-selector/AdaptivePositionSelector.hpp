#pragma once

#include <vector>

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/position-selector/PositionSelector.hpp"

class AdaptivePositionSelector : public PositionSelector {
  OperatorSelection<int>& operatorSelection;
  bool includeRandom = false;

 public:
  AdaptivePositionSelector(OperatorSelection<int>& operatorSelection)
      : operatorSelection(operatorSelection) {
    auto& opRef = operatorSelection.operatorsRef();
    includeRandom = std::find(opRef.begin(), opRef.end(), 0) != opRef.end();
  }

  auto select(const std::vector<int>& vec) -> int override {
    const int n = vec.size();
    const int k = operatorSelection.noOperators() - includeRandom;
    const int selected = operatorSelection.selectOperator();
    if (selected == 0) {
      return rng.random(n);
    }
    const int roundCarry = selected == k && n % k > 0;
    const int pollSize = n / k + roundCarry;
    return rng.random(pollSize) + (selected - 1) * (n / k);
  }

  void feedback(const double reward) override {
    operatorSelection.feedback(reward);
    operatorSelection.update();
  }
};