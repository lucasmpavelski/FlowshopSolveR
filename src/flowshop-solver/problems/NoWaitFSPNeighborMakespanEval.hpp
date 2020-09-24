#pragma once

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "flowshop-solver/problems/FSP.hpp"
#include "flowshop-solver/problems/FSPData.hpp"
#include "flowshop-solver/problems/NoWaitFSPEval.hpp"

/**
 * Fast evaluation for No-wait flowshop as show in
 * An improved iterated greedy algorithm for the no-waitflow shop scheduling
 * problem with makespan criterion
 * by Quan-Ke Pan, Ling Wang and Bao-Hua Zhao
 */
class NoWaitFSPNeighborMakespanEval : public moEval<FSPNeighbor> {
  const FSPData& fspData;
  [[maybe_unused]] NoWaitFSPEval& fullEval;

  auto partialMakespan(FSP& sol, unsigned j) -> int {
    if (sol.invalid()) {
      fullEval(sol);
    }
    int cmaxSol = sol.fitness();
    int pj = sol[j];
    const auto& T = fspData.jobProcTimesRef();

    if (j == 0) {
      int p1 = sol[1];
      return cmaxSol - fullEval.delay(pj, p1);
    }

    int pj_m1 = sol[j - 1];
    if (j == sol.size() - 1) {
      return cmaxSol - fullEval.delay(pj_m1, pj) - T[pj] + T[pj_m1];
    }

    int pj_p1 = sol[j + 1];
    return cmaxSol - fullEval.delay(pj_m1, pj) - fullEval.delay(pj, pj_p1) +
           fullEval.delay(pj_m1, pj_p1);
  }

  auto neighborMakespan(int partialCmax, FSP& sol, unsigned j, unsigned k)
      -> int {
    int pj = sol[j];
    const auto& T = fspData.jobProcTimesRef();

    if (k == 0) {
      return partialCmax + fullEval.delay(pj, sol[0]);
    }

    int pk_m1 = j < k ? sol[k] : sol[k - 1];
    if (k == sol.size() - 1) {
      return partialCmax + fullEval.delay(pk_m1, pj) - T[pk_m1] + T[pj];
    }

    int pk = j < k ? sol[k + 1] : sol[k];
    return partialCmax + fullEval.delay(pk_m1, pj) + fullEval.delay(pj, pk) -
           fullEval.delay(pk_m1, pk);
  }

 public:
  NoWaitFSPNeighborMakespanEval(const FSPData& fspData,
                                NoWaitFSPEval& fullEval)
      : fspData(fspData), fullEval(fullEval) {}

  void operator()(FSP& sol, FSPNeighbor& ngh) final {
    auto firstSecond = ngh.firstSecond(sol);
    auto j = firstSecond.first;
    auto k = firstSecond.second;
    int cMax_ll = partialMakespan(sol, j);
    ngh.fitness(neighborMakespan(cMax_ll, sol, j, k));
  }
};
