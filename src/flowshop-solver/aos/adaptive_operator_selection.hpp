#pragma once

#include <iostream>
#include <vector>

#include <paradiseo/eo/eo>

#include "flowshop-solver/continuators/myTimeStat.hpp"
#include "flowshop-solver/global.hpp"

template <typename OpT>
class OperatorSelection : public eoFunctorBase {
public:
  struct DummyNgh { using EOT = int; };
  using Continuator = moContinuator<DummyNgh>;
private:

  std::vector<OpT> operators;
  enum class WarmUpStrategy {
    RANDOM,
    FIXED
  } warmUpStrategy = WarmUpStrategy::FIXED;
  Continuator*  warmupContinuator;
  int                 fixedWarmUpParameter = 0;
  moTrueContinuator<DummyNgh> noWarmUp;

 protected:
  virtual auto selectOperatorIdx() -> int = 0;

 public:
  // lifecycle
  OperatorSelection(std::vector<OpT> operators)
      : operators{std::move(operators)}, warmupContinuator{&noWarmUp} {};

  // accessors
  [[nodiscard]] auto noOperators() const -> int { return operators.size(); };
  [[nodiscard]] auto doAdapt() const -> bool { return noOperators() > 1; };

  void setWarmUp(Continuator& warmup,
                 std::string        strategyStr,
                 int                fixedWarmUpParameter) {
    std::transform(begin(strategyStr), end(strategyStr), begin(strategyStr),
                   tolower);
    WarmUpStrategy strategy = WarmUpStrategy::FIXED;
    if (strategyStr == "random")
      strategy = WarmUpStrategy::RANDOM;
    setWarmUp(warmup, strategy, fixedWarmUpParameter);
  }

  void setWarmUp(Continuator& warmup,
                 WarmUpStrategy     strategy,
                 int                fixedWarmUpParameter) {
    this->warmupContinuator    = &warmup;
    this->warmUpStrategy       = strategy;
    this->fixedWarmUpParameter = fixedWarmUpParameter;
  }

  // main interface
  virtual void reset(double){};  // on init algorithm
  virtual void feedback(double){};
  virtual void update(){};  // on finish generation
  virtual auto printOn(std::ostream& os) -> std::ostream& = 0;

  auto selectOperator() -> OpT& {
    int dummy;
    if ((*warmupContinuator)(dummy)) {
      switch (warmUpStrategy) {
        case WarmUpStrategy::FIXED:
          return operators[fixedWarmUpParameter];
        case WarmUpStrategy::RANDOM:
          return rng.choice(operators);
      }
    }
    return operators[selectOperatorIdx()];
  };

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
