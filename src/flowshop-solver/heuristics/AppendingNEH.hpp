#pragma once
#pragma once

#include <cmath>
#include <algorithm>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/global.hpp"
#include "flowshop-solver/heuristics/InsertionStrategy.hpp"
#include "flowshop-solver/heuristics/NEH.hpp"

template <class Ngh, class EOT = typename Ngh::EOT>
class AppendingNEH : public NEH<Ngh> {
  eoInit<EOT>& init;
  double ratio;

 public:
  AppendingNEH(eoInit<EOT>& init,
               eoInit<EOT>& nehInit,
               InsertionStrategy<Ngh>& insert,
               double ratio)
      : NEH<Ngh>{nehInit, insert}, init{init}, ratio{ratio} {}

  void operator()(EOT& sol) override {
    EOT order1 = sol;
    init(order1);

    EOT order2 = initialOrder(sol);

    const int appendIdx = std::round(order2.size() * ratio);
    sol.resize(0);
    for (int i = 0; i < appendIdx; i++) {
      auto job = order1[i];
      sol.push_back(job);
      auto nehOrderPtr = std::find(begin(order2), end(order2), job);
      order2.erase(nehOrderPtr);
    }

    construction(order2, sol);
  }

  protected:
  using NEH<Ngh>::initialOrder;
  using NEH<Ngh>::construction;
};
