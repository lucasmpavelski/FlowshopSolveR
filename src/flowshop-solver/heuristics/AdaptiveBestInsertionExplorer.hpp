#pragma once

#include <algorithm>

#include <paradiseo/mo/mo>

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/global.hpp"
#include "flowshop-solver/heuristics/BestInsertionExplorer.hpp"
#include "flowshop-solver/heuristics/neighborhood_checkpoint.hpp"
#include "flowshop-solver/problems/FSP.hpp"

class PositionSelector : public eoFunctorBase {
public:
  virtual void init(const std::vector<int>&) {}
  virtual auto select(const std::vector<int>&) -> int = 0;
  virtual void feedback(const double) {};
};

class AdaptivePositionSelector : public PositionSelector {
  OperatorSelection<int>& operatorSelection;

 public:
  AdaptivePositionSelector(OperatorSelection<int>& operatorSelection)
      : operatorSelection(operatorSelection) {}

  auto select(const std::vector<int>& vec) -> int override {
    const int n = vec.size();
    const int k = operatorSelection.noOperators();
    const int selected = operatorSelection.selectOperator();
    if (selected == 0) {
      return rng.random(n);
    }
    int poll_size = n / k;
    if (selected == k && n % k > 0) {
      poll_size++;
    }
    return rng.random(poll_size) + (selected - 1) * (n / k);
  }

  void feedback(const double reward) override {
    operatorSelection.feedback(reward);
    operatorSelection.update();
  }
};


class AdaptiveNoReplacementPositionSelector : public AdaptivePositionSelector {
   std::vector<int> unselectedPositions;
 
 public:
  AdaptiveNoReplacementPositionSelector(OperatorSelection<int>& operatorSelection)
      : AdaptivePositionSelector(operatorSelection) {}

  void init(const std::vector<int>& sol) override {
    unselectedPositions = sol;
  }

  auto select(const std::vector<int>& sol) -> int override {
    int pos = AdaptivePositionSelector::select(unselectedPositions);
    int solPos = std::distance(sol.begin(), std::find(sol.begin(), sol.end(), unselectedPositions[pos]));
    unselectedPositions.erase(unselectedPositions.begin() + pos);
    return solPos;
  }
};


template <class EOT>
class AdaptiveBestInsertionExplorer
    : public moNeighborhoodExplorer<myShiftNeighbor<EOT>> {
  using Ngh = myShiftNeighbor<EOT>;

  NeigborhoodCheckpoint<Ngh>& neighborhoodCheckpoint;
  moNeighborComparator<Ngh>& neighborComparator;
  moSolNeighborComparator<Ngh>& solNeighborComparator;

  moEval<Ngh>& neighborEval;
  const NeighborhoodType neighborhoodType;

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
        neighborhoodType{neighborhoodType},
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
