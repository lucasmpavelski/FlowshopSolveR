#pragma once

#include "problems/FSPEvalFunc.hpp"
#include "problems/FSPData.hpp"
#include "problems/fastfspeval.hpp"

#include "FSPOrderHeuristics.hpp"

class FastNEH : public eoInit<FastFSPSolution> {
public:
  using ivec = std::vector<int>;

  CompiledSchedule compiledSchedule;
  const FSPData& fspData;
  ivec initialOrder;

  FastNEH(const FSPData& fspData)
    : fspData(fspData)
    , compiledSchedule(fspData.noJobs(), fspData.noMachines()) {
  }

  void operator()(FastFSPSolution& sol) final override {
    ivec initialOrder = getInitialOrder();
    sol.resize(0);
    sol.push_back(initialOrder[0]);
    for (unsigned i = 1; i < fspData.noJobs(); i++) {
      int jobToInsert = initialOrder[i];
      auto insert = findInsertionPosition(sol, jobToInsert);
      sol.insert(sol.begin() + insert.first, jobToInsert);
      sol.fitness(insert.second);
    }
  }

  virtual std::pair<int,FastFSPSolution::Fitness> findInsertionPosition(FastFSPSolution& sol, int jobToInsert) {
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
    return { idxToInsert, minFit };
  }

  virtual ivec getInitialOrder() {
    return totalProcTimes(fspData);
  }
};

class FastNEHRandom : public FastNEH {
public:
  FastNEHRandom(const FSPData& fspData)
    : FastNEH(fspData) {
  }

  using FastNEH::fspData;
  int cycle = 3;
  virtual ivec getInitialOrder() {
    ivec order = totalProcTimes(fspData);
    int n = fspData.noJobs();
    std::vector<int> a(cycle);
    std::generate(std::begin(a), std::end(a), [n]() { return rng.random(n); });
    int first = order[a[0]];
    for (int i = 0; i < cycle - 1; i++)
      order[a[i]] = order[a[i + 1]];
    order[a[cycle - 1]] = first;
    return order;
  }
};
