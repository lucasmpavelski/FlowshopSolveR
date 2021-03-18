#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/continuators/myTimeStat.hpp"
#include "flowshop-solver/heuristics/FitnessReward.hpp"
#include "flowshop-solver/heuristics/perturb/DestructionConstruction.hpp"
#include "flowshop-solver/heuristics/perturb/DestructionStrategy.hpp"

template <class EOT>
class AdaptivePositionDestructionStrategy : public DestructionStrategy<EOT> {
  DestructionSize& destructionSize;
  FitnessRewards<EOT>& rewards;
  OperatorSelection<int>& operatorSelection;
  int rewardType;
  bool printRewards;
  bool printChoices;
  int iteration = 0;
  myTimeStat<EOT> time;

  auto choosePosition(int size) -> std::pair<int, int> {
    if (iteration >= 2) {
      if (printRewards) {
        EOT sol;
        time(sol);
        std::cout << time.value() << ',' << rewards.initialGlobal() << ','
                  << rewards.lastGlobal() << ',' << rewards.initialLocal()
                  << ',' << rewards.lastLocal() << '\n';
      }
      operatorSelection.feedback(reward());
      operatorSelection.update();
    }
    iteration++;
    int option = operatorSelection.selectOperator();
    if (option == 0) {
      return {0, size / 3};
    } else if (option == 1) {
      return {size / 3 + 1, 2 * size / 3};
    } else {
      return {2 * size / 3 + 1, size};
    }
  }

  [[nodiscard]] auto reward() -> double {
    double pf, cf;
    switch (rewardType) {
      case 0:
        pf = rewards.initialGlobal();
        cf = rewards.lastGlobal();
        break;
      case 1:
        pf = rewards.initialGlobal();
        cf = rewards.lastLocal();
        break;
      case 2:
        pf = rewards.initialLocal();
        cf = rewards.lastGlobal();
        break;
      case 3:
        pf = rewards.initialLocal();
        cf = rewards.lastLocal();
        break;
      default:
        throw std::runtime_error{"invalid rewardType"};
    }
    return (pf - cf) / pf;
  }
  
 public:
  AdaptivePositionDestructionStrategy(DestructionSize& destructionSize,
                                      OperatorSelection<int>& operatorSelection,
                                      FitnessRewards<EOT>& rewards,
                                      int rewardType,
                                      bool printRewards = false,
                                      bool printChoices = false)
      : destructionSize(destructionSize),
        rewards(rewards),
        operatorSelection(operatorSelection),
        rewardType(rewardType),
        printRewards(printRewards),
        printChoices(printChoices) {}

  auto operator()(EOT& sol) -> EOT override {
    int n = sol.size();
    std::pair<int, int> minMax = choosePosition(n);
    EOT removed;
    int ds = std::min(destructionSize.value(), n);
    for (int k = 0; k < ds; k++) {
      unsigned int index =
          rng.random((minMax.second - minMax.first)) + minMax.first;
      removed.push_back(sol[index]);
      sol.erase(sol.begin() + index);
    }
    return removed;
  }

 private:
};
