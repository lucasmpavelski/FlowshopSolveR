#pragma once

#include <algorithm>
#include <cassert>

#include <paradiseo/mo/mo>

#include "flowshop-solver/global.hpp"
#include "flowshop-solver/heuristics/neighborhood_checkpoint.hpp"
#include "flowshop-solver/problems/FSP.hpp"
#include "flowshop-solver/neighborhood-size/NeighborhoodSize.hpp"

enum class NeighborhoodType { random, ordered };

template <class CharT>
auto fromString(const std::basic_string<CharT> str) -> NeighborhoodType {
  if (str == "random")
    return NeighborhoodType::random;
  else if (str == "ordered")
    return NeighborhoodType::ordered;
  assert(false);
  return NeighborhoodType::ordered;
}

template <class EOT>
class BestInsertionExplorer
    : public moNeighborhoodExplorer<myShiftNeighbor<EOT>> {
  using Ngh = myShiftNeighbor<EOT>;

  NeigborhoodCheckpoint<Ngh>& neighborhoodCheckpoint;
  moNeighborComparator<Ngh>& neighborComparator;
  moSolNeighborComparator<Ngh>& solNeighborComparator;

  moEval<Ngh>& neighborEval;

  bool improve;
  bool LO;
  std::vector<int> RandJOB;
  unsigned k;
  NeighborhoodSize& neighborhoodSize;
  const NeighborhoodType neighborhoodType;

 public:
  BestInsertionExplorer(
      moEval<Ngh>& neighborEval,
      NeigborhoodCheckpoint<Ngh>& neighborhoodCheckpoint,
      moNeighborComparator<Ngh>& neighborComparator,
      moSolNeighborComparator<Ngh>& solNeighborComparator,
      NeighborhoodSize& neighborhoodSize,
      NeighborhoodType neighborhoodType = NeighborhoodType::random)
      : moNeighborhoodExplorer<Ngh>{},
        neighborhoodCheckpoint{neighborhoodCheckpoint},
        neighborComparator{neighborComparator},
        solNeighborComparator{solNeighborComparator},
        neighborEval{neighborEval},
        neighborhoodSize(neighborhoodSize),
        neighborhoodType{neighborhoodType} {}

  void initParam(EOT& _solution) final {
    improve = false;
    LO = false;
    RandJOB = _solution;
    if (neighborhoodType == NeighborhoodType::random) {
      std::shuffle(RandJOB.begin(), RandJOB.end(),
                   ParadiseoRNGFunctor<unsigned int>());
    }
    RandJOB.resize(std::min<int>(_solution.size(), neighborhoodSize.getSize()));
    k = 0;
  }

  void updateParam(EOT&) final {
    if (k < RandJOB.size() - 1) {
      k++;
    } else {
      k = 0;
      if (improve) {
        if (neighborhoodType == NeighborhoodType::random) {
          std::shuffle(RandJOB.begin(), RandJOB.end(),
                       ParadiseoRNGFunctor<unsigned int>());
        }
        improve = false;
      } else {
        LO = true;
      }
    }
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
    neighborhoodCheckpoint.initNeighborhood(_solution);
    for (int position = 0; position < n; position++) {
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
      neighborhoodCheckpoint.neighborCall(neighbor);
    }
    if (solNeighborComparator(_solution, bestNeighbor)) {
      bestNeighbor.move(_solution);
      _solution.fitness(bestNeighbor.fitness());
      improve = true;
    }
    neighborhoodCheckpoint.lastCall(_solution);
  }

  auto isContinue(EOT&) -> bool final { return !LO; }
  void move(EOT&) final {}
  auto accept(EOT&) -> bool final { return true; }
  void terminate(EOT&) final {}
};
