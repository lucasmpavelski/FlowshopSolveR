#pragma once

#include <vector>

#include "paradiseo/eo/eoOp.h"
#include "paradiseo/mo/algo/moLocalSearch.h"
#include "paradiseo/eo/eoEvalFunc.h"

template <class Ngh>
class IGLocalSearchPartialSolution : public eoMonOp<typename Ngh::EOT> {
 public:
  using EOT = typename Ngh::EOT;
  using ivec = std::vector<int>;

  IGLocalSearchPartialSolution(eoEvalFunc<EOT>& evalFunction,
                               moLocalSearch<Ngh>& localSearch,
                               unsigned destructionSize,
                               moSolComparator<EOT>& comparator)
      : evalFunction(evalFunction),
        localSearch(localSearch),
        destructionSize(destructionSize),
        comparator(comparator) {}

  bool operator()(EOT& sol) {
    EOT before = sol;
    ivec removedJobs = deconstruction(sol);
    if (sol.size() > 0)
      localSearch(sol);
    construction(sol, removedJobs);
    return before != sol;
  }

  ivec deconstruction(EOT& sol) {
    ivec removedJobs;
    removedJobs.reserve(destructionSize);
    for (int i = 0; i < destructionSize; i++) {
      int pos = rng.random(sol.size());
      removedJobs.push_back(sol[pos]);
      sol.erase(sol.begin() + pos);
    }
    return removedJobs;
  }

  void construction(EOT& sol, const ivec& removedJobs) {
    for (auto job : removedJobs) {
      insertOnBestPosition(sol, job);
    }
  }

  void insertOnBestPosition(EOT& sol, int job) {
    const EOT temp = sol;
    sol.insert(sol.begin(), job);
    sol.invalidate();
    evalFunction(sol);
    int bestPosition = 0;
    EOT bestFitness;
    bestFitness.fitness(sol.fitness());
    for (int i = 1; i <= temp.size(); i++) {
      sol = temp;
      sol.insert(sol.begin() + i, job);
      sol.invalidate();
      evalFunction(sol);
      if (comparator(bestFitness, sol)) {
        bestPosition = i;
        bestFitness.fitness(sol.fitness());
      }
    }
    sol = temp;
    sol.insert(sol.begin() + bestPosition, job);
    sol.fitness(bestFitness.fitness());
  }

 private:
  eoEvalFunc<EOT>& evalFunction;
  moLocalSearch<Ngh>& localSearch;
  const unsigned destructionSize;
  moSolComparator<EOT>& comparator;
};