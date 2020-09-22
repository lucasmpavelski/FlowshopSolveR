#pragma once

#include <ostream>
#include <vector>

#include "flowshop-solver/problems/FSPData.hpp"
#include "flowshop-solver/problems/FSPEval.hpp"

#include <paradiseo/eo/eo>

class NoWaitCompletionTimeCompiler {
  const FSPData& fspData;
  std::vector<int> delayMatrix;

  [[nodiscard]] auto computeDelayMatrix() const -> std::vector<int> {
    const int N = fspData.noJobs();
    const int M = fspData.noMachines();
    const auto& p = fspData.procTimesRef();
    std::vector<int> _delayMatrix(N * N);

    // See : A heuristic for no-wait flow shop scheduling (2013) Sagar U. Sapkal
    // & Dipak Laha for the computation of the delay matrix
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        if (i != j) {
          int max = 0, idx = 0;
          for (int r = 1; r <= M; r++) {
            int s = 0;
            for (int h = 1; h < r; h++) {
              s += p[h * N + i];
            }
            for (int h = 0; h < r - 1; h++) {
              s -= p[h * N + j];
            }
            if (s < 0) {
              s = 0;
            }
            if (s > max) {
              max = s;
              idx = r;
            }
          }
          _delayMatrix[i * N + j] = p[0 * N + i] + max;
        } else {
          _delayMatrix[i * N + j] = 0;
        }
      }
    }
    return _delayMatrix;
  }

 public:
  NoWaitCompletionTimeCompiler(const FSPData& fspData)
      : fspData{fspData}, delayMatrix{computeDelayMatrix()} {}

  void compile(const std::vector<int>& _fsp, std::vector<int>& Ct) {
    const int N = fspData.noJobs();
    const int _N = _fsp.size();

    int delay = 0;
    Ct[0] = fspData.jobProcTimesRef()[_fsp[0]];

    for (int i = 1; i < _N; i++) {
      delay += delayMatrix[_fsp[i - 1] * N + _fsp[i]];
      Ct[i] = delay + fspData.jobProcTimesRef()[_fsp[i]];
    }
  }

  [[nodiscard]] auto delay(int i, int j) const -> int {
    return delayMatrix[i * fspData.noJobs() + j];
  }
};

class NoWaitFSPEval : virtual public FSPEval {
  NoWaitCompletionTimeCompiler compiler;

 public:
  NoWaitFSPEval(const FSPData& fspData) : compiler{fspData} {}

  [[nodiscard]] auto type() const -> std::string final { return "NOWAIT"; }

  [[nodiscard]] auto delay(int i, int j) const -> int {
    return compiler.delay(i, j);
  }

  using FSPEval::getData;
  auto printDelayMatrix(std::ostream& os) -> std::ostream& {
    for (int i = 0; i < getData().noJobs(); i++) {
      for (int j = 0; j < getData().noJobs(); j++) {
        os << delay(i, j) << ' ';
      }
      os << '\n';
    }
    return os;
  }

 protected:
  void compileCompletionTimes(const FSP& perm, std::vector<int>& ct) override {
    compiler.compile(perm, ct);
  }
};

class NoWaitFSPMakespanEval : public NoWaitFSPEval, public FSPMakespanEval {
 public:
  NoWaitFSPMakespanEval(const FSPData& fspData)
      : FSPEval{fspData}, NoWaitFSPEval{fspData} {}
};

class NoWaitFSPFlowtimeEval : public NoWaitFSPEval, public FSPFlowtimeEval {
 public:
  NoWaitFSPFlowtimeEval(const FSPData& fspData)
      : FSPEval{fspData}, NoWaitFSPEval{fspData} {}
};
