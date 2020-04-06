#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/global.hpp"
#include "flowshop-solver/problems/Problem.hpp"

struct Result {
  double fitness = 0, no_evals = 0, time = 0;

  friend auto operator==(const Result& a, const Result& b) -> bool {
    return a.fitness == b.fitness && a.no_evals == b.no_evals;
  }
};

inline auto operator<<(std::ostream& os, const Result& res) -> std::ostream& {
  return os << "fitness: " << res.fitness << '\n'
            << "no_evals: " << res.no_evals << '\n'
            << "time: " << res.time << '\n';
}

template <class Ngh, class EOT = typename Ngh::EOT>
auto runExperiment(eoInit<EOT>& init,
                   moLocalSearch<Ngh>& algo,
                   Problem<Ngh>& prob) -> Result {
  EOT sol;
  prob.checkpoint().init(sol);
  double time = Measure<>::execution([&init, &algo, &sol] {
    init(sol);
    algo(sol);
  });
  Result res;
  res.fitness = prob.bestSoFar().value().fitness();
  res.time = time;
  res.no_evals = prob.noEvals();
  return res;
}

inline auto getNhSize(int N, double proportion) -> int {
  const int max_nh_size = pow(N - 1, 2);
  const int min_nh_size = (N >= 20) ? 11 : 2;
  const int nh_interval = (N >= 20) ? 10 : 1;
  const int no_nh_sizes = (max_nh_size - min_nh_size) / nh_interval;
  const int scale_lsps = (no_nh_sizes + 1) * proportion / 10.0;
  return std::min(max_nh_size, min_nh_size + scale_lsps * nh_interval);
};