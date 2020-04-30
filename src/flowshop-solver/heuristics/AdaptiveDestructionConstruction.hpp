#pragma once

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/heuristics/DestructionConstruction.hpp"
#include "flowshop-solver/heuristics/FitnessReward.hpp"
#include "heuristics/InsertionStrategy.hpp"
#include "flowshop-solver/continuators/myTimeStat.hpp"

template <class Ngh>
class AdaptiveDestructionConstruction : public DestructionConstruction<Ngh> {
  using EOT = typename Ngh::EOT;

  FitnessReward<EOT>& fitnessReward;
  OperatorSelection<int>& operatorSelection;
  bool firstIteration = true;
  bool printChoices;
  
  myTimeStat<EOT> time;

 public:
  AdaptiveDestructionConstruction(InsertionStrategy<Ngh>& insert,
                                  OperatorSelection<int>& operatorSelection,
                                  FitnessReward<EOT>& fitnessReward,
                                  bool printChoices = false)
      : DestructionConstruction<Ngh>{insert, 2},
        fitnessReward{fitnessReward},
        operatorSelection{operatorSelection},
        firstIteration{true},
        printChoices{printChoices} {
          std::cout << "runtime,d\n";
        }

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
    if (printChoices) {
      time(sol);
      std::cout << time.value() << ',' << d << '\n'; 
    }
    destructionSize(d);
    return DestructionConstruction<Ngh>::operator()(sol);
  }
};
