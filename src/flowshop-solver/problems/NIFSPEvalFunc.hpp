#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

#include <eoEvalFunc.h>

#include "FSPEvalFunc.hpp"

template <class EOT>
class NIFSPEvalFunc : public FSPEvalFunc<EOT> {
 public:
  using FSPEvalFunc<EOT>::noJobs;
  using FSPEvalFunc<EOT>::noMachines;
  using FSPEvalFunc<EOT>::fsp_data;

  NIFSPEvalFunc(FSPData fspd, Objective ObjT = Objective::MAKESPAN)
      : FSPEvalFunc<EOT>(std::move(fspd), ObjT),
        S(noMachines()),
        H(noMachines() * noJobs()) {}

  std::string type() const final { return "NOIDLE"; }

 private:
  std::valarray<int> S;
  std::valarray<int> H;

  void completionTime(const EOT& _fsp, std::vector<int>& Ct) override {
    const int _N = _fsp.size();
    const int M = noMachines();
    const int N = noJobs();
    const auto& p = fsp_data.procTimesRef();

    for (int j = 0; j < M; j++) {
      H[j * N + 0] = p[j * N + _fsp[0]];  // j th machine, first job
      for (int i = 1; i < _N; i++) {
        H[j * N + i] =
            H[j * N + i - 1] + p[j * N + _fsp[i]];  // j th machine, ith job
      }
    }
    // calculate when a given machine can start processing with no needed idle
    // time
    S[0] = 0;
    for (int j = 1; j < M; j++) {
      int max = H[(j - 1) * N + 0];
      for (int i = 1; i < _N; i++) {
        int tmp = H[(j - 1) * N + i] - H[j * N + i - 1];
        if (tmp > max)
          max = tmp;
      }
      S[j] = S[j - 1] + max;
    }
    // calculate the completion time
    Ct[0] = S[M - 1] + p[(M - 1) * N + _fsp[0]];
    for (int i = 1; i < _N; i++)
      Ct[i] = Ct[i - 1] + p[(M - 1) * N + _fsp[i]];

    // std::cout << "n last machine time = " << S[M - 1] << "\n";
    // std::cout << "Ct:\n";
    // for (int i = 0; i < _N; i++)
    //  std::cout << Ct[i] << " ";
    // std::cout << "Ct:\n";
  }
};

template <class EOT>
class NoIdleFSPEvalFunc : public FSPEvalFunc<EOT> {
  std::vector<int> _F;
  auto F(int j, int m) -> int& { return _F[m * noJobs() + j]; }
  std::vector<int> _E;
  auto E(int j, int m) -> int& { return _E[m * noJobs() + j]; }

 public:
  using FSPEvalFunc<EOT>::noJobs;
  using FSPEvalFunc<EOT>::noMachines;
  using FSPEvalFunc<EOT>::fsp_data;

  NoIdleFSPEvalFunc(FSPData fspd, Objective ObjT = Objective::MAKESPAN)
      : FSPEvalFunc<EOT>(std::move(fspd), ObjT),
        _F(noJobs() * (noMachines() - 1)),
        _E(noJobs() * (noMachines() - 1)) {}

  [[nodiscard]] auto type() const -> std::string final { return "NOIDLE"; }

  void completionTime(const EOT& perm,
                      std::vector<int>& completionTimes) override {
    // forward pass calculation
    // for (int k = 0; k < noMachines() - 1; k++) {
    //   F(0, k) = fsp_data.pt(perm[0], k + 1);
    // }
    // for (int j = 1; j < noJobs(); j++) {
    //   for (int k = 0; k < noMachines() - 1; k++) {
    //     const auto p_j_k = fsp_data.pt(perm[j], k);
    //     const auto p_j_kp1 = fsp_data.pt(perm[j], k + 1);
    //     const auto F_jm1_k = F(j - 1, k);
    //     F(j, k) = std::max(F_jm1_k - p_j_k, 0) + p_j_kp1;
    //   }
    // }
    // int sum_f = 0;
    // for (int k = 0; k < noMachines() - 1; k++) {
    //   sum_f += F(noJobs() - 1, k);
    // }
    // completionTimes[noJobs() - 1] = sum_f + fsp_data.machineProcTime(0);
    // for (int j = noJobs() - 2; j >= 0; j--) {
    //   const auto p_jp1_m = fsp_data.pt(perm[j + 1], noMachines() - 1);
    //   completionTimes[j] = completionTimes[j + 1] - p_jp1_m;
    // }

    // backward pass calculation
    for (int k = 0; k < noMachines() - 1; k++) {
      E(noJobs() - 1, k) = fsp_data.pt(perm[noJobs() - 1], k);
    }
    for (int j = noJobs() - 2; j >= 0; j--) {
      for (int k = 0; k < noMachines() - 1; k++) {
        const auto p_j_k = fsp_data.pt(perm[j], k);
        const auto p_j_kp1 = fsp_data.pt(perm[j], k + 1);
        const auto E_jp1_k = E(j + 1, k);
        E(j, k) = std::max(E_jp1_k - p_j_kp1, 0) + p_j_k;
      }
    }
    int sum_f = 0;
    for (int k = 0; k < noMachines() - 1; k++) {
      sum_f += E(0, k);
    }
    completionTimes[0] = sum_f + fsp_data.pt(0, noMachines() - 1);
    for (int j = 0; j < noJobs() - 1; j++) {
      completionTimes[j + 1] = completionTimes[j] + fsp_data.pt(j + 1, noMachines() - 1);
    }
  }
};
