#pragma once

#include <cassert>
#include <cctype>
#include <iostream>
#include <limits>
#include <vector>

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/global.hpp"

template <typename OpT>
class EpsilonGreedy : public OperatorSelection<OpT> {
  std::vector<int> counters;
  std::vector<double> rewards;
  const double epsilon;
  int opIdx = -1;

 protected:
  auto selectOperatorIdx() -> int {
    const auto rnd = RNG::realUniform<double>();
    if (rnd < epsilon) {
      opIdx = RNG::intUniform(counters.size() - 1);
    } else {
      double max = rewards[0] / counters[0];
      opIdx = 0;
      for (int k = 1; k < counters.size(); k++) {
        if (rewards[k] / counters[k] >= max) {
          max = rewards[k] / counters[k];
          opIdx = k;
        }
      }
    }
    return opIdx;
  }

 public:
  EpsilonGreedy(const std::vector<OpT>& operators, const double epsilon)
      : OperatorSelection<OpT>(operators), epsilon(epsilon) {
    counters.assign(operators.size(), 0);
    rewards.assign(operators.size(), 0);
  }

  void doFeedback(double feedback) {
    if (opIdx == -1)
      return;
    counters[opIdx]++;
    rewards[opIdx] += feedback;
  };

  virtual auto printOn(std::ostream& os) -> std::ostream& {
    return os << "epsilon_greedy " << epsilon << '\n';
  }
};
