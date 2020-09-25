#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/RunOptions.hpp"
#include "flowshop-solver/continuators/myTimeStat.hpp"
#include "flowshop-solver/global.hpp"
#include "flowshop-solver/problems/Problem.hpp"
#include "flowshop-solver/heuristics/FitnessReward.hpp"

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

template <class EOT>
class myTimeFitnessPrinter : public moStatBase<EOT> {
  myTimeStat<EOT>& timer;
  EOT best;
  moSolComparator<EOT> compare;

 public:
  myTimeFitnessPrinter(myTimeStat<EOT>& timer)
      : moStatBase<EOT>{}, timer{timer} {
    best.fitness(std::numeric_limits<double>::max());
  }

  void init(EOT& sol) override {
    if (!sol.invalid())
      (*this)(sol);
  }

  void operator()(EOT& sol) final {
    if (compare(best, sol)) {
      timer(sol);
      std::cout << timer.value() << ',' << sol.fitness() << '\n';
      best.fitness(sol.fitness());
    }
  }
};

template <class Ngh, class EOT = typename Ngh::EOT>
auto runExperiment(eoInit<EOT>& init,
                   moLocalSearch<Ngh>& algo,
                   Problem<Ngh>& prob,
                   RunOptions options = RunOptions()) -> Result {
  myTimeStat<EOT> timer;
  myTimeFitnessPrinter<EOT> timeFitness{timer};
  if (options.printBestFitness) {
    std::puts("runtime,fitness");
    prob.checkpointGlobal().add(timeFitness);
  }
  // LocalFitnessReward<EOT> printReward{timer, options.printFitnessReward};
  // if (options.printFitnessReward) {
  //  prob.checkpoint().add(printReward);
  //}

  EOT sol;
  double time = Measure<>::execution([&]() {
    init(sol);
    if (sol.invalid()) {
      prob.eval()(sol);
    }
    prob.checkpoint().init(sol);
    prob.checkpointGlobal().init(sol);
    algo(sol);
    prob.checkpoint().lastCall(sol);
    prob.checkpointGlobal().lastCall(sol);

  });
  Result res;
  res.fitness = std::min(prob.bestSoFar().value().fitness(),
                         prob.bestLocalSoFar().value().fitness());
  res.time = time;
  res.no_evals = prob.noEvals();
  if (options.printLastFitness) {
    std::cout << res.fitness << ',' << res.time << ',' << res.no_evals << '\n';
  }
  return res;
}

inline auto getNhSize(int N, double proportion) -> int {
  const int max_nh_size = pow(N - 1, 2);
  const int min_nh_size = (N >= 20) ? 11 : 2;
  const int nh_interval = (N >= 20) ? 10 : 1;
  const int no_nh_sizes = 1 + (max_nh_size - min_nh_size) / nh_interval;
  const int scale_lsps = static_cast<int>(no_nh_sizes * proportion / 10.0);
  return std::min(max_nh_size, min_nh_size + scale_lsps * nh_interval);
}