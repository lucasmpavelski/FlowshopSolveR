#pragma once

#include <iterator>

#include "flowshop-solver/fla/FitnessHistoryFLA.hpp"

template <class EOT>
class AdaptiveWalkLengthFLA : public FitnessHistoryFLA<EOT> {
 public:
  AdaptiveWalkLengthFLA(const FitnessHistory<EOT>& fitnessHistory)
      : FitnessHistoryFLA<EOT>{fitnessHistory} {}

 protected:
  using citerator = typename FitnessHistory<EOT>::citerator;

  virtual double compute(citerator begin, citerator end) {
    const int n = std::distance(begin, end);
    return n;
  }
};