#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/problems/FSP.hpp"
#include "flowshop-solver/problems/FSPData.hpp"
#include "flowshop-solver/problems/FSPEval.hpp"

class NoIdleCompletionTimesCompiler {
  const FSPData& fspData;

  std::vector<int> _F;
  auto F(int j, int m) -> int& { return _F[m * fspData.noJobs() + j]; }

 public:
  NoIdleCompletionTimesCompiler(const FSPData& fspData)
      : fspData(fspData), _F(fspData.noJobs() * (fspData.noMachines() - 1)) {}

  void compile(const std::vector<int>& perm,
               std::vector<int>& completionTimes) {
    const int noJobs = perm.size();
    const int noMachines = fspData.noMachines();

    // forward pass calculation
    for (int k = 0; k < noMachines - 1; k++) {
      F(0, k) = fspData.pt(perm[0], k + 1);
    }
    for (int j = 1; j < noJobs; j++) {
      for (int k = 0; k < noMachines - 1; k++) {
        const auto p_j_k = fspData.pt(perm[j], k);
        const auto p_j_kp1 = fspData.pt(perm[j], k + 1);
        const auto F_jm1_k = F(j - 1, k);
        F(j, k) = std::max(F_jm1_k - p_j_k, 0) + p_j_kp1;
      }
    }
    int sum_f = 0;
    for (int k = 0; k < noMachines - 1; k++) {
      sum_f += F(noJobs - 1, k);
    }
    completionTimes[noJobs - 1] = sum_f + fspData.machineProcTime(0);
    for (int j = noJobs - 2; j >= 0; j--) {
      const auto p_jp1_m = fspData.pt(perm[j + 1], noMachines - 1);
      completionTimes[j] = completionTimes[j + 1] - p_jp1_m;
    }

    // backward pass calculation
    // for (int k = 0; k < noMachines - 1; k++) {
    //   E(noJobs - 1, k) = fspData.pt(perm[noJobs - 1], k);
    // }
    // for (int j = noJobs - 2; j >= 0; j--) {
    //   for (int k = 0; k < noMachines - 1; k++) {
    //     const auto p_j_k = fspData.pt(perm[j], k);
    //     const auto p_j_kp1 = fspData.pt(perm[j], k + 1);
    //     const auto E_jp1_k = E(j + 1, k);
    //     E(j, k) = std::max(E_jp1_k - p_j_kp1, 0) + p_j_k;
    //   }
    // }
    // int sum_f = 0;
    // for (int k = 0; k < noMachines - 1; k++) {
    //   sum_f += E(0, k);
    // }
    // completionTimes[0] = sum_f + fspData.pt(0, noMachines - 1);
    // for (int j = 0; j < noJobs - 1; j++) {
    //   completionTimes[j + 1] = completionTimes[j] + fspData.pt(j + 1,
    //   noMachines - 1);
    // }
  }
};

class NoIdleFSPEval : public virtual FSPEval {
  NoIdleCompletionTimesCompiler compiler;

 public:
  NoIdleFSPEval(const FSPData& data) : compiler(data) {}

  [[nodiscard]] auto type() const -> std::string override { return "NOIDLE"; }

 protected:
  void compileCompletionTimes(const FSP& sol, std::vector<int>& ct) override {
    compiler.compile(sol, ct);
  }
};

class NoIdleFSPMakespanEval : public FSPMakespanEval, public NoIdleFSPEval {
 public:
  NoIdleFSPMakespanEval(const FSPData& fspData)
      : FSPEval{fspData}, NoIdleFSPEval{fspData} {}
};

class NoIdleFSPFlowtimeEval : public FSPFlowtimeEval, public NoIdleFSPEval {
 public:
  NoIdleFSPFlowtimeEval(const FSPData& fspData)
      : FSPEval{fspData}, NoIdleFSPEval{fspData} {}
};
