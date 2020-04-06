#pragma once

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

  std::string type() const final { return "NOWAIT"; }

 private:
  /** delayMatrix[i][j] = the values between the start of any two consecutive
   * jobs i and j */
  std::valarray<int> delayMatrix;

  void completionTime(const EOT& _fsp, std::valarray<int>& Ct) final {
    const int N = noJobs();
    const int _N = _fsp.size();
    // for each job...
    // Ci = sum delay + totalProcessinTime
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
  std::valarray<int> computeDelayMatrix() const {
    const int N = noJobs();
    const int M = noMachines();
    const auto& p = fsp_data.procTimesRef();
    std::valarray<int> _delayMatrix(N * N);
    // See : A heuristic for no-wait flow shop scheduling (2013) Sagar U. Sapkal
    // & Dipak Laha for the computation of the delay matrix
    // for each job...
    for (int i = 0; i < N; i++) {
      // for each job...
      for (int j = 0; j < N; j++) {
        if (i != j) {
          int max = 0;
          for (int r = 1; r <= M; r++) {
            int s = 0;
            for (int h = 1; h < r; h++)
              s += p[h * N + i];
            for (int h = 0; h < r - 1; h++)
              s -= p[h * N + j];
            if (s < 0)
              s = 0;
            if (s > max)
              max = s;
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
