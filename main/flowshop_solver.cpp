//#include <cstdlib>
//#include <fstream>
//#include <iostream>
//#include <random>
//#include <sstream>
//#include <string>
// using namespace std;
//
//#include <utils/eoOStreamMonitor.h>
//#include <utils/eoParser.h>
//#include <utils/eoState.h>
//#include <utils/eoStdoutMonitor.h>
//#include <eo>
//#include <mo>
//
//// PB
////
//// IG greedy improvement
//#include <IGexplorer.hpp>
//#include <IGexplorerWithRepl.hpp>
//// IG components
//#include <OpPerturbDestConst.hpp>
//#include <acceptCritTemperature.hpp>
//// for the kick : here swap(=exchange) because neighborhood with insert
//#include <eoSwapMutation.h>
//// NILS perturb
//#include <randomNeutralWalkExplorer.hpp>
//// counterEval
//// continuator
//#include <continuator/moBestNoImproveContinuator.h>
//#include <myBestNoImproveContinuator.h>
//
//#include <neighborhood/moIndexNeighborhood.h>
//#include <neighborhood/moRndNeighborhood.h>
//#include <utils/eoRNG.h>
//
//#include "dummyAspiration.h"

#include "FlowshopProblem.h"
#include "metaheuristics/HCMetaheuristic.h"

#include "MetaheuristicSolve.h"

#include <iostream>

int main(int argc, char* argv[]) {
  using std::string;
  using std::cout;
  using std::endl;

  eoParser parser(argc, argv);

  // constant misc
  std::random_device rd;
  std::uniform_int_distribution<uint32_t> dist;
  string instances_folder = "/home/lucasmp/projects/meta-learning/instances";

  // defauts
  auto seed = dist(rd);
  string input_filename = instances_folder + "/taillard/20_5/20_5_01.txt";
  int max_eval = 0;
  int max_no_impr = 10;
  string init_strat = "RANDOM";
  string neigh_strat = "RANDOM";
  string comp_strat = "STRICT";
  int nh_size = 10;
  // hc
  string hc_alg = "FI";

  input_filename = parser
                       .createParam(input_filename, "input_file",
                                    "Instance file path", 'I', "Input")
                       .value();

  seed = parser.createParam(seed, "seed", "RNG seed", 'S', "Param").value();
  max_eval = parser
                 .createParam(max_eval, "max_eval", "Max number of evaluations",
                              'E', "Param")
                 .value();

  init_strat =
      parser
          .createParam(init_strat, "init_strat",
                       "Initialization strategy [RANDOM,NEH,NEH_RANDOM]", 'I',
                       "Param")
          .value();

  neigh_strat =
      parser
          .createParam(neigh_strat, "neigh_strat",
                       "Neighborhood strategy [ORDER,RANDOM]", 'N', "Param")
          .value();


  comp_strat =
      parser
          .createParam(comp_strat, "comp_strat",
                       "Comparison strategy [STRICT,EQUAL]", 'C', "Param")
          .value();

nh_size =
      parser.createParam(361u, "nh_size", "Neighborhood size", 'n', "Param")
          .value();


  hc_alg =
      parser
          .createParam(hc_alg, "hc_alg", "Algorithm name [FI,BEST,RANDOM_BEST]",
                       'P', "HC Param")
          .value();

  // max_no_impr =
  //     parser
  //         .createParam(max_no_impr, "max_no_impr",
  //                      "Maximum evaluations without improvement", 'e',
  //                      "Param")
  //         .value();

    // string objective_str =
  //     parser
  //         .createParam(string("MAKESPAN"), "objective",
  //                      "objective criteria [MAKESPAN,FLOWTIME]", 'O',
  //                      "Param")
  //         .value();
  // unsigned int objective;
  // if (objective_str == "MAKESPAN")
  //   objective = 0;
  // else if (objective_str == "FLOWTIME")
  //   objective = 1;
  // else
  //   assert(false);

  // string problem =
  //     parser
  //         .createParam("PERMUT"s, "problem",
  //                      "Problem type [PERMUT,NO_WAIT,NO_IDLE]", 'P', "Param")
  //         .value();

  rng.reseed(seed);

  /*** CONFIGURATION ***/
  FlowshopProblem fsp(input_filename, max_eval);

  HCMetaheuristic hc{fsp.size(), fsp.eval()};
  HCMetaheuristic::Params hc_params;
  //HCMetaheuristic::Params::fromSequence({0, 1, 1, 187, 2}, hc_params);
  hc_params.neigh_strat = strToEnum<Metaheuristic::NeighborStrat>(neigh_strat);
  hc_params.init_strat = strToEnum<Metaheuristic::InitStrat>(init_strat);
  hc_params.comp_strat = strToEnum<Metaheuristic::CompStrat>(comp_strat);
  hc_params.algo = strToEnum<HCMetaheuristic::Algorithm>(hc_alg);
  std::cout << "params: " << hc_params << '\n';
  std::cout << "results: " << solve(fsp, hc, hc_params) << '\n';
}

// void hc_example() {
//  using std::string;
//  using std::cout;
//  using std::endl;
//
//  rng.reseed(1234);
//
//  /*** CONFIGURATION ***/
//  string inputFilename =
//      "/home/lucasmp/projects/meta-learning/instances/taillard/20_5/"
//      "20_5_01.txt";
//  int no_max_eval = 0;
//  FlowshopProblem prob(inputFilename, no_max_eval);
//
//  //SAMetaheuristic::Params sa_params;
//  //sa_params.init_strat = SAMetaheuristic::InitStrat::NEH;
//  //sa_params.neigh_strat = SAMetaheuristic::NeighborStrat::RANDOM;
//  //sa_params.comp_strat = SAMetaheuristic::CompStrat::STRICT;
//  //sa_params.algo = SAMetaheuristic::Algorithm::SIMPLE;
//  //sa_params.nh_size = 20000;
//  //sa_params.init_temp = 100;
//  //sa_params.final_temp = 0.01;
//  //sa_params.alpha = 0.9;
//  //sa_params.span_simple = 100;
//  //sa_params.span_tries_max = 5000;
//  //sa_params.span_move_max = 100;
//  //sa_params.span_nb_max = 5;
//  //sa_params.max_iter_no_improve = 10;
//  //
//  //SAMetaheuristic sa(prob.permut.getN(), prob.fullEval);
//  //
//  //solve(sa, sa_params, prob);
//  //
//  //HCMetaheuristic hc(prob.permut.getN(), prob.fullEval);
//  //HCMetaheuristic::Params hc_params;
//  //solve(hc, hc_params, prob);
//  //
//  //TSMetaheuristic ts(prob.permut.getN(), prob.fullEval);
//  //TSMetaheuristic::Params ts_params;
//  //cout << solve(ts, ts_params, prob) << endl;
//
//  ILSMetaheuristic ils(prob.permut.getN(), prob.fullEval);
//  ILSMetaheuristic::Params ils_params;
//  cout << solve(ils, ils_params, prob) << endl;
//}
//
// int main(int argc, char *argv[]) {
//  hc_example();
//  return 0;
//
//  using std::string;
//  // Neighborhood =INsertion
//  typedef moShiftNeighbor<FSP> Ngh;
//
//  eoParser parser(argc, argv);
//
//  string instances_folder = "/home/lucasmp/projects/meta-learning/instances";
//  string inputFilename =
//      parser
//          .createParam(instances_folder + "/taillard/20_5/20_5_01.txt",
//                       "input_file", "Path of the input file", 'I', "Input")
//          .value();
//
//  std::random_device rd;
//  std::uniform_int_distribution<uint32_t> dist;
//  uint32_t SEED =
//      parser
//          .createParam(dist(rd), "seed", "Seed choice for the beginning", 'S',
//                       "Param")
//          .value();
//
//  int max_eval =
//      parser.createParam(0, "max_eval", "Maximum eval nb allowed", 'E',
//      "Param")
//          .value();
//
//  int maxNoImp =
//      parser
//          .createParam(10, "max_eval_wo_improvement",
//                       "Maximum evaluations without improvement", 'e',
//                       "Param")
//          .value();
//
//  string objective_str =
//      parser
//          .createParam(string("MAKESPAN"), "objective",
//                       "objective criteria [MAKESPAN,FLOWTIME]", 'O', "Param")
//          .value();
//  unsigned int objective;
//  if (objective_str == "MAKESPAN")
//    objective = 0;
//  else if (objective_str == "FLOWTIME")
//    objective = 1;
//  else
//    assert(false);
//
//  unsigned int nhSize = parser
//                            .createParam(361u, "neighborhood_size",
//                                         "Neighborhood size", 'n', "Param")
//                            .value();
//
//  string problem =
//      parser
//          .createParam("PERMUT"s, "problem",
//                       "Problem type [PERMUT,NO_WAIT,NO_IDLE]", 'P', "Param")
//          .value();
//
//  // switch strategies !
//  string init_strat =
//      parser
//          .createParam("RANDOM"s, "init_strat",
//                       "Initialization strategy [RANDOM,NEH,NEH_RANDOM]", 'I',
//                       "Param")
//          .value();
//
//  string neighbor_strat =
//      parser
//          .createParam("ORDER"s, "neighbor_strat",
//                       "Neighborhood strategy [ORDER,RANDOM]", 'N', "Param")
//          .value();
//
//  string alg_name =
//      parser
//          .createParam(
//              "FI_HC"s, "alg_name",
//              "Algorithm name [FI_HC,BEST_HC,RANDOM_BEST_HC,IG,IG_REPL]", 'P',
//              "Param")
//          .value();
//
//  string compare_strat =
//      parser
//          .createParam("STRICT"s, "compare_strat",
//                       "Comparison strategy [STRICT,EQUAL]", 'C', "Param")
//          .value();
//
//  string switch_strat =
//      parser
//          .createParam("ALWAYS"s, "switch_strat",
//                       "Switch strategy [ALWAYS,BETTER,TEMPERATURE]", 'S',
//                       "Param")
//          .value();
//
//  double switch_temperature =
//      parser
//          .createParam(0.4, "switch_temperature",
//                       "Rate for temperature for IG accept criterion if "
//                       "switch_strat==TEMPERATURE",
//                       'P', "Param")
//          .value();
//
//  string perturb_strat =
//      parser
//          .createParam("RESTART"s, "perturb_strat",
//                       "Perturbation strategy
//                       [RESTART,DESTRUCTION,KICK,NILS]",
//                       'P', "Param")
//          .value();
//
//  string perturb_strat_init =
//      parser
//          .createParam("RANDOM"s, "perturb_strat_init",
//                       "Initialization strategy for restart perturbation "
//                       "[RANDOM,NEH,NEH_RANDOM] if perturb_strat=RESTART",
//                       'P', "Param")
//          .value();
//  unsigned int perturb_restart_threshold =
//      parser
//          .createParam(1u, "perturb_restart_threshold",
//                       "perturb_restart_threshold for restart perturb if "
//                       "perturb_strat==RESTART",
//                       'Z', "Param")
//          .value();
//  unsigned int perturb_desconstruction_size =
//      parser
//          .createParam(
//              3u, "perturb_desconstruction_size",
//              "nbDeconst for IG perturb (if "
//              "perturb_strat==DESTRUCTION)  or (if "
//              "perturb_strat==NILS and perturb_nils_escape==DESTRUCTION)",
//              'Z', "Param")
//          .value();
//  unsigned int perturb_kick_size =
//      parser
//          .createParam(
//              3u, "perturb_kick_size",
//              "nbKick for kick perturb (if perturb_strat==KICK) or (if "
//              "perturb_strat==NILS and perturb_nils_escape==KICK)",
//              'Z', "Param")
//          .value();
//  string perturb_nils_escape =
//      parser
//          .createParam("RANDOM"s, "perturb_nils_escape",
//                       "NILS escape for the method to escape when no portal "
//                       "found [RANDOM,NEH,NEH_RANDOM,DESTRUCTION,KICK] if "
//                       "perturb_strat==NILS",
//                       'P', "Param")
//          .value();
//  unsigned int perturb_nils_mns =
//      parser
//          .createParam(
//              1000u, "perturb_nils_mns",
//              "Maximum number of solutions visited on plateaus for NILS "
//              "perturb if perturb_strat==NILS",
//              'P', "Param")
//          .value();
//
//  string str_status = parser.ProgramName() + ".status";
//  eoValueParam<string> statusParam(str_status.c_str(), "status", "Status
//  file",
//                                   'S');
//  parser.processParam(statusParam, "Persistence");
//  if (parser.userNeedsHelp()) {
//    parser.printHelp(cout);
//    exit(1);
//  }
//
//  if (statusParam.value() != "") {
//    ofstream os(statusParam.value().c_str());
//    os << parser;
//  }
//
//  rng.reseed(SEED);
//
//  // problem definition
//  fspEval<FSP> permut(inputFilename, objective);
//  nwfspEval<FSP> nowait(inputFilename, objective);
//  nifspEval<FSP> noidle(inputFilename, objective);
//
//  fspEval<FSP> *fullEvalTemp;
//  if (problem == "PERMUT")
//    fullEvalTemp = &permut;
//  else if (problem == "NO_WAIT")
//    fullEvalTemp = &nowait;
//  else if (problem == "NO_IDLE")
//    fullEvalTemp = &noidle;
//  else
//    assert(false);
//
//  if (max_eval == 0) max_eval = permut.getN() * 2000;
//  if (maxNoImp == 0) maxNoImp = permut.getN() * 100000;
//
//  std::cout << "MaxEval: " << max_eval << std::endl;
//
//  fspEvalCounter<FSP> fullEval(*fullEvalTemp);
//  fspEvalContinuator<Ngh> continuator(fullEval, max_eval, false);
//
//  //	moTimeContinuator<Ngh> continuator(maxTime, false);
//
//  // checkpoint pour best !!
//  moCheckpoint<Ngh> checkpoint(continuator);
//
//  // moBestFitnessStat<FSP> bestFound(true);
//  moBestSoFarStat<FSP> bestFound(false);
//  checkpoint.add(bestFound);
//
//  myBestNoImproveContinuator<Ngh> bestNoImprove(maxNoImp, false);
//  checkpoint.add(bestNoImprove);
//
//  // evaluation of neighbor
//  moFullEvalByCopy<Ngh> evalN(fullEval);
//
//  // neighborhood definition
//
//  const unsigned int nhSizeMax = pow((permut.getN() - 1), 2);
//  if (nhSize > nhSizeMax) nhSize = nhSizeMax;
//
//  moOrderNeighborhood<Ngh> neighborhood0(nhSize);
//  moRndWithoutReplNeighborhood<Ngh> neighborhood1(nhSize);
//  moRndWithReplNeighborhood<Ngh> neighborhood2(
//      nhSize);  // disable : need to modify paradiseo code !!!
//
//  moNeighborhood<Ngh> *neighborhood;
//  if (neighbor_strat == "ORDER")
//    neighborhood = &neighborhood0;
//  else if (neighbor_strat == "RANDOM")
//    neighborhood = &neighborhood1;
//  else
//    assert(false);
//
//  // init solution
//  eoInitPermutation<FSP> init0(permut.getN());  // random permut
//  initNEH<FSP> init1(fullEval);                 // NEH heuristics
//  initNEH<FSP> init2(fullEval, true);           // NEH heuristics randomized
//
//  eoInit<FSP> *init;
//  if (init_strat == "RANDOM")
//    init = &init0;
//  else if (init_strat == "NEH")
//    init = &init1;
//  else if (init_strat == "NEH_RANDOM")
//    init = &init2;
//  else
//    assert(false);
//
//  // comparator
//  moSolComparator<FSP> compSS0;               // comp sol/sol strict
//  moSolNeighborComparator<Ngh> compSN0;       // comp sol/Ngh strict
//  moNeighborComparator<Ngh> compNN0;          // comp Ngh/Ngh strict
//  moEqualSolComparator<FSP> compSS1;          // comp sol/sol with equal
//  moEqualSolNeighborComparator<Ngh> compSN1;  // comp sol/Ngh with equal
//  moEqualNeighborComparator<Ngh> compNN1;     // comp Ngh/Ngh with equal
//
//  moSolComparator<FSP> *compSS;
//  moSolNeighborComparator<Ngh> *compSN;
//  moNeighborComparator<Ngh> *compNN;
//  if (compare_strat == "STRICT") {
//    compSS = &compSS0;
//    compSN = &compSN0;
//    compNN = &compNN0;
//  } else if (compare_strat == "EQUAL") {
//    compSS = &compSS1;
//    compSN = &compSN1;
//    compNN = &compNN1;
//  }
//
//  // algos xxHC
//  moFirstImprHC<Ngh> algo0(*neighborhood, fullEval, evalN, checkpoint,
//  *compNN,
//                           *compSN);  // FIHC
//  moSimpleHC<Ngh> algo1(*neighborhood, fullEval, evalN, checkpoint, *compNN,
//                        *compSN);  // BestHC
//  moRandomBestHC<Ngh> algo2(*neighborhood, fullEval, evalN, checkpoint,
//  *compNN,
//                            *compSN);  // rndBestHC
//
//  //	moTimeContinuator<Ngh> continuatorGlobal(maxTime, false);
//  //	moTrueContinuator<Ngh> continuatorGlobal;
//  // fspEvalContinuator<Ngh> continuatorGlobal(fullEval, max_eval, true);
//
//  // checkpoint pour best !!
//  // moCheckpoint<Ngh> checkpointGlobal(continuatorGlobal);
//  moCheckpoint<Ngh> checkpointGlobal(continuator);
//  checkpointGlobal.add(bestFound);
//
//  // IG (Ruiz+Stuetzle)
//  IGexplorer<Ngh> igexplorer(
//      fullEval,
//      *compSS);  // iterative greedy improvement without replacement (IG)
//  moLocalSearch<Ngh> algo3(igexplorer, checkpoint, fullEval);
//  IGexplorerWithRepl<Ngh> igWithReplexplorer(
//      fullEval, *compSS);  // iterative greedy improvement with replacement
//  moLocalSearch<Ngh> algo4(igWithReplexplorer, checkpoint, fullEval);
//
//  moLocalSearch<Ngh> *algo;
//  if (alg_name == "FI_HC")
//    algo = &algo0;
//  else if (alg_name == "BEST_HC")
//    algo = &algo1;
//  else if (alg_name == "RANDOM_BEST_HC")
//    algo = &algo2;
//  else if (alg_name == "IG")
//    algo = &algo3;
//  else if (alg_name == "IG_REPL")
//    algo = &algo4;
//
//  //
//  // Accept in ILS
//  //
//
//  moAlwaysAcceptCrit<Ngh> accept0;
//  moBetterAcceptCrit<Ngh> accept1(
//      compSS0);  // no interest to accept equal solution here !
//  // IG accept criterion based on temperature
//  double Temperature = permut.getMaxCT();
//  Temperature =
//      switch_temperature * Temperature / (permut.getN() * permut.getM() * 10);
//  acceptCritTemperature<Ngh> accept2(Temperature);
//
//  moAcceptanceCriterion<Ngh> *accept;
//  if (switch_strat == "ALWAYS")
//    accept = &accept0;
//  else if (switch_strat == "BETTER")
//    accept = &accept1;
//  else if (switch_strat == "TEMPERATURE")
//    accept = &accept2;
//  else
//    assert(false);
//
//  //
//  // Perturb in ILS
//  //
//
//  eoInit<FSP> *perturbInit;
//  if (perturb_strat_init == "RANDOM")
//    perturbInit = &init0;
//  else if (perturb_strat_init == "NEH")
//    perturbInit = &init1;
//  else if (perturb_strat_init == "NEH_RANDOM")
//    perturbInit = &init2;
//  else
//    assert(false);
//
//  moRestartPerturb<Ngh> perturb0(*perturbInit, fullEval,
//                                 perturb_restart_threshold);
//
//  // IG perturbation
//  OpPerturbDestConst<FSP> OpPerturb(fullEval, perturb_desconstruction_size);
//  moMonOpPerturb<Ngh> perturb1(OpPerturb, fullEval);
//
//  // Kick perturbation with Exhange
//  eoSwapMutation<FSP> kickPerturb(perturb_kick_size);
//  moMonOpPerturb<Ngh> perturb2(kickPerturb, fullEval);
//
//  // init -> monOp in order to use it in nilsPerturb framework
//  eoInitAdaptor<FSP> initPerturb0(init0);  // init perturb random
//  eoInitAdaptor<FSP> initPerturb1(init1);  // init perturb NEH
//  eoInitAdaptor<FSP> initPerturb2(init2);  // init perturb NEHrnd
//  eoMonOp<FSP> *nilsEscape;
//  if (perturb_nils_escape == "RANDOM")
//    nilsEscape = &initPerturb0;
//  else if (perturb_nils_escape == "NEH")
//    nilsEscape = &initPerturb1;
//  else if (perturb_nils_escape == "NEH_RANDOM")
//    nilsEscape = &initPerturb2;
//  else if (perturb_nils_escape == "DESTRUCTION")  // need perturbNbDeconst
//    nilsEscape = &OpPerturb;
//  else if (perturb_nils_escape == "KICK")  // need perturb_kick_size
//    nilsEscape = &kickPerturb;
//
//  // NILS perturbation : nilsMNS
//  randomNeutralWalkExplorer<Ngh> nilsPerturb(
//      *neighborhood, evalN, fullEval, compSN0, perturb_nils_mns, *nilsEscape);
//  moLocalSearch<Ngh> nilsPerturbLS(nilsPerturb, checkpoint, fullEval);
//  moMonOpPerturb<Ngh> perturb3(nilsPerturbLS, fullEval);
//
//  moPerturbation<Ngh> *perturb;
//  if (perturb_strat == "RESTART")
//    perturb = &perturb0;
//  else if (perturb_strat == "DESTRUCTION")
//    perturb = &perturb1;
//  else if (perturb_strat == "KICK")
//    perturb = &perturb2;
//  else if (perturb_strat == "NILS")
//    perturb = &perturb3;
//  else
//    assert(false);
//
//  moILS<Ngh, Ngh> ils(*algo, fullEval, checkpointGlobal, *perturb, *accept);
//
//  FSP sol;
//  std::cout << "continuator (before init):   " << continuator.value()
//            << std::endl;
//  (*init)(sol);
//  std::cout << "continuator (after init):   " << continuator.value()
//            << std::endl;
//  fullEval(sol);
//  std::cout << "continuator (after eval):   " << continuator.value()
//            << std::endl;
//  std::cout << "initial: " << sol << std::endl;
//
//  ils(sol);
//  std::cout << "continuator (after run):   " << continuator.value()
//            << std::endl;
//  std::cout << "final:   " << bestFound.value() << std::endl;
//  // std::cout << "nb global final:   " << continuatorGlobal.value() <<
//  // std::endl;
//  // std::cout << "nb eval final:   " << continuator.value() << std::endl;
//  // std::cout << "final: " << sol << std::endl ;
//  //	return bestFound.value();
//  std::cout << "total_evals: " << fspEval<FSP>::global_eval << std::endl;
//  return 0;
//}
//
//// NILS (Marmion et al.): ./ILS_IN --neighbor_strat=1 --init_strat=0
//// --alg_name=0
//// --compare_strat=1 --switch_strat=0 --perturb_kick_size=3
//// --perturb_nils_escape=4
//// --perturb_strat=3
//// IG (Ruiz+Stuetzle): ./ILS_IN --neighbor_strat=1 --init_strat=1 --alg_name=3
//// --compare_strat=0 --switch_strat=1 --switch_temperature=0.4
//// --perturb_deconstruction_size=3
//// --perturb_strat=2
