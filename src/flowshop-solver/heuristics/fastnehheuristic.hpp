#pragma once

#include "FSPOrderHeuristics.hpp"
#include "problems/FSPData.hpp"
#include "problems/FSPEvalFunc.hpp"
#include "problems/FastFSPNeighborEval.hpp"

class FastNEH : public eoInit<FSP> {
 public:
  using ivec = std::vector<int>;

  const FSPData& fspData;
  CompiledSchedule compiledSchedule;
  ivec initialOrder;

  FastNEH(const FSPData& fspData)
      : fspData(fspData),
        compiledSchedule(fspData.noJobs(), fspData.noMachines()) {}

  void operator()(FSP& sol) final {
    ivec initialOrder = getInitialOrder();
    sol.resize(0);
    sol.push_back(initialOrder[0]);
    for (int i = 1; i < fspData.noJobs(); i++) {
      int jobToInsert = initialOrder[i];
      auto insert = findInsertionPosition(sol, jobToInsert);
      sol.insert(sol.begin() + insert.first, jobToInsert);
      sol.fitness(insert.second);
    }
  }

  virtual auto findInsertionPosition(FSP& sol,
                                                             int jobToInsert) -> std::pair<int, FSP::Fitness> {
    sol.push_back(jobToInsert);
    compiledSchedule.compile(fspData, sol);
    double minFit = std::numeric_limits<double>::infinity();
    unsigned idxToInsert = 0;
    for (unsigned i = 0; i < sol.size(); i++) {
      double fit = compiledSchedule.getMakespan(i);
      if (fit < minFit) {
        minFit = fit;
        idxToInsert = i;
      }
    }
    sol.pop_back();
    return {idxToInsert, minFit};
  }

  virtual ivec getInitialOrder() {
    FSP order;
    SUM_PIJ(fspData, false, "incr")(order);
    return std::move(order);
  }
};

class FastNEHRandom : public FastNEH {
 public:
  FastNEHRandom(const FSPData& fspData) : FastNEH(fspData) {}

  using FastNEH::fspData;
  int cycle = 3;
  virtual ivec getInitialOrder() {
    FSP order;
    SUM_PIJ(fspData, false, "incr")(order);
    int n = fspData.noJobs();
    std::vector<int> a(cycle);
    std::generate(std::begin(a), std::end(a), [n]() { return rng.random(n); });
    int first = order[a[0]];
    for (int i = 0; i < cycle - 1; i++)
      order[a[i]] = order[a[i + 1]];
    order[a[cycle - 1]] = first;
    return std::move(order);
  }
};
