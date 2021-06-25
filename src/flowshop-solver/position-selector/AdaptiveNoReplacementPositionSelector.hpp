#pragma once

#include <vector>

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/position-selector/AdaptivePositionSelector.hpp"

template <class EOT>
class AdaptiveNoReplacementPositionSelector : public AdaptivePositionSelector<EOT> {
   EOT unselectedPositions;
 
 public:
  AdaptiveNoReplacementPositionSelector(OperatorSelection<int>& operatorSelection)
      : AdaptivePositionSelector<EOT>(operatorSelection) {}

  void init(const EOT& sol) override {
    unselectedPositions = sol;
  }

  auto select(const EOT& sol) -> int override {
    if (unselectedPositions.empty()) {
      init(sol);
    }
    int pos = AdaptivePositionSelector<EOT>::select(unselectedPositions);
    int solPos = std::distance(sol.begin(), std::find(sol.begin(), sol.end(), unselectedPositions[pos]));
    unselectedPositions.erase(unselectedPositions.begin() + pos);
    return solPos;
  }
};