#pragma once

#include <eoFunctorStore.h>
#include <type_traits>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/MHParamsValues.hpp"
#include "flowshop-solver/problems/Problem.hpp"

template <class Ngh, class EOT = typename Ngh::EOT>
class eoFactory : public eoFunctorStore {
  const MHParamsValues& _params;
  Problem<Ngh>& _problem;

 protected:
  virtual auto domainInit() -> eoInit<EOT>* { return nullptr; }
  virtual auto domainAcceptanceCriterion() -> moAcceptanceCriterion<Ngh>* {
    return nullptr;
  }

 public:
  template <class T>
  using ptr = std::unique_ptr<T>;

  eoFactory(const MHParamsValues& params, Problem<Ngh>& problem)
      : _params{params}, _problem{problem} {};

  [[nodiscard]] auto params() const -> const MHParamsValues& { return _params; }

  auto problem() const -> const Problem<Ngh>& { return _problem; }

  [[nodiscard]] auto categoricalName(const std::string& name) const
      -> std::string {
    return _params.categoricalName(_params.mhName() + name);
  }

  [[nodiscard]] auto integer(const std::string& name) const -> int {
    return _params.integer(_params.mhName() + name);
  }

  auto buildInit() -> eoInit<EOT>* {
    const std::string init = categoricalName(".Init");
    if (init == "random")
      return &pack<eoInitPermutation<EOT>>(_problem.size(0));
    return domainInit();
  }

  auto buildAcceptanceCriterion() -> moAcceptanceCriterion<Ngh>* {
    const std::string name =
        params().categoricalName(params().mhName() + ".Accept");
    if (name == "always")
      return &pack<moAlwaysAcceptCrit<Ngh>>();
    else if (name == "better")
      return &pack<moBetterAcceptCrit<Ngh>>();
    return domainAcceptanceCriterion();
  }
};