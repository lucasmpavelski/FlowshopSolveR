#pragma once

#include <paradiseo/mo/mo>

#include "flowshop-solver/global.hpp"

template <class Neighbor>
class InsertionStrategy {
 public:
  using Ngh = Neighbor;
  using EOT = typename Neighbor::EOT;

  auto insertJob(EOT& sol, int jobToInsert) -> bool {
    sol.emplace_back(jobToInsert);
    return insert(sol, sol.size() - 1);
  }

  virtual auto insert(EOT& sol, int positionToInsert) -> bool = 0;
};

template <class Ngh>
class InsertBest : public InsertionStrategy<Ngh> {
  moEval<Ngh>& neighborEval;
  moNeighborComparator<Ngh>& neighborComparator;

 public:
  InsertBest(moEval<Ngh>& neighborEval,
             moNeighborComparator<Ngh>& neighborComparator)
      : neighborEval{neighborEval}, neighborComparator{neighborComparator} {}

  using EOT = typename InsertionStrategy<Ngh>::EOT;

  auto insert(EOT& sol, int positionToInsert) -> bool override {
    Ngh neighbor, bestNeighbor;
    for (unsigned position = 0; position <= sol.size(); position++) {
      if (positionToInsert == static_cast<int>(position))
        continue;
      neighbor.set(positionToInsert, position, sol.size());
      neighbor.invalidate();
      neighborEval(sol, neighbor);
      if (bestNeighbor.invalid() ||
          neighborComparator(bestNeighbor, neighbor)) {
        bestNeighbor = neighbor;
      }
    }
    neighbor.set(positionToInsert, positionToInsert, sol.size());
    neighbor.fitness(sol.fitness());
    if (neighborComparator(neighbor, bestNeighbor)) {
      bestNeighbor.move(sol);
      sol.fitness(bestNeighbor.fitness());
      return true;
    }
    return false;
  }
};

template <class Ngh>
class InsertFirstBest : public InsertBest<Ngh> {
  moNeighborComparator<Ngh> neighborComparator;

 public:
  InsertFirstBest(moEval<Ngh>& eval)
      : InsertBest<Ngh>{eval, neighborComparator} {}
};

template <class Ngh>
class InsertLastBest : public InsertBest<Ngh> {
  moEqualNeighborComparator<Ngh> neighborComparator;

 public:
  InsertLastBest(moEval<Ngh>& eval)
      : InsertBest<Ngh>{eval, neighborComparator} {}
};

template <class Ngh>
class myRandomNeighborComparator : public moNeighborComparator<Ngh> {
  double r;

 public:
  auto operator()(const Ngh& _neighbor1, const Ngh& _neighbor2)
      -> bool override {
    /**
     * Simple reservoir sampling from:
     * Fan, C.; Muller, M.E.; Rezucha, I. (1962). "Development of sampling plans
     * by using sequential (item by item) selection techniques and digital
     * computers". Journal of the American Statistical Association. 57 (298):
     * 387–402.
     */
    if (_neighbor1.fitness() < _neighbor2.fitness()) {
      r = RNG::realUniform<double>();
      return true;
    } else if (_neighbor1.fitness() == _neighbor2.fitness()) {
      const auto r2 = RNG::realUniform<double>();
      if (r2 >= r) {
        r = r2;
        return true;
      }
      return false;
    }
  }

  void reset() { r = 0.0; }
};

template <class Ngh>
class InsertRandomBest : public InsertBest<Ngh> {
  using EOT = typename InsertBest<Ngh>::EOT;
  myRandomNeighborComparator<Ngh> comparator;

 public:
  InsertRandomBest(moEval<Ngh>& eval) : InsertBest<Ngh>{eval, comparator} {}

  auto insert(EOT& sol, int positionToInsert) -> bool override {
    bool ret = InsertBest<Ngh>::insert(sol, positionToInsert);
    comparator.reset();
    return ret;
  }
};
