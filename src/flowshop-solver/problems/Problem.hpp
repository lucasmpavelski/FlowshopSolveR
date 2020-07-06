#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/heuristics/neighborhood_checkpoint.hpp"

/**
 * Optimization problem interface
 */
template <class Neighbor, class Sol = typename Neighbor::EOT>
struct Problem {
  using Ngh = Neighbor;
  using EOT = Sol;

  virtual auto eval() -> eoEvalFunc<EOT>& = 0;
  virtual auto neighborEval() -> moEval<Ngh>& = 0;
  virtual auto continuator() -> moContinuator<Ngh>& = 0;
  virtual auto checkpoint() -> moCheckpoint<Ngh>& = 0;
  virtual auto checkpointGlobal() -> moCheckpoint<Ngh>& = 0;
  virtual auto neighborhoodCheckpoint() -> NeigborhoodCheckpoint<Ngh>& = 0;
  virtual auto bestLocalSoFar() -> moBestSoFarStat<EOT>& = 0;
  virtual auto bestSoFar() -> moBestSoFarStat<EOT>& = 0;
  virtual void reset() = 0;
  
  [[nodiscard]] virtual auto size(int i = 0) const -> int = 0;
  [[nodiscard]] virtual auto upperBound() const -> double = 0;
  [[nodiscard]] virtual auto noEvals() const -> int = 0;
  [[nodiscard]] virtual auto getNeighborhoodSize(int size) const -> int = 0;

  [[nodiscard]] auto maxNeighborhoodSize() const -> int {
    return getNeighborhoodSize(size());
  }
};
