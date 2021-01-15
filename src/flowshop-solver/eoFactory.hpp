#pragma once

#include <type_traits>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/MHParamsValues.hpp"
#include "flowshop-solver/problems/Problem.hpp"
#include "flowshop-solver/heuristics/BestInsertionExplorer.hpp"
#include "flowshop-solver/heuristics/falseContinuator.hpp"


template <class Ngh>
class myOrderNeighborhood : public moOrderNeighborhood<Ngh>, public eoFunctorBase {
  using moOrderNeighborhood<Ngh>::moOrderNeighborhood;
};

template <class Ngh>
class myRndWithoutReplNeighborhood : public moRndWithoutReplNeighborhood<Ngh>, public eoFunctorBase {
  using moRndWithoutReplNeighborhood<Ngh>::moRndWithoutReplNeighborhood;
};

template <class Ngh, class EOT = typename Ngh::EOT>
class eoFactory : public eoFunctorStore {
  const MHParamsValues& _params;
  Problem<Ngh>& _problem;

 protected:
  virtual auto domainInit() -> eoInit<EOT>* { return nullptr; }
  virtual auto domainAcceptanceCriterion() -> moAcceptanceCriterion<Ngh>* {
    return nullptr;
  }
  virtual auto domainNeighborhood() -> moIndexNeighborhood<Ngh>* { return nullptr; }
  virtual auto domainPerturb() -> moPerturbation<Ngh>* { return nullptr; }

  virtual auto domainSolComparator() -> moSolComparator<Ngh>* {
    return nullptr;
  }
  virtual auto domainSolNeighborComparator() -> moSolNeighborComparator<Ngh>* {
    return nullptr;
  }
  virtual auto domainNeighborComparator() -> moNeighborComparator<Ngh>* {
    return nullptr;
  }

 public:
  template <class T>
  using ptr = std::unique_ptr<T>;

  eoFactory(const MHParamsValues& params, Problem<Ngh>& problem)
      : _params{params}, _problem{problem} {};

  [[nodiscard]] auto params() const -> const MHParamsValues& { return _params; }

  auto problem() const -> const Problem<Ngh>& { return _problem; }

  [[nodiscard]] auto categoricalName(const std::string& name) const
      -> std::string {
    return _params.categoricalName(_params.mhName() + name);
  }

  [[nodiscard]] auto categorical(const std::string& name) const -> int {
    return _params.categorical(_params.mhName() + name);
  }

  [[nodiscard]] auto integer(const std::string& name) const -> int {
    return _params.integer(_params.mhName() + name);
  }

  [[nodiscard]] auto real(const std::string& name) const -> double {
    return _params.real(_params.mhName() + name);
  }

  void params(MHParamsValues& _params) {
    this->_params = _params;
  }

  auto buildInit() -> eoInit<EOT>* {
    const std::string init = categoricalName(".Init");
    if (init == "random")
      return &pack<eoInitPermutation<EOT>>(_problem.size(0));
    return domainInit();
  }

  auto buildAcceptanceCriterion() -> moAcceptanceCriterion<Ngh>* {
    const std::string name = categoricalName(".Accept");
    if (name == "always")
      return &pack<moAlwaysAcceptCrit<Ngh>>();
    else if (name == "better") {
      const std::string comparison = categoricalName(".Accept.Better.Comparison");
      if (comparison == "strict") {
        return &pack<moBetterAcceptCrit<Ngh>>();
      } else if (comparison == "equal") {
        moSolComparator<EOT>& equal = pack<moEqualSolComparator<EOT>>();
        return &pack<moBetterAcceptCrit<Ngh>>(equal);
      }
      return nullptr;
    }
    return domainAcceptanceCriterion();
  }

  auto buildNeighborhood() -> moIndexNeighborhood<Ngh>* {
    return buildNeighborhood(_problem.maxNeighborhoodSize());
  }

  auto buildNeighborhood(const int max_size) -> moIndexNeighborhood<Ngh>* {
    const int size = max_size * real(".Neighborhood.Size");
    const std::string name = categoricalName(".Neighborhood.Strat");
    if (name == "ordered") {
      return &pack<myOrderNeighborhood<Ngh>>(size);
    } else if (name == "random") {
      return &pack<myRndWithoutReplNeighborhood<Ngh>>(size);
    }
    return domainNeighborhood();
  }

  auto buildSolComparator() -> moSolComparator<EOT>* {
    const std::string name = categoricalName(".Comp.Strat");
    if (name == "strict")
      return &pack<moSolComparator<EOT>>();
    else if (name == "equal")
      return &pack<moEqualSolComparator<EOT>>();
    else
      return domainSolComparator();
  }

  auto buildSolNeighborComparator() -> moSolNeighborComparator<Ngh>* {
    const std::string name = categoricalName(".Comp.Strat");
    if (name == "strict")
      return &pack<moSolNeighborComparator<Ngh>>();
    else if (name == "equal")
      return &pack<moEqualSolNeighborComparator<Ngh>>();
    else
      return domainSolNeighborComparator();
  }

  auto buildNeighborComparator() -> moNeighborComparator<Ngh>* {
    const std::string name = categoricalName(".Comp.Strat");
    if (name == "strict")
      return &pack<moNeighborComparator<Ngh>>();
    else if (name == "equal")
      return &pack<moEqualNeighborComparator<Ngh>>();
    else
      return domainNeighborComparator();
  }

  auto buildLocalSearch() -> moLocalSearch<Ngh>* {
    auto compNN = buildNeighborComparator();
    auto compSN = buildSolNeighborComparator();

    auto& eval = _problem.eval();
    auto& nEval = _problem.neighborEval();
    auto& cp = _problem.checkpoint();
    auto& nghCp = _problem.neighborhoodCheckpoint();

    const std::string name = categoricalName(".Local.Search");
    moLocalSearch<Ngh>* ret = nullptr;
    if (name == "none") {
      auto& explorer = pack<moDummyExplorer<Ngh>>();
      ret = &pack<moLocalSearch<Ngh>>(explorer, cp, eval);
    } else if (name == "first_improvement") {
      auto neighborhood = buildNeighborhood();
      ret = &pack<moFirstImprHC<Ngh>>(*neighborhood, eval, nEval, cp, *compNN,
                              *compSN);
    } else if (name == "best_improvement") {
      auto neighborhood = buildNeighborhood();
      ret = &pack<moSimpleHC<Ngh>>(*neighborhood, eval, nEval, cp, *compNN,
                            *compSN);
    } else if (name == "random_best_improvement") {
      auto neighborhood = buildNeighborhood();
      ret = &pack<moRandomBestHC<Ngh>>(*neighborhood, eval, nEval, cp, *compNN,
                                *compSN);
    } else if (name == "best_insertion") {
      auto explorer =
          &pack<BestInsertionExplorer<EOT>>(nEval, nghCp, *compNN, *compSN);
      ret = &pack<moLocalSearch<Ngh>>(*explorer, cp, eval);
    } else {
      return nullptr;
    }

    if (categorical(".LS.Single.Step")) {
      // auto& singleStepContinuator = pack<moCombinedContinuator<Ngh>>(cp);
      auto& falseCont = pack<falseContinuator<Ngh>>();
      cp.add(falseCont);
    }
    return ret;
  }

  auto buildPerturb() -> moPerturbation<Ngh>* {
    return domainPerturb();
  }

};