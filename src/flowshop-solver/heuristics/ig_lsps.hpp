#pragma once

#include <vector>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/heuristics/InsertionStrategy.hpp"

template <class Ngh, class EOT = typename Ngh::EOT>
class IGLocalSearchPartialSolution : public eoMonOp<typename Ngh::EOT> {
  InsertionStrategy<Ngh>& insert;
  moLocalSearch<Ngh>& localSearch;
  const unsigned destructionSize;
  moSolComparator<EOT>& comparator;

 public:
  IGLocalSearchPartialSolution(InsertionStrategy<Ngh>& insert,
                               moLocalSearch<Ngh>& localSearch,
                               unsigned destructionSize,
                               moSolComparator<EOT>& comparator)
      : insert(insert),
        localSearch(localSearch),
        destructionSize(destructionSize),
        comparator(comparator) {}

  auto operator()(EOT& sol) -> bool override {
    EOT before = sol;
    std::vector<int> removedJobs = deconstruction(sol);
    if (sol.size() > 0)
      localSearch(sol);
    construction(sol, removedJobs);
    return before != sol;
  }

  auto deconstruction(EOT& sol) -> std::vector<int> {
    std::vector<int> removedJobs;
    removedJobs.reserve(destructionSize);
    for (unsigned i = 0; i < destructionSize; i++) {
      int pos = rng.random(sol.size());
      removedJobs.push_back(sol[pos]);
      sol.erase(sol.begin() + pos);
    }
    return removedJobs;
  }

  void construction(EOT& sol, const std::vector<int>& removedJobs) {
    for (auto job : removedJobs)
      insert.insertJob(sol, job);
  }
};