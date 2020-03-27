#pragma once

#include "paradiseo/mo/continuator/moStat.h"

template <class EOT>
class myMovedSolutionStat : public moStat<EOT, bool> {
 public:
  myMovedSolutionStat()
      : moStat<EOT, bool>{
            true,
            "Indicates whether the solution was moved since the last "
            "iteration"} {};

  using moStat<EOT, bool>::value;

  void init(EOT& sol) final override { value() = true; }
  void operator()(EOT& sol) final override { value() = true; }
  void reset() { value() = false; }
};