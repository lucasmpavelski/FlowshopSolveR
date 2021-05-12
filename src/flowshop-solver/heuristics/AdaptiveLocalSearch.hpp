#pragma once

#include <paradiseo/mo/algo/moLocalSearch.h>
#include <paradiseo/mo/continuator/moContinuator.h>
#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/heuristics/FitnessReward.hpp"

template <class Ngh, class EOT = typename Ngh::EOT>
class AdaptiveLocalSearch : public moLocalSearch<Ngh> {
  std::vector<moLocalSearch<Ngh>*> localSearches;
  FitnessRewards<EOT>& fitnessRewards;
  const int rewardType;
  OperatorSelection<int>& operatorSelection;
  int iteration = 0;
  moContinuator<Ngh>& continuator;
  eoEvalFunc<EOT>& fullEval;

 public:
  AdaptiveLocalSearch(std::vector<moLocalSearch<Ngh>*> localSearches,
                      FitnessRewards<EOT>& fitnessRewards,
                      int rewardType,
                      OperatorSelection<int>& operatorSelection,
                      moContinuator<Ngh>& continuator,
                      eoEvalFunc<EOT>& fullEval)
      : moLocalSearch<Ngh>(localSearches[0]->getNeighborhoodExplorer(),
                           continuator,
                           fullEval),
        localSearches(std::move(localSearches)),
        fitnessRewards(fitnessRewards),
        rewardType(rewardType),
        operatorSelection(operatorSelection),
        continuator(continuator),
        fullEval(fullEval) {}

  virtual bool operator()(EOT& _solution) {
    if (iteration >= 2) {
      operatorSelection.feedback(fitnessRewards.reward(rewardType));
      operatorSelection.update();
    }
    iteration++;
    int loIdx = operatorSelection.selectOperator();
    moLocalSearch<Ngh> localSearch(
      localSearches[loIdx]->getNeighborhoodExplorer(),
      continuator,
      fullEval
    );
    return localSearch(_solution);
  }
};