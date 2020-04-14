#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/MHParamsValues.hpp"
#include "flowshop-solver/heuristics/acceptCritTemperature.hpp"

template <class Ngh, class EOT = typename Ngh::EOT>
class eoFactory {
  const MHParamsValues& _params;

 public:
  template <class T>
  using ptr = std::unique_ptr<T>;

  eoFactory(const MHParamsValues& params) : _params{params} {};

  auto params() const -> const MHParamsValues& { return _params; }

  virtual auto buildAcceptanceCriterion() -> ptr<moAcceptanceCriterion<Ngh>> {
    const std::string name = params().categoricalName("IG.Accept");
    if (name == "always") {
      return std::make_unique<moAlwaysAcceptCrit<Ngh>>();
    } else if (name == "better") {
      return std::make_unique<moBetterAcceptCrit<Ngh>>();
    } else {
      return {nullptr};
    }
  }
};