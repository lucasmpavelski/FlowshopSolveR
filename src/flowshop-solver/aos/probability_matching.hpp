#ifndef PROBABILITY_MATCHING_H
#define PROBABILITY_MATCHING_H

#include <limits>

#include "adaptive_operator_selection.hpp"
#include "global.hpp"

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

  void updateRewards();
  auto updateQualities() -> double;

 public:
  using OperatorSelection<OpT>::doAdapt;
  using OperatorSelection<OpT>::noOperators;

  ProbabilityMatching(const std::vector<OpT>& operators,
                      const RewardType& rew_type = RewardType::AvgAbs,
                      const double alpha = 0.8,
                      const double p_min = 0.1)
      : OperatorSelection<OpT>(operators),
        alpha(alpha),
        p_min(p_min),
        rew_type(rew_type),
        quality(noOperators()),
        reward(noOperators()),
        prob(noOperators()),
        S(noOperators()),
        nr_S(noOperators()),
        maior_S(noOperators()) {
    reset(std::numeric_limits<double>::infinity());
  };

  void reset(const double) final;
  auto selectOperator() -> OpT& final;
  void feedback(const double) final;
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
  };
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
auto ProbabilityMatching<OpT>::selectOperator() -> OpT& {
  const auto rnd = RNG::realUniform<double>();
  double aux = 0.0;
  for (int k = 0; k < noOperators(); k++) {
    aux += prob[k];
    if (rnd <= aux) {
      chosen_strat = k;
      return this->getOperator(k);
    }
  }
  assert(false);
  return this->getOperator(0);
}

template <typename OpT>
void ProbabilityMatching<OpT>::feedback(double feedback) {
  S[chosen_strat] += feedback;
  nr_S[chosen_strat]++;
  if (feedback > maior_S[chosen_strat])
    maior_S[chosen_strat] = feedback;
};

template <typename OpT>
void ProbabilityMatching<OpT>::update() {
  this->updateRewards();
  const double q_total = this->updateQualities();

  if (q_total == 0.0)  // no improvement, maintain the probabilities
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
        reward[k] = nr_S[k] > 0 ? rew_linha[k] / max_rew_linha : 0.0;
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

#endif  // PROBABILITY_MATCHING_H
