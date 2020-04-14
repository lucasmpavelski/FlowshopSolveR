#pragma once

#include <iterator>

#include "flowshop-solver/fla/FitnessHistoryFLA.hpp"

template <class EOT>
class AdaptiveWalkLengthFLA : public FitnessHistoryFLA<EOT> {
  double scale = 1.0;

 public:
  AdaptiveWalkLengthFLA(const FitnessHistory<EOT>& fitnessHistory, double scale)
      : FitnessHistoryFLA<EOT>{fitnessHistory}, scale{scale} {}

 protected:
  using citerator = typename FitnessHistory<EOT>::citerator;

  double compute(citerator begin, citerator end) override {
    const int n = std::distance(begin, end);
    return n * scale;
  }
};