#pragma once

#include <cctype>
#include <iostream>
#include <limits>
#include <vector>
#include <cassert>

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/global.hpp"

template <typename OpT>
class ProbabilityMatching : public OperatorSelection<OpT> {
 public:
  enum class RewardType { AvgAbs, AvgNorm, ExtAbs, ExtNorm };
  using real_vec = std::vector<double>;
  using int_vec = std::vector<int>;

 private:
  const double alpha;
  const double p_min;
  RewardType rew_type;
  double best_fitness;
  int chosen_strat;

  real_vec quality;
  real_vec reward;
  real_vec prob;
  real_vec S;
  int_vec nr_S;
  real_vec maior_S;

  int updateCounter = 0;
  int updateWindow = 5;

  void updateRewards();
  auto updateQualities() -> double;

  auto rewardTypeFromString(std::string str) -> RewardType {
    for (auto& c : str)
      c = tolower(c);
    if (str == "avgabs")
      return ProbabilityMatching<OpT>::RewardType::AvgAbs;
    if (str == "avgnorm")
      return ProbabilityMatching<OpT>::RewardType::AvgNorm;
    if (str == "extabs")
      return ProbabilityMatching<OpT>::RewardType::ExtAbs;
    if (str == "extnorm")
      return ProbabilityMatching<OpT>::RewardType::ExtNorm;
    throw std::invalid_argument(str + " cannot be converted to a reward type.");
    return ProbabilityMatching<OpT>::RewardType::AvgAbs;
  }

 protected:
  auto selectOperatorIdx() -> int {
    const auto rnd = RNG::realUniform<double>();
    double aux = 0.0;
    for (int k = 0; k < noOperators(); k++) {
      aux += prob[k];
      if (rnd <= aux) {
        chosen_strat = k;
        return k;
      }
    }
    assert(false);
    return 0;
  }

 public:
  using OperatorSelection<OpT>::doAdapt;
  using OperatorSelection<OpT>::noOperators;

  ProbabilityMatching(const std::vector<OpT>& operators,
                      const RewardType& rew_type,
                      const double alpha,
                      const double p_min,
                      const int updateWindow)
      : OperatorSelection<OpT>(operators),
        alpha(alpha),
        p_min(p_min),
        rew_type(rew_type),
        quality(operators.size()),
        reward(operators.size()),
        prob(operators.size()),
        S(operators.size()),
        nr_S(operators.size()),
        maior_S(operators.size()),
        updateCounter{1},
        updateWindow{updateWindow} {
    reset(std::numeric_limits<double>::infinity());
  };

  ProbabilityMatching(const std::vector<OpT>& operators,
                      const std::string& rew_type = "extabs",
                      const double alpha = 0.3,
                      const double p_min = 0.1,
                      const int updateWindow = 1)
      : ProbabilityMatching{operators, rewardTypeFromString(rew_type), alpha,
                            p_min, updateWindow} {}

  void reset(const double) final;
  void doFeedback(const double) final;
  void update() final;

  auto printOn(std::ostream& os) -> std::ostream& final {
    os << "  strategy: ProbabilityMatching\n"
       << "  alpha: " << alpha << '\n'
       << "  p_min: " << p_min << '\n'
       << "  reward: " << rew_type << '\n';
    return os;
  }

  friend auto operator<<(std::ostream& os, RewardType const& rt)
      -> std::ostream& {
    switch (rt) {
      case RewardType::AvgAbs:
        os << "AvgAbs";
        break;
      case RewardType::AvgNorm:
        os << "AvgNorm";
        break;
      case RewardType::ExtAbs:
        os << "ExtAbs";
        break;
      case RewardType::ExtNorm:
        os << "ExtNorm";
        break;
    }
    return os;
  }
};

template <typename OpT>
void ProbabilityMatching<OpT>::reset(double best_f) {
  best_fitness = best_f;
  chosen_strat = -1;
  using std::fill;
  fill(quality.begin(), quality.end(), 0.0);
  fill(prob.begin(), prob.end(), 1.0 / noOperators());
  fill(S.begin(), S.end(), 0.0);
  fill(nr_S.begin(), nr_S.end(), 0);
  fill(maior_S.begin(), maior_S.end(), 0.0);
}

template <typename OpT>
void ProbabilityMatching<OpT>::doFeedback(double feedback) {
  if (chosen_strat == -1)
    return;
  S[chosen_strat] += feedback;
  nr_S[chosen_strat]++;
  if (feedback > maior_S[chosen_strat])
    maior_S[chosen_strat] = feedback;
};

template <typename OpT>
void ProbabilityMatching<OpT>::update() {
  updateCounter++;
  if (updateCounter % updateWindow > 0)
    return;
  this->updateRewards();
  const double q_total = this->updateQualities();

  if (q_total <= 1e-6)  // no improvement, maintain the probabilities
    return;

  const double s = (1.0 - noOperators() * p_min) / q_total;
  for (int k = 0; k < noOperators(); ++k)
    prob[k] = p_min + s * quality[k];

  using std::fill;
  fill(S.begin(), S.end(), 0.0);
  fill(nr_S.begin(), nr_S.end(), 0);
  fill(maior_S.begin(), maior_S.end(), 0.0);
}

template <typename OpT>
void ProbabilityMatching<OpT>::updateRewards() {
  switch (rew_type) {
    case RewardType::AvgAbs: {
      for (int k = 0; k < noOperators(); ++k)
        reward[k] = nr_S[k] > 0 ? S[k] / nr_S[k] : 0.0;
      break;
    }
    case RewardType::AvgNorm: {
      std::vector<double> rew_linha(noOperators());
      double max_rew_linha = 0.0;
      // calcula o r_linha
      for (int k = 0; k < noOperators(); ++k) {
        rew_linha[k] = nr_S[k] > 0 ? S[k] / nr_S[k] : 0.0;
        // acha o maior r_linha
        if (rew_linha[k] > max_rew_linha)
          max_rew_linha = rew_linha[k];
      }
      // calcula o reward
      for (int k = 0; k < noOperators(); ++k)
        reward[k] = nr_S[k] > 0 ? rew_linha[k] / (max_rew_linha + 1e-6) : 0.0;
      break;
    }
    case RewardType::ExtAbs: {
      for (int k = 0; k < noOperators(); ++k)
        reward[k] = maior_S[k];
      break;
    }
    case RewardType::ExtNorm: {
      std::vector<double> rew_linha(noOperators());
      double max_rew_linha = 0.000001;
      // calcula r_linha
      for (int k = 0; k < noOperators(); k++) {
        rew_linha[k] = maior_S[k];
        // acha o maior r_linha
        if (rew_linha[k] > max_rew_linha)
          max_rew_linha = rew_linha[k];
      }
      // calcula o reward
      for (int k = 0; k < noOperators(); k++)
        reward[k] = rew_linha[k] / max_rew_linha;  // WO: division by 0!
      break;
    }
  }
}

template <typename OpT>
auto ProbabilityMatching<OpT>::updateQualities() -> double {
  double q_total = 0.0;
  for (int k = 0; k < noOperators(); ++k) {
    quality[k] = quality[k] + alpha * (reward[k] - quality[k]);
    q_total = q_total + quality[k];
  }
  return q_total;
}
