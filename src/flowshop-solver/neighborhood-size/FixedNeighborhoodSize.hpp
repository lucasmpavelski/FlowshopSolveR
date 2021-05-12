#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/neighborhood-size/NeighborhoodSize.hpp"

class FixedNeighborhoodSize : public NeighborhoodSize {
  const int size;

 public:
  FixedNeighborhoodSize(int size) : size(size) {}

  auto getSize() -> int override { return size; }
};
