#pragma once

#include <Eigen/Dense>
#include <array>
#include <cassert>

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/fla/FitnessLandscapeMetric.hpp"
#include "flowshop-solver/global.hpp"

class ProblemContext {
  std::vector<FitnessLandscapeMetric*> metrics;

 public:
  std::vector<double> compute() {
    std::vector<double> values;
    for (auto& metric : metrics) {
      values.push_back(metric->compute());
    }
    return values;
  }

  void add(FitnessLandscapeMetric& metric) { metrics.emplace_back(&metric); }

  int size() const { return metrics.size(); }
};

using Eigen::MatrixXd;
using Eigen::VectorXd;

template <class OpT>
class LinUCB : public OperatorSelection<OpT> {
  const double alpha;
  ProblemContext& context;
  std::vector<MatrixXd> A;
  std::vector<VectorXd> b;
  std::vector<VectorXd> theta;
  VectorXd x;
  VectorXd p;
  int opIdx = -1;

 public:
  LinUCB(std::vector<OpT> operators,
         ProblemContext& context,
         const double alpha = 0.3)
      : OperatorSelection<OpT>{operators},
        context{context},
        alpha{alpha},
        A{operators.size(), MatrixXd::Identity(context.size(), context.size())},
        b{operators.size(), VectorXd::Zero(context.size())},
        theta{operators.size(), VectorXd{context.size()}},
        x{context.size()},
        p{VectorXd::Constant(operators.size(), 0.5)} {
    assert(alpha >= 0);
  }

  void update(){};

  void feedback(const double cf, const double pf) {
    double reward = pf - cf;
    std::vector<double> features = context.compute();
    x = Eigen::Map<VectorXd>(features.data(), features.size());

    A[opIdx] = A[opIdx] + x * x.transpose();
    b[opIdx] = b[opIdx] + reward * x;
    theta[opIdx] = A[opIdx].colPivHouseholderQr().solve(b[opIdx]);

    double prod = x.transpose() * A[opIdx] * x;
    p(opIdx) = theta[opIdx].transpose() * x + alpha * std::sqrt(prod);
  }

  std::ostream& printOn(std::ostream& os) {
    os << "  strategy: LinUCB MAB\n"
       << "  alpha: " << alpha << '\n';
    return os;
  }

  using OperatorSelection<OpT>::getOperator;

  OpT& selectOperator() final override {
    p.maxCoeff(&opIdx);
    return getOperator(opIdx);
  }
};