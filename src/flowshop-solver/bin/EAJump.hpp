#pragma once

#include <algorithm>
#include <utility>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>
#include <paradiseo/eo/ga/eoBitOp.h>

inline int oneMax(const eoBit<int>& sol) {
  return std::accumulate(sol.begin(), sol.end(), 0);
}

inline int jump(const eoBit<int>& sol, const int k) {
  auto om = oneMax(sol);
  auto n = sol.size();
  if (om <= n - k || om == n) {
    return om + k;
  } else {
    return n - om;
  }
}

inline std::pair<int, int> eaJumpCost(int size, int k, double mu) {
  eoBitMutation<eoBit<int>> mutation(mu);
  int max_budget = exp(1) * pow(size, k);
  eoBit<int> sol(size);
  eoBit<int> sol_m(size);
  
  eoUniformGenerator<bool> uGen(0, 1);
  eoInitFixedLength<eoBit<int>> random(size, uGen);
  random(sol);
  
  int curr_fitness = jump(sol, k);
  
  int steps = 0;
  while (curr_fitness != size && steps <= max_budget) {
    sol_m = sol;
    mutation(sol_m);
    int fitness_m = jump(sol_m, k);
    if (curr_fitness < fitness_m) {
      sol = sol_m;
    }
    curr_fitness = std::max(curr_fitness, fitness_m);
    steps++;
  }
  return {curr_fitness, steps};
}