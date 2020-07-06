#pragma once

#include <algo/moLocalSearch.h>
#include <eoFunctor.h>
#include <neighborhood/moIndexNeighborhood.h>
#include <functional>
#include <utility>
#include <vector>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/heuristics/perturb/DestructionConstruction.hpp"

template <class Ngh, class EOT = typename Ngh::EOT>
class myResizableLocalSearch : public eoFunctorBase {
  moLocalSearch<Ngh>& localSearch;
  moIndexNeighborhood<Ngh>& neighborhood;
  std::function<int(int)> getMaxSize;

 public:
  myResizableLocalSearch(moLocalSearch<Ngh>& localSearch,
                         moIndexNeighborhood<Ngh>& neighborhood,
                         std::function<int(int)> getMaxSize)
      : localSearch{localSearch},
        neighborhood{neighborhood},
        getMaxSize{std::move(getMaxSize)} {}

  void operator()(EOT& sol){
    int size = sol.size();
    neighborhood.setNeighborhoodSize(getMaxSize(size));
    localSearch(sol);
  }
};

template <class Ngh, class EOT = typename Ngh::EOT>
class IGLocalSearchPartialSolution : public DestructionConstruction<Ngh> {
  myResizableLocalSearch<Ngh>& localSearch;

 public:
  IGLocalSearchPartialSolution(InsertionStrategy<Ngh>& insert,
                               DestructionSize& destructionSize,
                               myResizableLocalSearch<Ngh>& localSearch)
      : DestructionConstruction<Ngh>{insert, destructionSize},
        localSearch{localSearch} {}

  using DestructionConstruction<Ngh>::destruction;
  using DestructionConstruction<Ngh>::construction;

  auto operator()(EOT& sol) -> bool override {
    EOT before = sol;
    std::vector<int> removedJobs = destruction(sol);
    if (sol.size() > 0)
      localSearch(sol);
    construction(sol, removedJobs);
    return before != sol;
  }
};