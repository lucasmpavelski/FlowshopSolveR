#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/MHParamsValues.hpp"
#include "heuristics/acceptCritTemperature.hpp"

template <class Ngh, class EOT = typename Ngh::EOT>
class eoFactory {
  const MHParamsValues& params;
  template <class T>
  using ptr = std::unique_ptr<T>;

 public:
  eoFactory(const MHParamsValues& params) : params{params} {};

  auto buildAcceptanceCriterion(double temp_scale = 1.0)
      -> ptr<moAcceptanceCriterion<Ngh>> {
    moBetterAcceptCrit<Ngh> accept1;
    const double temperature =
        params.real("IG.Accept.Temperature") * temp_scale;
    acceptCritTemperature<Ngh> accept2(temperature);
    const std::string name = params.categoricalName("IG.Accept");
    if (name == "always") {
      return std::make_unique<moAlwaysAcceptCrit<Ngh>>();
    } else if (name == "better") {
      return std::make_unique<moBetterAcceptCrit<Ngh>>();
    } else if (name == "temperature") {
      return std::make_unique<acceptCritTemperature<Ngh>>(temperature);
    } else {
      return {nullptr};
    }
  }
};