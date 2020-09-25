#pragma once
#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/global.hpp"
#include "flowshop-solver/heuristics/InsertionStrategy.hpp"

template <class Ngh, class EOT = typename Ngh::EOT>
class NEH : public eoInit<EOT> {
  eoInit<EOT>& init;
  InsertionStrategy<Ngh>& insert;

 public:
  NEH(eoInit<EOT>& init, InsertionStrategy<Ngh>& insert)
      : init(init), insert(insert) {}

  virtual void operator()(EOT& sol) {
    EOT tmp = initialOrder(sol);
    sol.resize(0);
    construction(tmp, sol);
  }

 protected:
  auto initialOrder(const EOT& sol) -> EOT {
    EOT tmp = sol;
    init(tmp);
    return tmp;
  }

  void construction(const EOT& order, EOT& sol) {
    for (auto& job : order)
      insert.insertJob(sol, job);
  }
};
