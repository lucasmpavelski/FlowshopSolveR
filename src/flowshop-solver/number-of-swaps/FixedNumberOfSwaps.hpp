#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/number-of-swaps/NumberOfSwaps.hpp"

class FixedNumberOfSwaps : public NumberOfSwaps {
  const int size;

 public:
  FixedNumberOfSwaps(int size) : size(size) {}

  auto get() -> int override { return size; }
};
