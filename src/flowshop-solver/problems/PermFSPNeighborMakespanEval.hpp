#pragma once

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "flowshop-solver/problems/FSP.hpp"
#include "flowshop-solver/problems/FSPData.hpp"
#include "flowshop-solver/problems/FSPEval.hpp"
#include "flowshop-solver/problems/Problem.hpp"

class PermFSPNeighborMakespanEval : public moEval<FSPNeighbor> {

  struct CompiledSchedule {
    using ivec = std::vector<int>;
    const FSPData& fspData;
    ivec e_times, q_times, f_times, makespan;
    FSP compiledSolution;

    CompiledSchedule(const FSPData& fspData)
        : fspData(fspData),
          e_times((fspData.noJobs() + 1) * (fspData.noMachines() + 1)),
          q_times((fspData.noJobs() + 1) * (fspData.noMachines() + 2)),
          f_times((fspData.noJobs() + 1) * (fspData.noMachines() + 1)),
          makespan(fspData.noJobs()) {
      const int no_jobs = fspData.noJobs();
      const int no_machines = fspData.noMachines();
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

    inline auto e_(const int j, const int m) -> int& {
      return e_times[m * (fspData.noJobs() + 1) + j];
    }

    inline auto q_(const int j, const int m) -> int& {
      return q_times[m * (fspData.noJobs() + 1) + j];
    }

    inline auto f_(const int j, const int m) -> int& {
      return f_times[m * (fspData.noJobs() + 1) + j];
    }

    void compile(const ivec& seq) {
      compile(seq, 0);
    }

    void compile(const ivec& seq, int from) {
      const auto no_jobs = fspData.noJobs();
      const auto no_machines = fspData.noMachines();
      const auto seq_size = static_cast<int>(seq.size());

      for (int i = from + 1; i <= seq_size - 1; i++) {
        auto seq_i = seq[i - 1];
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
        int seq_i = seq[i - 1];
        for (int j = no_machines; j >= 1; j--) {
          q_(i, j) =
              std::max(q_(i, j + 1), q_(i + 1, j)) + fspData.pt(seq_i, j - 1);
        }
      }
      for (int i = 1; i <= seq_size; i++) {
        int seq_k = seq[seq_size - 1];
        for (int j = 1; j <= no_machines; j++) {
          f_(i, j) =
              std::max(f_(i, j - 1), e_(i - 1, j)) + fspData.pt(seq_k, j - 1);
        }
      }

      std::fill(makespan.begin(), makespan.end(), 0);
      for (int i = 1; i <= seq_size; i++) {
        for (int j = 1; j <= no_machines; j++) {
          int c_ij = f_(i, j) + q_(i, j);
          makespan[i - 1] = std::max(makespan[i - 1], c_ij);
        }
      }
    }

    auto getMakespan(const FSP& sol, const int first, const int second) -> int {
      auto mPtr = std::mismatch(compiledSolution.begin(), compiledSolution.end(),
                                sol.begin(), sol.end());
      int from = std::distance(sol.begin(), mPtr.second);

      if (mPtr.second != sol.end() || sol.size() != compiledSolution.size()) {
        EOT perm_i = sol;
        std::rotate(perm_i.begin() + first, perm_i.begin() + first + 1,
                    perm_i.end());

        compile(perm_i, std::max(0, from - 1));
        compiledSolution = sol;
      }
      return makespan[second];
    }
  };

  std::vector<CompiledSchedule> compiledSchedules;

 public:
  PermFSPNeighborMakespanEval(const FSPData& fspData) :
        compiledSchedules(fspData.noJobs(), fspData) {}

  void operator()(FSP& sol, FSPNeighbor& ngh) final {
    auto firstSecond = ngh.firstSecond(sol);
    auto& cache = compiledSchedules[firstSecond.first];
    ngh.fitness(cache.getMakespan(sol, firstSecond.first, firstSecond.second));
  }
};
