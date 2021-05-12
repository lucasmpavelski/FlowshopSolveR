#pragma once

#include <paradiseo/eo/eoFunctor.h>
#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

class NeighborhoodSize : public eoFunctorBase {
 public:
  virtual auto getSize() -> int = 0;
};
