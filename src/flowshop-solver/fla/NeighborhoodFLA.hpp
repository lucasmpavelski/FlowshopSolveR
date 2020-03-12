#pragma once

#include "flowshop-solvers/fla/FitnessLandscapeMetric.hpp"
#include "flowshop-solvers/fla/NeighborhoodFLA.hpp"

template <class EOT>
class NeighborhoodFLA : public FitnessLandscapeMetric {
  NeighborhoodFLA<EOT>& neighborhood;

  double compute() override { return compute(neighborhood); }

 protected:
  virtual double compute(NeighborhoodFLA& neighborhood) = 0;
};