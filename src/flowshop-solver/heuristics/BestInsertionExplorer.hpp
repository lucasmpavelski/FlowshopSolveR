#pragma once

#include <algorithm>

#include "global.hpp"
#include "heuristics/neighborhood_checkpoint.hpp"
#include "paradiseo/mo/comparator/moNeighborComparator.h"
#include "paradiseo/mo/explorer/moNeighborhoodExplorer.h"
#include "paradiseo/mo/problems/permutation/moShiftNeighbor.h"

template <class EOT>
class BestInsertionExplorer
    : public moNeighborhoodExplorer<myShiftNeighbor<EOT>> {
  using Ngh = myShiftNeighbor<EOT>;

  NeigborhoodCheckpoint<Ngh>& neighborhoodCheckpoint;
  moNeighborComparator<Ngh>& neighborComparator;
  moSolNeighborComparator<Ngh>& solNeighborComparator;

  // true if the solution has changed
  moEval<Ngh>& neighborEval;
  bool improve;
  bool LO;
  std::vector<int> RandJOB;
  int k;

 public:
  BestInsertionExplorer(moEval<Ngh>& neighborEval,
                        NeigborhoodCheckpoint<Ngh>& neighborhoodCheckpoint,
                        moNeighborComparator<Ngh>& neighborComparator,
                        moSolNeighborComparator<Ngh>& solNeighborComparator)
      : moNeighborhoodExplorer<Ngh>{},
        neighborhoodCheckpoint{neighborhoodCheckpoint},
        neighborComparator{neighborComparator},
        solNeighborComparator{solNeighborComparator},
        neighborEval{neighborEval} {}

  virtual void initParam(EOT& _solution) final override {
    improve = false;
    LO = false;
    RandJOB.resize(_solution.size());
    std::iota(RandJOB.begin(), RandJOB.end(), 0);
    std::shuffle(RandJOB.begin(), RandJOB.end(),
                 ParadiseoRNGFunctor<unsigned int>());
    k = 0;
  }

  virtual void updateParam(EOT& _solution) final override {
    if (k < RandJOB.size() - 1)
      k++;
    else {
      k = 0;
      std::shuffle(RandJOB.begin(), RandJOB.end(),
                   ParadiseoRNGFunctor<unsigned int>());
    }
    if (k == 0 && !improve) {
      LO = true;
    }
    //		improve=!improve;
    improve = false;
  }

  virtual void operator()(EOT& _solution) final override {
    const unsigned n = _solution.size();
    EOT tmp = _solution;
    auto insertPosition = std::find(tmp.begin(), tmp.end(), RandJOB[k]);
    std::rotate(tmp.begin(), insertPosition, insertPosition + 1);

    int neighborIdx =
        positionPairToKey(std::distance(tmp.begin(), insertPosition), 0, n);
    Ngh neighbor;
    neighbor.index(neighborIdx);
    neighborEval(_solution, neighbor);
    Ngh bestNeighbor = neighbor;

    // Ngh neighbor, bestNeighbor;
    // bestNeighbor.fitness(std::numeric_limits<double>::max());
    int bestPosition = 0;
    for (unsigned position = 1; position < n; position++) {
      neighbor.set(0, position, n);
      neighbor.invalidate();
      neighborEval(tmp, neighbor);
      if (neighborComparator(bestNeighbor, neighbor)) {
        bestPosition = position;
        bestNeighbor = neighbor;
      }
    }
    if (solNeighborComparator(_solution, bestNeighbor)) {
      _solution = tmp;
      if (bestPosition != 0) {
        bestNeighbor.move(_solution);
      }
      _solution.fitness(bestNeighbor.fitness());
      improve = true;
    }
  }

  virtual bool isContinue(EOT& _solution) final override { return !LO; }
  virtual void move(EOT& _solution) final override {}
  virtual bool accept(EOT& _solution) final override { return false; }
  virtual void terminate(EOT& _solution) final override {}
};
