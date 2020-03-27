#pragma once

#include <algorithm>

#include "global.hpp"
#include "paradiseo/mo/comparator/moNeighborComparator.h"
#include "paradiseo/mo/explorer/moNeighborhoodExplorer.h"
#include "paradiseo/mo/problems/permutation/moShiftNeighbor.h"

template <class EOT>
class BestInsertionExplorer
    : public moNeighborhoodExplorer<moShiftNeighbor<EOT>> {
  using Ngh = moShiftNeighbor<EOT>;

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
    RandJOB.resize(n);
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
    unsigned jobToInsert = 0;
    while (_solution[jobToInsert] != RandJOB[k]) {
      jobToInsert++;
    }
    Ngh neighbor, bestNeighbor;
    bestNeighbor.fitness(std::numeric_limits<double>::max());
    for (unsigned position = 0; position < n; position++) {
      if (jobToInsert == position)
        continue;
      int neighborIdx = positionPairToKey(jobToInsert, position, n);
      neighbor.index(neighborIdx);
      neighbor.invalidate();
      neighborEval(_solution, neighbor);
      if (neighborComparator(bestNeighbor, neighbor)) {
        bestNeighbor = neighbor;
      }
    }
    if (solNeighborComparator(_solution, bestNeighbor)) {
      bestNeighbor.move(_solution);
      _solution.fitness(bestNeighbor.fitness());
      improve = true;
    }
  }

  virtual bool isContinue(EOT& _solution) final override { return !LO; }
  virtual void move(EOT& _solution) final override {}
  virtual bool accept(EOT& _solution) final override { return false; }
  virtual void terminate(EOT& _solution) final override {}
  std::string className() final override const { return "IGexplorer"; }
};
