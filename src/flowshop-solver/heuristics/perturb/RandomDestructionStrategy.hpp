#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/heuristics/perturb/DestructionConstruction.hpp"
#include "flowshop-solver/heuristics/perturb/DestructionStrategy.hpp"

template <class EOT>
class RandomDestructionStrategy : public DestructionStrategy<EOT> {
  DestructionSize& destructionSize;

 public:
  RandomDestructionStrategy(DestructionSize& destructionSize) : 
    destructionSize{destructionSize} {}

  auto operator()(EOT& sol) -> EOT override {
    int n = sol.size();
    EOT removed;
    int ds = std::min(destructionSize.value(), n);
    for (int k = 0; k < ds; k++) {
      int index = rng.random(sol.size());
      removed.push_back(sol[index]);
      sol.erase(sol.begin() + index);
    }
    return removed;
  }
};
