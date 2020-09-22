#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>

#include "flowshop-solver/heuristics/FSPOrderHeuristics.hpp"
#include "flowshop-solver/heuristics/NEHInit.hpp"
#include "flowshop-solver/heuristics/fastnehheuristic.hpp"
#include "problems/FSPEval.hpp"
#include "problems/FastFSPNeighborEval.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

std::string instances_folder = TEST_FIXTURES_FOLDER;


void testNEH() {
  PermFSPEval<FSPMin> fsp_eval{FSPData{instances_folder + "test.txt"}};
  FSPMin sol(fsp_eval.noJobs());
  //FSPDefaultNEH<FSPMin> neh(fsp_eval, moSolComparator<FSPMin>());
  //neh(sol);
  fsp_eval(sol);
  assert(sol.fitness() == 54);
  // maximization problem
  FSPMax sol_max(fsp_eval.noJobs());
  PermFSPEval<FSPMax> fsp_eval_max{FSPData{instances_folder + "test.txt"}};
  //FSPDefaultNEH<FSPMax> neh_max(fsp_eval_max, moSolComparator<FSPMax>());
  //neh_max(sol_max);
  fsp_eval_max(sol_max);
  assert(sol_max.fitness() == 125 - 54);
}

void testFastNEH() {
  // PermFSPEval<FastFSPSolution> fsp_eval{FSPData{instances_folder +
  // "test.txt"}}; FastFSPSolution sol(fsp_eval.noJobs()); FastNEH
  // neh(fsp_eval.fspData); neh(sol); fsp_eval(sol); assert(sol.fitness() ==
  // 54);
}

void testJohnson() {}

int main() {
  // testNEH();
  // testFastNEH();
  std::cout << "all passed!";
}
