#pragma once

#include "FSPEvalFunc.hpp"
#include "NIFSPEvalFunc.hpp"
#include "NWFSPEvalFunc.hpp"
#include "Problem.hpp"
#include "continuators/myMovedSolutionStat.hpp"
#include "fastfspeval.hpp"
#include "flowshop-solver/moHiResTimeContinuator.hpp"
#include "paradiseo/eo/eoInt.h"
#include "paradiseo/eo/eoScalarFitness.h"
#include "paradiseo/mo/continuator/moBestSoFarStat.h"
#include "paradiseo/mo/continuator/moCheckpoint.h"
#include "paradiseo/mo/continuator/moCombinedContinuator.h"
#include "paradiseo/mo/continuator/moContinuator.h"
#include "paradiseo/mo/continuator/moEvalsContinuator.h"
#include "paradiseo/mo/continuator/moFitContinuator.h"
#include "paradiseo/mo/continuator/moStatBase.h"
#include "paradiseo/mo/eval/moFullEvalByCopy.h"
#include "paradiseo/mo/problems/permutation/moShiftNeighbor.h"

using FSPMax = eoInt<eoMaximizingFitness>;
using FSPMin = eoInt<eoMinimizingFitness>;
using FSP = FSPMin;

// m1 000000122222233...................
// m2       00001  222222233333333......
// m3           0000011111222     333333
//   1234567890123456789012345678901234

template <class EOT>
class myBestSoFarStat : public moBestSoFarStat<EOT> {
 public:
  myBestSoFarStat(bool _reInitSol = true) : moBestSoFarStat<EOT>(_reInitSol) {}

  using moBestSoFarStat<EOT>::operator();
  void lastCall(EOT& sol) final override { operator()(sol); }
};

template <class EOT, class TimeT = std::chrono::milliseconds>
class TimeStat : public moStat<EOT, int> {
  std::chrono::system_clock::time_point start;

 public:
  using moStat<EOT, int>::value;

  TimeStat() : moStat<EOT, int>(0, "timer in miliseconds") {
    start = std::chrono::system_clock::now();
  }

  void operator()(EOT& sol) final override {
    auto now = std::chrono::system_clock::now();
    value() = std::chrono::duration_cast<TimeT>(now - start).count();
  };
};

struct FSPProblem : public Problem<moShiftNeighbor<FSP>> {
  using EOT = FSP;
  using Ngh = moShiftNeighbor<EOT>;
  std::unique_ptr<FSPEvalFunc<EOT>> eval_func;
  eoEvalFuncCounter<EOT> eval_counter;
  std::unique_ptr<moEval<Ngh>> eval_neighbor;
  moEvalCounter<Ngh> eval_neighbor_counter;
  std::unique_ptr<moContinuator<Ngh>> continuator_ptr;
  std::unique_ptr<moCheckpoint<Ngh>> checkpoint_ptr;
  std::unique_ptr<moCheckpoint<Ngh>> checkpointGlobal_ptr;
  myBestSoFarStat<EOT> bestFound;
  myBestSoFarStat<EOT> bestFoundGlobal;
  myMovedSolutionStat<EOT> movedStat;

  const std::string stopping_criterium;
  const std::string budget;
  const unsigned lower_bound;

  TimeStat<EOT> timer;
  prefixedPrinter print;
  prefixedPrinter printg;

  FSPProblem(FSPData dt,
             std::string type,
             std::string obj,
             const std::string& _budget,
             std::string _stopping_criterium,
             unsigned lower_bound = 0)
      : Problem<moShiftNeighbor<FSP>>(),
        eval_func(getEvalFunc(type, obj, dt)),
        eval_counter(*eval_func),
        eval_neighbor(getNeighborEvalFunc(type, obj, dt)),
        eval_neighbor_counter(*eval_neighbor),
        stopping_criterium(_stopping_criterium),
        budget(_budget),
        lower_bound(lower_bound),
        print("local_best:", " "),
        printg("global best:", " ") {
    reset();
  }

  friend std::ostream& operator<<(std::ostream& o, const FSPProblem& d) {
    o << d.getData() << '\n'
      << "objective: " << d.eval_func->ObjT << '\n'
      << "type: " << d.eval_func->type() << '\n'
      << "budget: " << d.budget << '\n'
      << "stopping_criterium: " << d.stopping_criterium << '\n';
    return o;
  }

  eoEvalFuncCounter<EOT>& eval() override { return eval_counter; }
  moEval<Ngh>& neighborEval() override { return eval_neighbor_counter; }
  moContinuator<Ngh>& continuator() override { return *continuator_ptr; }

  moCheckpoint<Ngh>& checkpoint() final override { return *checkpoint_ptr; };

  moCheckpoint<Ngh>& checkpointGlobal() final override {
    return *checkpointGlobal_ptr;
  };

  int size(int i = 0) const override {
    switch (i) {
      case 0:
        return eval_func->noJobs();
      case 1:
        return eval_func->noMachines();
      case 2:
        return eval_func->noJobs() * eval_func->noMachines();
      default:
        return 0;
    }
  }

  void reset() override {
    // const auto total_evals = std::stoi(eval_counter.getValue()) +
    //                         std::stoi(eval_neighbor_counter.getValue());
    // std::cout << "Reseting... previous: " << total_evals << '\n';
    eval_counter.setValue("0");
    eval_neighbor_counter.setValue("0");
    continuator_ptr.reset(newContinuator());
    checkpoint_ptr.reset(new moCheckpoint<Ngh>(*continuator_ptr));
    checkpointGlobal_ptr.reset(new moCheckpoint<Ngh>(*continuator_ptr));
    bestFound = myBestSoFarStat<EOT>(true);
    EOT dummy;
    checkpoint_ptr->add(bestFound);
    checkpointGlobal_ptr->add(bestFoundGlobal);
    bestFoundGlobal = myBestSoFarStat<EOT>(false);
    dummy.fitness(std::numeric_limits<double>::infinity());
    bestFoundGlobal.init(dummy);

    printg.add(timer);
    printg.add(bestFoundGlobal);
    checkpointGlobal_ptr->add(timer);
    checkpointGlobal_ptr->add(printg);
  }

  moBestSoFarStat<EOT>& bestLocalSoFar() override { return bestFound; }
  moBestSoFarStat<EOT>& bestSoFar() override { return bestFoundGlobal; }

  int noEvals() const override {
    return std::strtol(eval_counter.getValue().c_str(), nullptr, 10) +
           std::strtol(eval_neighbor_counter.getValue().c_str(), nullptr, 10);
  }

  const FSPData& getData() const { return eval_func->getData(); }

  double upperBound() const override { return getData().maxCT(); }

  template <class Ngh, class EOT = typename Ngh::EOT>
  struct moFitAndEvalsContinuator : public moCombinedContinuator<Ngh> {
    moEvalsContinuator<Ngh> evals_continuator;
    moFitContinuator<Ngh> fit_continuator;

    moFitAndEvalsContinuator(double maxFit,
                             eoEvalFuncCounter<EOT>& _fullEval,
                             moEvalCounter<Ngh>& _neighborEval,
                             unsigned int _maxEvals,
                             bool _restartCounter = true)
        : moCombinedContinuator<Ngh>(evals_continuator),
          evals_continuator(_fullEval,
                            _neighborEval,
                            _maxEvals,
                            _restartCounter),
          fit_continuator(maxFit) {
      this->add(fit_continuator);
    };
  };

  moContinuator<Ngh>* newContinuator() {
    const auto& dt = getData();
    if (stopping_criterium == "EVALS")
      return new moEvalsContinuator<Ngh>(eval_counter, eval_neighbor_counter,
                                         getMaxEvals(), false);
    if (stopping_criterium == "TIME")
      return new moHighResTimeContinuator<Ngh>(getMaxTime(), false, true);
    if (stopping_criterium == "FIXEDTIME")
      return new moHighResTimeContinuator<Ngh>(getFixedTime(), false, true);
    if (stopping_criterium == "FITNESS") {
      return new moFitAndEvalsContinuator<Ngh>(getMaxFitness(), eval_counter,
                                               eval_neighbor_counter,
                                               2 * getMaxEvals(), false);
    }
    throw std::runtime_error("Unknown stopping criterium: " +
                             stopping_criterium);
    return nullptr;
  }

  unsigned getMaxEvals() const {
    unsigned eval_multiplier = 0;
    if (budget == "low")
      eval_multiplier = 10u;
    else if (budget == "med")
      eval_multiplier = 100u;
    else if (budget == "high")
      eval_multiplier = 1000u;
    else
      throw std::runtime_error("Unknown budget: " + budget);
    return getData().noJobs() * getData().noMachines() * eval_multiplier;
  }

  unsigned getMaxTime() {
    double mult = 0;
    if (budget == "low")
      mult = 2e-4;
    else if (budget == "med")
      mult = 2e-3;
    else if (budget == "high")
      mult = 2e-2;
    else
      throw std::runtime_error("Unknown budget: " + budget);
    return getData().noJobs() * getData().noJobs() * getData().noMachines() *
           mult;
  }

  unsigned getFixedTime() {
    return 60 * getData().noJobs() * getData().noMachines();
  }

  double getMaxFitness() {
    double mult = 0.0;
    if (budget == "low")
      mult = 1.003;
    else if (budget == "med")
      mult = 1.002;
    else if (budget == "high")
      mult = 1.001;
    else
      throw std::runtime_error("Unknown budget: " + budget);
    return lower_bound * mult;
  }

  std::unique_ptr<FSPEvalFunc<EOT>> getEvalFunc(const std::string& type,
                                                const std::string& obj,
                                                const FSPData& dt) {
    std::unique_ptr<FSPEvalFunc<EOT>> ret(nullptr);
    if (type == "PERM" && obj == "MAKESPAN") {
      ret.reset(new PermFSPEvalFunc<EOT>(dt, Objective::MAKESPAN));
    } else if (type == "PERM" && obj == "FLOWTIME") {
      ret.reset(new PermFSPEvalFunc<EOT>(dt, Objective::FLOWTIME));
    } else if (type == "NOWAIT" && obj == "MAKESPAN") {
      ret.reset(new NWFSPEvalFunc<EOT>(dt, Objective::MAKESPAN));
    } else if (type == "NOWAIT" && obj == "FLOWTIME") {
      ret.reset(new NWFSPEvalFunc<EOT>(dt, Objective::FLOWTIME));
    } else if (type == "NOIDLE" && obj == "MAKESPAN") {
      ret.reset(new NIFSPEvalFunc<EOT>(dt, Objective::MAKESPAN));
    } else if (type == "NOIDLE" && obj == "FLOWTIME") {
      ret.reset(new NIFSPEvalFunc<EOT>(dt, Objective::FLOWTIME));
    } else {
      throw std::runtime_error("No FSP problem for type " + type +
                               " and objective " + obj);
    }
    return ret;
  }

  std::unique_ptr<moEval<Ngh>> getNeighborEvalFunc(const std::string& type,
                                                   const std::string& obj,
                                                   const FSPData& dt) {
    std::unique_ptr<moEval<Ngh>> ret(nullptr);
    if (type == "PERM" && obj == "MAKESPAN") {
      ret.reset(new FastFSPNeighborEval(dt, movedStat));
    } else {
      ret.reset(new moFullEvalByCopy<Ngh>(*eval_func));
    }
    return ret;
  }
};
