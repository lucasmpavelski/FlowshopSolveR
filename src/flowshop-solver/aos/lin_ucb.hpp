#pragma once

#include <paradiseo/eo/eo>

#include <Eigen/Dense>
#include <array>
#include <cassert>

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/fla/FitnessLandscapeMetric.hpp"
#include "flowshop-solver/global.hpp"

class ProblemContext : public eoFunctorBase {
  std::vector<FitnessLandscapeMetric*> metrics;

 public:
  auto compute() -> std::vector<double> {
    std::vector<double> values;
    for (auto& metric : metrics) {
      values.push_back(metric->compute());
    }
    return values;
  }

  void add(FitnessLandscapeMetric& metric) { metrics.emplace_back(&metric); }

  [[nodiscard]] auto size() const -> int { return metrics.size(); }
};

using Eigen::MatrixXd;
using Eigen::VectorXd;

template <class OpT>
class LinUCB : public OperatorSelection<OpT> {
  const double          alpha;
  ProblemContext&       context;
  std::vector<MatrixXd> A;
  std::vector<VectorXd> b;
  std::vector<VectorXd> theta;
  VectorXd              x;
  VectorXd              p;
  int                   opIdx = -1;

  protected:

  auto selectOperatorIdx() -> int override {
    p.maxCoeff(&opIdx);
    return opIdx;
  }

 public:
  LinUCB(std::vector<OpT>             operators,
         ProblemContext&              context,
         const double                 alpha = 0.3)
      : OperatorSelection<OpT>{operators},
        alpha{alpha},
        context{context},
        A{operators.size(), MatrixXd::Identity(context.size(), context.size())},
        b{operators.size(), VectorXd::Zero(context.size())},
        theta{operators.size(), VectorXd{context.size()}},
        x{context.size()},
        p{VectorXd::Constant(operators.size(), 0.5)} {
    assert(alpha >= 0);
  }

  void update() final{};

  void doFeedback(double reward) final {
    if (opIdx == -1)
      return;
    std::vector<double> features = context.compute();
    x = Eigen::Map<VectorXd>(features.data(), features.size());

    A[opIdx]     = A[opIdx] + x * x.transpose();
    b[opIdx]     = b[opIdx] + reward * x;
    theta[opIdx] = A[opIdx].colPivHouseholderQr().solve(b[opIdx]);

    double prod = x.transpose() * A[opIdx] * x;
    p(opIdx)    = theta[opIdx].transpose() * x + alpha * std::sqrt(prod);
  }

  auto printOn(std::ostream& os) -> std::ostream& final {
    os << "  strategy: LinUCB MAB\n"
       << "  alpha: " << alpha << '\n';
    return os;
  } 
};