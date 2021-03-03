#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/fla/LONSampling.hpp"
#include "flowshop-solver/fla/LocalOptimaNetwork.hpp"
#include "flowshop-solver/problems/FSPProblem.hpp"

#include "flowshop-solver/MHParamsSpecs.hpp"
#include "flowshop-solver/MHParamsValues.hpp"
#include "flowshop-solver/eoFSPFactory.hpp"

#include "flowshop-solver/FSPProblemFactory.hpp"
#include "flowshop-solver/MHParamsSpecsFactory.hpp"

template<class EOT>
class LONBuilder {
  LocalOptimaNetwork<EOT>& lon;
  EOT storedFromNode;
  EOT storedSampleStart;
  int storedNoSteps;

  public:

  explicit LONBuilder(LocalOptimaNetwork<EOT>& lon) : lon(lon) {}

  void setFromNode(EOT& fromNode) {
    storedFromNode = fromNode;
    lon.addNode(fromNode, storedSampleStart, storedNoSteps);
  }

  void setSampleStart(EOT& sampleStart) {
    storedSampleStart = sampleStart;
    storedNoSteps = 0;
  }

  void incrementNoSteps() {
    storedNoSteps++;
  }

  void setToNode(EOT& toNode) {
    if (storedFromNode.size() > 0) {
      lon.addNode(toNode, storedSampleStart, storedNoSteps);
      auto edge = lon.getEdge(storedFromNode, toNode);
      if (edge != nullptr) {
        edge->weight++;
      } else {
        lon.addEdge(storedFromNode, toNode, 1);
      }
    }
  }
};


template <class EOT>
class LONBuilderStat : public moStatBase<EOT> {
  LONBuilder<EOT>& lonBuilder;

 public:
  LONBuilderStat(LONBuilder<EOT>& lonBuilder) : lonBuilder(lonBuilder) {}

  void init(EOT&) override {}

  void operator()(EOT& sol) override { 
      lonBuilder.setFromNode(sol);
  }

  void lastCall(EOT&) override { }
};

template <class EOT>
class LocalLONBuilderStat : public moStatBase<EOT> {
  LONBuilder<EOT>& lonBuilder;

 public:
  LocalLONBuilderStat(LONBuilder<EOT>& lonBuilder) : lonBuilder(lonBuilder) {}

  void init(EOT& sol) override {
      lonBuilder.setSampleStart(sol);
  }

  void operator()(EOT&) override { 
      lonBuilder.incrementNoSteps();
  }

  void lastCall(EOT& sol) override { 
      lonBuilder.setToNode(sol);
  }
};

class MarkovChainLONSampling : public LONSampling {

public:
    MarkovChainLONSampling() = default;

  auto sampleLON(
      const std::unordered_map<std::string, std::string>& prob_params,
      const std::unordered_map<std::string, std::string>& sampling_params,
      unsigned seed) -> LocalOptimaNetwork<FSPProblem::EOT> override {
    rng.reseed(seed);
    FSPProblem problem = FSPProblemFactory::get(prob_params);
    using EOT = FSPProblem::EOT;
    using Ngh = FSPProblem::Ngh;

    MHParamsSpecs specs = MHParamsSpecsFactory::get("MarkovChainLONSampling");
    MHParamsValues params(&specs);
    params.readValues(sampling_params);

    eoFSPFactory factory(params, problem);
    eoInit<EOT>* init = factory.buildInit();
    moLocalSearch<Ngh>* localSearch = factory.buildLocalSearch();
    moPerturbation<Ngh>* perturbation = factory.buildPerturb();
    moAcceptanceCriterion<Ngh>* accept = factory.buildAcceptanceCriterion();

    LocalOptimaNetwork<EOT> fullLon;
    const int noSamples = params.integer("MarkovChainLONSampling.NumberOfSamples");
    for (int i = 0; i < noSamples; i++) {
      LocalOptimaNetwork<EOT> lon;
      LONBuilder<EOT> lonBuilder(lon);

      moTrueContinuator<Ngh> localStopCondition;
      LocalLONBuilderStat localLonBuilder(lonBuilder);
      problem.checkpoint().add(localLonBuilder);

      moIterContinuator<Ngh> stopCondition(params.integer("MarkovChainLONSampling.NumberOfIterations"), false);
      moCheckpoint<Ngh> checkpoint(stopCondition);

      LONBuilderStat<EOT> globalLonBuilder(lonBuilder);
      checkpoint.add(globalLonBuilder);
      
      moILS<Ngh, Ngh> ils(*localSearch, problem.eval(), checkpoint, *perturbation, *accept);

      EOT sol;
      (*init)(sol);
      stopCondition.init(sol);
      ils(sol);

      fullLon.merge(lon);
    }
    return fullLon;
  }
};