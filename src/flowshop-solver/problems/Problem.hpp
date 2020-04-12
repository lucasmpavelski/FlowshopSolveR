#pragma once

#include <paradiseo/eo/eoEvalFuncCounter.h>
#include <paradiseo/mo/continuator/moBestSoFarStat.h>
#include <paradiseo/mo/continuator/moCheckpoint.h>
#include <paradiseo/mo/continuator/moContinuator.h>
#include <paradiseo/mo/eval/moEval.h>

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
  virtual auto bestLocalSoFar() -> moBestSoFarStat<EOT>& = 0;
  virtual auto bestSoFar() -> moBestSoFarStat<EOT>& = 0;
  virtual auto size(int i = 0) const -> int = 0;
  virtual void reset() = 0;
  virtual auto upperBound() const -> double = 0;
  virtual auto noEvals() const -> int = 0;
};
