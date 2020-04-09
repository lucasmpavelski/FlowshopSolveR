#pragma once

#include <algorithm>

#include <paradiseo/mo/mo>

#include "flowshop-solver/global.hpp"
#include "flowshop-solver/heuristics/neighborhood_checkpoint.hpp"
#include "flowshop-solver/problems/FSP.hpp"

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
  unsigned k;

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

  void initParam(EOT& _solution) final {
    improve = false;
    LO = false;
    RandJOB = _solution;
    std::shuffle(RandJOB.begin(), RandJOB.end(),
                 ParadiseoRNGFunctor<unsigned int>());
    k = 0;
  }

  void updateParam(EOT&) final {
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

  void operator()(EOT& _solution) final {
    const auto n = static_cast<int>(_solution.size());
    EOT tmp = _solution;
    auto insertPtr = std::find(tmp.begin(), tmp.end(), RandJOB[k]);
    int insertPosition = std::distance(tmp.begin(), insertPtr);
    // std::rotate(tmp.begin(), insertPosition, insertPosition + 1);

    Ngh neighbor, bestNeighbor;

    // Ngh neighbor, bestNeighbor;
    // bestNeighbor.fitness(std::numeric_limits<double>::max());
    int bestPosition = -1;
    for (int position = 0; position <= n; position++) {
      if (insertPosition == position)
        continue;
      neighbor.set(insertPosition, position, n);
      neighbor.invalidate();
      neighborEval(tmp, neighbor);
      if (bestNeighbor.invalid() ||
          neighborComparator(bestNeighbor, neighbor)) {
        bestPosition = position;
        bestNeighbor = neighbor;
      }
    }
    if (solNeighborComparator(_solution, bestNeighbor)) {
      bestNeighbor.move(_solution);
      _solution.fitness(bestNeighbor.fitness());
      improve = true;
    }
  }

  auto isContinue(EOT&) -> bool final { return !LO; }
  void move(EOT&) final {}
  auto accept(EOT&) -> bool final { return false; }
  void terminate(EOT&) final {}
};
