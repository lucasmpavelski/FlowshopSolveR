#pragma once

#include "paradiseo/mo/comparator/moSolComparator.h"

#include "fla/FitnessLandscapeMetric.hpp"
#include "heuristics/neighborhood_stat.hpp"

template <class EOT>
class NeutralityFLA : public NeigborhoodStat<EOT, double>,
                      public FitnessLandscapeMetric {
  moSolComparator<EOT>& compare;
  EOT currentFitness;
  int equalCount = 0, totalCount = 0;

 public:
  NeutralityFLA(moSolComparator<EOT>& compare)
      : NeigborhoodStat<EOT, double>{-1.0,
                                     "Average normalized neutrality degree"},
        compare{compare} {}

  using NeigborhoodStat<EOT, double>::value;

  void init(EOT& sol) final override {
    value() = -1.0;
    equalCount = totalCount = 0;
    currentFitness.invalidate();
  }

  void initNeighborhood(EOT& sol) final override {
    currentFitness.fitness(sol.fitness());
  }

  void neighborCall(EOT& sol) final override {
    if (compare.equals(currentFitness, sol)) {
      equalCount++;
    }
    totalCount++;
  }

  void lastCall(EOT& sol) final override {
    value() = double(equalCount) / totalCount;
  }

  void operator()(EOT& sol) final override {}

  double compute() { return value(); };
};