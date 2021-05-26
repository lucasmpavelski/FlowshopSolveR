#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/number-of-swaps/NumberOfSwaps.hpp"

template <class EOT>
class ilsKickOp : public eoMonOp<EOT> {
  NumberOfSwaps& numberOfSwaps;

 public:
  ilsKickOp(NumberOfSwaps& numberOfSwaps)
      : numberOfSwaps(numberOfSwaps) {
  }

  virtual auto className() const -> std::string { return "eoSwapMutation"; }

  auto operator()(EOT& solution) -> bool {
    const unsigned n = solution.size();
    unsigned i, j;
    for (unsigned int swap = 0; swap < numberOfSwaps.get(); swap++) {
      // generate two different indices
      i = eo::rng.random(n);
      j = (i + 1) % n;
      // swap
      std::swap(solution[i], solution[j]);
    }
    i = eo::rng.random(n);
    j = (i + eo::rng.random(std::max<int>(n / 5, 30))) % n;
    std::swap(solution[i], solution[j]);
    solution.invalidate();
    return true;
  }
};