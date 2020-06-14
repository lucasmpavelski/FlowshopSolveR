#pragma once

#include <iostream>
#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "Problem.hpp"
#include "continuators/myMovedSolutionStat.hpp"
#include "flowshop-solver/continuators/myTimeStat.hpp"
#include "flowshop-solver/moHiResTimeContinuator.hpp"
#include "flowshop-solver/problems/FSPEvalFunc.hpp"
#include "flowshop-solver/problems/NIFSPEvalFunc.hpp"
#include "flowshop-solver/problems/NWFSPEvalFunc.hpp"
#include "flowshop-solver/problems/fastfspeval.hpp"
#include "problems/FSPData.hpp"

using FSPMax = eoInt<eoMaximizingFitness>;
using FSPMin = eoInt<eoMinimizingFitness>;
using FSP    = FSPMin;

// m1 000000122222233...................
// m2       00001  222222233333333......
// m3           0000011111222     333333
//   1234567890123456789012345678901234

template <class EOT>
class myBestSoFarStat : public moBestSoFarStat<EOT> {
  bool reinit{};

 public:
  myBestSoFarStat(bool _reInitSol = true) : moBestSoFarStat<EOT>(_reInitSol) {}

  using moBestSoFarStat<EOT>::operator();

  void lastCall(EOT& sol) final { operator()(sol); }
};

struct FSPProblem : public Problem<FSPNeighbor> {
  using EOT = FSP;
  using Ngh = FSPNeighbor;
  FSPData                             _data;
  std::unique_ptr<FSPEvalFunc<EOT>>   eval_func;
  eoEvalFuncCounter<EOT>              eval_counter;
  std::unique_ptr<moEval<Ngh>>        eval_neighbor;
  moEvalCounter<Ngh>                  eval_neighbor_counter;
  std::unique_ptr<moContinuator<Ngh>> continuator_ptr;
  std::unique_ptr<moCheckpoint<Ngh>>  checkpoint_ptr;
  std::unique_ptr<moCheckpoint<Ngh>>  checkpointGlobal_ptr;
  myBestSoFarStat<EOT>                bestFound;
  myBestSoFarStat<EOT>                bestFoundGlobal;
  myMovedSolutionStat<EOT>            movedStat;
  NeigborhoodCheckpoint<Ngh>          _neighborhoodCheckpoint;

  const std::string stopping_criterium;
  const std::string budget;
  const unsigned    lower_bound;

  FSPProblem(FSPData            dt,
             const std::string& type,
             const std::string& obj,
             std::string        _budget,
             std::string        _stopping_criterium,
             unsigned           lower_bound = 0)
      : Problem<FSPNeighbor>(),
        _data{std::move(dt)},
        eval_func(getEvalFunc(type, obj)),
        eval_counter(*eval_func),
        eval_neighbor(getNeighborEvalFunc(type, obj)),
        eval_neighbor_counter(*eval_neighbor),
        stopping_criterium(std::move(_stopping_criterium)),
        budget(std::move(_budget)),
        lower_bound(lower_bound) {
    reset();
  }

  friend auto operator<<(std::ostream& o, const FSPProblem& d)
      -> std::ostream& {
    o << d.getData() << '\n'
      << "objective: " << d.eval_func->ObjT << '\n'
      << "type: " << d.eval_func->type() << '\n'
      << "budget: " << d.budget << '\n'
      << "stopping_criterium: " << d.stopping_criterium << '\n';
    return o;
  }

  auto eval() -> eoEvalFunc<EOT>& override { return eval_counter; }
  auto neighborEval() -> moEval<Ngh>& override { return eval_neighbor_counter; }
  auto continuator() -> moContinuator<Ngh>& override {
    return *continuator_ptr;
  }

  [[nodiscard]] auto data() const -> const FSPData& { return _data; }

  auto checkpoint() -> moCheckpoint<Ngh>& final { return *checkpoint_ptr; };

  auto checkpointGlobal() -> moCheckpoint<Ngh>& final {
    return *checkpointGlobal_ptr;
  };

  auto neighborhoodCheckpoint() -> NeigborhoodCheckpoint<Ngh>& final {
    return _neighborhoodCheckpoint;
  }

  void reset() override {
    // const auto total_evals = std::stoi(eval_counter.getValue()) +
    //                         std::stoi(eval_neighbor_counter.getValue());
    // std::cout << "Reseting... previous: " << total_evals << '\n';
    eval_counter.setValue("0");
    eval_neighbor_counter.setValue("0");
    continuator_ptr.reset(newContinuator());
    checkpoint_ptr = std::make_unique<moCheckpoint<Ngh>>(*continuator_ptr);
    checkpointGlobal_ptr =
        std::make_unique<moCheckpoint<Ngh>>(*continuator_ptr);
    bestFound = myBestSoFarStat<EOT>(true);
    EOT dummy;
    checkpoint_ptr->add(bestFound);
    checkpointGlobal_ptr->add(bestFoundGlobal);
    bestFoundGlobal = myBestSoFarStat<EOT>(false);
    dummy.fitness(std::numeric_limits<double>::infinity());
    bestFoundGlobal.init(dummy);

    // printg.add(timer);
    // printg.add(bestFoundGlobal);
    // checkpointGlobal_ptr->add(printg);
  }

  auto bestLocalSoFar() -> moBestSoFarStat<EOT>& override { return bestFound; }
  auto bestSoFar() -> moBestSoFarStat<EOT>& override { return bestFoundGlobal; }

  [[nodiscard]] auto noEvals() const -> int override {
    return static_cast<int>(
        std::strtol(eval_counter.getValue().c_str(), nullptr, 10) +
        std::strtol(eval_neighbor_counter.getValue().c_str(), nullptr, 10));
  }

  [[nodiscard]] auto getData() const -> const FSPData& {
    return eval_func->getData();
  }

  [[nodiscard]] auto upperBound() const -> double override {
    return getData().maxCT();
  }

  [[nodiscard]] auto size(int i = 0) const -> int override {
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

  [[nodiscard]] auto maxNeighborhoodSize() const -> int override {
    return std::pow(eval_func->noJobs() - 1, 2);
  }

  template <class Ngh, class EOT = typename Ngh::EOT>
  struct moFitAndEvalsContinuator : public moCombinedContinuator<Ngh> {
    moEvalsContinuator<Ngh> evals_continuator;
    moFitContinuator<Ngh>   fit_continuator;

    moFitAndEvalsContinuator(double                  maxFit,
                             eoEvalFuncCounter<EOT>& _fullEval,
                             moEvalCounter<Ngh>&     _neighborEval,
                             unsigned int            _maxEvals,
                             bool                    _restartCounter = true)
        : moCombinedContinuator<Ngh>(evals_continuator),
          evals_continuator(_fullEval,
                            _neighborEval,
                            _maxEvals,
                            _restartCounter),
          fit_continuator(maxFit) {
      this->add(fit_continuator);
    };
  };

  auto newContinuator() -> moContinuator<Ngh>* {
    if (stopping_criterium == "EVALS")
      return new moEvalsContinuator<Ngh>(eval_counter, eval_neighbor_counter,
                                         getMaxEvals(), false);
    if (stopping_criterium == "TIME")
      return new moHighResTimeContinuator<Ngh>(getMaxTime(), false, true);
    if (stopping_criterium.find("FIXEDTIME") == 0)
      return new moHighResTimeContinuator<Ngh>(getFixedTime(),
                                               false, true);
    if (stopping_criterium == "FITNESS") {
      return new moFitAndEvalsContinuator<Ngh>(getMaxFitness(), eval_counter,
                                               eval_neighbor_counter,
                                               2 * getMaxEvals(), false);
    }
    throw std::runtime_error("Unknown stopping criterium: " +
                             stopping_criterium);
    return nullptr;
  }

  [[nodiscard]] auto getMaxEvals() const -> unsigned {
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

  auto getMaxTime() -> unsigned {
    double mult = 0;
    if (budget == "low")
      mult = 2e-4;
    else if (budget == "med")
      mult = 2e-3;
    else if (budget == "high")
      mult = 2e-2;
    else
      throw std::runtime_error("Unknown budget: " + budget);
    return static_cast<unsigned>(getData().noJobs() * getData().noJobs() *
                                 getData().noMachines() * mult);
  }

  auto getFixedTime() -> unsigned {
    auto split      = stopping_criterium.find('_');
    int  multiplier = 1;
    if (split != std::string::npos) {
      auto times_str = stopping_criterium.substr(
          split + 1, split + 1 - stopping_criterium.size());
      multiplier = std::stoi(times_str);
    }
    return 60 * getData().noJobs() * getData().noMachines() * multiplier;
  }

  auto getMaxFitness() -> double {
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

  auto getEvalFunc(const std::string& type, const std::string& obj)
      -> std::unique_ptr<FSPEvalFunc<EOT>> {
    std::unique_ptr<FSPEvalFunc<EOT>> ret(nullptr);
    if (type == "PERM" && obj == "MAKESPAN") {
      ret = std::make_unique<PermFSPEvalFunc<EOT>>(_data, Objective::MAKESPAN);
    } else if (type == "PERM" && obj == "FLOWTIME") {
      ret = std::make_unique<PermFSPEvalFunc<EOT>>(_data, Objective::FLOWTIME);
    } else if (type == "NOWAIT" && obj == "MAKESPAN") {
      ret = std::make_unique<NWFSPEvalFunc<EOT>>(_data, Objective::MAKESPAN);
    } else if (type == "NOWAIT" && obj == "FLOWTIME") {
      ret = std::make_unique<NWFSPEvalFunc<EOT>>(_data, Objective::FLOWTIME);
    } else if (type == "NOIDLE" && obj == "MAKESPAN") {
      ret = std::make_unique<NIFSPEvalFunc<EOT>>(_data, Objective::MAKESPAN);
    } else if (type == "NOIDLE" && obj == "FLOWTIME") {
      ret = std::make_unique<NIFSPEvalFunc<EOT>>(_data, Objective::FLOWTIME);
    } else {
      throw std::runtime_error("No FSP problem for type " + type +
                               " and objective " + obj);
    }
    return ret;
  }

  auto getNeighborEvalFunc(const std::string& type, const std::string& obj)
      -> std::unique_ptr<moEval<Ngh>> {
    if (type == "PERM" && obj == "MAKESPAN") {
      return std::make_unique<FastFSPNeighborEval>(_data, *eval_func);
    } else {
      return std::make_unique<moFullEvalByCopy<Ngh>>(*eval_func);
    }
    return nullptr;
  }
};
