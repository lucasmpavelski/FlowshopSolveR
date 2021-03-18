#pragma once

#include <algorithm>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/heuristics/InsertionStrategy.hpp"
#include "flowshop-solver/heuristics/perturb/DestructionStrategy.hpp"

class DestructionSize : public eoFunctorBase {
  public:
  virtual auto value() -> int = 0;
};

class FixedDestructionSize : public DestructionSize {
  int destructionSize;

  public:
  FixedDestructionSize(int destructionSize) :
    destructionSize{destructionSize} {};

  auto value() -> int override {
    return destructionSize;
  }
};

template <class Ngh, typename EOT = typename Ngh::EOT>
class DestructionConstruction : public moPerturbation<Ngh> {
  InsertionStrategy<Ngh>& insertionStrategy;
  DestructionStrategy<EOT>& destructionStrategy;

protected:
  auto construction(EOT& sol, const EOT& jobsToInsert) -> bool {
    EOT tmp = sol;
    for (const auto& jobToInsert : jobsToInsert)
      insertionStrategy.insertJob(sol, jobToInsert);
    return true; // !std::equal(tmp.begin(), tmp.end(), sol.begin(), sol.end());
  }

  auto destruction(EOT& sol) -> EOT {
    return destructionStrategy(sol);
  }

 public:
  DestructionConstruction(InsertionStrategy<Ngh>& insertionStrategy,
                          DestructionStrategy<EOT>& destructionStrategy)
      : insertionStrategy(insertionStrategy),
        destructionStrategy(destructionStrategy) {}

  auto operator()(EOT& sol) -> bool override {
    return construction(sol, destruction(sol));
  }

  void init(EOT&) override{};
  void add(EOT&, Ngh&) override{};
  void update(EOT&, Ngh&) override{};
  void clearMemory() override{};
};
