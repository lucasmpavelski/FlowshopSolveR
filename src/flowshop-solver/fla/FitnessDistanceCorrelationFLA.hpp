#pragma once

#include "flowshop-solver/fla/FitnessHistoryFLA.hpp"

template <class EOT>
class FitnessDistanceCorrelationFLA : public FitnessHistoryFLA<EOT> {
 public:
  FitnessDistanceCorrelationFLA(const FitnessHistory<EOT>& fitnessHistory)
      : FitnessHistoryFLA<EOT>{fitnessHistory} {}

 protected:
  using citerator = typename FitnessHistory<EOT>::citerator;

  double compute(citerator begin, citerator end) override {
    auto last = end;
    const int n = std::distance(begin, last);
    if (n == 1)
      return 1.0;
    const double mean = std::accumulate(begin, last, 0.0) / n;
    auto varAcc = [mean](double sum, double current) {
      return sum + (current - mean) * (current - mean);
    };
    const double sd =
        std::sqrt(std::accumulate(begin, last, 0, varAcc) / (n - 1));
    const double sdDist = std::sqrt((n - 1) * (n + 1) / 12.0);
    const double meanDist = n / 2.0;
    const double scale = 1.0 / std::max((n - 1) * sd * sdDist, 1e-6);
    double sum = 0.0;
    for (int i = 0; i < n - 1; i++) {
      const double xDiff = *(begin + i) - mean;
      const double yDiff = n - i - meanDist;
      sum += xDiff * yDiff;
    }
    return scale * sum;
  }
};