#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include <paradiseo/eo/eo>

#include "flowshop-solver/problems/FSP.hpp"
#include "flowshop-solver/problems/FSPData.hpp"

class FSPEval : public eoEvalFunc<FSP> {
  const FSPData& fspData;
  std::vector<int> compTimes;

 public:
  using Fitness = typename FSP::Fitness;

  FSPEval(const FSPData& fd) : fspData{fd}, compTimes(fspData.noJobs()) {}

  [[nodiscard]] auto noJobs() const -> int { return fspData.noJobs(); }
  [[nodiscard]] auto noMachines() const -> int { return fspData.noMachines(); }
  [[nodiscard]] auto getData() const -> const FSPData& { return fspData; }
  [[nodiscard]] virtual auto type() const -> std::string = 0;
  [[nodiscard]] virtual auto objective() const -> std::string = 0;

  void printOn(std::ostream& o) {
    o << "FSPEval\n"
      << "  type: " << type() << '\n'
      << "  objective: " << objective() << '\n'
      << fspData;
  }

 protected:
  auto completionTime(int i) -> int& { return compTimes[i]; }
  auto completionTimesRef() -> std::vector<int>& { return compTimes; }
  void compile(const FSP& perm) { compileCompletionTimes(perm, compTimes); }

  virtual void compileCompletionTimes(const FSP& perm,
                                      std::vector<int>& ct) = 0;
};

class FSPMakespanEval : virtual public FSPEval {
 public:
  FSPMakespanEval() {};

  void operator()(FSP& perm) override {
    compile(perm);
    perm.fitness(completionTime(static_cast<int>(perm.size()) - 1));
  }

  [[nodiscard]] auto objective() const -> std::string override {
    return "MAKESPAN";
  }
};

class FSPFlowtimeEval : virtual public FSPEval {
 public:
  FSPFlowtimeEval() {};
  
  void operator()(FSP& perm) override {
    compile(perm);
    auto beg = begin(completionTimesRef());
    perm.fitness(std::accumulate(beg, beg + perm.size(), 0));
  }

  [[nodiscard]] auto objective() const -> std::string override {
    return "MAKESPAN";
  }
};
