#pragma once

#include <vector>

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/aos/beta_distribution.hpp"
#include "flowshop-solver/global.hpp"

template <class OpT>
class ThompsonSampling : public OperatorSelection<OpT> {
 protected:
  std::vector<int> alphas;
  std::vector<int> betas;
  int              noSamples;
  int              opIdx = 0;

  using OperatorSelection<OpT>::noOperators;

  auto selectOperatorIdx() -> int override {
    opIdx            = 0;
    double maxSample = 0.0;
    for (int i = 0; i < noOperators(); i++) {
      beta_distribution<double> dist(alphas[i], betas[i]);
      double                    meanSample = 0;
      for (int j = 0; j < noSamples; j++) {
        meanSample += dist(RNG::engine) / noSamples;
      }
      if (meanSample > maxSample) {
        opIdx     = i;
        maxSample = meanSample;
      }
    }
    return opIdx;
  }

 public:
  ThompsonSampling(const std::vector<OpT>& strategies)
      : OperatorSelection<OpT>{strategies},
        alphas(strategies.size(), 1),
        betas(strategies.size(), 1),
        noSamples{1} {}

  void update() override{};

  void feedback(double fb) override {
    if (fb > 0) {
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

  void reset(double) override {
    alphas.assign(noOperators(), 0);
    betas.assign(noOperators(), 0);
  }
};

template <class OpT>
class DynamicThompsonSampling : public ThompsonSampling<OpT> {
  int    threshold;
  double scale;

 protected:
  using ThompsonSampling<OpT>::alphas;
  using ThompsonSampling<OpT>::betas;
  using ThompsonSampling<OpT>::opIdx;

 public:
  DynamicThompsonSampling(const std::vector<OpT>& strategies, int threshold = 1)
      : ThompsonSampling<OpT>{strategies},
        threshold{threshold},
        scale{threshold / (threshold + 1.0)} {}

  void feedback(double fb) override {
    if (alphas[opIdx] + betas[opIdx] < threshold) {
      if (fb > 0) {
        alphas[opIdx]++;
      } else {
        betas[opIdx]++;
      }
    } else {
      if (fb > 0) {
        alphas[opIdx] = (alphas[opIdx] + 1) * scale;
      } else {
        betas[opIdx] = (betas[opIdx] + 1) * scale;
      }
    }
  }
};
