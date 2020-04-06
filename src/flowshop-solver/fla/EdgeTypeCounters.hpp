#pragma once

#include <iostream>
#include <vector>

#include "paradiseo/mo/comparator/moSolComparator.h"

#include "flowshop-solver/heuristics/NeigborhoodStat.hpp"

template <class EOT>
class EdgeTypeCounters : public NeigborhoodStatBase<EOT> {
  EOT currentFitness;
  std::vector<int> equal;
  std::vector<int> better;
  std::vector<int> worse;
  moSolComparator<EOT>& compare;

 public:
  EdgeTypeCounters(moSolComparator<EOT>& compare) : compare{compare} {}

  void init(EOT& sol) final {
    equal.clear();
    better.clear();
    worse.clear();
  }

  void initNeighborhood(EOT& sol) final {
    equal.push_back(0);
    better.push_back(0);
    worse.push_back(0);
    currentFitness.fitness(sol.fitness());
  }

  void neighborCall(EOT& sol) final {
    if (compare.equals(currentFitness, sol)) {
      equal.back()++;
    } else if (compare(currentFitness, sol)) {
      better.back()++;
    } else {
      worse.back()++;
    }
  }

  std::ostream& exportStats(std::ostream& out) {
    out << "neighborhood,better,equal,worse\n";
    for (int i = 0; i < better.size(); i++) {
      out << i << ',' << better[i] << ',' << equal[i] << ',' << worse[i]
          << '\n';
    }
    return out;
  }
};