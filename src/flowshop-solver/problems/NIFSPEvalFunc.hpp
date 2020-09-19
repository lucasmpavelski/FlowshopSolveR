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
      H[j * N + 0] = p[j * N + _fsp[0]];
      for (int i = 1; i < _N; i++) {
        H[j * N + i] = H[j * N + i - 1] + p[j * N + _fsp[i]];
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
