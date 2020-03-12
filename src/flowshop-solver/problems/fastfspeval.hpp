#pragma once

#include <vector>
#include <algorithm>
#include <utility>

#include <mo>

#include "moHiResTimeContinuator.hpp"
#include "heuristics/fspmoshiftneighbor.hpp"
#include "problems/FSPData.hpp"
#include "problems/Problem.hpp"
#include "problems/FSP.hpp"
#include "problems/FSPEvalFunc.hpp"
using ivec = std::vector<int>;

struct CompiledSchedule {

  unsigned no_jobs, no_machines;
  ivec e_times, q_times, f_times;
  ivec makespan, flowtime;
  bool compiled = false;

  CompiledSchedule(unsigned no_jobs, unsigned no_machines)
    : no_jobs(no_jobs)
    , no_machines(no_machines)
    , e_times((no_jobs + 1) * (no_machines + 1))
    , q_times((no_jobs + 1) * (no_machines + 2))
    , f_times((no_jobs + 1) * (no_machines + 1))
    , makespan(no_jobs)
    , flowtime(no_jobs)
  {
    e_(0, 0) = 0;
    for (unsigned i = 1; i <= no_machines; i++)
      e_(0, i) = 0;
    for (unsigned i = 1; i <= no_jobs; i++)
      e_(i, 0) = 0;
    q_(no_jobs, no_machines) = 0;
    for (unsigned j = no_machines; j >= 1; j--)
      q_(no_jobs, j) = 0;
    for (unsigned i = no_jobs - 1; i >= 1; i--)
      q_(i, no_machines + 1) = 0;
    for (unsigned i = 0; i <= no_jobs; i++)
      f_(i, 0) = 0;
  }

    inline int& e_(const unsigned j, const unsigned m) {
      return e_times[m * (no_jobs + 1) + j];
    }

    inline int& q_(const unsigned j, const unsigned m) {
      return q_times[m * (no_jobs + 1) + j];
    }

    inline int& f_(const unsigned j, const unsigned m) {
      return f_times[m * (no_jobs + 1) + j];
    }

  void compile(const FSPData& fspData, const ivec& seq) {
    const unsigned seq_size = static_cast<unsigned>(seq.size());
    for (unsigned i = 1; i <= seq_size - 1; i++) {
      unsigned seq_i = static_cast<unsigned>(seq[i - 1]);
      for (unsigned j = 1; j <= no_machines; j++) {
        e_(i, j) = std::max(e_(i, j - 1), e_(i - 1, j)) + fspData.pt(seq_i, j-1);
      }
    }
    for (unsigned i = seq_size - 1; i >= 1; i--) {
      unsigned seq_i = static_cast<unsigned>(seq[i - 1]);
      for (unsigned j = no_machines; j >= 1; j--) {
        q_(i, j) = std::max(q_(i, j + 1), q_(i + 1, j)) + fspData.pt(seq_i, j - 1);
      }
    }
    for (unsigned i = 1; i <= seq_size; i++) {
      unsigned seq_k = static_cast<unsigned>(seq[seq_size - 1]);
      for (unsigned j = 1; j <= no_machines; j++) {
        f_(i, j) = std::max(f_(i, j - 1), e_(i - 1, j)) + fspData.pt(seq_k, j - 1);
      }
    }

    std::fill(makespan.begin(), makespan.end(), 0);
    std::fill(flowtime.begin(), flowtime.end(), 0);
    for (unsigned i = 1; i <= seq_size; i++) {
      for (unsigned j = 1; j <= no_machines; j++) {
        int c_ij = f_(i, j) + q_(i, j);
        makespan[i - 1] = std::max(makespan[i - 1], c_ij);
        flowtime[i - 1] = flowtime[i - 1] + makespan[i - 1];
      }
    }
  }

  inline int getMakespan(const unsigned i) const {
    return makespan[i];
  }

  inline int getFlowtime(const unsigned i) const {
    return flowtime[i];
  }

};

class FastFSPSolution : public FSP {
public:
  std::vector<CompiledSchedule> compiledSchedules;
  ivec compiledSolution;
  std::vector<short> isCompiled;

  FastFSPSolution() = default;
  FastFSPSolution(const FastFSPSolution& other) = default;
  FastFSPSolution(FastFSPSolution&& other) = default;
  FastFSPSolution& operator=(const FastFSPSolution& sol) = default;
  FastFSPSolution& operator=(FastFSPSolution&& sol) = default;
  FastFSPSolution(unsigned size) : FSP(size) {}

  FSP::Fitness neighborFitness(const FSPData& fspData,
                               unsigned first,
                               unsigned second,
                               Objective objective);

  virtual std::string className() const;
};

class FastFSPNeighborEval : public moEval<fspShiftNeighbor<FastFSPSolution>> {
  const FSPData& fspData;
  const Objective objective;

public:
  FastFSPNeighborEval(const FSPData& fspData, Objective objective) : fspData(fspData), objective(objective) {}

  void operator()(FastFSPSolution& sol, fspShiftNeighbor<FastFSPSolution>& ngh) final override {
    ngh.setSize(static_cast<unsigned>(sol.size()));
    ngh.translate(ngh.index() + 1);
    ngh.fitness(sol.neighborFitness(fspData, ngh.getFirst(), ngh.getSecond(), objective));
  }

  FastFSPSolution::Fitness fastEvaluate(FastFSPSolution& sol, unsigned first, unsigned second) {
    return sol.neighborFitness(fspData, first, second, objective);
  }
};

#include <utility>
/*
struct FastFSPProblem : public Problem<fspShiftNeighbor<FastFSPSolution>> {
  using Ngh = fspShiftNeighbor<FastFSPSolution>;
  using EOT = FastFSPSolution;
  std::unique_ptr<FSPEvalFunc<EOT>> eval_func;
  eoEvalFuncCounter<EOT> eval_counter;
  std::unique_ptr<FastFSPNeighborEval> eval_neighbor;
  moEvalCounter<Ngh> eval_neighbor_counter;
  moEvalsContinuator<Ngh> eval_continuator;
  moHighResTimeContinuator<Ngh> time_continuator;
  std::string stopping_criterium;


  FastFSPProblem(FSPData dt, std::string type, std::string obj,
                 std::string stopping_criterium, std::string budget, unsigned lb = 0)
      : Problem<fspShiftNeighbor<FastFSPSolution>>(),
        eval_func(getEvalFunc(type, obj, dt)),
        eval_counter(*eval_func),
        eval_neighbor(std::make_unique<FastFSPNeighborEval>(eval_func->fsp_data, objectiveFromString(obj))),
        eval_neighbor_counter(*eval_neighbor),
        time_continuator(max_time),
        eval_continuator(eval_counter, eval_neighbor_counter, max_eval, false),
        stopping_criterium(stopping_criterium) {
    eval_counter.setValue("0");
    eval_neighbor_counter.setValue("0");
    EOT dummy;
    eval_continuator.init(dummy);
    reset();
  }

  FastFSPProblem(FSPData dt, std::string type, std::string obj,
                 unsigned max_eval)
      : FastFSPProblem(dt, type, obj, "EVALS") {
  }

  FastFSPProblem(const FastFSPProblem&) = default;
  FastFSPProblem(FastFSPProblem&&) = default;
  FastFSPProblem& operator=(const FastFSPProblem&) = default;
  FastFSPProblem& operator=(FastFSPProblem&&) = default;
  virtual ~FastFSPProblem() = default;

  eoEvalFuncCounter<FastFSPSolution> &eval() override { return eval_counter; }
  moEvalCounter<Ngh> &neighborEval() override { return eval_neighbor_counter; }
  moContinuator<Ngh> &continuator() override {
    if (stopping_criterium == "TIME")
        return time_continuator;
    return eval_continuator;
  }

  int size(int i = 0) const override {
    switch (i) {
    case 0:  return eval_func->noJobs();
    case 1:  return eval_func->noMachines();
    case 2:  return eval_func->noJobs() * eval_func->noMachines();
    default: return 0;
    }
  }

  void reset() override {
    //const auto total_evals = std::stoi(eval_counter.getValue()) +
    //                         std::stoi(eval_neighbor_counter.getValue());
    //std::cout << "Reseting... previous: " << total_evals << '\n';
    eval_counter.setValue("0");
    eval_neighbor_counter.setValue("0");
  }

  int noEvals() {
    int evals_ctr = 0;
    if (eval_counter.getValue() != "")
      evals_ctr = std::stoi(eval_counter.getValue());

    int ngh_ctr = 0;
    if (eval_neighbor_counter.getValue() != "")
      ngh_ctr = std::stoi(eval_neighbor_counter.getValue());

    return evals_ctr + ngh_ctr;
  }

  const FSPData &getData() const {
    return eval_func->getData();
  }

  double upperBound() const override {
    return eval_func->fsp_data.maxCT();
  }

private:
  std::unique_ptr<FSPEvalFunc<EOT>> getEvalFunc(const std::string &type,
                                                const std::string &obj,
                                                const FSPData &dt) {
    std::unique_ptr<FSPEvalFunc<EOT>> ret(nullptr);
    if        (type == "PERM" && obj == "MAKESPAN") {
      ret.reset(new PermFSPEvalFunc<EOT>(dt, Objective::MAKESPAN));
    } else if (type == "PERM" && obj == "FLOWTIME") {
      ret.reset(new PermFSPEvalFunc<EOT>(dt, Objective::FLOWTIME));
    } else {
      throw std::runtime_error("No FSP problem for type " + type + " and objective " + obj);
    }
    return ret;
  }
};
*/