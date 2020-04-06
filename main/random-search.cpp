#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>

// paradiseo libs
#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "problems/FSPEvalFunc.hpp"
#include "problems/FSPProblem.hpp"
#include "problems/NIFSPEvalFunc.hpp"
#include "problems/NWFSPEvalFunc.hpp"

#include "heuristics/FSPOrderHeuristics.hpp"
#include "heuristics/NEHInit.hpp"
#include "heuristics/dummyAspiration.hpp"
#include "heuristics/moFirstBestTS.hpp"
#include "heuristics/moFirstTS.hpp"

#include "MHParamsSpecs.hpp"
#include "MHParamsValues.hpp"
#include "MHSolve.hpp"

int main(int argc, char *argv[]) {
  using std::cout;
  using std::string;

  cout << "SEED: " << RNG::seed() << "\n";

  eoParser parser(argc, argv);
  string s = " --instance=../../data/instances/generated_intances/"
             "generated_instances_taill-like/"
             "taill-like_20_10_2010109.gen"
             " --obj=PERM "
             " --specs_file=../../data/specs/ils_specs.txt"
             " --budget=low"
             " --no_samples=5"
             " --log_file=test.log"
             " --result_file=test.out";
  // std::stringstream test_input(s);
  // parser.readFrom(test_input);
  string instance = "";
  string obj = "";
  string specs_file = "";
  string budget = "";
  int no_samples = 5;
  string log_file = parser.ProgramName() + ".log";
  string result_file = parser.ProgramName() + ".txt";
  string str_status = parser.ProgramName() + ".status";
  instance =
      parser.createParam(instance, "instance", "Instance path", 'I').value();
  obj = parser.createParam(obj, "obj", "Objective", 'O').value();
  specs_file =
      parser.createParam(specs_file, "specs_file", "Algorithm specs", 'A')
          .value();
  no_samples = parser
                   .createParam(no_samples, "no_samples",
                                "Number of samples (per eval)", 'S', "", true)
                   .value();
  log_file = parser
                 .createParam(log_file, "log_file",
                              "File path to log all evaluations", 'S', "", true)
                 .value();
  result_file =
      parser
          .createParam(result_file, "result_file",
                       "Write the best configuration found", 'S', "", true)
          .value();
  budget = parser
               .createParam(budget, "budget", "Problem budget (low,med,high)",
                            'B', "", true)
               .value();

  eoValueParam<string> statusParam(str_status.c_str(), "status", "Status file",
                                   'S');
  parser.processParam(statusParam, "Persistence");
  if (parser.userNeedsHelp()) {
    parser.printHelp(cout);
    exit(0);
  }
  if (statusParam.value() != "") {
    std::ofstream os(statusParam.value().c_str());
    os << parser;
  }

  FSPData data(instance);
  FSPProblem prob(data, obj, "MAKESPAN", "budget", "EVALS");

  std::ifstream specs(specs_file);
  MHParamsSpecs mh_specs;
  specs >> mh_specs;
  specs.close();
  cout << "specs:\n" << mh_specs << "\n";

  std::ofstream log(log_file);

  MHParamsValues params(&mh_specs), best(&mh_specs);
  double result, best_result = std::numeric_limits<double>::infinity();

  int no_evals = 0;
  if (data.noJobs() == 20)
    no_evals = 5000;
  else if (data.noJobs() == 30)
    no_evals = 12500;
  else
    assert(false);
  cout << "no_evals: " << no_evals << '\n';

  auto start = std::chrono::system_clock::now();
  for (int i = 0; i < no_evals / no_samples; i++) {
    params.randomizeValues(RNG::engine);
    result = evaluateMean(params, prob, totalProcTimes(data), no_samples);
    params.printValues(log) << result << '\n';
    if (result < best_result) {
      best_result = result;
      best = params;
    }
  }
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<float> elapsed_seconds = end - start;

  std::ofstream result_out(result_file);
  best.printValues(result_out) << best_result << '\n';

  cout << "elapsed_seconds: " << elapsed_seconds.count() << '\n';

  return 0;
}
