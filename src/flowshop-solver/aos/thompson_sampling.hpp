#pragma once

#include <vector>

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/aos/beta_distribution.hpp"
#include "flowshop-solver/global.hpp"

template <class OpT>
class ThompsonSampling : public OperatorSelection<OpT> {
  std::vector<int> alphas;
  std::vector<int> betas;
  int noSamples;
  int opIdx = 0;

 public:
  ThompsonSampling(const std::vector<OpT>& strategies, int noSamples = 1000)
      : OperatorSelection<OpT>{strategies},
        alphas(strategies.size()),
        betas(strategies.size()),
        noSamples{noSamples} {
    std::fill(alphas.begin(), alphas.end(), 0);
    std::fill(betas.begin(), betas.end(), 0);
  }

  void update() final{};

  void feedback(const double cf, const double pf) final {
    if (cf > pf) {
      alphas[opIdx]++;
    } else {
      betas[opIdx]++;
    }
  }

  auto printOn(std::ostream& os) -> std::ostream& final {
    os << "  strategy: Thomson sampling MAB\n"
       << "  no_samples: " << noSamples << '\n';
    return os;
  }

  using OperatorSelection<OpT>::getOperator;
  using OperatorSelection<OpT>::noOperators;

  auto selectOperator() -> OpT& final {
    opIdx = 0;
    double maxSample = 0.0;
    for (int i = 0; i < noOperators(); i++) {
      beta_distribution<double> dist(alphas[i], betas[i]);
      double meanSample = 0;
      for (int j = 0; j < noSamples; j++) {
        meanSample += dist(RNG::engine) / noSamples;
      }
      if (meanSample > maxSample) {
        opIdx = i;
        maxSample = meanSample;
      }
    }
    return getOperator(opIdx);
  }
};