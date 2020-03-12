#pragma once

#include <algorithm>
#include <cmath>
#include <iterator>
#include <numeric>
#include <utility>
#include <vector>

#include <mo>

#include "global.hpp"

template <class EOT>
class NEHInit : public eoInit<EOT> {
  eoEvalFunc<EOT>& eval;
  moSolComparator<EOT> comp;

 public:
  const int size;

  NEHInit(eoEvalFunc<EOT>& eval,
          int size,
          moSolComparator<EOT> comp = moSolComparator<EOT>())
      : eval(eval), size(size), comp(comp) {}

  virtual void operator()(EOT& _fsp) {
    std::vector<int> order(size);
    init(order);
    _fsp.resize(0);
    _fsp.push_back(order[0]);
    EOT tmp;
    for (int k = 1; k < order.size(); k++) {
      int length = _fsp.size();
      EOT vBest;
      vBest.invalidate();
      int index = -1;
      tmp = _fsp;
      for (int i = 0; i <= length; ++i) {
        tmp.insert(tmp.begin() + i, order[k]);
        tmp.invalidate();
        eval(tmp);
        if (vBest.invalid() || comp(vBest, tmp)) {
          vBest.fitness(tmp.fitness());
          index = i;
        }
        tmp.erase(tmp.begin() + i);
      }
      _fsp.insert(_fsp.begin() + index, order[k]);
      _fsp.fitness(vBest.fitness());
    }
  }

 protected:
  virtual void init(std::vector<int>& order) {
    std::iota(std::begin(order), std::end(order), 0);
    std::shuffle(std::begin(order), std::end(order), ParadiseoRNGFunctor<>());
  }
};

template <class EOT>
class NEHInitOrdered : public NEHInit<EOT> {
  std::vector<int> order;

 public:
  NEHInitOrdered(eoEvalFunc<EOT>& eval,
                 std::vector<int> order,
                 moSolComparator<EOT> comp = moSolComparator<EOT>())
      : NEHInit<EOT>(eval, order.size(), comp), order(std::move(order)) {}

  template <class F>
  NEHInitOrdered(eoEvalFunc<EOT>& eval,
                 int n,
                 F criteria,
                 moSolComparator<EOT> comp = moSolComparator<EOT>())
      : NEHInit<EOT>(eval, n, comp), order(n) {
    std::iota(std::begin(order), std::end(order), 0);
    std::sort(std::begin(order), std::end(order), criteria);
  }

 protected:
  virtual void init(std::vector<int>& order) override { order = this->order; }
};

template <class EOT>
class NEHInitRandom : public NEHInit<EOT> {
  int cycle;
  using NEHInit<EOT>::size;

 public:
  NEHInitRandom(eoEvalFunc<EOT>& eval,
                int size,
                int cycle = 3,
                moSolComparator<EOT> comp = moSolComparator<EOT>())
      : NEHInit<EOT>(eval, size, comp), cycle(cycle) {}

 protected:
  virtual void init(std::vector<int>& order) override {
    int n = size;
    order.resize(n);
    std::iota(std::begin(order), std::end(order), 0);
    std::vector<int> a(cycle);
    std::generate(std::begin(a), std::end(a), [n]() { return rng.random(n); });
    int first = order[a[0]];
    for (int i = 0; i < cycle - 1; i++)
      order[a[i]] = order[a[i + 1]];
    order[a[cycle - 1]] = first;
  }
};
