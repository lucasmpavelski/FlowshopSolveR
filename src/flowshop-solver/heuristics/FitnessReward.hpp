#pragma once

#include <stdexcept>
#include <utility>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/continuators/myTimeStat.hpp"
#include "flowshop-solver/heuristics/neighborhood_checkpoint.hpp"

template <class EOT>
struct FitnessPair : public moStat<EOT, std::pair<double, double>> {
  FitnessPair()
      : moStat<EOT, std::pair<double, double>>{
            std::make_pair(-1.0, -1.0),
            "Pair of the initial and final fitness"} {}

  using moStat<EOT, std::pair<double, double>>::value;

  void init(EOT& sol) final {
    if (sol.invalid()) {
      return;
    }
    value().first = sol.fitness();
  }

  void operator()(EOT&) final {
  }

  void lastCall(EOT& sol) final {
    if (sol.invalid())
      return;
    value().second = sol.fitness();
  }
};


template <class EOT>
struct FitnessPairTime : public moStat<EOT, std::pair<double, double>> {
  FitnessPairTime()
      : moStat<EOT, std::pair<double, double>>{
            std::make_pair(-1.0, -1.0),
            "Pair of the initial and final fitness"} {}

  using moStat<EOT, std::pair<double, double>>::value;


  void operator()(EOT& sol) final {
    value().second = value().first;
    value().first = sol.fitness();
  }
};

template <class EOT>
class FitnessRewards : public eoFunctorBase {
  FitnessPair<EOT> local;
  FitnessPairTime<EOT> global;

  auto throwIfInvalid(double val) const {
    if (val == -1)
      throw std::runtime_error("invalid fitness in reward computation");
    return val;
  }

 public:
  auto localStat() -> moStatBase<EOT>& { return local; }

  auto globalStat() -> moStatBase<EOT>& { return global; }

  [[nodiscard]] auto initialLocal() const -> double {
    return throwIfInvalid(local.value().first);
  }

  [[nodiscard]] auto lastLocal() const -> double {
    return throwIfInvalid(local.value().second);
  }

  [[nodiscard]] auto initialGlobal() const -> double {
    return throwIfInvalid(global.value().second);
  }

  [[nodiscard]] auto lastGlobal() const -> double {
    return throwIfInvalid(global.value().first);
  }

  [[nodiscard]] auto reward(int rewardType) -> double {
    double pf, cf;
    switch (rewardType) {
      case 0:
        pf = initialGlobal();
        cf = lastGlobal();
        break;
      case 1:
        pf = initialGlobal();
        cf = lastLocal();
        break;
      case 2:
        pf = initialLocal();
        cf = lastGlobal();
        break;
      case 3:
        pf = initialLocal();
        cf = lastLocal();
        break;
      default:
        throw std::runtime_error{"invalid rewardType"};
    }
    return (pf - cf) / pf;
  }
};
