#pragma once

#include <vector>

#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "heuristics/FitnessReward.hpp"
#include "paradiseo/eo/eoEvalFunc.h"
#include "paradiseo/eo/eoOp.h"
#include "paradiseo/eo/utils/eoRNG.h"
#include "paradiseo/mo/comparator/moComparator.h"

/**
 * the main algorithm of the local search
 */
template <class EOT>
class AdaptiveDestruction : public eoMonOp<EOT> {
 public:
  eoEvalFunc<EOT>& eval;
  FitnessReward<EOT>& fitness_reward;
  OperatorSelection<int>& operator_selection;
  moSolComparator<EOT> comp;
  bool firstIteration = true;

  AdaptiveDestruction(eoEvalFunc<EOT>& eval,
                      OperatorSelection<int>& operator_selection,
                      FitnessReward<EOT>& fitness_reward,
                      moSolComparator<EOT> comp = moSolComparator<EOT>())
      : eval(eval),
        fitness_reward(fitness_reward),
        operator_selection(operator_selection),
        comp(comp),
        firstIteration{true} {}

  bool operator()(EOT& sol) final override {
    // std::cerr << "destruct " << sol.fitness() << '\n';
    if (!firstIteration) {
      operator_selection.feedback(fitness_reward.current(),
                                  fitness_reward.previous());
      operator_selection.update();
    }
    int d = operator_selection.selectOperator();
    firstIteration = false;

    assert(d <= static_cast<int>(sol.size()));
    int index;
    int length = sol.size();
    std::vector<int> D;
    EOT tmp;
    for (int k = 0; k < d; k++) {
      index = rng.random(sol.size());
      D.push_back(sol[index]);
      sol.erase(sol.begin() + index);
    }
    for (int k = 0; k < d; k++) {
      EOT vBest;
      vBest.invalidate();
      tmp = sol;
      length = sol.size();
      for (int i = 0; i <= length; ++i) {
        tmp.insert(tmp.begin() + i, D[k]);
        tmp.invalidate();
        eval(tmp);
        if (vBest.invalid() || comp(vBest, tmp)) {
          vBest.fitness(tmp.fitness());
          index = i;
        }
        tmp.erase(tmp.begin() + i);
      }
      sol.insert(sol.begin() + index, D[k]);
      sol.fitness(vBest.fitness());
    }
    return true;
  }
};
