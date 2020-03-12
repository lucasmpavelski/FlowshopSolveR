#pragma once

#include <paradiseo/mo/continuator/moContinuator.h>

template <class Neighbor>
class falseContinuator : public moContinuator<Neighbor> {
 public:
  using EOT = typename Neighbor::EOT;

  bool operator()(EOT& _solution) final override { return false; }

  void init(EOT& _solution) final override {}
};
