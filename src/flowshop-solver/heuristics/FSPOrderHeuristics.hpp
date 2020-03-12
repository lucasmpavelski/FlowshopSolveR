#pragma once

#include "problems/FSPEvalFunc.hpp"
#include "problems/FSPData.hpp"

struct compareByTotalProcTimes {
  const FSPData& fsp_data;

  compareByTotalProcTimes(const FSPData& _fsp_data) : fsp_data(_fsp_data) {}
  bool operator()(int& i, int& j) const {
    return fsp_data.jobProcTimesRef()[i] > fsp_data.jobProcTimesRef()[j];
  }
};

std::vector<int> totalProcTimes(const FSPData& dt) {
  std::vector<int> fsp(dt.noJobs());
  std::iota(fsp.begin(), fsp.end(), 0);
  std::sort(fsp.begin(), fsp.end(), compareByTotalProcTimes(dt));
  return fsp;
}
