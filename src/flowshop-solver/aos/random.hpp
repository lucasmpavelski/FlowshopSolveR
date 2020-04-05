#pragma once

#pragma once

#include <random>
#include <vector>

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/global.hpp"

template <class OpT>
class Random : public OperatorSelection<OpT> {
  std::uniform_int_distribution<int> dist;

 public:
  Random(const std::vector<OpT>& strategies)
      : OperatorSelection<OpT>{strategies}, dist(0, strategies.size() - 1) {}

  void update() final override{};

  void feedback(const double, const double) final override {}

  std::ostream& printOn(std::ostream& os) final override {
    os << "  strategy: Random\n";
    return os;
  }

  using OperatorSelection<OpT>::getOperator;

  OpT& selectOperator() final override {
    return getOperator(dist(RNG::engine));
  }
};