#pragma once

#include <eoInit.h>
#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/MHParamsValues.hpp"
#include "flowshop-solver/eoFactory.hpp"
#include "flowshop-solver/heuristics/FSPOrderHeuristics.hpp"
#include "flowshop-solver/heuristics/NEH.hpp"
#include "flowshop-solver/heuristics/acceptCritTemperature.hpp"
#include "flowshop-solver/problems/FSPProblem.hpp"
#include "problems/FSP.hpp"
#include "problems/FSPProblem.hpp"

class eoFSPFactory : public eoFactory<FSPProblem::Ngh> {
  FSPProblem& _problem;

 public:
  eoFSPFactory(const MHParamsValues& params, FSPProblem& problem)
      : eoFactory<FSPProblem::Ngh>{params, problem}, _problem{problem} {};

  using EOT = FSPProblem::EOT;
  using Ngh = FSPProblem::Ngh;

 protected:
  auto domainAcceptanceCriterion() -> moAcceptanceCriterion<Ngh>* override {
    const std::string name = categoricalName(".Accept");
    if (name == "temperature") {
      const int noJobs = _problem.getData().noJobs();
      const int noMachines = _problem.getData().noMachines();
      const int maxCT = _problem.getData().maxCT();
      const double tempScale = maxCT / (10.0 * noJobs * noMachines);
      const double temperature =
          params().real("IG.Accept.Temperature") * tempScale;
      const std::string name = params().categoricalName("IG.Accept");
      return &pack<acceptCritTemperature<Ngh>>(temperature);
    }
    return nullptr;
  }

  auto domainInit() -> eoInit<EOT>* override {
    const std::string name = categoricalName(".Init");
    if (name == "neh") {
      auto nehPriority = categoricalName(".Init.NEH.Priority");
      auto nehPriorityOrder = categoricalName(".Init.NEH.PriorityOrder");
      auto nehPriorityWeighted = integer(".Init.NEH.PriorityWeighted");

      auto priority = buildPriority(_problem.data(), nehPriority,
                                    nehPriorityWeighted, nehPriorityOrder);
      storeFunctor(priority);

      auto nehInsertion = categoricalName(".Init.NEH.Insertion");
      auto insertion =
          buildInsertionStrategy(nehInsertion, _problem.neighborEval());
      storeFunctor(insertion);

      return &pack<NEH<Ngh>>(*priority, *insertion);
    }
    return nullptr;
  }
};