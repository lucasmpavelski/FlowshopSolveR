#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>
#include <string>

#include "flowshop-solver/heuristics.hpp"
#include "flowshop-solver/RunOptions.hpp"
#include "flowshop-solver/eoFactory.hpp"

template <class Ngh, class EOT = typename Problem<Ngh>::EOT>
auto solveWithIG(Problem<Ngh>& prob,
                 eoFactory<Ngh>& factory,
                 const RunOptions& runOptions) -> Result {
  auto init = factory.buildInit();
  auto algo = factory.buildLocalSearch();
  auto accept = factory.buildAcceptanceCriterion();
  auto perturb = factory.buildPerturb();
  moILS<Ngh, Ngh> ils(*algo, prob.eval(), prob.checkpointGlobal(), *perturb, *accept);
  return runExperiment(*init, ils, prob, runOptions);
}
