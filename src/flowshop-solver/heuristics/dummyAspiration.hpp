#pragma once

#include <memory/moAspiration.h>

/**
 * Aspiration criteria accepts a solution better than the best so far
 */
template <class Neighbor>
class dummyAspiration : public moAspiration<Neighbor> {
 public:
  typedef typename Neighbor::EOT EOT;
  void init(EOT&) {}
  void update(EOT&, Neighbor&) {}
  bool operator()(EOT&, Neighbor&) { return false; }
};
