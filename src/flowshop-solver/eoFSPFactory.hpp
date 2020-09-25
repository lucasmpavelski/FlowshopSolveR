#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/MHParamsValues.hpp"
#include "flowshop-solver/eoFactory.hpp"

#include "global.hpp"
#include "heuristics/perturb/DestructionConstruction.hpp"
#include "problems/FSP.hpp"
#include "problems/FSPProblem.hpp"

#include "flowshop-solver/heuristics/FSPOrderHeuristics.hpp"
#include "flowshop-solver/heuristics/NEH.hpp"

#include "flowshop-solver/heuristics/acceptCritTemperature.hpp"
#include "flowshop-solver/problems/FSPProblem.hpp"

#include "flowshop-solver/heuristics/perturb/perturb.hpp"

#include "flowshop-solver/heuristics/BestInsertionExplorer.hpp"
#include "flowshop-solver/heuristics/perturb/IGLocalSearchPartialSolution.hpp"

#include "flowshop-solver/aos/frrmab.hpp"
#include "flowshop-solver/aos/lin_ucb.hpp"
#include "flowshop-solver/aos/probability_matching.hpp"
#include "flowshop-solver/aos/random.hpp"
#include "flowshop-solver/aos/thompson_sampling.hpp"
#include "flowshop-solver/fla/AdaptiveWalkLengthFLA.hpp"
#include "flowshop-solver/fla/AutocorrelationFLA.hpp"
#include "flowshop-solver/fla/FitnessDistanceCorrelationFLA.hpp"
#include "flowshop-solver/fla/FitnessHistory.hpp"
#include "flowshop-solver/fla/NeutralityFLA.hpp"

#include "flowshop-solver/heuristics/AppendingNEH.hpp"

class eoFSPFactory : public eoFactory<FSPProblem::Ngh> {
  FSPProblem& _problem;

 public:
  eoFSPFactory(const MHParamsValues& params, FSPProblem& problem)
      : eoFactory<FSPProblem::Ngh>{params, problem}, _problem{problem} {};

  using EOT = FSP;
  using Ngh = FSPNeighbor;

 protected:
  auto domainAcceptanceCriterion() -> moAcceptanceCriterion<Ngh>* override {
    const std::string name = categoricalName(".Accept");
    if (name == "temperature") {
      const int noJobs = _problem.getData().noJobs();
      const int noMachines = _problem.getData().noMachines();
      const int maxCT = _problem.getData().maxCT();
      const double tempScale = maxCT / (10.0 * noJobs * noMachines);
      const double temperature = real(".Accept.Temperature") * tempScale;
      return &pack<acceptCritTemperature<Ngh>>(temperature);
    }
    return nullptr;
  }

  auto domainInit() -> eoInit<EOT>* override {
    const std::string name = categoricalName(".Init");
    if (name == "neh") {
      auto firstPriority = categoricalName(".Init.NEH.First.Priority");
      auto firstPriorityWeighted = integer(".Init.NEH.First.PriorityWeighted");
      auto firstPriorityOrder =
          categoricalName(".Init.NEH.First.PriorityOrder");

      auto firstPriorityInit =
          buildPriority(_problem.data(), firstPriority, firstPriorityWeighted,
                        firstPriorityOrder)
              .release();
      storeFunctor(firstPriorityInit);

      auto nehPriority = categoricalName(".Init.NEH.Priority");
      auto nehPriorityWeighted = integer(".Init.NEH.PriorityWeighted");
      auto nehPriorityOrder = categoricalName(".Init.NEH.PriorityOrder");

      auto priority = buildPriority(_problem.data(), nehPriority,
                                    nehPriorityWeighted, nehPriorityOrder)
                          .release();
      storeFunctor(priority);

      auto nehInsertion = categoricalName(".Init.NEH.Insertion");
      auto insertion =
          buildInsertionStrategy(nehInsertion, _problem.neighborEval());
      storeFunctor(insertion);

      auto ratio = real(".Init.NEH.Ratio");

      return &pack<AppendingNEH<Ngh>>(*firstPriorityInit, *priority,
                                      *insertion, ratio);
    }
    return nullptr;
  }

  auto buildLSPSLocalSearch() -> myResizableLocalSearch<Ngh>* {
    auto neighborhood = buildNeighborhood(_problem.maxNeighborhoodSize());
    auto compNN = buildNeighborComparator();
    auto compSN = buildSolNeighborComparator();
    auto& eval = _problem.eval();
    auto& nEval = _problem.neighborEval();
    auto& cont = _problem.continuator();
    auto& nghCp = _problem.neighborhoodCheckpoint();

    moLocalSearch<Ngh>* localSearch = nullptr;
    const std::string name = categoricalName(".LSPS.Local.Search");
    if (name == "none") {
      localSearch = &pack<moDummyLS<Ngh>>(eval);
    } else if (name == "first_improvement") {
      localSearch = &pack<moFirstImprHC<Ngh>>(*neighborhood, eval, nEval, cont,
                                              *compNN, *compSN);
    } else if (name == "best_improvement") {
      localSearch = &pack<moSimpleHC<Ngh>>(*neighborhood, eval, nEval, cont,
                                           *compNN, *compSN);
    } else if (name == "random_best_improvement") {
      localSearch = &pack<moRandomBestHC<Ngh>>(*neighborhood, eval, nEval, cont,
                                               *compNN, *compSN);
    } else if (name == "best_insertion") {
      auto explorer =
          &pack<BestInsertionExplorer<EOT>>(nEval, nghCp, *compNN, *compSN);
      localSearch = &pack<moLocalSearch<Ngh>>(*explorer, cont, eval);
    }
    return &pack<myResizableLocalSearch<Ngh>>(
        *localSearch, *neighborhood,
        [this](int size) { return this->_problem.getNeighborhoodSize(size); });
  }

  auto buildOperatorSelection() -> OperatorSelection<int>* {
    std::string opts = categoricalName(".AOS.Options");
    std::vector<std::string> opts_strs = tokenize(opts, '_');
    std::vector<int> options(opts_strs.size());
    std::transform(begin(opts_strs), end(opts_strs), begin(options),
                   [](std::string& s) { return std::stoi(s); });
    const std::string name = categoricalName(".AOS.Strategy");
    OperatorSelection<int>* strategy = nullptr;
    if (name == "probability_matching") {
      strategy = &pack<ProbabilityMatching<int>>(
          options, categoricalName(".AOS.PM.RewardType"), real(".AOS.PM.Alpha"),
          real(".AOS.PM.PMin"), integer(".AOS.PM.UpdateWindow"));
    } else if (name == "frrmab") {
      strategy = &pack<FRRMAB<int>>(options, integer(".AOS.FRRMAB.WindowSize"),
                                    real(".AOS.FRRMAB.Scale"),
                                    real(".AOS.FRRMAB.Decay"));
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

      strategy =
          &pack<LinUCB<int>>(options, context, real(".AOS.LINUCB.Alpha"));
    } else if (name == "thompson_sampling") {
      if (categoricalName(".AOS.TS.Strategy") == "static") {
        strategy = &pack<ThompsonSampling<int>>(options);
      } else if (categoricalName(".AOS.TS.Strategy") == "dynamic") {
        strategy =
            &pack<DynamicThompsonSampling<int>>(options, integer(".AOS.TS.C"));
      }
    } else if (name == "random") {
      strategy = &pack<Random<int>>(options);
    }

    auto warmUpProportion = real(".AOS.WarmUp.Proportion");
    auto warmUpStrategy = categoricalName(".AOS.WarmUp.Strategy");
    auto maxTime = _problem.getFixedTime() * warmUpProportion;
    auto& warmUpContinuator =
        pack<moHighResTimeContinuator<OperatorSelection<int>::DummyNgh>>(
            maxTime, false);
    strategy->setWarmUp(warmUpContinuator, warmUpStrategy, 1);

    return strategy;
  }

  auto buildDestructionSize() -> DestructionSize* {
    const std::string name =
        categoricalName(".Perturb.DestructionSizeStrategy");
    DestructionSize* destructionSize = nullptr;
    if (name == "fixed") {
      const int fixedDs = integer(".Perturb.DestructionSize");
      destructionSize = &pack<FixedDestructionSize>(fixedDs);
    } else if (name == "adaptive") {
      auto& rewards = pack<FitnessRewards<EOT>>();
      _problem.checkpoint().add(rewards.localStat());
      _problem.checkpointGlobal().add(rewards.globalStat());
      auto operator_selection = buildOperatorSelection();
      int rewardType = categorical(".AOS.RewardType");
      return &pack<AdaptiveDestructionSize<Ngh>>(*operator_selection, rewards,
                                                 rewardType);
    }
    return destructionSize;
  }

  auto domainPerturb() -> moPerturbation<Ngh>* override {
    auto insertionName = categoricalName(".Perturb.Insertion");
    auto insertion =
        buildInsertionStrategy(insertionName, _problem.neighborEval());
    storeFunctor(insertion);
    auto destructionSize = buildDestructionSize();

    const std::string name = categoricalName(".Perturb");
    if (name == "rs") {
      return &pack<DestructionConstruction<Ngh>>(*insertion, *destructionSize);
    } else if (name == "lsps") {
      auto lspsLocalSearch = buildLSPSLocalSearch();
      return &pack<IGLocalSearchPartialSolution<Ngh>>(
          *insertion, *destructionSize, *lspsLocalSearch);
    }
    return nullptr;
  }
};