
#pragma once

#include <algorithm>
#include <cmath>
#include <numeric>

#include <paradiseo/eo/eo>

#include "flowshop-solver/problems/FSP.hpp"
#include "flowshop-solver/problems/FSPData.hpp"

class FSPOrderHeuristic : public eoInit<FSP> {
 protected:
  const FSPData& fspData;

 private:
  const bool weighted;
  std::vector<int> order;
  std::vector<double> indicator;

 public:
  FSPOrderHeuristic(const FSPData& fspData,
                    bool weighted,
                    const std::string& orderType)
      : fspData{fspData}, weighted{weighted}, order{getOrder(orderType)} {}

  auto getOrder(const std::string& orderType) -> std::vector<int> {
    const int n = fspData.noJobs();
    std::vector<int> order(n);
    std::iota(begin(order), end(order), 0);
    if (orderType == "incr") {
      return order;
    }
    if (orderType == "decr") {
      std::reverse(begin(order), end(order));
    }
    if (orderType == "hill") {
      std::transform(begin(order), end(order), begin(order), [&n](int i) {
        return i < n / 2 ? 2 * i : 2 * (n - i) - 1;
      });
    }
    if (orderType == "valley") {
      std::transform(begin(order), end(order), begin(order), [&n](int i) {
        return i < n / 2 ? n - 2 * i - 1 : 2 * i - n;
      });
    }
    if (orderType == "hi_hilo") {
      std::transform(begin(order), end(order), begin(order), [&n](int i) {
        return i % 2 == 0 ? n - i / 2 - 1 : (i - 1) / 2;
      });
    }
    if (orderType == "hi_lohi") {
      std::transform(begin(order), end(order), begin(order), [&n](int i) {
        return i % 2 == 0 ? i / 2 : n - i / 2 - 1;
      });
    }
    if (orderType == "lo_hilo") {
      std::transform(begin(order), end(order), begin(order), [&n](int i) {
        return i % 2 == 0 ? n / 2 + i / 2 : n / 2 - (i - 1) / 2 - 1;
      });
    }
    if (orderType == "lo_lohi") {
      std::transform(begin(order), end(order), begin(order), [&n](int i) {
        return i % 2 == 0 ? n / 2 - i / 2 - 1 : n / 2 + (i - 1) / 2;
      });
    }
    return order;
  }

  void operator()(FSP& sol) override {
    if (indicator.empty()) {
      indicator = sortingOrder();
    }

    if (sol.empty()) {
      sol.resize(fspData.noJobs());
      std::iota(begin(sol), end(sol), 0);
    }

    std::sort(begin(sol), end(sol), [this](int i, int j) {
      return this->indicator[i] < this->indicator[j];
    });

    FSP tmp = sol;
    for (unsigned i = 0; i < sol.size(); i++)
      sol[i] = tmp[order[i]];
  }

  [[nodiscard]] auto w(int i) const -> int {
    return 1 + (weighted & (fspData.noMachines() - i));
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

struct DEV_PIJ : public FSPOrderHeuristic {
  using FSPOrderHeuristic::FSPOrderHeuristic;
  auto sortingOrder() -> std::vector<double> override {
    std::vector<double> pt(fspData.noJobs(), 0);
    for (int j = 0; j < fspData.noJobs(); j++) {
      double avg = 0;
      for (int i = 0; i < fspData.noMachines(); i++)
        avg += fspData.pt(j, i);
      avg = avg / fspData.noMachines();
      int dev = 0;
      for (int i = 0; i < fspData.noMachines(); i++)
        dev += std::pow(avg - fspData.pt(j, i), 2);
      pt[j] = std::sqrt(dev);
    }
    return pt;
  }
};

struct AVGDEV_PIJ : public FSPOrderHeuristic {
  using FSPOrderHeuristic::FSPOrderHeuristic;
  auto sortingOrder() -> std::vector<double> override {
    std::vector<double> pt(fspData.noJobs(), 0);
    for (int j = 0; j < fspData.noJobs(); j++) {
      int avg = 0;
      for (int i = 0; i < fspData.noMachines(); i++)
        avg += fspData.pt(j, i);
      avg = avg / fspData.noMachines();
      int dev = 0;
      for (int i = 0; i < fspData.noMachines(); i++) {
        dev += std::pow(avg - fspData.pt(j, i), 2);
      }
      double std = std::sqrt((1.0 / (fspData.noMachines() - 1)) * dev);
      pt[j] = avg + std;
    }
    return pt;
  }
};

struct StinsonSmithFOH : public FSPOrderHeuristic {
  using FSPOrderHeuristic::FSPOrderHeuristic;
  [[nodiscard]] auto r(int j, int jl, int i) const -> int {
    return i == 0 ? 0 : fspData.pt(j, i) - fspData.pt(jl, i - 1);
  }

  [[nodiscard]] auto rs(int j, int jl, int i) const -> int {
    const auto carryOver = std::min(r(j, jl, i - 1), 0);
    return r(j, jl, i) + carryOver;
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

/**
 * FSP priority rule orders
 * - sum_pij from the original NEH
 * - dev_pig and avgdev_pij from An improved NEH-based heuristic for the
 * permutation flowshop problem by Xingye Dong, Houkuan Huang and Ping Chen
 * - remaining orders from Different initial sequences for the heuristic of
 * Nawaz, Enscore and Ham to minimize makespan, idletime or flowtime in the
 * static permutation flowshop sequencing problem by
 * Jose M. Framinan, Rainer Leisten and Chandrasekharan Rajendran
 */
inline auto buildPriority(const FSPData& data,
                          const std::string& name,
                          bool weighted,
                          const std::string& order) -> FSPOrderHeuristic* {
  if (name == "sum_pij")
    return new SUM_PIJ(data, weighted, order);
  if (name == "dev_pij")
    return new DEV_PIJ(data, weighted, order);
  if (name == "avgdev_pij")
    return new AVGDEV_PIJ(data, weighted, order);
  if (name == "abs_dif")
    return new ABS_DIF(data, weighted, order);
  if (name == "ss_sra")
    return new SS_SRA(data, weighted, order);
  if (name == "ss_srs")
    return new SS_SRS(data, weighted, order);
  if (name == "ss_srn_rcn")
    return new SS_SRN_RCN(data, weighted, order);
  if (name == "ss_sra_rcn")
    return new SS_SRA_RCN(data, weighted, order);
  if (name == "ss_srs_rcn")
    return new SS_SRS_RCN(data, weighted, order);
  if (name == "ss_sra_2rcn")
    return new SS_SRA_2RCN(data, weighted, order);
  if (name == "ra_c1")
    return new RA_C1(data, weighted, order);
  if (name == "ra_c2")
    return new RA_C2(data, weighted, order);
  if (name == "ra_c3")
    return new RA_C3(data, weighted, order);
  return nullptr;
};