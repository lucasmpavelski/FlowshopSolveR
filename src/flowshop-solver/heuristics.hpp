#pragma once

#include <paradiseo/eo/eoInit.h>
#include <paradiseo/mo/algo/moLocalSearch.h>
#include <paradiseo/mo/continuator/moContinuator.h>

#include "global.hpp"
#include "problems/Problem.hpp"

struct Result {
  double fitness = 0, no_evals = 0, time = 0;

  friend bool operator==(const Result& a, const Result& b) {
    return a.fitness == b.fitness && a.no_evals == b.no_evals;
  }
};

inline std::ostream& operator<<(std::ostream& os, const Result& res) {
  return os << "fitness: " << res.fitness << '\n'
            << "no_evals: " << res.no_evals << '\n'
            << "time: " << res.time << '\n';
}

template <class Ngh, class EOT = typename Ngh::EOT>
Result runExperiment(eoInit<EOT>& init,
                     moLocalSearch<Ngh>& algo,
                     Problem<Ngh>& prob) {
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

inline int getNhSize(int N, double proportion) {
  const int max_nh_size = pow(N - 1, 2);
  const int min_nh_size = (N >= 20) ? 11 : 2;
  const int nh_interval = (N >= 20) ? 10 : 1;
  const int no_nh_sizes = (max_nh_size - min_nh_size) / nh_interval;
  const int scale_lsps = (no_nh_sizes + 1) * proportion / 10.0;
  return std::min(max_nh_size, min_nh_size + scale_lsps * nh_interval);
};