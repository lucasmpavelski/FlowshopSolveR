#pragma once

#include <memory/moAspiration.h>

/**
 * Aspiration criteria accepts a solution better than the best so far
 */
template <class Neighbor>
class dummyAspiration : public moAspiration<Neighbor> {
 public:
  typedef typename Neighbor::EOT EOT;
  void init(EOT& _sol) {}
  void update(EOT& _sol, Neighbor& _neighbor) {}
  bool operator()(EOT& _sol, Neighbor& _neighbor) { return false; }
};
