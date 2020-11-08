#pragma once

#include <neighborhood/moRndWithoutReplNeighborhood.h>
#include <vector>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

template <class Ngh, class EOT = typename Ngh::EOT>
class SampleRandomWalk {
  using ivec = std::vector<int>;
  const int solSize;
  eoInitPermutation<EOT> init;
  eoEvalFunc<EOT>& fullEval;
  moFullEvalByCopy<Ngh> neighborEval;
  moRndWithoutReplNeighborhood<Ngh> neighborhood;

 public:
  SampleRandomWalk(const int solSize,
                   eoEvalFunc<EOT>& fullEval,
                   moEval<Ngh>& neighborEval)
      : solSize(solSize),
        init(solSize),
        fullEval(fullEval),
        neighborEval(fullEval),
        neighborhood((solSize - 1) * (solSize - 1)) {}

  auto sample(const unsigned noSamples, const std::string& samplingStrat)
      -> std::vector<double> {
    if (samplingStrat == "RANDOM")
      return sampleStrat(moAutocorrelationSampling<Ngh>(
          init, neighborhood, fullEval, neighborEval, noSamples));
    else if (samplingStrat == "MH")
      return sampleStrat(moMHRndFitnessCloudSampling<Ngh>(
          init, neighborhood, fullEval, neighborEval, noSamples));
    else
      assert(false);
    return {};
  }

 private:
  auto sampleStrat(moSampling<Ngh>&& sampleStrat) -> std::vector<double> {
    sampleStrat();
    return sampleStrat.getValues(0);
  }
};