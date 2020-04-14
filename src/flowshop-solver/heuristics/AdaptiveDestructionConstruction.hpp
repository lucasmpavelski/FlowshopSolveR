#pragma once

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/heuristics/DestructionConstruction.hpp"
#include "flowshop-solver/heuristics/FitnessReward.hpp"
#include "heuristics/InsertionStrategy.hpp"

template <class Ngh>
class AdaptiveDestructionConstruction : public DestructionConstruction<Ngh> {
  using EOT = typename Ngh::EOT;

  FitnessReward<EOT>& fitnessReward;
  OperatorSelection<int>& operatorSelection;
  bool firstIteration = true;

 public:
  AdaptiveDestructionConstruction(InsertionStrategy<Ngh>& insert,
                                  OperatorSelection<int>& operatorSelection,
                                  FitnessReward<EOT>& fitnessReward)
      : DestructionConstruction<Ngh>{insert, 2},
        fitnessReward{fitnessReward},
        operatorSelection{operatorSelection},
        firstIteration{true} {}

  using DestructionConstruction<Ngh>::destructionSize;

  auto operator()(EOT& sol) -> bool override {
    // std::cerr << "destruct " << sol.fitness() << '\n';
    if (!firstIteration) {
      operatorSelection.feedback(fitnessReward.current(),
                                 fitnessReward.previous());
      operatorSelection.update();
    }
    firstIteration = false;
    int d = operatorSelection.selectOperator();
    destructionSize(d);
    return DestructionConstruction<Ngh>::operator()(sol);
  }
};