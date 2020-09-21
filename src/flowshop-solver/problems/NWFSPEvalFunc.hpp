#pragma once

#include <ostream>
#include <vector>

#include "FSPEvalFunc.hpp"

#include <paradiseo/eo/eo>

template <class EOT>
class NWFSPEvalFunc : public FSPEvalFunc<EOT> {
 public:
  using FSPEvalFunc<EOT>::noJobs;
  using FSPEvalFunc<EOT>::noMachines;
  using FSPEvalFunc<EOT>::fsp_data;

  NWFSPEvalFunc(FSPData fd, Objective ObjT = Objective::MAKESPAN)
      : FSPEvalFunc<EOT>(std::move(fd), ObjT),
        delayMatrix(computeDelayMatrix()) {}

  [[nodiscard]] auto type() const -> std::string final { return "NOWAIT"; }

  [[nodiscard]] auto delay(int i, int j) const -> int {
    return delayMatrix[i * fsp_data.noJobs() + j];
  }

  auto printDelayMatrix(std::ostream& os) -> std::ostream& {
    for (int i = 0; i < fsp_data.noJobs(); i++) {
      for (int j = 0; j < fsp_data.noJobs(); j++) {
        os << delay(i, j) << ' ';
      }
      os << '\n';
    }
    return os;
  }

 private:
  /** delayMatrix[i][j] = the values between the start of any two consecutive
   * jobs i and j */
  std::vector<int> delayMatrix;

  void completionTime(const EOT& _fsp, std::vector<int>& Ct) override {
    const int N = noJobs();
    const int _N = _fsp.size();

    int delay = 0;
    Ct[0] = fsp_data.jobProcTimesRef()[_fsp[0]];

    for (int i = 1; i < _N; i++) {
      delay += delayMatrix[_fsp[i - 1] * N + _fsp[i]];
      Ct[i] = delay + fsp_data.jobProcTimesRef()[_fsp[i]];
    }
  }

  /**
   * computation of the delay matrix
   * delayMatrix[i][j] = the delay during the start of two jobs
   */
  [[nodiscard]] auto computeDelayMatrix() const -> std::vector<int> {
    const int N = noJobs();
    const int M = noMachines();
    const auto& p = fsp_data.procTimesRef();
    std::vector<int> _delayMatrix(N * N);

    // See : A heuristic for no-wait flow shop scheduling (2013) Sagar U. Sapkal
    // & Dipak Laha for the computation of the delay matrix
    // for each job...
    for (int i = 0; i < N; i++) {
      // for each job...
      for (int j = 0; j < N; j++) {
        if (i != j) {
          int max = 0, idx = 0;
          for (int r = 1; r <= M; r++) {
            int s = 0;
            for (int h = 1; h < r; h++)
              s += p[h * N + i];
            for (int h = 0; h < r - 1; h++)
              s -= p[h * N + j];
            if (s < 0)
              s = 0;
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
};
