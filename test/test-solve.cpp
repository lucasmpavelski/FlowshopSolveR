#include "flowshop-solver/heuristics/ils.hpp"

/*
void testSolveWithFactories() {

  try {
  std::unordered_map<std::string, std::string> prob;
  prob["problem"] = "flowshop";
  prob["budget"] = "low";
  prob["type"] = "NOIDLE";
  prob["objective"] = "FLOWTIME";
  prob["instance"] = "taill-like_10_5_1005104.gen";
  int seed = 481571373;
  std::unordered_map<std::string, float> params;
  params["Comp.Strat"] =  0;
  params["Init.Strat"] = 0;
  params["Neighborhood.Size"] =  1.0;
  params["ILS.NeighborhoodStrat"] = 1;
  params["ILS.AcceptTemperature"] = 0.3f;
  params["ILS.PerturbDestructionSize"] = 8;
  params["ILS.LS"] = 1;
  params["ILS.LS.CompStrat"] = 1;
  params["ILS.LS.NeighborhoodStrat"] = 1;
  params["ILS.LS.NeighborhoodSize"] = 1.0;
  std::cout << (solveWithFactories("ILS2", prob, seed, params)) << " ";
  std::cout << solveWithFactories("ILS2", prob, seed, params);

  } catch (std::runtime_error e) {
    std::cout << e.what() << std::endl;
  }
}

#include "flowshop-solver/problems/PermFSPNeighborMakespanEval.hpp"

void testSolve() {
  int seed = 481571373;
  FSPData
fspData("data/instances/flowshop/binom_rand_30_20_01.dat");
  rng.reseed(seed);
  std::unordered_map<std::string, float> params;
  params["Comp.Strat"] =  0;
  params["Init.Strat"] = 0;
  params["Neighborhood.Size"] =  10.0;
  params["ILS.NeighborhoodStrat"] = 1;
  params["ILS.AcceptTemperature"] = 0.3f;
  params["ILS.PerturbDestructionSize"] = 3;
  params["ILS.LS"] = 1;
  params["ILS.LS.CompStrat"] = 1;
  params["ILS.LS.NeighborhoodStrat"] = 1;
  params["ILS.LS.NeighborhoodSize"] = 10.0;
  MHParamsSpecsFactory::init("data/specs/",
true); MHParamsSpecs mh_specs = MHParamsSpecsFactory::get("ILS2");
  MHParamsValues values(&mh_specs);
  for (auto ps : mh_specs) {
    if (params.find(ps->name) == params.end())
      throw std::runtime_error("Parameter " + ps->name + " needs a value!");
    values[ps->name] = params.at(ps->name);
  }
  FSPProblem problem(fspData, "PERM", "MAKESPAN", "high", "EVALS");
  std::vector<int> order = totalProcTimes(problem.getData());
  double result = evaluateMean(values, problem, order, 30);
  std::cout << result << std::endl;
}

void testFastSolve() {
  unsigned seed = 481571373;
  FSPData
fspData("data/instances/flowshop/binom_rand_30_20_01.dat");
  //system("cat
data/instances/generated_intances/generated_instances_all/taill-like_30_5_3005104.gen.bestKnown");
  rng.reseed(seed);
  std::unordered_map<std::string, float> params;
  params["Comp.Strat"] =  0;
  params["Init.Strat"] = 0;
  params["Neighborhood.Size"] =  1.0;
  params["ILS.NeighborhoodStrat"] = 0;
  params["ILS.AcceptTemperature"] = 0.3f;
  params["ILS.PerturbDestructionSize"] = 3;
  params["ILS.LS"] = 1;
  params["ILS.LS.CompStrat"] = 1;
  params["ILS.LS.NeighborhoodStrat"] = 0;
  params["ILS.LS.NeighborhoodSize"] = 10.0;
  MHParamsSpecsFactory::init("data/specs/",
true); MHParamsSpecs mh_specs = MHParamsSpecsFactory::get("ILS"); MHParamsValues
values(&mh_specs); for (auto ps : mh_specs) { if (params.find(ps->name) ==
params.end()) throw std::runtime_error("Parameter " + ps->name + " needs a
value!"); values[ps->name] = params.at(ps->name);
  }
  FSPProblem problem(fspData, "PERM", "MAKESPAN", "high", "EVALS");
  std::vector<int> order = totalProcTimes(problem.getData());
  double result = evaluateMean(values, problem, order, 30);
  std::cout << result << std::endl;
}
*/

void testSolve() {}

auto main(int, char* []) -> int {
  // testSolveWithFactories();
  testSolve();
  // testFastSolve();
  return 0;
}
