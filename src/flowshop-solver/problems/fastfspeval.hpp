#pragma once

#include <algorithm>
#include <mo>
#include <utility>
#include <vector>

#include "continuators/myMovedSolutionStat.hpp"
#include "moHiResTimeContinuator.hpp"
#include "problems/FSP.hpp"
#include "problems/FSPData.hpp"
#include "problems/FSPEvalFunc.hpp"
#include "problems/Problem.hpp"

using ivec = std::vector<int>;

struct CompiledSchedule {
  unsigned no_jobs, no_machines;
  ivec e_times, q_times, f_times;
  ivec makespan;
  bool compiled = false;

  CompiledSchedule(unsigned no_jobs, unsigned no_machines)
      : no_jobs(no_jobs),
        no_machines(no_machines),
        e_times((no_jobs + 1) * (no_machines + 1)),
        q_times((no_jobs + 1) * (no_machines + 2)),
        f_times((no_jobs + 1) * (no_machines + 1)),
        makespan(no_jobs) {
    e_(0, 0) = 0;
    for (unsigned i = 1; i <= no_machines; i++)
      e_(0, i) = 0;
    for (unsigned i = 1; i <= no_jobs; i++)
      e_(i, 0) = 0;
    q_(no_jobs, no_machines) = 0;
    for (unsigned j = no_machines; j >= 1; j--)
      q_(no_jobs, j) = 0;
    for (unsigned i = no_jobs - 1; i >= 1; i--)
      q_(i, no_machines + 1) = 0;
    for (unsigned i = 0; i <= no_jobs; i++)
      f_(i, 0) = 0;
  }

  inline int& e_(const unsigned j, const unsigned m) {
    return e_times[m * (no_jobs + 1) + j];
  }

  inline int& q_(const unsigned j, const unsigned m) {
    return q_times[m * (no_jobs + 1) + j];
  }

  inline int& f_(const unsigned j, const unsigned m) {
    return f_times[m * (no_jobs + 1) + j];
  }

  void compile(const FSPData& fspData, const ivec& seq) {
    const unsigned seq_size = static_cast<unsigned>(seq.size());
    for (unsigned i = 1; i <= seq_size - 1; i++) {
      unsigned seq_i = static_cast<unsigned>(seq[i - 1]);
      for (unsigned j = 1; j <= no_machines; j++) {
        e_(i, j) =
            std::max(e_(i, j - 1), e_(i - 1, j)) + fspData.pt(seq_i, j - 1);
      }
    }
    for (unsigned i = seq_size - 1; i >= 1; i--) {
      unsigned seq_i = static_cast<unsigned>(seq[i - 1]);
      for (unsigned j = no_machines; j >= 1; j--) {
        q_(i, j) =
            std::max(q_(i, j + 1), q_(i + 1, j)) + fspData.pt(seq_i, j - 1);
      }
    }
    for (unsigned i = 1; i <= seq_size; i++) {
      unsigned seq_k = static_cast<unsigned>(seq[seq_size - 1]);
      for (unsigned j = 1; j <= no_machines; j++) {
        f_(i, j) =
            std::max(f_(i, j - 1), e_(i - 1, j)) + fspData.pt(seq_k, j - 1);
      }
    }

    std::fill(makespan.begin(), makespan.end(), 0);
    for (unsigned i = 1; i <= seq_size; i++) {
      for (unsigned j = 1; j <= no_machines; j++) {
        int c_ij = f_(i, j) + q_(i, j);
        makespan[i - 1] = std::max(makespan[i - 1], c_ij);
      }
    }
  }

  inline int getMakespan(const unsigned i) const { return makespan[i]; }
};

int positionPairToKey(int first, int second, int size);

std::pair<int, int> keyToPositionPair(int val, int size);

class FastFSPNeighborEval : public moEval<moShiftNeighbor<FSP>> {
  const FSPData fspData;
  std::vector<CompiledSchedule> compiledSchedules;
  myMovedSolutionStat<FSP>& movedStat;
  std::vector<bool> isCompiled;

 public:
  FastFSPNeighborEval(const FSPData& fspData,
                      myMovedSolutionStat<FSP>& movedStat)
      : fspData(fspData),
        movedStat(movedStat),
        compiledSchedules(
            fspData.noJobs(),
            CompiledSchedule(fspData.noJobs(), fspData.noMachines())),
        isCompiled(fspData.noJobs(), false) {}

  void operator()(FSP& sol, moShiftNeighbor<FSP>& ngh) final override {
    auto firstSecond = keyToPositionPair(ngh.index() + 1, sol.size());
    int first = firstSecond.first;
    int second = firstSecond.second;
    // if (compiledSolution.size() == 0) {
    //   compiledSchedules.assign(
    //       size(), CompiledSchedule(fspData.noJobs(), fspData.noMachines()));
    //   isCompiled.assign(size(), 0);
    // }
    if (movedStat.value()) {
      movedStat.reset();
      isCompiled.assign(isCompiled.size(), 0);
    }
    if (!isCompiled[first]) {
      ivec perm_i = sol;
      std::rotate(perm_i.begin() + first, perm_i.begin() + first + 1,
                  perm_i.end());
      compiledSchedules[first].compile(fspData, perm_i);
      isCompiled[first] = 1;
    }
    if (first < second)
      second--;
    ngh.fitness(compiledSchedules[first].getMakespan(second));
  }
};
