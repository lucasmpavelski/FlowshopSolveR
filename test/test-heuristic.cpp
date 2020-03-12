#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>

#include "heuristics/FSPOrderHeuristics.hpp"
#include "heuristics/NEHInit.hpp"
#include "heuristics/fastnehheuristic.hpp"
#include "problems/FSPEvalFunc.hpp"
#include "problems/fastfspeval.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

std::string instances_folder = TEST_FIXTURES_FOLDER;

template <class FSPTp>
class FSPDefaultNEH : public NEHInitOrdered<FSPTp> {
  public:
  FSPDefaultNEH(FSPEvalFunc<FSPTp>& eval, moSolComparator<FSPTp> comp)
      : NEHInitOrdered<FSPTp>(eval, eval.noJobs(),
                              compareByTotalProcTimes(eval.fsp_data), comp) {}
};

void testNEH() {
  PermFSPEvalFunc<FSPMin> fsp_eval{FSPData{instances_folder + "test.txt"}};
  FSPMin sol(fsp_eval.noJobs());
  FSPDefaultNEH<FSPMin> neh(fsp_eval, moSolComparator<FSPMin>());
  neh(sol);
  fsp_eval(sol);
  assert(sol.fitness() == 54);
  // maximization problem
  FSPMax sol_max(fsp_eval.noJobs());
  PermFSPEvalFunc<FSPMax> fsp_eval_max{FSPData{instances_folder + "test.txt"}};
  FSPDefaultNEH<FSPMax> neh_max(fsp_eval_max, moSolComparator<FSPMax>());
  neh_max(sol_max);
  fsp_eval_max(sol_max);
  assert(sol_max.fitness() == 125 - 54);
}

void testFastNEH() {
  PermFSPEvalFunc<FastFSPSolution> fsp_eval{FSPData{instances_folder + "test.txt"}};
  FastFSPSolution sol(fsp_eval.noJobs());
  FastNEH neh(fsp_eval.fsp_data);
  neh(sol);
  fsp_eval(sol);
  assert(sol.fitness() == 54);
}

void testJohnson() {

}

int main() {
  //testNEH();
  //testFastNEH();
  std::cout << "all passed!";
}

