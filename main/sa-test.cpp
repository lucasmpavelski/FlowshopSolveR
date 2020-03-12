#include <iostream>

//#include "meta/Parameter.h"
//
#include <eo>
#include <eoInt.h>
#include <eoScalarFitness.h>
#include <mo>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
using namespace std;

#include <utils/eoOStreamMonitor.h>
#include <utils/eoParser.h>
#include <utils/eoState.h>
#include <utils/eoStdoutMonitor.h>
#include <eo>
#include <mo>

// PB
#include <fsp/FSP.h>
#include "fsp/FSPEvalFunc.h"
// Init NEH
#include <NEHInit.h>
// IG greedy improvement
#include <IGexplorer.hpp>
#include <IGexplorerWithRepl.hpp>
// IG components
#include <OpPerturbDestConst.hpp>
#include <acceptCritTemperature.hpp>
// for the kick : here swap(=exchange) because neighborhood with insert
#include <eoSwapMutation.h>
// NILS perturb
#include <randomNeutralWalkExplorer.hpp>
// counterEval
#include <fspEvalContinuator.h>
#include <fspEvalCounter.h>
// continuator
#include <myBestNoImproveContinuator.h>
#include <continuator/moBestNoImproveContinuator.h>

class prefixedPrinter : public eoStdoutMonitor {
 public:
  prefixedPrinter(std::string _prefix, std::string _delim = "\t",
                  unsigned int _width = 20, char _fill = ' ')
      : eoStdoutMonitor{_delim, _width, _fill}, prefix{_prefix, "prefix"} {
    add(prefix);
  }

  eoValueParam<std::string> prefix;
};

int main() {
  using std::string;
  using std::cout;
  using std::endl;

  std::random_device rd;
  std::uniform_int_distribution<int> unif;
  rng.reseed(unif(rd));

  /*** CONFIGURATION ***/
  string inputFilename =
      "/home/lucasmp/projects/meta-learning/instances/taillard/20_5/"
      "20_5_01.txt";
  string init_strat = "RANDOM";
  string neighbor_strat = "RANDOM";
  string compare_strat = "STRICT";
  string alg_name = "SIMPLE_COOLING";
  int nhSize = 2;
  double init_temp = 100;
  double final_temp = 0.01;
  double alpha = 0.5;
  double span_simple = 100;
  double span_tries_max = 5000;
  double span_move_max = 100;
  double span_nb_max = 5;
  int maxEval = 0;
  int max_iter_no_improve = 10000000;

  /*** PROBLEM EVALUATION ***/
  typedef moShiftNeighbor<FSP> Ngh;
  FSPData fd(inputFilename);
  PermFSPEvalFunc<FSP> permut(fd);
  if (maxEval == 0) maxEval = permut.noJobs() * 200;
  cout << "maxEval: " << maxEval << endl;
  fspEvalCounter<FSP> fullEval(permut);
  fspEvalContinuator<Ngh> max_eval(fullEval, maxEval, false);
  moFullEvalByCopy<Ngh> evalN(fullEval);

  /// SA CONTINUATOR
  // SA stops after max_iter_no_improve evaluations
  moSolutionStat<FSP> bestFound;//(true);
  myBestNoImproveContinuator<Ngh> best_no_improve(max_iter_no_improve, true);
  // SA also stops when the max_eval is reached
  moCheckpoint<Ngh> checkpoint(max_eval);
  checkpoint.add(bestFound);
  // store best value found (even if its not accepted by SA)
  moBestSoFarStat<FSP> bestFoundGlobal(false);
  checkpoint.add(bestFoundGlobal);
  // output SA best fitness (part of the stopping criterion)
  prefixedPrinter print("sa_best:", " ");
  print.add(bestFound);
  checkpoint.add(print);
  // put best_no_improve condition before bestFound update
  moCombinedContinuator<Ngh> sa_continuator(best_no_improve);
  sa_continuator.add(checkpoint);

  /*** NEIGHBORHOOD FUNCTION ***/
  const unsigned int nhSizeMax = pow((permut.noJobs() - 1), 2);
  if (nhSize > nhSizeMax) nhSize = nhSizeMax;
  cout << "nhSize:" << nhSize << endl;
  moOrderNeighborhood<Ngh> neighborhood0(nhSize);
  moRndWithoutReplNeighborhood<Ngh> neighborhood1(nhSize);
  moNeighborhood<Ngh> *neighborhood;
  if (neighbor_strat == "ORDER")
    neighborhood = &neighborhood0;
  else if (neighbor_strat == "RANDOM")
    neighborhood = &neighborhood1;
  else
    assert(false);

  /*** INITIALIZATION ***/
  eoInitPermutation<FSP> init0(permut.noJobs());  // random permut
  NEHInitOrdered<FSP> init1(fullEval, fd.noJobs(),
                            compareByTotalProcTimes(fd));  // NEH heuristics
  NEHInitRandom<FSP> init2(fullEval, fd.noJobs());
  eoInit<FSP> *init;
  if (init_strat == "RANDOM")
    init = &init0;
  else if (init_strat == "NEH")
    init = &init1;
  else if (init_strat == "NEH_RANDOM")
    init = &init2;
  else
    assert(false);

  /*** COMPARATOR ***/
  moSolComparator<FSP> compSS0;               // comp sol/sol strict
  moSolNeighborComparator<Ngh> compSN0;       // comp sol/Ngh strict
  moNeighborComparator<Ngh> compNN0;          // comp Ngh/Ngh strict
  moEqualSolComparator<FSP> compSS1;          // comp sol/sol with equal
  moEqualSolNeighborComparator<Ngh> compSN1;  // comp sol/Ngh with equal
  moEqualNeighborComparator<Ngh> compNN1;     // comp Ngh/Ngh with equal
  moSolComparator<FSP> *compSS;
  moSolNeighborComparator<Ngh> *compSN;
  moNeighborComparator<Ngh> *compNN;
  if (compare_strat == "STRICT") {
    compSS = &compSS0;
    compSN = &compSN0;
    compNN = &compNN0;
  } else if (compare_strat == "EQUAL") {
    compSS = &compSS1;
    compSN = &compSN1;
    compNN = &compNN1;
  }

  /*** ALGORITHM ***/
  moSimpleCoolingSchedule<FSP> cooling0(init_temp, alpha, span_simple,
                                        final_temp);
  moDynSpanCoolingSchedule<FSP> cooling1(init_temp, alpha, span_tries_max,
                                         span_move_max, span_nb_max);
  moCoolingSchedule<FSP> *cooling;
  if (alg_name == "SIMPLE_COOLING")
    cooling = &cooling0;
  else if (alg_name == "SIMPLE_COOLING")
    cooling = &cooling1;
  else
    assert(false);

  /*** SA ***/
  moSA<Ngh> algo(*neighborhood, fullEval, evalN, *cooling, *compSN,
                 sa_continuator);

  /*** ILS ***/
  moBestSoFarStat<FSP> ilsBestFound(false);
  moCheckpoint<Ngh> ils_checkpoint(max_eval);
  ils_checkpoint.add(ilsBestFound);
  prefixedPrinter print_ils("isl best:", " ");
  print_ils.add(ilsBestFound);
  ils_checkpoint.add(print_ils);
  moRestartPerturb<Ngh> perturb(init0, fullEval, 1);
  moBetterAcceptCrit<Ngh> accept(compSS0);
  moILS<Ngh, Ngh> ils(algo, fullEval, ils_checkpoint, perturb, accept);

  /*** RUN ***/
  FSP sol;
  (*init)(sol);
  fullEval(sol);
  cout << "initial: " << sol << endl;
  ils(sol);

  /*** RESULTS ***/
  cout << "number of evaluations: " << max_eval.value() << endl;
  cout << "sa final best: " << bestFound.value() << endl;
  cout << "ils final best: " << ilsBestFound.value() << endl;
  cout << "global best: " << bestFoundGlobal.value() << endl;



  //using FSP = eoInt<eoMaximizingFitness>;
  //typedef moShiftNeighbor<FSP> Ngh;
  moRndWithoutReplNeighborhood<Ngh> rnd_neighborhood(2);
  FSP solution(10);
  for (int i = 0; i < 10; i++) solution[i] = i;
  FSP orig_sol = solution;
  std::cout << "Original solution:\n" << solution << "\n";
  std::cout << "Neighbours:\n";
  Ngh neighbor;
  rnd_neighborhood.init(sol, neighbor);
  rnd_neighborhood.next(solution, neighbor);
  neighbor.move(solution);
  std::cout << solution << "\n";
  
  //Problem p;
  //HCParamsSpecs hc_params(10);
  //MHParamsValues values(hc_params);
  //values["Neigh.Size"] = 3;
  //std::cout << apply(values, p);
  return 0;
}
