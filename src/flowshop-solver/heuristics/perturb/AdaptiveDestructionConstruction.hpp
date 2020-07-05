#pragma once

#include <iostream>
#include <stdexcept>
#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/continuators/myTimeStat.hpp"
#include "flowshop-solver/heuristics/perturb/DestructionConstruction.hpp"
#include "flowshop-solver/heuristics/FitnessReward.hpp"
#include "heuristics/InsertionStrategy.hpp"

template <class Ngh>
class AdaptiveDestructionConstruction : public DestructionConstruction<Ngh> {
  using EOT = typename Ngh::EOT;

  FitnessRewards<EOT>& rewards;
  OperatorSelection<int>& operatorSelection;
  int rewardType;
  int iteration = 0;
  bool printChoices;
  bool printRewards;

  myTimeStat<EOT> time;

 public:
  AdaptiveDestructionConstruction(InsertionStrategy<Ngh>& insert,
                                  OperatorSelection<int>& operatorSelection,
                                  FitnessRewards<EOT>& rewards,
                                  int rewardType,
                                  bool printRewards = false,
                                  bool printChoices = false)
      : DestructionConstruction<Ngh>{insert, 2},
        rewards{rewards},
        operatorSelection{operatorSelection},
        rewardType{rewardType},
        printChoices{printChoices},
        printRewards{printRewards} {
    if (printChoices)
      std::cout << "runtime,d\n";
    if (printRewards)
      std::cout << "runtime,ig,lg,il,ll\n";
  }

  using DestructionConstruction<Ngh>::destructionSize;

  [[nodiscard]] auto reward() -> double  {
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

  auto operator()(EOT& sol) -> bool override {
    if (iteration >= 2) {
      if (printRewards) {
        time(sol);
        std::cout << time.value() << ',' << rewards.initialGlobal() << ','
                  << rewards.lastGlobal() << ',' << rewards.initialLocal()
                  << ',' << rewards.lastLocal() << '\n';
      }
      operatorSelection.feedback(reward());
      operatorSelection.update();
    }
    iteration++;
    int d = operatorSelection.selectOperator();
    if (printChoices) {
      time(sol);
      std::cout << time.value() << ',' << d << '\n';
    }
    destructionSize(d);
    return DestructionConstruction<Ngh>::operator()(sol);
  }

  void init(EOT&) override{};
  void add(EOT&, Ngh&) override{};
  void update(EOT&, Ngh&) override{};
  void clearMemory() override{};
};
