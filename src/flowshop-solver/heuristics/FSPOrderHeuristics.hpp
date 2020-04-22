#pragma once

#include <algorithm>
#include <cmath>
#include <numeric>

#include <paradiseo/eo/eo>

#include "flowshop-solver/problems/FSP.hpp"
#include "flowshop-solver/problems/FSPData.hpp"

class FSPOrderHeuristic : eoInit<FSP> {
 protected:
  const FSPData& fspData;

 private:
  const bool weighted;
  std::vector<double> orderMetric;

 public:
  FSPOrderHeuristic(const FSPData& fspData, bool weighted)
      : fspData{fspData}, weighted{weighted} {}

  void operator()(FSP& sol) override {
    if (orderMetric.empty())
      orderMetric = sortingOrder();
    if (sol.empty()) {
      sol.resize(fspData.noJobs());
      std::iota(begin(sol), end(sol), 0);
    }
    std::sort(begin(sol), end(sol), [this](int i, int j) {
      return this->orderMetric[i] < this->orderMetric[j];
    });
  }

  auto w(int i) const -> int {
    return 1 + weighted & (fspData.noMachines() - i);
  }

  virtual auto sortingOrder() -> std::vector<double> = 0;
};

struct SUM_PIJ : public FSPOrderHeuristic {
  using FSPOrderHeuristic::FSPOrderHeuristic;
  auto sortingOrder() -> std::vector<double> override {
    std::vector<double> pt(fspData.noJobs(), 0);
    for (int j = 0; j < fspData.noJobs(); j++) {
      for (int i = 0; i < fspData.noMachines(); i++) {
        pt[j] += w(i) * fspData.pt(j, i);
      }
    }
    return pt;
  }
};

struct ABS_DIF : public FSPOrderHeuristic {
  using FSPOrderHeuristic::FSPOrderHeuristic;
  auto sortingOrder() -> std::vector<double> override {
    std::vector<double> pt(fspData.noJobs(), 0);
    for (int j = 0; j < fspData.noJobs(); j++) {
      for (int i = 0; i < fspData.noMachines(); i++) {
        for (int k = 0; k < fspData.noJobs(); k++) {
          pt[j] += w(i) * std::abs(fspData.pt(j, i) - fspData.pt(k, i));
        }
      }
    }
    return pt;
  }
};

struct StinsonSmithFOH : public FSPOrderHeuristic {
  using FSPOrderHeuristic::FSPOrderHeuristic;
  auto r(int j, int jl, int m) const -> int {
    return fspData.pt(j, m) - fspData.pt(jl, m - 1);
  }

  auto rs(int j, int jl, int m) const -> int {
    const auto carryOver = std::min(r(j, jl, m - 1), 0);
    return r(j, jl, m) + carryOver;
  }

  virtual auto value(int j, int jl, int m) -> double = 0;

  auto sortingOrder() -> std::vector<double> override {
    int n = fspData.noJobs();
    int m = fspData.noMachines();
    std::vector<double> pj(n, 0);
    for (int j = 0; j < n; j++) {
      for (int jl = 0; jl < n; jl++) {
        for (int i = 1; i < m; i++) {
          if (j != jl) {
            pj[j] += w(i) * value(j, jl, i);
          }
        }
      }
    }
    return pj;
  }
};

struct SS_SRA : public StinsonSmithFOH {
  using StinsonSmithFOH::StinsonSmithFOH;
  auto value(int j, int jl, int m) -> double override {
    return std::abs(r(j, jl, m));
  }
};

struct SS_SRS : public StinsonSmithFOH {
  using StinsonSmithFOH::StinsonSmithFOH;
  auto value(int j, int jl, int m) -> double override {
    return std::pow(r(j, jl, m), 2);
  }
};

struct SS_SRN_RCN : public StinsonSmithFOH {
  using StinsonSmithFOH::StinsonSmithFOH;
  auto value(int j, int jl, int m) -> double override {
    return std::abs(std::min(0, rs(j, jl, m)));
  }
};

struct SS_SRA_RCN : public StinsonSmithFOH {
  using StinsonSmithFOH::StinsonSmithFOH;
  auto value(int j, int jl, int m) -> double override {
    return std::abs(rs(j, jl, m));
  }
};

struct SS_SRS_RCN : public StinsonSmithFOH {
  using StinsonSmithFOH::StinsonSmithFOH;
  auto value(int j, int jl, int m) -> double override {
    return std::pow(rs(j, jl, m), 2);
  }
};

struct SS_SRA_2RCN : public StinsonSmithFOH {
  using StinsonSmithFOH::StinsonSmithFOH;
  auto value(int j, int jl, int m) -> double override {
    return std::max(r(j, jl, m), 0) + 2 * std::abs(std::min(r(j, jl, m), 0));
  }
};

class RagendranFOH : public FSPOrderHeuristic {
  using FSPOrderHeuristic::FSPOrderHeuristic;
  std::vector<int> lb_cache;
  void compute() {
    const auto n = fspData.noJobs();
    const auto m = fspData.noMachines();
    lb_cache.resize(n * m);
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < n; j++) {
        int sum = 0;
        for (int k = 0; k < n; k++)
          sum += fspData.pt(j, k);
        lb(j, i) = sum;
      }
    }
  }

  int current_j = -1;
  std::vector<int> lbjl_cache;
  void computeLbjl(int j) {
    const auto n = fspData.noJobs();
    const auto m = fspData.noMachines();
    lbjl_cache.resize(n * m);
    for (int i = 0; i < m; i++) {
      for (int jl = 0; jl < fspData.noJobs(); jl++) {
        int sum = 0;
        for (int k = 0; k < i; k++)
          sum += fspData.pt(jl, k);
        lbjl_cache[i * n + jl] =
            std::max(lb(j, 0) + sum, lb(j, i) + fspData.pt(jl, i));
      }
    }
    current_j = j;
  }

 public:
  auto lb(int j, int m) -> int& { return lb_cache[m * fspData.noJobs() + j]; }

  auto lbjl(int j, int jl, int m) -> int {
    if (m < 0)
      return 0;
    if (current_j != j)
      computeLbjl(j);
    return lbjl_cache[m * fspData.noJobs() + jl];
  }

  auto sortingOrder() -> std::vector<double> override {
    compute();
    int n = fspData.noJobs();
    int m = fspData.noMachines();
    std::vector<double> pj(n, 0);
    for (int j = 0; j < n; j++) {
      for (int jl = 0; jl < n; jl++) {
        for (int i = 1; i < m; i++) {
          if (j != jl) {
            pj[j] += w(i) * value(j, jl, i);
          }
        }
      }
    }
    return pj;
  }

  virtual auto value(int j, int jl, int m) -> double = 0;
};

struct RA_C1 : public RagendranFOH {
  using RagendranFOH::RagendranFOH;
  auto value(int j, int jl, int m) -> double override {
    return lb(j, m) + lbjl(j, jl, m);
  }
};

struct RA_C2 : public RagendranFOH {
  using RagendranFOH::RagendranFOH;
  auto value(int j, int jl, int m) -> double override {
    return std::max(-lb(j, m) + lbjl(j, jl, m - 1), 0);
  }
};

struct RA_C3 : public RagendranFOH {
  using RagendranFOH::RagendranFOH;
  auto value(int j, int jl, int m) -> double override {
    return std::abs(lb(j, m) - lbjl(j, jl, m - 1));
  }
};

class FSPOrderHeuristicFactory {
  const FSPData& data;

 public:
  FSPOrderHeuristicFactory(const FSPData& data) : data{data} {}

  auto build(const std::string& name, bool weighted)
      -> std::unique_ptr<FSPOrderHeuristic> {
    if (name == "sum_pij")
      return std::make_unique<SUM_PIJ>(data, weighted);
    if (name == "abs_dif")
      return std::make_unique<ABS_DIF>(data, weighted);
    if (name == "ss_sra")
      return std::make_unique<SS_SRA>(data, weighted);
    if (name == "ss_srs")
      return std::make_unique<SS_SRS>(data, weighted);
    if (name == "ss_srn_rcn")
      return std::make_unique<SS_SRN_RCN>(data, weighted);
    if (name == "ss_sra_rcn")
      return std::make_unique<SS_SRA_RCN>(data, weighted);
    if (name == "ss_srs_rcn")
      return std::make_unique<SS_SRS_RCN>(data, weighted);
    if (name == "ss_sra_2rcn")
      return std::make_unique<SS_SRA_2RCN>(data, weighted);
    if (name == "ra_c1")
      return std::make_unique<RA_C1>(data, weighted);
    if (name == "ra_c2")
      return std::make_unique<RA_C2>(data, weighted);
    if (name == "ra_c3")
      return std::make_unique<RA_C3>(data, weighted);
    return {nullptr};
  }
};