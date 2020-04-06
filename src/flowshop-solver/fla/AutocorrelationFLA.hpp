#pragma once

#include <algorithm>
#include <iterator>

#include "flowshop-solver/fla/FitnessHistoryFLA.hpp"

template <class EOT>
class AutocorrelationFLA : public FitnessHistoryFLA<EOT> {
  int delay;

 public:
  AutocorrelationFLA(const FitnessHistory<EOT>& fitnessHistory, int delay = 1)
      : FitnessHistoryFLA<EOT>{fitnessHistory}, delay{delay} {}

 protected:
  using citerator = typename FitnessHistory<EOT>::citerator;

  double compute(citerator begin, citerator end) override {
    const int n = std::distance(begin, end);
    if (n == 1)
      return 1.0;
    const double mean = std::accumulate(begin, end, 0.0) / n;
    auto varAcc = [mean](double sum, double current) {
      return sum + (current - mean) * (current - mean);
    };
    const double var = std::accumulate(begin, end, 0, varAcc) / (n - 1.0);
    const double scale = 1.0 / std::max((n - delay) * var, 1e-6);
    double sum = 0.0;
    for (int i = 0; i < n - delay; i++) {
      const double xi = *(begin + i);
      const double xid = *(begin + i + delay);
      sum += (xi - mean) * (xid - mean);
    }
    return scale * sum;
  }
};