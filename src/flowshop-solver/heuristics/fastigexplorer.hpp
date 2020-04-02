#pragma once

#include <algorithm>

#include "continuators/myMovedSolutionStat.hpp"

template <class Ngh, typename EOT = typename Ngh::EOT>
class DestructionConstruction : public eoMonOp<EOT> {
  moEval<Ngh>& neighborEval;
  unsigned _destructionSize;
  myMovedSolutionStat<EOT>& movedStat;
  moNeighborComparator<Ngh>& neighborComparator;
  moNeighborComparator<Ngh> defaultComparator;

 public:
  DestructionConstruction(moEval<Ngh>& neighborEval,
                          unsigned _destructionSize,
                          myMovedSolutionStat<EOT>& movedStat,
                          moNeighborComparator<Ngh>& neighborComparator)
      : neighborEval{neighborEval},
        neighborComparator{neighborComparator},
        movedStat{movedStat} {
    destructionSize(_destructionSize);
  }

  DestructionConstruction(moEval<Ngh>& neighborEval,
                          unsigned _destructionSize,
                          myMovedSolutionStat<EOT>& movedStat)
      : DestructionConstruction{neighborEval, _destructionSize, movedStat,
                                defaultComparator} {}

  int destructionSize() const { return _destructionSize; }
  void destructionSize(int destructionSize) {
    _destructionSize = destructionSize;
  }

  std::vector<int> destruction(EOT& sol) {
    int n = sol.size();
    std::vector<int> removed;
    int ds = std::min(destructionSize(), n);
    for (int k = 0; k < ds; k++) {
      int index = rng.random(sol.size());
      removed.push_back(sol[index]);
      sol.erase(sol.begin() + index);
    }
    return removed;
  }

  void construction(EOT& sol, std::vector<int> jobsToInsert) {
    const auto n = sol.size();
    for (const auto& jobToInsert : jobsToInsert) {
      sol.push_back(jobToInsert);
      Ngh neighbor, bestNeighbor;
      bestNeighbor.fitness(std::numeric_limits<double>::max());
      int bestPosition = -1;
      for (unsigned position = 0; position < sol.size(); position++) {
        neighbor.set(sol.size() - 1, position, sol.size());
        neighborEval(sol, neighbor);
        if (neighborComparator(bestNeighbor, neighbor)) {
          bestNeighbor = neighbor;
          bestPosition = position;
        }
      }
      if (bestPosition != 0)
        bestNeighbor.move(sol);
      movedStat(sol);
      sol.fitness(bestNeighbor.fitness());
    }
  }

  virtual bool operator()(EOT& sol) final override {
    auto removed = destruction(sol);
    construction(sol, removed);
    return true;
  }
};
