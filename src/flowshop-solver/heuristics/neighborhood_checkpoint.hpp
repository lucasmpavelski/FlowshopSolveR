#pragma once

#include "paradiseo/mo/continuator/moCheckpoint.h"
#include "paradiseo/mo/continuator/moTrueContinuator.h"

#include "flowshop-solver/heuristics/neighborhood_stat.hpp"

template <class Neighbor, class EOT = typename Neighbor::EOT>
class NeigborhoodCheckpoint : public moCheckpoint<Neighbor> {
  std::vector<NeigborhoodStatBase<Neighbor>*> neighborhoodStats;
  moTrueContinuator<Neighbor> tc;

 public:
  NeigborhoodCheckpoint(moContinuator<Neighbor>& _cont,
                        unsigned int _interval = 1)
      : moCheckpoint<Neighbor>{_cont, _interval} {}

  NeigborhoodCheckpoint() : moCheckpoint<Neighbor>{tc, 1} {}

  void initNeighborhood(EOT& sol) {
    for (auto& ns : neighborhoodStats)
      ns->initNeighborhood(sol);
  }

  void neighborCall(Neighbor& neighbor) {
    for (auto& ns : neighborhoodStats)
      ns->neighborCall(neighbor);
  }

  void lastNeighborhoodCall(EOT& sol) {
    for (auto& ns : neighborhoodStats)
      ns->lastNeighborhoodCall(sol);
  }

  void add(NeigborhoodStatBase<Neighbor>& ns) {
    neighborhoodStats.push_back(&ns);
  }
};
