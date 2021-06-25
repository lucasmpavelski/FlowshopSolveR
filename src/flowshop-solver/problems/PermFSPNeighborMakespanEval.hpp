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
          e_times((fspData.noJobs() + 1) * (fspData.noMachines() + 1), 0),
          q_times((fspData.noJobs() + 1) * (fspData.noMachines() + 2), 0),
          f_times((fspData.noJobs() + 1) * (fspData.noMachines() + 1), 0),
          makespan(fspData.noJobs()) {
    }

    inline auto idx(const int& j, const int& m, const int& no_jobs) -> std::size_t {
      return m * (no_jobs + 1) + j;
    }

    void compile(const FSP& seq) {
      compile(seq, 0);
    }

    void compile(const FSP& seq, int from) {
      const auto no_jobs = fspData.noJobs();
      const auto no_machines = fspData.noMachines();
      const auto seq_size = static_cast<int>(seq.size());

      for (int i = from + 1; i <= seq_size - 1; i++) {
        auto seq_i = seq[i - 1];
        for (int j = 1; j <= no_machines; j++) {
          const auto e_i_jm1 = e_times[idx(i, j - 1, no_jobs)];
          const auto e_im1_j = e_times[idx(i - 1, j, no_jobs)];
          e_times[idx(i, j, no_jobs)] =
              std::max(e_i_jm1, e_im1_j) + fspData.pt(seq_i, j - 1);
        }
      }

      for (int i = no_jobs; i >= seq_size - 1; i--) {
        for (int j = no_machines; j >= 1; j--) {
          q_times[idx(i, j, no_jobs)] = 0;
        }
      }
      for (int i = seq_size - 1; i >= 1; i--) {
        int seq_i = seq[i - 1];
        for (int j = no_machines; j >= 1; j--) {
          const auto q_i_jp1 = q_times[idx(i, j + 1, no_jobs)];
          const auto q_ip1_j = q_times[idx(i + 1, j, no_jobs)];
          q_times[idx(i, j, no_jobs)] =
              std::max(q_i_jp1, q_ip1_j) + fspData.pt(seq_i, j - 1);
        }
      }
      for (int i = 1; i <= seq_size; i++) {
        int seq_k = seq[seq_size - 1];
        for (int j = 1; j <= no_machines; j++) {
          const auto f_i_jm1 = f_times[idx(i, j - 1, no_jobs)];
          const auto e_im1_j = e_times[idx(i - 1, j, no_jobs)];
          f_times[idx(i, j, no_jobs)] =
              std::max(f_i_jm1, e_im1_j) + fspData.pt(seq_k, j - 1);
        }
      }

      std::fill(makespan.begin(), makespan.end(), 0);
      for (int i = 1; i <= seq_size; i++) {
        for (int j = 1; j <= no_machines; j++) {
          const auto f_i_j = f_times[idx(i, j, no_jobs)];
          const auto q_i_j = q_times[idx(i, j, no_jobs)];
          int c_ij = f_i_j + q_i_j;
          makespan[i - 1] = std::max(makespan[i - 1], c_ij);
        }
      }
    }

    auto getMakespan(const FSP& sol, const int first, const int second) -> int {
      // auto mPtr = std::mismatch(compiledSolution.begin(), compiledSolution.end(),
      //                           sol.begin(), sol.end());
      // int from = std::distance(sol.begin(), mPtr.second);

      // if (mPtr.second != sol.end() || sol.size() != compiledSolution.size()) {
      //   EOT perm_i = sol;
      //   std::rotate(perm_i.begin() + first, perm_i.begin() + first + 1,
      //               perm_i.end());

      //   compile(perm_i, std::max(0, from - 1));
      //   compiledSolution = sol;
      // }
      if (sol != compiledSolution) {
          EOT perm_i = sol;
          std::rotate(perm_i.begin() + first, perm_i.begin() + first + 1,
                      perm_i.end());
          
          compile(perm_i);
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
