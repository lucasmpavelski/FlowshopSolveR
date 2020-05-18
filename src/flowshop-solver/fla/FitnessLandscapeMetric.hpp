#pragma once

#include <paradiseo/eo/eo>

class FitnessLandscapeMetric : public eoFunctorBase {
 public:
  virtual auto compute() -> double = 0;
};
