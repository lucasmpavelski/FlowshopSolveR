#pragma once

#include <paradiseo/eo/eoOp.h>
#include <paradiseo/mo/perturb/moPerturbation.h>
#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/heuristics/FitnessReward.hpp"

template <class Ngh, class EOT = typename Ngh::EOT>
class AdaptivePerturb : public eoMonOp<EOT> {
  std::vector<moPerturbation<Ngh>*> perturbations;
  FitnessRewards<EOT>& fitnessRewards;
  const int rewardType;
  OperatorSelection<int>& operatorSelection;
  int iteration = 0;

 public:
  AdaptivePerturb(std::vector<moPerturbation<Ngh>*> perturbations,
                  FitnessRewards<EOT>& fitnessRewards,
                  int rewardType,
                  OperatorSelection<int>& operatorSelection)
      : perturbations(std::move(perturbations)),
        fitnessRewards(fitnessRewards),
        rewardType(rewardType),
        operatorSelection(operatorSelection) {}

  auto operator()(EOT& _solution) -> bool override {
    if (iteration >= 2) {
      operatorSelection.feedback(fitnessRewards.reward(rewardType));
      operatorSelection.update();
    }
    iteration++;
    int loIdx = operatorSelection.selectOperator();
    moPerturbation<Ngh>& perturb = *perturbations[loIdx];
    return perturb(_solution);
  }
};