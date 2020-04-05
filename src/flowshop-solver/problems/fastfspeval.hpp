#pragma once

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "flowshop-solver/problems/FSP.hpp"
#include "flowshop-solver/problems/FSPData.hpp"
#include "flowshop-solver/problems/FSPEvalFunc.hpp"
#include "flowshop-solver/problems/Problem.hpp"

using ivec = std::vector<int>;

struct CompiledSchedule {
  int no_jobs, no_machines;
  ivec e_times, q_times, f_times;
  ivec makespan;
  bool compiled = false;

  CompiledSchedule(int no_jobs, int no_machines)
      : no_jobs(no_jobs),
        no_machines(no_machines),
        e_times((no_jobs + 1) * (no_machines + 1)),
        q_times((no_jobs + 1) * (no_machines + 2)),
        f_times((no_jobs + 1) * (no_machines + 1)),
        makespan(no_jobs) {
    e_(0, 0) = 0;
    for (int i = 1; i <= no_machines; i++)
      e_(0, i) = 0;
    for (int i = 1; i <= no_jobs; i++)
      e_(i, 0) = 0;
    q_(no_jobs, no_machines) = 0;
    for (int j = no_machines; j >= 1; j--)
      q_(no_jobs, j) = 0;
    for (int i = no_jobs - 1; i >= 1; i--)
      q_(i, no_machines + 1) = 0;
    for (int i = 0; i <= no_jobs; i++)
      f_(i, 0) = 0;
  }

  inline int& e_(const int j, const int m) {
    return e_times[m * (no_jobs + 1) + j];
  }

  inline int& q_(const int j, const int m) {
    return q_times[m * (no_jobs + 1) + j];
  }

  inline int& f_(const int j, const int m) {
    return f_times[m * (no_jobs + 1) + j];
  }

  void compile(const FSPData& fspData, const ivec& seq) {
    compile(fspData, seq, 0);
  }

  void printMatrix(ivec& m) {
    for (int i = 0; i < no_jobs; i++) {
      for (int j = 0; j < no_machines; j++) {
        std::cerr << ' ' << m[j * (no_jobs + 1) + i];
      }
      std::cerr << '\n';
    }
  }

  void compile(const FSPData& fspData, const ivec& seq, int from) {
    const int seq_size = static_cast<int>(seq.size());
    for (int i = from + 1; i <= seq_size - 1; i++) {
      int seq_i = static_cast<int>(seq[i - 1]);
      for (int j = 1; j <= no_machines; j++) {
        e_(i, j) =
            std::max(e_(i, j - 1), e_(i - 1, j)) + fspData.pt(seq_i, j - 1);
      }
    }

    for (int i = no_jobs; i >= seq_size - 1; i--) {
      for (int j = no_machines; j >= 1; j--) {
        q_(i, j) = 0;
      }
    }
    for (int i = seq_size - 1; i >= 1; i--) {
      int seq_i = static_cast<int>(seq[i - 1]);
      for (int j = no_machines; j >= 1; j--) {
        q_(i, j) =
            std::max(q_(i, j + 1), q_(i + 1, j)) + fspData.pt(seq_i, j - 1);
      }
    }
    for (int i = from + 1; i <= seq_size; i++) {
      int seq_k = static_cast<int>(seq[seq_size - 1]);
      for (int j = 1; j <= no_machines; j++) {
        f_(i, j) =
            std::max(f_(i, j - 1), e_(i - 1, j)) + fspData.pt(seq_k, j - 1);
      }
    }

    // std::cerr << "e_times" << '\n';
    // printMatrix(e_times);
    // std::cerr << "q_times" << '\n';
    // printMatrix(q_times);
    // std::cerr << "f_times" << '\n';
    // printMatrix(f_times);

    std::fill(makespan.begin(), makespan.end(), 0);
    for (int i = 1; i <= seq_size; i++) {
      for (int j = 1; j <= no_machines; j++) {
        int c_ij = f_(i, j) + q_(i, j);
        makespan[i - 1] = std::max(makespan[i - 1], c_ij);
      }
    }
  }

  inline int getMakespan(const int i) const { return makespan[i]; }
};

int positionPairToKey(int first, int second, int size);

std::pair<int, int> keyToPositionPair(int val, int size);

class FastFSPNeighborEval : public moEval<FSPNeighbor> {
  const FSPData fspData;
  [[maybe_unused]] eoEvalFunc<EOT>& fullEval;
  std::vector<CompiledSchedule> compiledSchedules;
  std::vector<EOT> compiledSolutions;

 public:
  FastFSPNeighborEval(const FSPData& fspData, eoEvalFunc<EOT>& fullEval)
      : fspData(fspData),
        fullEval(fullEval),
        compiledSchedules(
            fspData.noJobs(),
            CompiledSchedule(fspData.noJobs(), fspData.noMachines())),
        compiledSolutions(fspData.noJobs()) {}

  void operator()(FSP& sol, FSPNeighbor& ngh) final override {
    auto firstSecond = ngh.firstSecond(sol);
    int first = firstSecond.first;
    int second = firstSecond.second;

    auto& compiledSolution = compiledSolutions[first];

    auto mPtr = std::mismatch(compiledSolution.begin(), compiledSolution.end(),
                              sol.begin());

    if (mPtr.second != sol.end()) {
      int from = 0;  // std::distance(sol.begin(), mPtr.second);

      // std::cerr << "Compiling for: " << sol << '\n';
      // std::cerr << "Previous: " << compiledSolution << '\n';
      // std::cerr << "Mismatch at: " << from << '\n';
      EOT perm_i = sol;
      std::rotate(perm_i.begin() + first, perm_i.begin() + first + 1,
                  perm_i.end());

      compiledSchedules[first].compile(fspData, perm_i, from);
      compiledSolutions[first] = sol;
    }
    if (first < second) {
      second--;
    }
    ngh.fitness(compiledSchedules[first].getMakespan(second));

    // EOT tmp = sol;
    // ngh.move(tmp);
    // fullEval(tmp);

    // if (ngh.fitness() != tmp.fitness()) {
    //   std::cerr << "ops" << ' ' << first << ' ' << second
    //             << ' '
    //             // << (tmp.fitness() ==
    //             //     compiledSchedules[first].getMakespan(second - 1))
    //             << '\n';
    // }
  }
};
