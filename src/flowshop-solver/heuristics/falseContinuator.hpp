#pragma once

#include <paradiseo/mo/continuator/moContinuator.h>

template <class Neighbor>
class falseContinuator : public moContinuator<Neighbor> {
 public:
  using EOT = typename Neighbor::EOT;

  bool operator()(EOT&) final { return false; }

  void init(EOT&) final {}
};
