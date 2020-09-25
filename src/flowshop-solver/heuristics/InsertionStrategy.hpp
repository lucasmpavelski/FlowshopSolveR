#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/global.hpp"

template <class Ngh, class EOT = typename Ngh::EOT>
class InsertionStrategy : public eoBF<EOT&, int, bool> {
 public:
  moEval<Ngh>& neighborEval;

  InsertionStrategy(moEval<Ngh>& neighborEval) : neighborEval{neighborEval} {}

  auto insertJob(EOT& sol, int jobToInsert) -> bool {
    sol.emplace_back(jobToInsert);
    Ngh neighbor;
    const int positionToInsert = sol.size() - 1;
    neighbor.set(positionToInsert, positionToInsert, sol.size());
    neighborEval(sol, neighbor);
    sol.fitness(neighbor.fitness());
    return insert(sol, positionToInsert);
  }

  virtual auto insert(EOT& sol, int positionToInsert) -> bool = 0;

  auto operator()(EOT& sol, int positionToInsert) -> bool override {
    return insert(sol, positionToInsert);
  }
};

template <class Ngh, class EOT = typename Ngh::EOT>
class InsertBest : public InsertionStrategy<Ngh> {
  moNeighborComparator<Ngh>& neighborComparator;

 public:
  InsertBest(moEval<Ngh>& neighborEval, moNeighborComparator<Ngh>& neighborComparator)
      : InsertionStrategy<Ngh>{neighborEval}, neighborComparator{neighborComparator} {}

  using InsertionStrategy<Ngh>::neighborEval;

  auto insert(EOT& sol, int positionToInsert) -> bool override {
    if (sol.size() == 1)
      return false;
    Ngh neighbor, bestNeighbor;
    for (unsigned position = 0; position < sol.size(); position++) {
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

    // auto insert(EOT& sol, int positionToInsert) -> bool override {
    //   Ngh bestNeighbor;
    //   bestNeighbor.set(positionToInsert, positionToInsert, sol.size());
    //   if (sol.isInvalid()) {
    //     neighborEval(sol, bestNeighbor);
    //     sol.fitness(bestNeighbor.fitness());
    //   } else {
    //     bestNeighbor.fitness(sol.fitness());
    //   }

    //   Ngh neighbor;    
    //   for (unsigned position = 0; position < sol.size(); position++) {
    //     if (positionToInsert == static_cast<int>(position))
    //       continue;
    //     neighbor.set(positionToInsert, position, sol.size());
    //     neighbor.invalidate();
    //     neighborEval(sol, neighbor);
    //     if (bestNeighbor.invalid() ||
    //         neighborComparator(bestNeighbor, neighbor)) {
    //       bestNeighbor = neighbor;
    //     }
    //   }

    //   neighbor.set(positionToInsert, positionToInsert, sol.size());
    //   neighbor.fitness(sol.fitness());

    //   if (neighborComparator(neighbor, bestNeighbor)) {
    //     bestNeighbor.move(sol);
    //     sol.fitness(bestNeighbor.fitness());
    //     return true;
    //   }
    //   return false;
    // }
};

template <class Ngh, class EOT = typename Ngh::EOT>
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
  double r{};

 public:

  myRandomNeighborComparator() : r{RNG::realUniform<double>()} {}

  auto operator()(const Ngh& _neighbor1, const Ngh& _neighbor2)
      -> bool override {
    /**
     * Simple reservoir sampling from:
     * Fan, C.; Muller, M.E.; Rezucha, I. (1962). "Development of sampling plans
     * by using sequential (item by item) selection techniques and digital
     * computers". Journal of the American Statistical Association. 57 (298):
     * 387-402.
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
    } else {
      return false;
    }
  }

  void reset() { r = 0.0; }
};

template <class Ngh, class EOT = typename Ngh::EOT>
class InsertRandomBest : public InsertBest<Ngh> {
  myRandomNeighborComparator<Ngh> comparator;

 public:
  InsertRandomBest(moEval<Ngh>& eval) : InsertBest<Ngh>{eval, comparator} {}

  auto insert(EOT& sol, int positionToInsert) -> bool override {
    bool ret = InsertBest<Ngh>::insert(sol, positionToInsert);
    comparator.reset();
    return ret;
  }
};

template <class Ngh>
auto buildInsertionStrategy(const std::string& name, moEval<Ngh>& eval)
    -> InsertionStrategy<Ngh>* {
  if (name == "first_best")
    return new InsertFirstBest<Ngh>{eval};
  if (name == "last_best")
    return new InsertLastBest<Ngh>{eval};
  if (name == "random_best")
    return new InsertRandomBest<Ngh>{eval};
  return nullptr;
}
