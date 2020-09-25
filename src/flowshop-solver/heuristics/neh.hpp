#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>
#include <string>

#include "flowshop-solver/heuristics.hpp"
#include "flowshop-solver/RunOptions.hpp"
#include "flowshop-solver/eoFactory.hpp"

template <class Ngh, class EOT = typename Problem<Ngh>::EOT>
auto solveWithNEH(Problem<Ngh>& prob,
                 eoFactory<Ngh>& factory,
                 const RunOptions& runOptions) -> Result {
  auto init = factory.buildInit();
  auto algo = moDummyLS<Ngh>(prob.eval());
  return runExperiment(*init, algo, prob, runOptions);
}
