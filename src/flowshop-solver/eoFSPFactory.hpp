#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/MHParamsValues.hpp"
#include "flowshop-solver/eoFactory.hpp"
#include "problems/FSP.hpp"
#include "problems/FSPProblem.hpp"

class eoFSPFactory : public eoFactory<FSPProblem::Ngh> {
  const FSPProblem& _problem;

 public:
  eoFSPFactory(const MHParamsValues& params, const FSPProblem& problem)
      : eoFactory{params}, _problem{problem} {};

  using EOT = FSPProblem::EOT;
  using Ngh = FSPProblem::Ngh;
  template <class T>
  using ptr = typename eoFactory<FSPProblem::Ngh>::ptr<T>;

  auto buildAcceptanceCriterion() -> ptr<moAcceptanceCriterion<Ngh>> override {
    const int noJobs = _problem.getData().noJobs();
    const int noMachines = _problem.getData().noMachines();
    const int maxCT = _problem.getData().maxCT();
    const double tempScale = maxCT / (10.0 * noJobs * noMachines);
    const double temperature =
        params().real("IG.Accept.Temperature") * tempScale;
    const std::string name = params().categoricalName("IG.Accept");
    auto ret = eoFactory<Ngh>::buildAcceptanceCriterion();
    if (!ret && name == "temperature") {
      return std::make_unique<acceptCritTemperature<Ngh>>(temperature);
    }
    return ret;
  }
};