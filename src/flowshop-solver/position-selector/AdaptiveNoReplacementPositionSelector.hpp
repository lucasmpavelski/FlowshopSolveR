#pragma once

#include <vector>

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/position-selector/AdaptivePositionSelector.hpp"


class AdaptiveNoReplacementPositionSelector : public AdaptivePositionSelector {
   std::vector<int> unselectedPositions;
 
 public:
  AdaptiveNoReplacementPositionSelector(OperatorSelection<int>& operatorSelection)
      : AdaptivePositionSelector(operatorSelection) {}

  void init(const std::vector<int>& sol) override {
    unselectedPositions = sol;
  }

  auto select(const std::vector<int>& sol) -> int override {
    if (unselectedPositions.empty()) {
      init(sol);
    }
    int pos = AdaptivePositionSelector::select(unselectedPositions);
    int solPos = std::distance(sol.begin(), std::find(sol.begin(), sol.end(), unselectedPositions[pos]));
    unselectedPositions.erase(unselectedPositions.begin() + pos);
    return solPos;
  }
};