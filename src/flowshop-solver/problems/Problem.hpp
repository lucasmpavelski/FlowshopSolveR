#pragma once

#include <paradiseo/eo/eoEvalFuncCounter.h>
#include <paradiseo/mo/continuator/moBestSoFarStat.h>
#include <paradiseo/mo/continuator/moCheckpoint.h>
#include <paradiseo/mo/continuator/moContinuator.h>
#include <paradiseo/mo/eval/moEval.h>

/**
 * Optimization problem interface
 */
template <class Ngh, class EOT = typename Ngh::EOT>
struct Problem {
  virtual eoEvalFunc<EOT>& eval() = 0;
  virtual moEval<Ngh>& neighborEval() = 0;
  virtual moContinuator<Ngh>& continuator() = 0;
  virtual moCheckpoint<Ngh>& checkpoint() = 0;
  virtual moCheckpoint<Ngh>& checkpointGlobal() = 0;
  virtual moBestSoFarStat<EOT>& bestLocalSoFar() = 0;
  virtual moBestSoFarStat<EOT>& bestSoFar() = 0;
  virtual int size(int i = 0) const = 0;
  virtual void reset() = 0;
  virtual double upperBound() const = 0;
  virtual int noEvals() const = 0;
};
