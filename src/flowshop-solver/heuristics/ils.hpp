#pragma once

#include <unordered_map>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/heuristics/IGexplorer.hpp"
#include "flowshop-solver/MHParamsValues.hpp"
#include "flowshop-solver/heuristics/acceptCritTemperature.hpp"
#include "flowshop-solver/heuristics/falseContinuator.hpp"
#include "flowshop-solver/heuristics.hpp"
#include "flowshop-solver/MHParamsSpecsFactory.hpp"
#include "flowshop-solver/FSPProblemFactory.hpp"
#include "flowshop-solver/heuristics/op_cooling_schedule.hpp"

#include "flowshop-solver/heuristics/perturb/perturb.hpp"
#include "flowshop-solver/number-of-swaps/FixedNumberOfSwaps.hpp"

template <class Ngh, class EOT = typename Problem<Ngh>::EOT>
auto solveWithILS(Problem<Ngh>& prob, const MHParamsValues& params) -> Result {
  const int N = prob.size(0);
  const int M = prob.size(1);
  const int max_nh_size = pow(N - 1, 2);
  const double max_ct = prob.upperBound();

  // continuator
  eoEvalFunc<EOT>& fullEval = prob.eval();
  moEval<Ngh>& evalN = prob.neighborEval();

  moCheckpoint<Ngh>& checkpoint = prob.checkpoint();
  moCheckpoint<Ngh>& checkpointGlobal = prob.checkpointGlobal();

  // comparator strategy
  moSolComparator<EOT> compSS0;               // comp sol/sol strict
  moSolNeighborComparator<Ngh> compSN0;       // comp sol/Ngh strict
  moNeighborComparator<Ngh> compNN0;          // comp Ngh/Ngh strict
  moEqualSolComparator<EOT> compSS1;          // comp sol/sol with equal
  moEqualSolNeighborComparator<Ngh> compSN1;  // comp sol/Ngh with equal
  moEqualNeighborComparator<Ngh> compNN1;     // comp Ngh/Ngh with equal
  moSolComparator<EOT>* compSS = nullptr;
  moSolNeighborComparator<Ngh>* compSN = nullptr;
  moNeighborComparator<Ngh>* compNN = nullptr;
  switch (params.categorical("ILS.Comp.Strat")) {
    case 0:
      compSS = &compSS0;
      compSN = &compSN0;
      compNN = &compNN0;
      break;
    case 1:
      compSS = &compSS1;
      compSN = &compSN1;
      compNN = &compNN1;
      break;
    default:
      assert(false);
      break;
  }

  // initialization
  eoInit<EOT>* init = nullptr;

  // neighborhood size
  const int min_nh_size = (N >= 20) ? 11 : 2;
  const int nh_interval = (N >= 20) ? 10 : 1;
  const int no_nh_sizes = (max_nh_size - min_nh_size) / nh_interval;
  const int scale =
      int((no_nh_sizes + 1) * params.real("ILS.Neighborhood.Size") / 10.0);
  const int nh_size = std::min(max_nh_size, min_nh_size + scale * nh_interval);

  moOrderNeighborhood<Ngh> neighborhood0(nh_size);
  moRndWithoutReplNeighborhood<Ngh> neighborhood1(nh_size);
  moNeighborhood<Ngh>* neighborhood = nullptr;
  switch (params.categorical("ILS.Neighborhood.Strat")) {
    case 0:
      neighborhood = &neighborhood0;
      break;
    case 1:
      neighborhood = &neighborhood1;
      break;
    default:
      assert(false);
      break;
  }

  // algos xxHC
  moFirstImprHC<Ngh> algo0(*neighborhood, fullEval, evalN, checkpoint, *compNN,
                           *compSN);  // FIHC
  moSimpleHC<Ngh> algo1(*neighborhood, fullEval, evalN, checkpoint, *compNN,
                        *compSN);  // BestHC
  moRandomBestHC<Ngh> algo2(*neighborhood, fullEval, evalN, checkpoint, *compNN,
                            *compSN);  // rndBestHC

  // IG (Ruiz+Stuetzle)
  // iterative greedy improvement without replacement (IG)
  // FastIGexplorer igexplorer(evalN, *compNN, *compSN);
  IGexplorer<Ngh> igexplorer(fullEval, N, *compSS);
  moLocalSearch<Ngh> algo3(igexplorer, checkpoint, fullEval);
  moLocalSearch<Ngh>* algo;
  switch (params.categorical("ILS.Algo")) {
    case 0:
      algo = &algo0;
      break;
    case 1:
      algo = &algo1;
      break;
    case 2:
      algo = &algo2;
      break;
    case 3:
      algo = &algo3;
      break;
    default:
      assert(false);
      break;
  }
  moCombinedContinuator<Ngh> singleStepContinuator(checkpoint);
  falseContinuator<Ngh> falseCont;
  singleStepContinuator.add(falseCont);
  if (params.categorical("ILS.LS.Single.Step")) {
    algo->setContinuator(singleStepContinuator);
  }

  moAlwaysAcceptCrit<Ngh> accept0;
  moBetterAcceptCrit<Ngh> accept1(
      compSS0);  // no interest to accept equal solution here !
  // IG accept criterion based on temperature
  const double temperature =
      params.real("ILS.Accept.Temperature") * max_ct / (N * M * 10);
  acceptCritTemperature<Ngh> accept2(temperature);

  moAcceptanceCriterion<Ngh>* accept;
  switch (params.categorical("ILS.Accept")) {
    case 0:
      accept = &accept0;
      break;
    case 1:
      accept = &accept1;
      break;
    case 2:
      accept = &accept2;
      break;
    default:
      assert(false);
      break;
  }

  /****
  *** Perturb in ILS
  ****/

  eoInit<EOT>* perturbRestartInit = nullptr;
  // switch (params.categorical("ILS.Perturb.Restart.Init")) {

  moRestartPerturb<Ngh> perturb0(
      *perturbRestartInit, fullEval,
      params.integer("ILS.Perturb.Restart.Threshold"));
  // IG perturbation
  //  OpPerturbDestConst<EOT> OpPerturb(fullEval,
  //  params.integer("ILS.Perturb.Destruction.Size"));
  // OpPerturbDestConst<EOT> OpPerturb(fullEval,
  // params.integer("ILS.Perturb.Destruction.Size")); moMonOpPerturb<Ngh>
  // perturb1(OpPerturb, fullEval);
  // Kick perturbation with Exhange
  FixedNumberOfSwaps noSwaps(params.integer("ILS.Perturb.No.Kick"));
  ilsKickOp<EOT> kickPerturb(noSwaps);
  moMonOpPerturb<Ngh> perturb1(kickPerturb, fullEval);

  // init -> monOp in order to use it in nilsPerturb framework
  eoInitAdaptor<EOT> initPerturb0(*init);  // init perturb random
  eoInitAdaptor<EOT> initPerturb1(*init);  // init perturb NEH
  eoInitAdaptor<EOT> initPerturb2(*init);  // init perturb NEHrnd
  OpPerturbDestConst<EOT> NILSOpPerturb(
      fullEval, params.integer("ILS.Perturb.NILS.Destruction.Size"));
  // eoSwapMutation<EOT> NILSkickPerturb(
  //    params.integer("ILS.Perturb.NILS.No.Kick"));

  eoMonOp<EOT>* nilsEscape;
  switch (params.categorical("ILS.Perturb.NILS.Escape")) {
    case 0:
      nilsEscape = &initPerturb0;
      break;
    case 1:
      nilsEscape = &initPerturb1;
      break;
    case 2:
      nilsEscape = &initPerturb2;
      break;
    case 3:
      nilsEscape = &NILSOpPerturb;
      break;
    case 4:
      nilsEscape = nullptr;  // &NILSkickPerturb;
      break;
    default:
      assert(false);
      break;
  }

  // NILS perturbation : nilsMNS
  randomNeutralWalkExplorer<Ngh> nilsPerturb(
      *neighborhood, evalN, fullEval, compSN0,
      params.integer("ILS.Perturb.NILS.MNS"), *nilsEscape);
  moLocalSearch<Ngh> nilsPerturbLS(nilsPerturb, checkpoint, fullEval);
  moMonOpPerturb<Ngh> perturb2(nilsPerturbLS, fullEval);

  moPerturbation<Ngh>* perturb;
  switch (params.categorical("ILS.Perturb")) {
    case 0:
      perturb = &perturb0;
      break;
    case 1:
      perturb = &perturb1;
      break;
    case 2:
      perturb = &perturb2;
      break;
    // case 3: perturb=&perturb3; break;
    default:
      assert(false);
      break;
  }

  /****
  *** ILS
  ****/
  moILS<Ngh, Ngh> ils(*algo, fullEval, checkpointGlobal, *perturb, *accept);

  return runExperiment(*init, ils, prob);
}
