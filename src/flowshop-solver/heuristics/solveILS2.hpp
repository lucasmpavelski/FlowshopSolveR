#include <random>

// paradiseo libs
#include <continuator/moBestNoImproveContinuator.h>
#include <continuator/moContinuator.h>
#include <eo>
#include <mo>
#include <neighborhood/moNeighborhood.h>
#include <utils/eoParam.h>

// problem
#include "global.hpp"
#include "problems/FSPEvalFunc.hpp"
#include "problems/NIFSPEvalFunc.hpp"
#include "problems/NWFSPEvalFunc.hpp"

// NEH
#include "heuristics/FSPOrderHeuristics.hpp"
#include "heuristics/NEHInit.hpp"

// TS
#include "heuristics/dummyAspiration.hpp"
#include "heuristics/moFirstBestTS.hpp"
#include "heuristics/moFirstTS.hpp"

// ILS
#include "IGexplorer.hpp"
#include "IGexplorerWithRepl.hpp"
// IG components
#include "OpPerturbDestConst.hpp"
#include "acceptCritTemperature.hpp"
// for the kick : here swap(=exchange) because neighborhood with insert
#include <eoSwapMutation.h>
// NILS perturb
#include "randomNeutralWalkExplorer.hpp"

#include "problems/FSPProblem.hpp"
#include "MHParamsValues.hpp"
#include "fspproblemfactory.hpp"
#include "specsdata.hpp"

template<class Ngh, class EOT = typename Ngh::EOT>
void solveILS2(const int N, const int M, const int max_ct, const std::vector<int> &order,
              eoEvalFuncCounter<EOT> &fullEval, moEvalCounter<Ngh> &evalN,
              moCheckpoint<Ngh> &checkpoint, moCheckpoint<Ngh> &checkpointGlobal,
              const MHParamsValues &params) {
  const int max_nh_size = pow(N - 1, 2);

  // comparator strategy
  moSolComparator<EOT> compSS0;              // comp sol/sol strict
  moSolNeighborComparator<Ngh> compSN0;      // comp sol/Ngh strict
  moNeighborComparator<Ngh> compNN0;         // comp Ngh/Ngh strict
  moEqualSolComparator<EOT> compSS1;         // comp sol/sol with equal
  moEqualSolNeighborComparator<Ngh> compSN1; // comp sol/Ngh with equal
  moEqualNeighborComparator<Ngh> compNN1;    // comp Ngh/Ngh with equal
  moSolComparator<EOT> *compSS = nullptr;
  moSolNeighborComparator<Ngh> *compSN = nullptr;
  moNeighborComparator<Ngh> *compNN = nullptr;
  switch (params.categorical("Comp.Strat")) {
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

  moSolComparator<EOT> *lscompSS = nullptr;
  moSolNeighborComparator<Ngh> *lscompSN = nullptr;
  moNeighborComparator<Ngh> *lscompNN = nullptr;
  switch (params.categorical("ILS.LS.CompStrat")) {
  case 0:
    lscompSS = &compSS0;
    lscompSN = &compSN0;
    lscompNN = &compNN0;
    break;
  case 1:
    lscompSS = &compSS1;
    lscompSN = &compSN1;
    lscompNN = &compNN1;
    break;
  default:
    assert(false);
    break;
  }

  // initialization
  eoInitPermutation<EOT> init0(N);
  NEHInitOrdered<EOT> init1(fullEval, order);
  NEHInitRandom<EOT> init2(fullEval, N);
  eoInit<EOT> *init = nullptr;
  switch (params.categorical("Init.Strat")) {
  case 0:
    init = &init0;
    break;
  case 1:
    init = &init1;
    break;
  case 2:
    init = &init2;
    break;
  default:
    assert(false);
    break;
  }

  // neighborhood

  // neighborhood size
  const int min_nh_size = (N >= 20) ? 11 : 2;
  const int nh_interval = (N >= 20) ? 10 : 1;
  const int no_nh_sizes = (max_nh_size - min_nh_size) / nh_interval;

//  const int ils_nh_size = params.real("Neighborhood.Size") * N / 10.0;

  // ls neighborhood size
  const int scale_ls =
      int((no_nh_sizes + 1) * params.real("Neighborhood.Size") / 10.0f);
  const int ls_nh_size = std::min(max_nh_size, min_nh_size + scale_ls * nh_interval);

  moOrderNeighborhood<Ngh> neighborhood0(ls_nh_size);
  moRndWithoutReplNeighborhood<Ngh> neighborhood1(ls_nh_size);
  moNeighborhood<Ngh> *neighborhood = nullptr;
  switch (params.categorical("ILS.LS.NeighborhoodStrat")) {
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
  moFirstImprHC<Ngh> algo0(*neighborhood, fullEval, evalN, checkpoint, *lscompNN, *lscompSN); // FIHC
  moSimpleHC<Ngh> algo1(*neighborhood, fullEval, evalN, checkpoint, *lscompNN, *lscompSN); //BestHC
  moRandomBestHC<Ngh> algo2(*neighborhood, fullEval, evalN, checkpoint, *lscompNN, *lscompSN); //rndBestHC

  moLocalSearch<Ngh> *algo;
  switch (params.categorical("ILS.LS")) {
  case 0: algo=&algo0; break;
  case 1: algo=&algo1; break;
  case 2: algo=&algo2; break;
  default:
    assert(false);
    break;
  }

  // IG accept criterion based on temperature
  const double temperature = params.real("ILS.AcceptTemperature") * max_ct / (N * M * 10.0f);
  acceptCritTemperature<Ngh> accept2(temperature);

  /****
  *** Perturb in ILS
  ****/
  // IG perturbation
  OpPerturbDestConst<EOT> OpPerturb(fullEval, params.integer("ILS.PerturbDestructionSize"), *compSS);
  moMonOpPerturb<Ngh> perturb1(OpPerturb, fullEval);

  /****
  *** ILS
  ****/
  moILS<Ngh,Ngh> ils(*algo, fullEval, checkpointGlobal, perturb1, accept2);
  iterateUntilMaxEval(*init, ils, fullEval, checkpointGlobal);
}

