#pragma once

#include <paradiseo/eo/eo>

#include "flowshop-solver/problems/FSP.hpp"
#include "flowshop-solver/problems/FSPData.hpp"

class NoIdleFSPNeighborEvalCompiler {
  class NoIdleFSPNeighborEvalCache {
    const FSPData& fspData;
    FSP compiledCache;
    using job_t = unsigned short;

    std::vector<job_t> _F;
    std::vector<job_t> _E;
    std::vector<job_t> _Ff;

    [[nodiscard]] auto pt(int j, int m) -> job_t { return fspData.pt(j, m); }

   public:
    NoIdleFSPNeighborEvalCache(const FSPData& fspData)
        : fspData(fspData),
          compiledCache(fspData.noJobs(), -1),
          _F(fspData.noJobs() * fspData.noMachines(), 0),
          _E(fspData.noJobs() * fspData.noMachines(), 0),
          _Ff(fspData.noJobs() * fspData.noMachines(), 0) {}

    auto F(int j, int m) -> job_t& { return _F[m * fspData.noJobs() + j]; }
    auto E(int j, int m) -> job_t& { return _E[m * fspData.noJobs() + j]; }
    auto Ff(int j, int m) -> job_t& { return _Ff[m * fspData.noJobs() + j]; }

    [[nodiscard]] auto F(int j, int m) const -> job_t {
      return _F[m * fspData.noJobs() + j];
    }
    [[nodiscard]] auto E(int j, int m) const -> job_t {
      return _E[m * fspData.noJobs() + j];
    }
    [[nodiscard]] auto Ff(int j, int m) const -> job_t {
      return _Ff[m * fspData.noJobs() + j];
    }

    void compile(const FSP& partialPerm) {
      auto firstDiff = std::mismatch(begin(partialPerm), end(partialPerm),
                                     begin(compiledCache));
      if (firstDiff.first == partialPerm.end()) {
        // std::cout << "cached!\n";
        return;
      }

      const int noJobs = partialPerm.size();
      const int noMachines = fspData.noMachines();

      // Calculate F(p^E_j, k, k+1)
      for (int k = 0; k < noMachines - 1; k++) {
        F(0, k) = pt(partialPerm[0], k + 1);
      }
      for (int j = 1; j < noJobs; j++) {
        for (int k = 0; k < noMachines - 1; k++) {
          const auto p_j_k = pt(partialPerm[j], k);
          const auto p_j_kp1 = pt(partialPerm[j], k + 1);
          const auto F_jm1_k = F(j - 1, k);
          F(j, k) = std::max(F_jm1_k - p_j_k, 0) + p_j_kp1;
        }
      }

      // Calculate E(p^E_j, k, k+1)
      for (int k = 0; k < noMachines - 1; k++) {
        E(noJobs - 1, k) = pt(partialPerm[noJobs - 1], k);
      }
      for (int j = noJobs - 2; j >= 0; j--) {
        for (int k = 0; k < noMachines - 1; k++) {
          const auto p_j_k = pt(partialPerm[j], k);
          const auto p_j_kp1 = pt(partialPerm[j], k + 1);
          const auto E_jp1_k = E(j + 1, k);
          E(j, k) = std::max(E_jp1_k - p_j_kp1, 0) + p_j_k;
        }
      }

      // Calculate F(p^F_j, k, k+1)
      for (int k = 0; k < noMachines - 1; k++) {
        Ff(noJobs - 1, k) = pt(partialPerm[noJobs - 1], k + 1);
      }
      for (int j = noJobs - 2; j >= 0; j--) {
        for (int k = 0; k < noMachines - 1; k++) {
          const auto p_j_kp1 = pt(partialPerm[j], k + 1);
          const auto E_jp1_k = E(j + 1, k);
          const auto F_jp1_k = Ff(j + 1, k);

          Ff(j, k) = std::max(p_j_kp1 - E_jp1_k, 0) + F_jp1_k;
        }
      }

      compiledCache = partialPerm;
    }
  };

  const FSPData& fspData;
  std::vector<NoIdleFSPNeighborEvalCache> caches;
  std::vector<int> _Ct;

 public:
  NoIdleFSPNeighborEvalCompiler(const FSPData& fspData)
      : fspData(fspData),
        caches(fspData.noJobs(), fspData),
        _Ct(fspData.noJobs(), 0) {}

  // [[nodiscard]] auto noJobs() -> int { return fspData.noJobs(); }

  void compile(const std::vector<int>& perm,
               const std::pair<unsigned, unsigned>& move) {
    const auto& t = move.first;
    const auto& h = move.second;

    FSP partialPerm = perm;
    partialPerm.erase(begin(partialPerm) + t);

    caches[t].compile(partialPerm);
    const auto& cache = caches[t];

    const int noMachines = fspData.noMachines();

    // Calculate F(pEh, k, k+1) appending job h
    std::vector<int> FpEh(noMachines - 1, 0);
    if (h == 0) {
      for (int k = 0; k < noMachines - 1; k++) {
        FpEh[k] = fspData.pt(perm[t], k + 1);
      }

    } else {
      for (int k = 0; k < noMachines - 1; k++) {
        const auto p_h_k = fspData.pt(perm[t], k);
        const auto p_h_kp1 = fspData.pt(perm[t], k + 1);
        const auto F_hm1_k = cache.F(static_cast<int>(h) - 1, k);
        FpEh[k] = std::max(F_hm1_k - p_h_k, 0) + p_h_kp1;
      }
    }

    // Calculate F(p, k, k+1) appending job h
    std::vector<int> Fp(noMachines - 1, 0);
    for (int k = 0; k < noMachines - 1; k++) {
      const auto Fe_k = FpEh[k];
      const auto E_h_k = cache.E(h, k);
      const auto Ff_h_k = cache.Ff(h, k);
      Fp[k] = std::max(Fe_k - E_h_k, 0) + Ff_h_k;
    }

    FSP newPerm = partialPerm;
    newPerm.insert(begin(newPerm) + h, perm[t]);
    _Ct[perm.size() - 1] =
        std::accumulate(begin(Fp), end(Fp), 0) + fspData.machineProcTime(0);

    for (int j = static_cast<int>(newPerm.size()) - 2; j >= 0; j--) {
      _Ct[j] = _Ct[j + 1] - fspData.pt(newPerm[j + 1], noMachines - 1);
    }
  }

  [[nodiscard]] auto compiledCompletionTime(int i) const -> int {
    return _Ct[i];
  }

  [[nodiscard]] auto sumCompletionTimes() const -> int {
    return std::accumulate(begin(_Ct), end(_Ct), 0);
  }
};

class NoIdleFSPNeighborMakespanEval : public moEval<FSPNeighbor> {
  NoIdleFSPNeighborEvalCompiler compiler;

 public:
  NoIdleFSPNeighborMakespanEval(const FSPData& data) : compiler{data} {}

  void operator()(FSP& perm, FSPNeighbor& ngh) final {
    const auto move = ngh.firstSecond(perm.size());
    compiler.compile(perm, move);
    ngh.fitness(
        compiler.compiledCompletionTime(static_cast<int>(perm.size()) - 1));
  }
};

class NoIdleFSPNeighborFlowtimeEval : public moEval<FSPNeighbor> {
  NoIdleFSPNeighborEvalCompiler compiler;

 public:
  NoIdleFSPNeighborFlowtimeEval(const FSPData& data) : compiler{data} {}

  void operator()(FSP& perm, FSPNeighbor& ngh) final {
    const auto move = ngh.firstSecond(perm.size());
    compiler.compile(perm, move);
    ngh.fitness(compiler.sumCompletionTimes());
  }
};