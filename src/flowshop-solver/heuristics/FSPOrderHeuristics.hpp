#pragma once

#include <algorithm>
#include <cmath>
#include <numeric>

#include <paradiseo/eo/eo>

#include "flowshop-solver/problems/FSP.hpp"
#include "flowshop-solver/problems/FSPData.hpp"

struct FSPOrderHeuristic : eoInit<FSP> {
  const FSPData& fspData;

  FSPOrderHeuristic(const FSPData& fspData) : fspData{fspData} {}

  void operator()(FSP& sol) override {
    sol.resize(fspData.noJobs());
    std::iota(begin(sol), end(sol), 0);
    auto order = soringOrder();
    std::sort(begin(sol), end(sol),
              [&order](int i, int j) { return order[i] < order[j]; });
  }

  virtual auto soringOrder() -> std::vector<double> = 0;
};

struct SumPijFOH : public FSPOrderHeuristic {
  auto soringOrder() -> std::vector<double> override {
    std::vector<double> pt(fspData.noJobs());
    for (int j = 0; j < fspData.noJobs(); j++) {
      for (int i = 0; i < fspData.noMachines(); i++) {
        pt[j] += fspData.pt(j, i);
      }
    }
    return pt;
  }
};

struct WSumPijFOH : public FSPOrderHeuristic {
  auto soringOrder() -> std::vector<double> override {
    std::vector<double> pt(fspData.noJobs());
    for (int j = 0; j < fspData.noJobs(); j++) {
      for (int i = 0; i < fspData.noMachines(); i++) {
        pt[j] += (fspData.noMachines() - i + 1) * fspData.pt(j, i);
      }
    }
    return pt;
  }
};

struct AbsDifFOH : public FSPOrderHeuristic {
  auto soringOrder() -> std::vector<double> override {
    std::vector<double> pt(fspData.noJobs());
    for (int j = 0; j < fspData.noJobs(); j++) {
      for (int i = 0; i < fspData.noMachines(); i++) {
        for (int k = 0; k < fspData.noJobs(); k++) {
          pt[j] += std::abs(fspData.pt(j, i) - fspData.pt(k, i));
        }
      }
    }
    return pt;
  }
};

struct WAbsDifFOH : public FSPOrderHeuristic {
  auto soringOrder() -> std::vector<double> override {
    std::vector<double> pt(fspData.noJobs());
    for (int j = 0; j < fspData.noJobs(); j++) {
      for (int i = 0; i < fspData.noMachines(); i++) {
        for (int k = 0; k < fspData.noJobs(); k++) {
          pt[j] += (fspData.noMachines() - i + 1) *
                   std::abs(fspData.pt(j, i) - fspData.pt(k, i));
        }
      }
    }
    return pt;
  }
};

struct WAbsDifFOH : public FSPOrderHeuristic {
  auto soringOrder() -> std::vector<double> override {
    std::vector<double> pt(fspData.noJobs());
    for (int j = 0; j < fspData.noJobs(); j++) {
      for (int i = 0; i < fspData.noMachines(); i++) {
        for (int k = 0; k < fspData.noJobs(); k++) {
          pt[j] += (fspData.noMachines() - i + 1) *
                   std::abs(fspData.pt(j, i) - fspData.pt(k, i));
        }
      }
    }
    return pt;
  }
};