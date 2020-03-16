#pragma once

#include <vector>

#include <comparator/moSolComparator.h>
#include <eoEvalFunc.h>
#include <eoOp.h>
#include <utils/eoRNG.h>

/**
 * the main algorithm of the local search
 */
template <class EOT>
class OpPerturbDestConst : public eoMonOp<EOT> {
 public:
  enum strat { random, ordered } nh_strategy;

  OpPerturbDestConst(eoEvalFunc<EOT>& eval,
                     int d,
                     moSolComparator<EOT> comp = moSolComparator<EOT>(),
                     const int nh_size = -1,
                     strat nh_strategy = strat::ordered)
      : eval(eval), d(d), comp(comp) {}

  virtual bool operator()(EOT& sol) {
    assert(d <= sol.size());
    int length = sol.size();
    std::vector<int> D;
    EOT tmp;
    for (int k = 0; k < d; k++) {
      int index = rng.random(sol.size());
      D.push_back(sol[index]);
      sol.erase(sol.begin() + index);
    }
    for (int k = 0; k < d; k++) {
      EOT vBest;
      vBest.invalidate();
      tmp = sol;
      length = sol.size();
      std::vector<int> ties;
      for (int i = 0; i <= length; ++i) {
        tmp.insert(tmp.begin() + i, D[k]);
        tmp.invalidate();
        eval(tmp);
        if (vBest.invalid() || comp(vBest, tmp)) {
          vBest.fitness(tmp.fitness());
          ties = {i};
        } else if (comp.equals(vBest, tmp)) {
          ties.push_back(i);
        }
        tmp.erase(tmp.begin() + i);
      }
      int index = ties[RNG::intUniform(ties.size() - 1)];
      sol.insert(sol.begin() + index, D[k]);
      sol.fitness(vBest.fitness());
    }
    return true;
  }

 private:
  int d;  // nb de deconstruction
  eoEvalFunc<EOT>& eval;
  moSolComparator<EOT> comp;
};
