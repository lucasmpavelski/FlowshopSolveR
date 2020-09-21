#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <vector>

#include <eoEvalFunc.h>

#include "flowshop-solver/problems/FSP.hpp"
#include "flowshop-solver/problems/FSPEvalFunc.hpp"

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
    for (int k = 0; k < noMachines() - 1; k++) {
      F(0, k) = fsp_data.pt(perm[0], k + 1);
    }
    for (int j = 1; j < noJobs(); j++) {
      for (int k = 0; k < noMachines() - 1; k++) {
        const auto p_j_k = fsp_data.pt(perm[j], k);
        const auto p_j_kp1 = fsp_data.pt(perm[j], k + 1);
        const auto F_jm1_k = F(j - 1, k);
        F(j, k) = std::max(F_jm1_k - p_j_k, 0) + p_j_kp1;
      }
    }
    int sum_f = 0;
    for (int k = 0; k < noMachines() - 1; k++) {
      sum_f += F(noJobs() - 1, k);
    }
    completionTimes[noJobs() - 1] = sum_f + fsp_data.machineProcTime(0);
    for (int j = noJobs() - 2; j >= 0; j--) {
      const auto p_jp1_m = fsp_data.pt(perm[j + 1], noMachines() - 1);
      completionTimes[j] = completionTimes[j + 1] - p_jp1_m;
    }

    // backward pass calculation
    // for (int k = 0; k < noMachines() - 1; k++) {
    //   E(noJobs() - 1, k) = fsp_data.pt(perm[noJobs() - 1], k);
    // }
    // for (int j = noJobs() - 2; j >= 0; j--) {
    //   for (int k = 0; k < noMachines() - 1; k++) {
    //     const auto p_j_k = fsp_data.pt(perm[j], k);
    //     const auto p_j_kp1 = fsp_data.pt(perm[j], k + 1);
    //     const auto E_jp1_k = E(j + 1, k);
    //     E(j, k) = std::max(E_jp1_k - p_j_kp1, 0) + p_j_k;
    //   }
    // }
    // int sum_f = 0;
    // for (int k = 0; k < noMachines() - 1; k++) {
    //   sum_f += E(0, k);
    // }
    // completionTimes[0] = sum_f + fsp_data.pt(0, noMachines() - 1);
    // for (int j = 0; j < noJobs() - 1; j++) {
    //   completionTimes[j + 1] = completionTimes[j] + fsp_data.pt(j + 1,
    //   noMachines() - 1);
    // }
  }
};

class NoIdleFSPNeighborEval : public moEval<FSPNeighbor> {
  const FSPData& fspData;
  const Objective objective;

  std::vector<int> _F;
  auto F(int j, int m) -> int& { return _F[m * fspData.noJobs() + j]; }
  std::vector<int> _E;
  auto E(int j, int m) -> int& { return _E[m * fspData.noJobs() + j]; }
  std::vector<int> _Ff;
  auto Ff(int j, int m) -> int& { return _Ff[m * fspData.noJobs() + j]; }
  std::vector<int> _Ct;

  [[nodiscard]] auto pt(int j, int m) -> int { return fspData.pt(j, m); }

 public:
  NoIdleFSPNeighborEval(const FSPData& fspData, const Objective& objective)
      : fspData(fspData),
        objective(objective),
        _F(fspData.noJobs() * fspData.noMachines(), 0),
        _E(fspData.noJobs() * fspData.noMachines(), 0),
        _Ff(fspData.noJobs() * fspData.noMachines(), 0),
        _Ct(fspData.noJobs(), 0) {}

  // [[nodiscard]] auto noJobs() -> int { return fspData.noJobs(); }
  [[nodiscard]] auto noMachines() -> int { return fspData.noMachines(); }

  void operator()(FSP& perm, FSPNeighbor& ngh) final {
    const auto firstSecond = ngh.firstSecond(perm);
    const auto& t = firstSecond.first;
    const auto& h = firstSecond.second;

    FSP partialPerm = perm;
    partialPerm.erase(begin(partialPerm) + t);

    const int noJobs = partialPerm.size();

    // Calculate F(p^E_j, k, k+1)
    for (int k = 0; k < noMachines() - 1; k++) {
      F(0, k) = pt(partialPerm[0], k + 1);
    }
    for (int j = 1; j < noJobs; j++) {
      for (int k = 0; k < noMachines() - 1; k++) {
        const auto p_j_k = pt(partialPerm[j], k);
        const auto p_j_kp1 = pt(partialPerm[j], k + 1);
        const auto F_jm1_k = F(j - 1, k);
        F(j, k) = std::max(F_jm1_k - p_j_k, 0) + p_j_kp1;
      }
    }

    // Calculate E(p^E_j, k, k+1)
    for (int k = 0; k < noMachines() - 1; k++) {
      E(noJobs - 1, k) = pt(partialPerm[noJobs - 1], k);
    }
    for (int j = noJobs - 2; j >= 0; j--) {
      for (int k = 0; k < noMachines() - 1; k++) {
        const auto p_j_k = pt(partialPerm[j], k);
        const auto p_j_kp1 = pt(partialPerm[j], k + 1);
        const auto E_jp1_k = E(j + 1, k);
        E(j, k) = std::max(E_jp1_k - p_j_kp1, 0) + p_j_k;
      }
    }

    // Calculate F(p^F_j, k, k+1)
    for (int k = 0; k < noMachines() - 1; k++) {
      Ff(partialPerm.size() - 1, k) = pt(partialPerm[partialPerm.size() - 1], k + 1);
    }
    for (int j = partialPerm.size() - 2; j >= 0; j--) {
      for (int k = 0; k < noMachines() - 1; k++) {
        const auto p_j_kp1 = pt(partialPerm[j], k + 1);//p 2 3
        const auto E_jp1_k = E(j + 1, k);//2
        const auto F_jp1_k = Ff(j + 1, k);//4

        Ff(j, k) = std::max(p_j_kp1 - E_jp1_k, 0) + F_jp1_k;
      }
    }

    // for (int i = 0; i < fspData.noJobs(); i++) {
    //   for (int j = 0; j < noMachines(); j++) {
    //     std::cout << "F(" << i << ", " << j << ") " << F(i, j) << ' ';
    //   }
    //   std::cout << '\n';
    // }
    // for (int i = 0; i < fspData.noJobs(); i++) {
    //   for (int j = 0; j < noMachines(); j++) {
    //     std::cout << "E(" << i << ", " << j << ") " << E(i, j) << ' ';
    //   }
    //   std::cout << '\n';
    // }
    // for (int i = 0; i < fspData.noJobs(); i++) {
    //   for (int j = 0; j < noMachines(); j++) {
    //     std::cout << "Ff(" << i << ", " << j << ") " << Ff(i, j) << ' ';
    //   }
    //   std::cout << '\n';
    // }

    // Calculate F(pEh, k, k+1) appending job h
    std::vector<int> FpEh(noMachines() -  1);
    for (int k = 0; k < noMachines() - 1; k++) {
      const auto p_h_k = pt(perm[t], k);
      const auto p_h_kp1 = pt(perm[t], k + 1);
      const auto F_hm1_k = F(h - 1, k);
      FpEh[k] = std::max(F_hm1_k - p_h_k, 0) + p_h_kp1;
    }

    // Calculate F(p, k, k+1) appending job h
    std::vector<int> Fp(noMachines() -  1);
    for (int k = 0; k < noMachines() - 1; k++) {
      const auto Fe_k = FpEh[k];
      const auto E_h_k = E(h, k);
      const auto Ff_h_k = Ff(h, k);
      Fp[k] = std::max(Fe_k - E_h_k, 0) + Ff_h_k;
    }

    FSP newPerm = partialPerm;
    newPerm.insert(begin(newPerm) + h, perm[t]);
    _Ct[perm.size() - 1] = std::accumulate(begin(Fp), end(Fp), 0) + fspData.machineProcTime(0);

    if (objective == Objective::MAKESPAN) {
      ngh.fitness(_Ct[perm.size() - 1]);
      return;
    }

    for (int j = newPerm.size() - 2; j >= 0; j--) {
      _Ct[j] = _Ct[j + 1] - pt(newPerm[j + 1], noMachines() - 1);
    }

    ngh.fitness(std::accumulate(begin(_Ct), end(_Ct), 0));
  }
};
