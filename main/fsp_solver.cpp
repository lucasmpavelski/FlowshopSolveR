#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <paradiseo/eo/eo>

#include "flowshop-solver/fla_methods.hpp"
#include "flowshop-solver/fspproblemfactory.hpp"
#include "flowshop-solver/heuristics.hpp"
#include "flowshop-solver/heuristics/all.hpp"
#include "flowshop-solver/specsdata.hpp"

std::vector<std::string> split(const std::string &val) {
  std::vector<std::string> res;
  std::stringstream ss(val);
  std::string token;
  while (std::getline(ss, token, ',')) {
    res.push_back(token);
  }
  return res;
}

std::vector<double> splitDouble(const std::string &val) {
  std::vector<double> res;
  std::stringstream ss(val);
  std::string token;
  while (std::getline(ss, token, ',')) {
    try {
      res.push_back(std::stod(token));
    } catch (std::exception e) {
      std::cerr << "Erro converting " << token << " to double.";
      std::exit(1);
    }
  }
  return res;
}

int main(int argc, char *argv[]) {
  eoParser parser(argc, argv);

  std::string data_folder;
  std::string mh;
  std::vector<std::string> problem_names;
  std::vector<std::string> problem_values;
  long seed = 123;
  std::vector<std::string> params_names;
  std::vector<double> params_values;

  data_folder =
      parser.createParam(data_folder, "data_folder", "specs and instances path")
          .value();
  mh = parser.createParam(mh, "mh", "metaheuristic").value();
  seed = parser.createParam(seed, "seed", "rng seed").value();
  problem_names =
      split(parser.createParam(data_folder, "problem_names", "problem names")
                .value());
  problem_values =
      split(parser.createParam(data_folder, "problem_values", "problem values")
                .value());
  params_names =
      split(parser.createParam(data_folder, "params_names", "parameters names")
                .value());
  params_values = splitDouble(
      parser.createParam(data_folder, "params_values", "parameters values")
          .value());

  MHParamsSpecsFactory::init(data_folder + "/specs", true);
  FSPProblemFactory::init(data_folder);
  RNG::seed(seed);

  std::unordered_map<std::string, double> params;
  for (unsigned i = 0; i < params_names.size(); i++) {
    params[params_names[i]] = params_values[i];
  }
  std::unordered_map<std::string, std::string> problem;
  for (unsigned i = 0; i < problem_names.size(); i++) {
    problem[problem_names[i]] = problem_values[i];
  }

  bool printConfig =
      parser
          .createParam(false, "--print-config", "print configuration and exit")
          .value();
  if (printConfig) {
    std::cout << "{" << '\n' << "seed: " << seed << '\n' << "problem: {\n";
    for (auto kv : problem)
      std::cout << kv.first << ": " << kv.second << '\n';
    std::cout << "mh: " << mh << '\n';
    for (auto kv : params)
      std::cout << kv.first << ": " << kv.second << '\n';
  }
  solveWith(mh, problem, params);
  return 0;
}