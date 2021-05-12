#pragma once

#include <algo/moLocalSearch.h>
#include <continuator/moCombinedContinuator.h>
#include <paradiseo/mo/algo/moLocalSearch.h>
#include <paradiseo/mo/perturb/moPerturbation.h>
#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>
#include <stdexcept>

#include "flowshop-solver/MHParamsValues.hpp"
#include "flowshop-solver/eoFactory.hpp"

#include "flowshop-solver/global.hpp"
#include "flowshop-solver/heuristics/AdaptiveBestInsertionExplorer.hpp"
#include "flowshop-solver/heuristics/FitnessReward.hpp"
#include "flowshop-solver/heuristics/InsertionStrategy.hpp"
#include "flowshop-solver/heuristics/perturb/DestructionConstruction.hpp"

#include "flowshop-solver/heuristics/perturb/AdaptivePositionDestructionStrategy.hpp"
#include "flowshop-solver/heuristics/perturb/DestructionStrategy.hpp"
#include "flowshop-solver/heuristics/perturb/RandomDestructionStrategy.hpp"

#include "flowshop-solver/problems/FSP.hpp"
#include "flowshop-solver/problems/FSPProblem.hpp"

#include "flowshop-solver/heuristics/FSPOrderHeuristics.hpp"
#include "flowshop-solver/heuristics/NEH.hpp"

#include "flowshop-solver/heuristics/acceptCritTemperature.hpp"
#include "flowshop-solver/problems/FSPProblem.hpp"

#include "flowshop-solver/heuristics/perturb/perturb.hpp"

#include "flowshop-solver/heuristics/AdaptiveLocalSearch.hpp"
#include "flowshop-solver/heuristics/AdaptivePerturb.hpp"
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

#include "flowshop-solver/position-selector/AdaptiveNoReplacementPositionSelector.hpp"
#include "flowshop-solver/position-selector/AdaptivePositionSelector.hpp"
#include "flowshop-solver/position-selector/PositionSelector.hpp"

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

  auto buildInsertion(const std::string& name) -> InsertionStrategy<Ngh>* {
    auto& neval = _problem.neighborEval();
    auto insertUPtr = buildInsertionStrategy<Ngh>(name, neval);
    if (insertUPtr.get() == nullptr) {
      insertUPtr = buildInsertionStrategyFSP(name, neval, _problem.data());
    }
    if (insertUPtr.get() == nullptr) {
      return nullptr;
    } else {
      InsertionStrategy<Ngh>* nehInsert = insertUPtr.release();
      storeFunctor(nehInsert);
      return nehInsert;
    }
  }

  auto domainInit() -> eoInit<EOT>* override {
    const std::string name = categoricalName(".Init");
    if (name == "neh") {
      const auto ratioStr = categoricalName(".Init.NEH.Ratio");
      const auto ratio = std::stod(ratioStr);

      eoInit<EOT>* firstOrder = nullptr;

      if (ratio > 0.0) {
        auto firstPriority = categoricalName(".Init.NEH.First.Priority");
        auto firstPriorityWeighted =
            categoricalName(".Init.NEH.First.PriorityWeighted") == "yes";
        auto firstPriorityOrder =
            categoricalName(".Init.NEH.First.PriorityOrder");

        firstOrder = buildPriority(_problem.data(), firstPriority,
                                   firstPriorityWeighted, firstPriorityOrder)
                         .release();
        storeFunctor(firstOrder);
        if (ratio == 1.0)
          return firstOrder;
      }

      if (ratio < 1.0) {
        auto nehPriority = categoricalName(".Init.NEH.Priority");
        auto nehPriorityWeighted =
            categoricalName(".Init.NEH.PriorityWeighted") == "yes";
        auto nehPriorityOrder = categoricalName(".Init.NEH.PriorityOrder");

        eoInit<EOT>* nehOrder =
            buildPriority(_problem.data(), nehPriority, nehPriorityWeighted,
                          nehPriorityOrder)
                .release();
        storeFunctor(nehOrder);

        auto nehInsertion = categoricalName(".Init.NEH.Insertion");
        auto nehInsert = buildInsertion(nehInsertion);

        if (ratio == 0.0) {
          return &pack<NEH<Ngh>>(*nehOrder, *nehInsert);
        } else {
          return &pack<AppendingNEH<Ngh>>(*firstOrder, *nehOrder, *nehInsert,
                                          ratio);
        }
      }
    }
    return nullptr;
  }

  auto buildLSPSLocalSearch() -> myResizableLocalSearch<Ngh>* {
    auto neighborhood = buildNeighborhood(_problem.maxNeighborhoodSize());
    auto compNN = buildNeighborComparator();
    auto compSN = buildSolNeighborComparator();
    auto& eval = _problem.eval();
    auto& nEval = _problem.neighborEval();
    auto& cp = _problem.continuator();
    auto& nghCp = _problem.neighborhoodCheckpoint();

    moLocalSearch<Ngh>* localSearch = nullptr;
    const std::string name = categoricalName(".LSPS.Local.Search");
    if (name == "none") {
      localSearch = &pack<moDummyLS<Ngh>>(eval);
    } else if (name == "first_improvement") {
      localSearch = &pack<moFirstImprHC<Ngh>>(*neighborhood, eval, nEval, cp,
                                              *compNN, *compSN);
    } else if (name == "best_improvement") {
      localSearch = &pack<moSimpleHC<Ngh>>(*neighborhood, eval, nEval, cp,
                                           *compNN, *compSN);
    } else if (name == "random_best_improvement") {
      localSearch = &pack<moRandomBestHC<Ngh>>(*neighborhood, eval, nEval, cp,
                                               *compNN, *compSN);
    } else if (name == "best_insertion") {
      auto explorer =
          &pack<BestInsertionExplorer<EOT>>(nEval, nghCp, *compNN, *compSN);
      localSearch = &pack<moLocalSearch<Ngh>>(*explorer, cp, eval);
    }

    if (categoricalName(".LSPS.Single.Step") == "1") {
      auto singleStepContinuator = &pack<moCombinedContinuator<Ngh>>(cp);
      auto falseCont = &pack<falseContinuator<Ngh>>();
      singleStepContinuator->add(*falseCont);
      localSearch->setContinuator(*singleStepContinuator);
    }

    return &pack<myResizableLocalSearch<Ngh>>(
        *localSearch, *neighborhood,
        [this](int size) { return this->_problem.getNeighborhoodSize(size); });
  }

  auto buildDestructionSize() -> DestructionSize* {
    const std::string name =
        categoricalName(".Perturb.DestructionSizeStrategy");
    if (name == "fixed") {
      const int fixedDs = integer(".Perturb.DestructionSize");
      return &pack<FixedDestructionSize>(fixedDs);
    } else if (name == "adaptive") {
      auto operator_selection = buildOperatorSelection("");
      int rewardType = categorical(".AOS.RewardType");
      return &pack<AdaptiveDestructionSize<Ngh>>(*operator_selection,
                                                 *getRewards(), rewardType);
    }
    return nullptr;
  }

  auto buildDestructionStrategy() -> DestructionStrategy<FSP>* {
    auto destructionSize = buildDestructionSize();
    auto destructionName = categoricalName(".DestructionStrategy");
    if (destructionName == "random") {
      return &pack<RandomDestructionStrategy<FSP>>(*destructionSize);
    } else if (destructionName == "adaptive_position") {
      auto positionSelector = buildPositionSelector(".AdaptivePosition");
      int rewardType = categorical(".AdaptivePosition.RewardType");
      return &pack<AdaptivePositionDestructionStrategy<FSP>>(
          *destructionSize, *positionSelector, *getRewards(), rewardType);
    }
    return nullptr;
  }

  auto domainPerturb() -> moPerturbation<Ngh>* override {
    const std::string name = categoricalName(".Perturb");
    return domainPerturbByName(name);
  }

  auto domainPerturbByName(const std::string& name) -> moPerturbation<Ngh>* {
    auto& eval = _problem.eval();
    if (name == "rs") {
      auto insertionName = categoricalName(".Perturb.Insertion");
      auto insertion =
          buildInsertionStrategy(insertionName, _problem.neighborEval())
              .release();
      storeFunctor(insertion);
      auto destructionStrategy = buildDestructionStrategy();
      return &pack<DestructionConstruction<Ngh>>(*insertion,
                                                 *destructionStrategy);
    } else if (name == "lsps") {
      auto insertionName = categoricalName(".Perturb.Insertion");
      auto insertion =
          buildInsertionStrategy(insertionName, _problem.neighborEval())
              .release();
      storeFunctor(insertion);
      auto destructionStrategy = buildDestructionStrategy();
      auto lspsLocalSearch = buildLSPSLocalSearch();
      return &pack<IGLocalSearchPartialSolution<Ngh>>(
          *insertion, *destructionStrategy, *lspsLocalSearch);
    } else if (name == "swap") {
      auto kickPerturb = &pack<ilsKickOp<EOT>>(2, 0);
      return &pack<moMonOpPerturb<Ngh>>(*kickPerturb, eval);
    } else if (name == "adaptive") {
      std::vector<moPerturbation<Ngh>*> perturbations = {
          domainPerturbByName("rs"),
          domainPerturbByName("lsps"),
          domainPerturbByName("swap")};
      std::vector<int> options = {0, 1, 2};
      auto operatorSelection =
          buildOperatorSelection(".AdaptivePerturb", options);
      int rewardType = categorical(".AdaptivePerturb.AOS.RewardType");
      auto op = &pack<AdaptivePerturb<Ngh>>(perturbations, *getRewards(),
                                             rewardType, *operatorSelection);
      return &pack<moMonOpPerturb<Ngh>>(*op, eval);
    }
    return nullptr;
  }

  auto buildPositionSelector(const std::string& prefix) -> PositionSelector* {
    std::vector<int> options;
    const std::string noArms = categoricalName(prefix + ".NoArms");
    if (noArms == "no_jobs") {
      options.resize(_problem.size());
    } else if (noArms.find("fixed_") == 0) {
      const int parsedNoArms = std::stoi(noArms.substr(6));
      options.resize(parsedNoArms);
    }
    std::iota(options.begin(), options.end(), 1);
    if (categoricalName(prefix + ".RandomArm") == "yes") {
      options.push_back(0);
    }
    auto operatorSelection = buildOperatorSelection(prefix, options);
    auto replace = categoricalName(prefix + ".Replace");
    if (replace == "yes") {
      return &pack<AdaptivePositionSelector>(*operatorSelection);
    } else if (replace == "no") {
      return &pack<AdaptiveNoReplacementPositionSelector>(*operatorSelection);
    }
    assert(false);
    return nullptr;
  }

  auto domainLocalSearch() -> moLocalSearch<Ngh>* override {
    auto compNN = buildNeighborComparator();
    auto compSN = buildSolNeighborComparator();

    auto& eval = _problem.eval();
    auto& nEval = _problem.neighborEval();
    auto& cp = _problem.checkpoint();
    auto& nghCp = _problem.neighborhoodCheckpoint();

    std::string name = categoricalName(".Local.Search");
    if (name == "adaptive") {
      std::vector<moLocalSearch<Ngh>*> localSearches = {
          buildLocalSearchByName("first_improvement", false),
          buildLocalSearchByName("best_improvement", false),
          buildLocalSearchByName("random_best_improvement", false),
          buildLocalSearchByName("best_insertion", false)};
      std::vector<int> options = {0, 1, 2, 3};
      auto operatorSelection =
          buildOperatorSelection(".AdaptiveLocalSearch", options);
      int rewardType = categorical(".AdaptiveLocalSearch.AOS.RewardType");
      return &pack<AdaptiveLocalSearch<Ngh>>(localSearches, *getRewards(),
                                             rewardType, *operatorSelection, cp,
                                             eval);
    } else if (name == "adaptive_best_insertion") {
      auto positionSelector = buildPositionSelector(".AdaptiveBestInsertion");
      auto explorer = &pack<AdaptiveBestInsertionExplorer<EOT>>(
          *positionSelector, nEval, nghCp, *compNN, *compSN);
      return &pack<moLocalSearch<Ngh>>(*explorer, cp, eval);
    }
    throw std::runtime_error("Local search type not found: " + name + ".");
    return nullptr;
  }
};