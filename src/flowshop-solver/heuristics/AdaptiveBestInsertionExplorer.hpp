#pragma once

#include <algorithm>

#include <paradiseo/mo/mo>

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/heuristics/BestInsertionExplorer.hpp"
#include "flowshop-solver/heuristics/neighborhood_checkpoint.hpp"
#include "flowshop-solver/problems/FSP.hpp"

#include "flowshop-solver/position-selector/PositionSelector.hpp"

template <class EOT>
class AdaptiveBestInsertionExplorer
    : public moNeighborhoodExplorer<myShiftNeighbor<EOT>> {
  using Ngh = myShiftNeighbor<EOT>;

  NeigborhoodCheckpoint<Ngh>& neighborhoodCheckpoint;
  moNeighborComparator<Ngh>& neighborComparator;
  moSolNeighborComparator<Ngh>& solNeighborComparator;

  moEval<Ngh>& neighborEval;

  bool improve;
  bool LO;
  int k;
  
  PositionSelector& positionSelector;

 public:
  AdaptiveBestInsertionExplorer(
      PositionSelector& positionSelector,
      moEval<Ngh>& neighborEval,
      NeigborhoodCheckpoint<Ngh>& neighborhoodCheckpoint,
      moNeighborComparator<Ngh>& neighborComparator,
      moSolNeighborComparator<Ngh>& solNeighborComparator)
      : moNeighborhoodExplorer<Ngh>{},
        neighborhoodCheckpoint{neighborhoodCheckpoint},
        neighborComparator{neighborComparator},
        solNeighborComparator{solNeighborComparator},
        neighborEval{neighborEval},
        positionSelector{positionSelector} {}

  void initParam(EOT& sol) final {
    improve = false;
    LO = false;
    k = 0;
    positionSelector.init(sol);
  }

  void updateParam(EOT& sol) final {
    if (k < sol.size() - 1) {
      k++;
      return;
    }
    if (!improve) {
      LO = true;
      return;
    }
    improve = false;
    k = 0;
    positionSelector.init(sol);
  }

  void operator()(EOT& sol) final {
    const auto n = static_cast<int>(sol.size());
    int insertPosition = positionSelector.select(sol);
    Ngh neighbor, bestNeighbor;

    bestNeighbor.fitness(std::numeric_limits<double>::max());
    int bestPosition = -1;
    neighborhoodCheckpoint.initNeighborhood(sol);
    for (int position = 0; position < n; position++) {
      if (insertPosition == position)
        continue;
      neighbor.set(insertPosition, position, n);
      neighbor.invalidate();
      neighborEval(sol, neighbor);
      if (bestNeighbor.invalid() ||
          neighborComparator(bestNeighbor, neighbor)) {
        bestPosition = position;
        bestNeighbor = neighbor;
      }
      neighborhoodCheckpoint.neighborCall(neighbor);
    }
    positionSelector.feedback((sol.fitness() - bestNeighbor.fitness()) / sol.fitness());
    if (solNeighborComparator(sol, bestNeighbor)) {
      bestNeighbor.move(sol);
      sol.fitness(bestNeighbor.fitness());
      improve = true;
    }
    neighborhoodCheckpoint.lastCall(sol);
  }

  auto isContinue(EOT&) -> bool final { return !LO; }
  void move(EOT&) final {}
  auto accept(EOT&) -> bool final { return true; }
  void terminate(EOT&) final {}
};
