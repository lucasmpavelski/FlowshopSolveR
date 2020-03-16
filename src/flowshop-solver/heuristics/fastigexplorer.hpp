#pragma once

#include <algorithm>

#include <neighborhood/moNeighborhood.h>
#include <utils/eoRNG.h>
#include "explorer/moNeighborhoodExplorer.h"
#include "paradiseo/mo/comparator/moNeighborComparator.h"

#include "global.hpp"
#include "problems/fastfspeval.hpp"

class FastIGexplorer : public moNeighborhoodExplorer<moShiftNeighbor<FSP>> {
 public:
  using EOT = FSP;
  using Ngh = moShiftNeighbor<FSP>;

  FastIGexplorer(moEval<Ngh>& neighborEval,
                 moNeighborComparator<Ngh> neighborComparator,
                 moSolNeighborComparator<Ngh> solNeighborComparator)
      : moNeighborhoodExplorer<Ngh>(),
        neighborComparator(neighborComparator),
        solNeighborComparator(solNeighborComparator),
        neighborEval(neighborEval) {}

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

  virtual void terminate(EOT& _solution) final override {}

  virtual void operator()(EOT& _solution) final override {
    unsigned j = 0;
    while (_solution[j] != RandJOB[k]) {
      j++;
    }
    unsigned jobToInsert = j;
    Ngh neighbor, bestNeighbor;
    bestNeighbor.fitness(std::numeric_limits<double>::max());
    for (unsigned position = 0; position < _solution.size(); position++) {
      if (jobToInsert == position)
        continue;
      neighbor.index(
          positionPairToKey(jobToInsert, position, _solution.size()));
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

  std::string className() const { return "IGexplorer"; }

 private:
  moNeighborComparator<Ngh> neighborComparator;
  moSolNeighborComparator<Ngh> solNeighborComparator;

  // true if the solution has changed
  moEval<Ngh>& neighborEval;
  bool improve;
  bool LO;
  std::vector<int> RandJOB;
  int k;
};

template <class Ngh, typename EOT = typename Ngh::EOT>
class FastOpPerturbDestConst : public eoMonOp<EOT> {
 public:
  FastOpPerturbDestConst(moEval<Ngh>& neighborEval,
                         unsigned destructionSize,
                         moNeighborComparator<Ngh> neighborComparator)
      : neighborEval(neighborEval),
        destructionSize(destructionSize),
        neighborComparator(neighborComparator) {}

  virtual bool operator()(EOT& sol) final override {
    int index;
    int length = sol.size();
    std::vector<int> D;
    EOT tmp;
    destructionSize = std::min(destructionSize, sol.size());
    for (int k = 0; k < destructionSize; k++) {
      index = rng.random(sol.size() - k);
      D.push_back(sol[index]);
      sol.erase(sol.begin() + index);
    }
    for (int k = 0; k < destructionSize; k++) {
      unsigned jobToInsert = D[k];
      sol.push_back(jobToInsert);
      Ngh neighbor, bestNeighbor;
      bestNeighbor.fitness(std::numeric_limits<double>::max());
      for (unsigned position = 0; position <= sol.size(); position++) {
        neighbor.setPositions(sol.size() - 1, position);
        neighbor.invalidate();
        neighborEval(sol, neighbor);
        if (neighborComparator(bestNeighbor, neighbor)) {
          bestNeighbor = neighbor;
        }
      }
      bestNeighbor.move(sol);
    }
    return true;
  }

 private:
  moEval<Ngh>& neighborEval;
  unsigned destructionSize;
  moNeighborComparator<Ngh> neighborComparator;
};
