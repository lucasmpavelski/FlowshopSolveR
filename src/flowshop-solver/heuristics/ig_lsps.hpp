#pragma once

#include <vector>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/heuristics/InsertionStrategy.hpp"

template <class Ngh, class EOT = typename Ngh::EOT>
class IGLocalSearchPartialSolution : public moPerturbation<Ngh> {
  InsertionStrategy<Ngh>& insert;
  moLocalSearch<Ngh>& localSearch;
  const unsigned destructionSize;

 public:
  IGLocalSearchPartialSolution(InsertionStrategy<Ngh>& insert,
                               moLocalSearch<Ngh>& localSearch,
                               unsigned destructionSize)
      : insert(insert),
        localSearch(localSearch),
        destructionSize(destructionSize) {}

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

  void init(EOT&) override{};
  void add(EOT&, Ngh&) override{};
  void update(EOT&, Ngh&) override{};
  void clearMemory() override{};
};