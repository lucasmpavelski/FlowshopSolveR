#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/MHParamsValues.hpp"
#include "flowshop-solver/eoFactory.hpp"

#include "problems/FSP.hpp"
#include "problems/FSPProblem.hpp"

#include "flowshop-solver/heuristics/FSPOrderHeuristics.hpp"
#include "flowshop-solver/heuristics/NEH.hpp"

#include "flowshop-solver/heuristics/acceptCritTemperature.hpp"
#include "flowshop-solver/problems/FSPProblem.hpp"

#include "flowshop-solver/heuristics/perturb/perturb.hpp"

#include "flowshop-solver/heuristics/BestInsertionExplorer.hpp"
#include "flowshop-solver/heuristics/ig_lsps.hpp"

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

class eoFSPFactory : public eoFactory<FSPProblem::Ngh> {
  FSPProblem& _problem;

 public:
  eoFSPFactory(const MHParamsValues& params, FSPProblem& problem)
      : eoFactory<FSPProblem::Ngh>{params, problem}, _problem{problem} {};

  using EOT = FSPProblem::EOT;
  using Ngh = FSPProblem::Ngh;

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
      auto nehPriority = categoricalName(".Init.NEH.Priority");
      auto nehPriorityOrder = categoricalName(".Init.NEH.PriorityOrder");
      auto nehPriorityWeighted = integer(".Init.NEH.PriorityWeighted");

      auto priority = buildPriority(_problem.data(), nehPriority,
                                    nehPriorityWeighted, nehPriorityOrder);
      storeFunctor(priority);

      auto nehInsertion = categoricalName(".Init.NEH.Insertion");
      auto insertion =
          buildInsertionStrategy(nehInsertion, _problem.neighborEval());
      storeFunctor(insertion);

      return &pack<NEH<Ngh>>(*priority, *insertion);
    }
    return nullptr;
  }

  auto buildLSPSLocalSearch(const int destruction_size) -> moLocalSearch<Ngh>* {
    const int nh_size_lsps = pow(_problem.size() - destruction_size - 1, 2);
    auto neighborhood = buildNeighborhood(nh_size_lsps);

    const std::string name = categoricalName("IG.LSPS.Local.Search");

    auto compNN = buildNeighborComparator();
    auto compSN = buildSolNeighborComparator();
    auto& eval = _problem.eval();
    auto& nEval = _problem.neighborEval();
    auto& cont = _problem.continuator();
    auto& nghCp = _problem.neighborhoodCheckpoint();

    if (name == "first_improvement") {
      return &pack<moFirstImprHC<Ngh>>(*neighborhood, eval, nEval, cont,
                                       *compNN, *compSN);
    } else if (name == "best_improvement") {
      return &pack<moSimpleHC<Ngh>>(*neighborhood, eval, nEval, cont, *compNN,
                                    *compSN);
    } else if (name == "random_best_improvement") {
      return &pack<moRandomBestHC<Ngh>>(*neighborhood, eval, nEval, cont,
                                        *compNN, *compSN);
    } else if (name == "best_insertion") {
      auto explorer =
          pack<BestInsertionExplorer<EOT>>(nEval, nghCp, *compNN, *compSN);
      return &pack<moLocalSearch<Ngh>>(explorer, cont, eval);
    }
    return nullptr;
  }

  auto buildOperatorSelection() -> OperatorSelection<int>* {
    std::vector<int> options = {2, 4, 8};
    const std::string name = categoricalName(".AOS.Strategy");
    if (name == "probability_matching") {
      return &pack<ProbabilityMatching<int>>(options);
    } else if (name == "frrmab") {
      return &pack<FRRMAB<int>>(options);
    } else if (name == "linucb") {
      auto fitnessHistory = pack<FitnessHistory<EOT>>();
      auto& awSize = pack<AdaptiveWalkLengthFLA<EOT>>(fitnessHistory,
                                                      1.0 / _problem.size());
      auto& autocorr = pack<AutocorrelationFLA<EOT>>(fitnessHistory);
      auto& fdc = pack<FitnessDistanceCorrelationFLA<EOT>>(fitnessHistory);
      //auto& neutralityFLA = pack<NeutralityFLA<Ngh>>();

      _problem.checkpoint().add(fitnessHistory);
      //_problem.neighborhoodCheckpoint().add(neutralityFLA);
      _problem.checkpoint().add(_problem.neighborhoodCheckpoint());
      //_problem.checkpoint().add(neutralityFLA);

      ProblemContext context;
      context.add(awSize);
      // context.add(neutralityFLA);
      context.add(autocorr);
      context.add(fdc);

      return &pack<LinUCB<int>>(options, context);
    } else if (name == "thompson_sampling") {
      return &pack<ThompsonSampling<int>>(options);
    } else if (name == "random") {
      return &pack<Random<int>>(options);
    }
    return nullptr;
  }

  auto domainPerturb() -> moPerturbation<Ngh>* override {
    const int destructionSize = integer(".Perturb.DestructionSize");
    
    auto insertionName = categoricalName(".Perturb.Insertion");
    auto insertion =
        buildInsertionStrategy(insertionName, _problem.neighborEval());
    storeFunctor(insertion);

    const std::string name = categoricalName(".Perturb");
    if (name == "rs") {
      return &pack<DestructionConstruction<Ngh>>(*insertion, destructionSize);
    } else if (name == "lsps") {
      auto lspsLocalSearch = buildLSPSLocalSearch(destructionSize);
      return &pack<IGLocalSearchPartialSolution<Ngh>>(
          *insertion, *lspsLocalSearch, destructionSize);
    } else if (name == "adaptive") {
      FitnessRewards<EOT> rewards;
      _problem.checkpoint().add(rewards.localStat());
      _problem.checkpointGlobal().add(rewards.globalStat());
      auto operator_selection = buildOperatorSelection();
      return &pack<AdaptiveDestructionConstruction<Ngh>>(
          *insertion, *operator_selection, rewards,
          categorical(".AOS.RewardType"));
    }
    return nullptr;
  }
};