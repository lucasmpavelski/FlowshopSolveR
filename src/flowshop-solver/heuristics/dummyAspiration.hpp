#pragma once

#include <memory/moAspiration.h>

/**
 * Aspiration criteria accepts a solution better than the best so far
 */
template <class Neighbor>
class dummyAspiration : public moAspiration<Neighbor> {
 public:
  using EOT = typename Neighbor::EOT;
  void init(EOT&) override {}
  void update(EOT&, Neighbor&) override {}
  bool operator()(EOT&, Neighbor&) override { return false; }
};
