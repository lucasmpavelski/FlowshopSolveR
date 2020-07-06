#pragma once

#include <algorithm>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/heuristics/InsertionStrategy.hpp"

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
  InsertionStrategy<Ngh>& _insertionStrategy;
  DestructionSize& _destructionSize;

 public:
  DestructionConstruction(InsertionStrategy<Ngh>& insertionStrategy,
                          DestructionSize& destructionSize)
      : _insertionStrategy{insertionStrategy},
        _destructionSize{destructionSize} {}

  [[nodiscard]] auto destructionSize() const -> const DestructionSize& { return _destructionSize; }
  
  auto destruction(EOT& sol) -> std::vector<int> {
    int n = sol.size();
    std::vector<int> removed;
    int ds = std::min(_destructionSize.value(), n);
    for (int k = 0; k < ds; k++) {
      int index = rng.random(sol.size());
      removed.push_back(sol[index]);
      sol.erase(sol.begin() + index);
    }
    return removed;
  }

  auto construction(EOT& sol, const std::vector<int>& jobsToInsert) -> bool {
    EOT tmp = sol;
    for (const auto& jobToInsert : jobsToInsert)
      _insertionStrategy.insertJob(sol, jobToInsert);
    return !std::equal(tmp.begin(), tmp.end(), sol.begin(), sol.end());
  }

  auto operator()(EOT& sol) -> bool override {
    return construction(sol, destruction(sol));
  }

  void init(EOT&) override{};
  void add(EOT&, Ngh&) override{};
  void update(EOT&, Ngh&) override{};
  void clearMemory() override{};
};
