#pragma once

#include <paradiseo/eo/eo>

class FitnessLandscapeMetric {
 public:
  virtual auto compute() -> double = 0;
};
