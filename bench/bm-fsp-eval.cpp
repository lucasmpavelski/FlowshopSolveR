#include <string>
#include <vector>

#include "benchmark/benchmark_api.h"
#include <paradiseo/eo/eo>

#include "flowshop-solver/FSP.h"
#include "flowshop-solver/FSPEvalFunc.h"
#include "flowshop-solver/fspEval.h"

static void BM_FSPEval(benchmark::State &state) {
  using std::string;
  string instances_folder = "/home/lucasmp/projects/meta-learning/instances",
         dist = "unif", m = "m50", no_tasks = "500", no_machines = "20",
         instance = "01";
  string input_filename = instances_folder + "/" + dist + "." + m + "/" +
                          no_tasks + "_" + no_machines + "/" + no_tasks + "_" +
                          no_machines + "_" + instance + ".txt";

  FSP sol;
  eoInitPermutation<FSP> init_random(200);
  init_random(sol);
  fspEval<FSP> fsp_eval{input_filename, 0};
  while (state.KeepRunning()) {
    fsp_eval(sol);
  }
}
BENCHMARK(BM_FSPEval);

static void BM_FSPEval2(benchmark::State &state) {
  using std::string;

  FSP sol;
  eoInitPermutation<FSP> init_random(200);
  init_random(sol);
  PermFSPEvalFunc<FSP> fsp_eval{FSPData{500, 20}};
  while (state.KeepRunning()) {
    fsp_eval(sol);
  }
}
BENCHMARK(BM_FSPEval2);

// static void BM_FSPEval_Eigen(benchmark::State &state) {
//  using std::string;
//  FSP sol;
//  eoInitPermutation<FSP> init_random(200);
//  init_random(sol);
//  PermFSPEvalFuncEigen<FSP> fsp_eval{FSPData{500, 20}};
//  while (state.KeepRunning()) {
//    fsp_eval(sol);
//  }
//}
// BENCHMARK(BM_FSPEval_Eigen);

BENCHMARK_MAIN();
