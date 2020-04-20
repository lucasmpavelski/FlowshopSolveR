#pragma once

#include <paradiseo/mo/mo>

#include "flowshop-solver/continuators/myTimeStat.hpp"
#include "flowshop-solver/heuristics/neighborhood_checkpoint.hpp"

template <class EOT>
class FitnessReward : public moStatBase<EOT> {
  bool firstIteration = true;
  double initialFitness = -1;
  double finalFitness = -1;
  myTimeStat<EOT>& timer;
  bool print = false;

 public:
  FitnessReward(myTimeStat<EOT>& timer, bool print = false)
      : timer{timer}, print{print} {
    if (print) {
      std::puts("runtime,initialFitness,finalFitness\n");
    }
  }

  void init(EOT& sol) final {
    // discart first iteration (no destruction to be rewarded)
    if (sol.invalid() || firstIteration) {
      firstIteration = false;
      return;
    }
    initialFitness = sol.fitness();
  }

  void operator()(EOT&) final {}

  void lastCall(EOT& sol) final {
    timer(sol);
    if (sol.invalid())
      return;
    finalFitness = sol.fitness();
    if (print) {
      std::cout << timer.value() << ',' << initialFitness << ',' << finalFitness
                << '\n';
    }
  }

  auto isAvailable() const -> bool {
    return !firstIteration && initialFitness != -1 && finalFitness != -1;
  }

  auto current() const -> double { return finalFitness; }
  auto previous() const -> double { return initialFitness; }
};