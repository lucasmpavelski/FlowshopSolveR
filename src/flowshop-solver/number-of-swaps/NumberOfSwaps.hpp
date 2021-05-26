#pragma once

#include <paradiseo/eo/eoFunctor.h>
#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

class NumberOfSwaps : public eoFunctorBase {
 public:
  virtual auto get() -> int = 0;
};
