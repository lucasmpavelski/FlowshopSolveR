#pragma once

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "flowshop-solver/problems/FSP.hpp"
#include "flowshop-solver/problems/FSPData.hpp"
#include "flowshop-solver/problems/NWFSPEvalFunc.hpp"

/**
 * Fast evaluation for No-wait flowshop as show in
 * An improved iterated greedy algorithm for the no-waitflow shop scheduling 
 * problem with makespan criterion
 * by Quan-Ke Pan, Ling Wang and Bao-Hua Zhao
 */
class FastNWNeighborMakespanEval : public moEval<FSPNeighbor> {
  const FSPData& fspData;
  [[maybe_unused]] NWFSPEvalFunc<EOT>& fullEval;

 public:
  FastNWNeighborMakespanEval(const FSPData& fspData,
                             NWFSPEvalFunc<EOT>& fullEval)
      : fspData(fspData), fullEval(fullEval) {}

  void operator()(FSP& sol, FSPNeighbor& ngh) final {
    auto firstSecond = ngh.firstSecond(sol);
    int j = firstSecond.first;
    int k = firstSecond.second;
    const auto& T = fspData.jobProcTimesRef();

    if (sol.invalid()) {
      fullEval(sol);
    }
    int cmaxSol = sol.fitness();

    int cmaxSolll;
    if (j == 0) {
      cmaxSolll = cmaxSol - fullEval.delay(j, 1);
    } else if (j == fspData.noJobs() - 1) {
      cmaxSolll = cmaxSol - fullEval.delay(j - 1, j) - T[j] + T[j - 1];
    } else {
      cmaxSolll = cmaxSol - fullEval.delay(j - 1, j) -
                  fullEval.delay(j, j + 1) + fullEval.delay(j - 1, j + 1);
    }

    if (k == 0) {
      ngh.fitness(cmaxSolll + fullEval.delay(j, k));
    } else if (k == fspData.noJobs() - 1) {
      ngh.fitness(cmaxSolll + fullEval.delay(k - 1, j) - T[k - 1] + T[j]);
    } else {
      ngh.fitness(cmaxSolll + fullEval.delay(k - 1, j) + fullEval.delay(j, k) 
        - fullEval.delay(k - 1, k));
    }
  }
};
