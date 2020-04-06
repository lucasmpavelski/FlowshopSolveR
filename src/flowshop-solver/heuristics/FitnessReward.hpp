#pragma once

#include "flowshop-solver/heuristics/neighborhood_checkpoint.hpp"

template <class EOT>
class FitnessReward : public moStatBase<EOT> {
  bool firstIteration = true;
  double initialFitness = -1;
  double finalFitness = -1;

 public:
  FitnessReward() = default;

  void init(EOT& sol) final {
    // discart first iteration (no destruction to be rewarded)
    if (sol.invalid() || firstIteration) {
      firstIteration = false;
      return;
    }
    initialFitness = sol.fitness();
    // std::cerr << "initialFitness:" << initialFitness << '\n';
  }

  void operator()(EOT&) final {}

  void lastCall(EOT& sol) final {
    if (sol.invalid())
      return;
    finalFitness = sol.fitness();
  }

  bool isAvailable() const {
    return !firstIteration && initialFitness != -1 && finalFitness != -1;
  }
  double current() const { return finalFitness; }
  double previous() const { return initialFitness; }
};