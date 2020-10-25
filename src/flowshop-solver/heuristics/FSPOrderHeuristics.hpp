
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
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

    sol.invalidate();
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
    const int n = fspData.noJobs();
    const int m = fspData.noMachines();
    std::vector<double> pt(n);
    for (int j = 0; j < n; j++) {
      int avg = 0;
      for (int i = 0; i < m; i++) {
        avg += fspData.pt(j, i);
      }
      avg = avg / m;
      int sumSqDiff = 0, sumCubDiff = 0;
      for (int i = 0; i < m; i++) {
        sumSqDiff += std::pow(avg - fspData.pt(j, i), 2);
        sumCubDiff += std::pow(avg - fspData.pt(j, i), 3);
      }
      double std = std::sqrt((1.0 / (fspData.noMachines() - 1)) * sumSqDiff);
      double skew = std::sqrt((1.0 / fspData.noMachines()) * sumCubDiff) /
                  std::pow((1.0 / fspData.noMachines()) * sumSqDiff, 3.0 / 2.0);
      pt[j] = avg + std + std::abs(skew);
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
        for (int k = 0; k < i; k++)
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

class LR : public FSPOrderHeuristic {
  bool enableIT, enableAJ, enableCT;
public:
  LR(const FSPData& fspData, bool weighted, const std::string& orderType,
    bool enableIT, bool enableAJ, bool enableCT) :
    FSPOrderHeuristic{fspData, weighted, orderType},
    enableIT{enableIT}, enableAJ{enableAJ}, enableCT{enableCT} {}

  auto sortingOrder() -> std::vector<double> override {
    const auto& p = fspData.procTimesRef();
    const int noJobs = fspData.noJobs();
    const int noMachines = fspData.noMachines();

    std::vector<int> ct(noJobs * noMachines, 0);
    std::vector<int> scheduled;
    std::vector<int> unscheduled(noJobs);
    std::iota(begin(unscheduled), end(unscheduled), 0);

    std::vector<double> it(noJobs);
    std::vector<double> at(noJobs);

    while (!unscheduled.empty()) {
      const int k = scheduled.size();

      if (k > 0) {
        // TODO: use cached permutation compiler
        ct[0 * noJobs + 0] = p[0 * noJobs + scheduled[0]];
        for (std::vector<int>::size_type i = 1; i < scheduled.size(); i++) {
          ct[0 * noJobs + i] = ct[0 * noJobs + i - 1] + p[0 * noJobs + scheduled[i]];
        }
        for (int j = 1; j < noMachines; j++) {
          ct[j * noJobs + 0] = ct[(j - 1) * noJobs + 0] + p[j * noJobs + scheduled[0]];
        }
        for (int i = std::max(1, k - 1); i < k; i++) {
          int pt_index = scheduled[i];
          for (int j = 1; j < noMachines; j++) {
            int ct_jm1_m = ct[j * noJobs + i - 1];
            int ct_j_mm1 = ct[(j - 1) * noJobs + i];
            int pt_i = p[j * noJobs + pt_index];
            ct[j * noJobs + i] = 
              std::max(ct_jm1_m, ct_j_mm1) + pt_i;
          }
        }
      }

      for (auto i : unscheduled) {
        // calculate artificial times
        it[i] = 0.0;
        int cim = w(0) * ((k == 0 ? 0 : ct[0 * noJobs + k - 1]) + p[0 * noJobs + i]);
        double cpm = w(0) * (cim + artificialJobTime(i, 0, unscheduled));
        for (int j = 1; j < noMachines; j++) {
          int ct_k_j = k == 0 ? 0 : ct[j * noJobs + k - 1];
          int pt_i = p[j * noJobs + i]; 
          it[i] += w(j) * itWeight(j, k) * std::max(cim - ct_k_j, 0);
          cim = w(j) * (std::max(ct_k_j, cim) + pt_i);
          cpm = w(j) * (std::max(static_cast<double>(cim), cpm) + artificialJobTime(i, j, unscheduled));
        }
        at[i] = enableCT * cim + enableAJ * cpm;
      }

      auto selected = std::min_element(begin(unscheduled), end(unscheduled), [&](int i, int j) {
        double indexI = enableIT * (noJobs - k - 1) * it[i] + at[i];
        double indexJ = enableIT * (noJobs - k - 1) * it[j] + at[j];
        return indexI < indexJ;
      });

      scheduled.push_back(*selected);
      unscheduled.erase(selected);
    }

    std::vector<double> order(noJobs);
    for (int i = 0; i < noJobs; i++) {
      order[scheduled[i]] = i;
    }

    return order;
  }

  auto artificialJobTime(int i, int j, const std::vector<int>& unscheduled) -> double {
    double pt = 0;
    for (int job : unscheduled) {
      if (job != i) {
        pt += fspData.pt(job, j);
      }
    }  
    return pt / static_cast<double>(unscheduled.size() - 1);
  }

  auto itWeight(int j, int k) -> double {
    j++;
    k++;
    const double n = fspData.noJobs();
    const double m = fspData.noMachines();
    return m / (j + k * (m - j) / (n - 2));
  }
};

class NaganoMoccellin : public FSPOrderHeuristic {
public:
  using FSPOrderHeuristic::FSPOrderHeuristic;

  auto sortingOrder() -> std::vector<double> override {
    auto noJobs = fspData.noJobs();
    auto noMachines = fspData.noMachines();
    std::vector<int> lb(noJobs * noJobs);
    std::vector<int> ub(noJobs * noJobs);
    for (int j = 0; j < noJobs; j++) {
      for (int k = 0; k < noJobs; k++) {
        if (j != k) {
          lb[j * noJobs + k] = 0;
          ub[j * noJobs + k] = 0;
          for (int i = 1; i < noMachines; i++) {
            int p_ij = fspData.pt(j, i);
            int p_im1k = fspData.pt(k, i - 1);
            int ub_jk_m1 = ub[j * noJobs + k - 1];
            ub[j * noJobs + k] = std::max(0, ub_jk_m1 + p_im1k - p_ij);
            lb[j * noJobs + k] += w(i) * std::max(0, p_ij - p_im1k - ub[j * noJobs + k]);
          }
        }
      }
    }
    std::vector<double> order(noJobs);
    for (int k = 0; k < noJobs; k++) {
      int max_lb = 0;
      for (int j = 0; j < noJobs; j++) {
        if (j != k && lb[j * noJobs + k] > max_lb) {
          max_lb = lb[j * noJobs + k];
        }
      }
      order[k] = fspData.jobProcTimesRef()[k] - max_lb;
    }
    return order;
  }
};

class KK1 : public FSPOrderHeuristic {
public:
  using FSPOrderHeuristic::FSPOrderHeuristic;

  auto sortingOrder() -> std::vector<double> override {
    auto noJobs = fspData.noJobs();
    auto noMachines = fspData.noMachines();
    std::vector<double> order(noJobs);
    int mw = (noMachines - 1) * (noMachines - 2) / 2;
    for (int j = 0; j < noJobs; j++) {
      int a = 0, b = 0;
      for (int i = 0; i < noMachines; i++) {
        int p_ij = fspData.pt(j, i);
        a += (mw + noMachines - i - 1) * p_ij * w(i);
        b += (mw + i) * p_ij * w(i);
      }
      order[j] = std::min(a, b);
    }
    return order;
  }
};

class KK2 : public FSPOrderHeuristic {
public:
  using FSPOrderHeuristic::FSPOrderHeuristic;

  auto sortingOrder() -> std::vector<double> override {
    auto noJobs = fspData.noJobs();
    auto noMachines = fspData.noMachines();
    const int s = std::floor(noMachines / 2.0);
    const int t = std::ceil(noMachines / 2.0);
    const double eps = 1e-6;
    std::vector<double> order(noJobs);
    for (int j = 0; j < noJobs; j++) {
      double u = 0.0;
      for (int h = 1; h <= s; h++) {
        const int hi = h - 1;
        const double whs = (h - 0.75) / (s - 0.75) - eps;
        const int p_sp1mh_j = fspData.pt(j, s + 1 - hi);
        const int p_tph_j = fspData.pt(j, t + hi);
        u += whs * (p_sp1mh_j - p_tph_j) * w(hi);
      }
      double a = fspData.jobProcTime(j) + u;
      double b = fspData.jobProcTime(j) - u;
      order[j] = std::min(a, b);
    }
    return order;
  }
};

/**
 * FSP priority rule orders
 * - sum_pij from the original NEH
 * - dev_pig and avgdev_pij from An improved NEH-based heuristic for the
 * permutation flowshop problem by Xingye Dong, Houkuan Huang and Ping Chen, 
 * with skew addition by A new improved NEH heuristic for permutation flowshop 
 * scheduling problems by Weibo Liu, Yan Jin and Mark Price.
 * - remaining orders from Different initial sequences for the heuristic of
 * Nawaz, Enscore and Ham to minimize makespan, idletime or flowtime in the
 * static permutation flowshop sequencing problem by
 * Jose M. Framinan, Rainer Leisten and Chandrasekharan Rajendran
 * - lr by Liu, J., and C. R. Reeves.2001. Constructive and Composite Heuristic 
 *  Solutions to the P//Ct Scheduling Problem. European Journal of Operational 
 *  Research 132 (2): 439?452.
 */
inline auto buildPriority(const FSPData& data,
                          const std::string& name,
                          bool weighted,
                          const std::string& order) -> std::unique_ptr<FSPOrderHeuristic> {
  if (name == "sum_pij")
    return std::make_unique<SUM_PIJ>(data, weighted, order);
  if (name == "dev_pij")
    return std::make_unique<DEV_PIJ>(data, weighted, order);
  if (name == "avgdev_pij")
    return std::make_unique<AVGDEV_PIJ>(data, weighted, order);
  if (name == "abs_dif")
    return std::make_unique<ABS_DIF>(data, weighted, order);
  if (name == "ss_sra")
    return std::make_unique<SS_SRA>(data, weighted, order);
  if (name == "ss_srs")
    return std::make_unique<SS_SRS>(data, weighted, order);
  if (name == "ss_srn_rcn")
    return std::make_unique<SS_SRN_RCN>(data, weighted, order);
  if (name == "ss_sra_rcn")
    return std::make_unique<SS_SRA_RCN>(data, weighted, order);
  if (name == "ss_srs_rcn")
    return std::make_unique<SS_SRS_RCN>(data, weighted, order);
  if (name == "ss_sra_2rcn")
    return std::make_unique<SS_SRA_2RCN>(data, weighted, order);
  if (name == "ra_c1")
    return std::make_unique<RA_C1>(data, weighted, order);
  if (name == "ra_c2")
    return std::make_unique<RA_C2>(data, weighted, order);
  if (name == "ra_c3")
    return std::make_unique<RA_C3>(data, weighted, order);
  if (name == "lr_it_aj_ct")
    return std::make_unique<LR>(data, weighted, order, true, true, true);
  if (name == "lr_it_ct")
    return std::make_unique<LR>(data, weighted, order, true, false, true);
  if (name == "lr_it")
    return std::make_unique<LR>(data, weighted, order, true, false, false);
  if (name == "lr_aj")
    return std::make_unique<LR>(data, weighted, order, false, true, false);
  if (name == "lr_ct")
    return std::make_unique<LR>(data, weighted, order, false, false, true);
  if (name == "nm")
    return std::make_unique<NaganoMoccellin>(data, weighted, order);
  if (name == "kk1")
    return std::make_unique<KK1>(data, weighted, order);
  if (name == "kk2")
    return std::make_unique<KK2>(data, weighted, order);
  return nullptr;
};