#pragma once

#include <iostream>
#include <vector>

#include "flowshop-solver/global.hpp"

template <typename OpT>
class OperatorSelection {
  std::vector<OpT> operators;

 public:
  // lifecycle
  OperatorSelection(std::vector<OpT> operators)
      : operators{std::move(operators)} {};

  virtual ~OperatorSelection() = default;

  // accessors
  auto noOperators() const -> int { return operators.size(); };
  auto doAdapt() const -> bool { return noOperators() > 1; };
  auto getOperator(const int idx) -> OpT& { return operators.at(idx); };

  // main interface
  virtual void reset(double){};  // on init algorithm
  virtual auto selectOperator() -> OpT& { return operators[0]; };
  virtual void feedback(const double, const double){};
  virtual void update(){};  // on finish generation
  virtual auto printOn(std::ostream& os) -> std::ostream& = 0;

  // misc
  template <typename OpT2>
  friend auto operator<<(std::ostream& os, OperatorSelection<OpT2> const& aos)
      -> std::ostream& {
    os << "AdaptiveOperatorSelection:\n";
    if (aos.doAdapt()) {
      os << "  operators:";
      for (const OpT& op : aos.operators)
        os << "\n" << op;
    } else {
      os << "  constant_operator:\n" << aos.operators[0];
    }
    return aos.printOn(os);
  }
};
