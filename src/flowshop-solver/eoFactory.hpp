#pragma once

#include <type_traits>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/MHParamsValues.hpp"
#include "flowshop-solver/aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/aos/thompson_sampling.hpp"
#include "flowshop-solver/heuristics/AdaptiveBestInsertionExplorer.hpp"
// #include "flowshop-solver/heuristics/BestInsertionExplorer.hpp"
#include "flowshop-solver/heuristics/InitLocalSearch.hpp"
#include "flowshop-solver/heuristics/falseContinuator.hpp"
#include "flowshop-solver/problems/Problem.hpp"

// #include "flowshop-solver/neighborhood-size/NeighborhoodSize.hpp"
// #include "flowshop-solver/neighborhood-size/FixedNeighborhoodSize.hpp"
// #include "flowshop-solver/neighborhood-size/AdaptiveNeighborhoodSize.hpp"

#include "aos/thompson_sampling.hpp"

template <class Ngh>
class myOrderNeighborhood : public moOrderNeighborhood<Ngh>,
                            public eoFunctorBase {
  using moOrderNeighborhood<Ngh>::moOrderNeighborhood;
};

template <class Ngh>
class myRndWithoutReplNeighborhood : public moRndWithoutReplNeighborhood<Ngh>,
                                     public eoFunctorBase {
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
  virtual auto domainNeighborhood() -> moIndexNeighborhood<Ngh>* {
    return nullptr;
  }

  virtual auto domainLocalSearch() -> moLocalSearch<Ngh>* { return nullptr; }

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

  void params(MHParamsValues& _params) { this->_params = _params; }

  auto buildInit() -> eoInit<EOT>* {
    eoInit<EOT>* init = nullptr;
    if (categoricalName(".Init") == "random")
      init = &pack<eoInitPermutation<EOT>>(_problem.size(0));
    else {
      init = domainInit();
    }
    return init;
    //  auto initLocalSearch = buildLocalSearchByName(
    //      categoricalName(".Init.LocalSearch"),
    //      categorical(".Init.LocalSearch.SingleStep") == 1);
    //  return &pack<InitLocalSearch<Ngh>>(*init, *initLocalSearch);
  }

  auto buildAcceptanceCriterion() -> moAcceptanceCriterion<Ngh>* {
    const std::string name = categoricalName(".Accept");
    if (name == "always")
      return &pack<moAlwaysAcceptCrit<Ngh>>();
    else if (name == "better") {
      const std::string comparison =
          categoricalName(".Accept.Better.Comparison");
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
    return buildLocalSearchByName(categoricalName(".Local.Search"),
                                  categorical(".LS.Single.Step") == 1);
  }

  // auto buildNeighborhoodSize() -> NeighborhoodSize* {
  //   const std::string name = categoricalName(".Neighborhood.Strat");
  //   if (name == "random" || name == "ordered") {
  //     const int size = _problem.size(0) * real(".Neighborhood.Size");
  //     return &pack<FixedNeighborhoodSize>(size);
  //   } else if (name == "adaptive") {
  //       int noPartitions = categorical(".AdaptiveNeighborhood.AOS.NoArms");
  //       std::vector<int> options(noPartitions);
  //       std::iota(options.begin(), options.end(), 1);
  //       int rewardType = categorical(".AdaptiveNeighborhood.AOS.RewardType");
  //       auto operatorSelection =
  //           buildOperatorSelection(".AdaptiveNeighborhood", options);
  //       return &pack<AdaptiveNeighborhoodSize<EOT>>(
  //           _problem.size(0), operatorSelection, *getRewards(), rewardType);
  //     }
  //   return nullptr;
  // }

  auto buildLocalSearchByName(const std::string& name, bool singleStep)
      -> moLocalSearch<Ngh>* {
    auto compNN = buildNeighborComparator();
    auto compSN = buildSolNeighborComparator();

    auto& eval = _problem.eval();
    auto& nEval = _problem.neighborEval();
    auto& cp = _problem.checkpoint();
    auto& nghCp = _problem.neighborhoodCheckpoint();
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
      // auto neighborhoodSize = buildNeighborhoodSize();
      auto explorer = &pack<BestInsertionExplorer<EOT>>(
          nEval, nghCp, *compNN, *compSN); //, neighborhoodSize);
      ret = &pack<moLocalSearch<Ngh>>(*explorer, cp, eval);
    } else {
      ret = domainLocalSearch();
    }

    if (singleStep) {
      auto falseCont = &pack<falseContinuator<Ngh>>();
      cp.add(*falseCont);
      // ret->setContinuator(*singleStepContinuator);
    }

    return ret;
  }

  auto buildPerturb() -> moPerturbation<Ngh>* { return domainPerturb(); }

  auto buildOperatorSelection(const std::string& prefix,
                              const std::vector<int>& options)
      -> OperatorSelection<int>* {
    const std::string name = categoricalName(prefix + ".AOS.Strategy");
    OperatorSelection<int>* strategy = nullptr;
    if (name == "probability_matching") {
      strategy = &pack<ProbabilityMatching<int>>(
          options, categoricalName(prefix + ".AOS.PM.RewardType"),
          real(prefix + ".AOS.PM.Alpha"), real(prefix + ".AOS.PM.PMin"),
          integer(prefix + ".AOS.PM.UpdateWindow"));
    } else if (name == "frrmab") {
      strategy = &pack<FRRMAB<int>>(options,
                                    integer(prefix + ".AOS.FRRMAB.WindowSize"),
                                    real(prefix + ".AOS.FRRMAB.Scale"),
                                    real(prefix + ".AOS.FRRMAB.Decay"));
    } else if (name == "linucb") {
      auto& fitnessHistory = pack<FitnessHistory<EOT>>();
      auto& awSize = pack<AdaptiveWalkLengthFLA<EOT>>(fitnessHistory,
                                                      1.0 / _problem.size());
      auto& autocorr = pack<AutocorrelationFLA<EOT>>(fitnessHistory);
      auto& fdc = pack<FitnessDistanceCorrelationFLA<EOT>>(fitnessHistory);
      auto& neutralityFLA = pack<NeutralityFLA<Ngh>>();

      _problem.checkpoint().add(fitnessHistory);
      _problem.neighborhoodCheckpoint().add(neutralityFLA);
      _problem.checkpoint().add(_problem.neighborhoodCheckpoint());
      _problem.checkpoint().add(neutralityFLA);

      auto& context = pack<ProblemContext>();
      context.add(awSize);
      context.add(neutralityFLA);
      context.add(autocorr);
      context.add(fdc);

      strategy = &pack<LinUCB<int>>(options, context,
                                    real(prefix + ".AOS.LINUCB.Alpha"));
    } else if (name == "thompson_sampling") {
      if (categoricalName(prefix + ".AOS.TS.Strategy") == "static") {
        strategy = &pack<ThompsonSampling<int>>(options);
      } else if (categoricalName(prefix + ".AOS.TS.Strategy") == "dynamic") {
        strategy = &pack<DynamicThompsonSampling<int>>(
            options, integer(prefix + ".AOS.TS.C"));
      }
    } else if (name == "random") {
      strategy = &pack<Random<int>>(options);
    }

    auto warmUpProportion = integer(prefix + ".AOS.WarmUp");
    auto warmUpStrategy = categoricalName(prefix + ".AOS.WarmUp.Strategy");
    auto& warmUpContinuator =
        pack<moIterContinuator<OperatorSelection<int>::DummyNgh>>(
            warmUpProportion, false);
    strategy->setWarmUp(warmUpContinuator, warmUpStrategy, 0);

    return strategy;
  }

  FitnessRewards<EOT>* cachedRewards = nullptr;
  auto getRewards() -> FitnessRewards<FSP>* {
    if (cachedRewards != nullptr)
      return cachedRewards;
    auto& rewards = pack<FitnessRewards<EOT>>();
    _problem.checkpoint().add(rewards.localStat());
    _problem.checkpointGlobal().add(rewards.globalStat());
    this->cachedRewards = &rewards;
    return &rewards;
  }

  auto buildOperatorSelection(const std::string& prefix)
      -> OperatorSelection<int>* {
    std::string opts = categoricalName(prefix + ".AOS.Options");
    std::vector<std::string> opts_strs = tokenize(opts, '_');
    std::vector<int> options(opts_strs.size());
    std::transform(begin(opts_strs), end(opts_strs), begin(options),
                   [](std::string& s) { return std::stoi(s); });
    return buildOperatorSelection(prefix, options);
  }
};