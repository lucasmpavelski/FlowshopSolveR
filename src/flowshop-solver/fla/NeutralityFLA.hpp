#pragma once

#include <paradiseo/mo/mo>

#include "flowshop-solver/fla/FitnessLandscapeMetric.hpp"
#include "flowshop-solver/heuristics/neighborhood_stat.hpp"

template <class Ngh, class EOT = typename Ngh::EOT>
class NeutralityFLA : public NeigborhoodStat<Ngh, double>,
                      public FitnessLandscapeMetric {
  moSolNeighborComparator<Ngh> compare;
  EOT currentFitness;
  int equalCount = 0, totalCount = 0;

 public:
  NeutralityFLA()
      : NeigborhoodStat<Ngh, double>{-1.0,
                                     "Average normalized neutrality degree"} {}

  using NeigborhoodStat<Ngh, double>::value;

  void init(EOT&) final {
    value() = -1.0;
    equalCount = totalCount = 0;
    currentFitness.invalidate();
  }

  void initNeighborhood(EOT& sol) final {
    currentFitness.fitness(sol.fitness());
  }

  void neighborCall(Ngh& sol) final {
    if (compare.equals(currentFitness, sol)) {
      equalCount++;
    }
    totalCount++;
  }

  void lastCall(EOT&) final { value() = double(equalCount) / totalCount; }

  void operator()(EOT&) final {}

  auto compute() -> double final { return value(); };
};