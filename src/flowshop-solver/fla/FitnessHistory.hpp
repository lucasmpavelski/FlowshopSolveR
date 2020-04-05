#pragma once

#include <vector>

#include "paradiseo/mo/continuator/moStatBase.h"

template <class EOT>
class FitnessHistory : public moStatBase<EOT> {
  std::vector<double> fitness;

 public:
  using citerator = typename std::vector<double>::const_iterator;

  FitnessHistory() { fitness.reserve(100); }

  void init(EOT&) final override { fitness.clear(); }

  void operator()(EOT& sol) final override { fitness.push_back(sol.fitness()); }

  // void lastCall(EOT& sol) { std::cerr << printSeq(fitness) << '\n'; }

  citerator cbegin() const { return fitness.cbegin(); }
  citerator cend() const { return fitness.cend(); }
};