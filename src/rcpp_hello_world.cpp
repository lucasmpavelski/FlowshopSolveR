#include <Rcpp.h>
using namespace Rcpp;

#include "flowshop-solver/specsdata.hpp"
#include "flowshop-solver/fspproblemfactory.hpp"
// #include "flowshop-solver/fla_methods.hpp"
// #include "flowshop-solver/heuristics/all.hpp"
// #include "flowshop-solver/heuristics.hpp"

// This is a simple function using Rcpp that creates an R list
// containing a character vector and a numeric vector.
//
// Learn more about how to use Rcpp at:
//
//   http://www.rcpp.org/
//   http://adv-r.had.co.nz/Rcpp.html
//
// and browse examples of code using Rcpp at:
//
//   http://gallery.rcpp.org/
//

// [[Rcpp::export]]
void initFactories(std::string data_folder)
{
  MHParamsSpecsFactory::init(data_folder + "/specs", true);
  FSPProblemFactory::init(data_folder);
}

// // [[Rcpp::export]]
// List solveFSP(std::string mh, Rcpp::CharacterVector rproblem, long seed,
//               Rcpp::NumericVector rparams, bool verbose = false)
// {
//   Result result;
//   try
//   {
//     RNG::seed(seed);
//     auto params_names = Rcpp::as<std::vector<std::string>>(rparams.names());
//     auto params_values = Rcpp::as<std::vector<double>>(rparams);
//     std::unordered_map<std::string, double> params;
//     for (int i = 0; i < params_names.size(); i++)
//     {
//       params[params_names[i]] = params_values[i];
//       //  Rcpp::Rcout << params_names[i] << ": " << params_values[i] << '\n';
//     }
//     if (verbose) {
//       for (int i = 0; i < params_names.size(); i++)
//         Rcpp::Rcerr << params_names[i] << ": " << params_values[i] << '\n';
//     }
//     auto problem_names = Rcpp::as<std::vector<std::string>>(rproblem.names());
//     auto problem_values = Rcpp::as<std::vector<std::string>>(rproblem);
//     std::unordered_map<std::string, std::string> problem;
//     for (int i = 0; i < problem_names.size(); i++)
//     {
//       problem[problem_names[i]] = problem_values[i];
//       //  Rcpp::Rcout << problem_names[i] << ": " << problem_values[i] << '\n';
//     }
//     if (verbose) {
//       for (int i = 0; i < problem_names.size(); i++)
//         Rcpp::Rcerr << problem_names[i] << ": " << problem_values[i] << '\n';
//     }
//     result = solveWith(mh, problem, params);
//   }
//   catch (std::exception &ex)
//   {
//     throw Rcpp::exception(ex.what());
//   }
// 
//   // Rcpp::DataFrame()
//   //double result = solveWithFactories(mh, problem, seed, params);
//   // std::cerr << "no_solves: " << ++no_solves << '\n';
//   using namespace Rcpp;
//   return List::create(
//       Named("fitness") = result.fitness,
//       Named("time") = result.time,
//       Named("no_evals") = result.no_evals);
// }
// 
// std::unordered_map<std::string, std::string>
// rcharVec2map(Rcpp::CharacterVector charvec)
// {
//   auto names = Rcpp::as<std::vector<std::string>>(charvec.names());
//   auto values = Rcpp::as<std::vector<std::string>>(charvec);
//   std::unordered_map<std::string, std::string> ret;
//   for (int i = 0; i < names.size(); i++)
//   {
//     ret[names[i]] = values[i];
//   }
//   return ret;
// }
// 
// // [[Rcpp::export]]
// std::vector<double> randomWalk(Rcpp::CharacterVector rproblem,
//                                Rcpp::CharacterVector rsampling,
//                                long seed)
// {
//   auto prob_data = rcharVec2map(rproblem);
//   auto samp_data = rcharVec2map(rsampling);
//   return randomWalk(prob_data, samp_data, seed);
// }
// 
// // [[Rcpp::export]]
// List adaptiveWalk(Rcpp::CharacterVector rproblem,
//                  Rcpp::CharacterVector rsampling,
//                  long seed)
// {
//   auto prob_data = rcharVec2map(rproblem);
//   auto samp_data = rcharVec2map(rsampling);
//   auto res = adaptiveWalk(prob_data, samp_data, seed);
//   std::vector<double> fitness(res.size(), 0.0d);
//   std::transform(res.begin(), res.end(), fitness.begin(), [](FSPProblem::EOT sol){
//     return sol.fitness();
//   });
//   using namespace Rcpp;
//   return List::create(
//     Named("fitness") = fitness,
//     Named("solutions") = res
//   );
// }
// 
// // [[Rcpp::export]]
// double walkSamplingAutocorrFLA(Rcpp::CharacterVector rproblem,
//                                Rcpp::CharacterVector rsampling,
//                                long seed)
// {
//   auto prob_data = rcharVec2map(rproblem);
//   auto samp_data = rcharVec2map(rsampling);
//   return walkSamplingAutocorr(prob_data, samp_data, seed);
// }
// 
// // [[Rcpp::export]]
// int adaptiveWalkLengthFLA(Rcpp::CharacterVector rproblem,
//                           Rcpp::CharacterVector rsampling,
//                           long seed)
// {
//   auto prob_data = rcharVec2map(rproblem);
//   auto samp_data = rcharVec2map(rsampling);
//   return adaptiveWalkLength(prob_data, samp_data, seed);
// }
// 
// // [[Rcpp::export]]
// List solutionStatisticsFLA(Rcpp::CharacterVector rproblem,
//                            Rcpp::CharacterVector rsampling,
//                            long seed)
// {
//   auto prob_data = rcharVec2map(rproblem);
//   auto samp_data = rcharVec2map(rsampling);
//   auto res = solutionStatistics(prob_data, samp_data, seed);
//   using namespace Rcpp;
//   return List::create(
//       Named("up") = res.up,
//       Named("down") = res.down,
//       Named("side") = res.side,
//       Named("slmin") = res.slmin,
//       Named("lmin") = res.lmin,
//       Named("iplat") = res.iplat,
//       Named("ledge") = res.ledge,
//       Named("slope") = res.slope,
//       Named("lmax") = res.lmax,
//       Named("slmax") = res.slmax
//   );
// }
// 
// // [[Rcpp::export]]
// std::vector<double> enumerateAllFitness(Rcpp::CharacterVector rproblem)
// {
//   auto prob_data = rcharVec2map(rproblem);
//   return enumerateAll(prob_data);
// }
// 
// template<class EOT>
// std::vector<int> solToVec(const EOT& sol) {
//   std::vector<int> vec(sol.size());
//   std::copy(sol.begin(), sol.end(), vec.begin());
//   return vec;
// }
// 
// // [[Rcpp::export]]
// List enumerateSolutions(Rcpp::List fspInstance, Rcpp::CharacterVector fspProblem) {
//   std::vector<int> pts =  Rcpp::as<std::vector<int>>(fspInstance["pt"]);
//   int no_jobs = fspInstance["no_jobs"];
//   FSPData dt(pts, no_jobs);
//   auto prob_data = rcharVec2map(fspProblem);
//   FSPProblem problem(
//       dt,
//       prob_data["fsp_type"],
//       prob_data["objective"],
//       prob_data["budget"],
//       prob_data["stopping_criterium"]
//   );
//   auto solutions = enumerateAllSolutions(problem);
//   std::vector<std::vector<int>> sample_solutions;
//   sample_solutions.reserve(solutions.size());
//   IntegerVector sample_fitness;
//   sample_fitness.reserve(fitness.size());
//   for (const auto& sol : solutions) {
//     sample_solutions.push_back(solToVec(sol));
//     sample_fitness.push_back(sol.fitness());
//   }
//   return List::create(
//     Named("solutions") = sample_solutions,
//     Named("fitness") = sample_fitness
//   );
// }
// 
// // [[Rcpp::export]]
// List sampleLON(Rcpp::CharacterVector rproblem, Rcpp::CharacterVector rsampling, long seed) {
//   graph<FSPProblem::EOT> ret = sampleLON(
//     rcharVec2map(rproblem),
//     rcharVec2map(rsampling),
//     seed
//   );
//   using namespace Rcpp;
//   std::vector<double> fitness(ret.nodes.size());
//   std::transform(ret.nodes.begin(), ret.nodes.end(), fitness.begin(), [](FSPProblem::EOT sol) {
//     return sol.fitness();
//   });
//   std::vector<int> from_edges;
//   std::vector<int> to_edges;
//   std::vector<int> weight;
//   for (int from = 0; from < ret.edges.size(); from++) {
//     for (const auto& to : ret.edges[from]) {
//       from_edges.push_back(from + 1);
//       to_edges.push_back(to.node_idx + 1);
//       weight.push_back(to.weight);
//     }
//   }
//   std::vector<std::vector<int>> nodes(ret.nodes.size());
//   int i = 0;
//   for (auto& nd : ret.nodes) {
//     nodes[i].resize(nd.size());
//     std::copy(nd.begin(), nd.end(), nodes[i].begin());
//     i++;
//   }
//   std::vector<std::vector<int>> sample_solutions;
//   IntegerVector local_optima_idx;
//   IntegerVector sample_no_steps;
//   IntegerVector sample_fitness;
//   for (int i = 0; i < ret.samples.size(); i++) {
//     for (const auto& sample : ret.samples[i]) {
//       local_optima_idx.push_back(i + 1);
//       sample_solutions.push_back(solToVec(sample.sol));
//       sample_no_steps.push_back(sample.no_steps);
//       sample_fitness.push_back(sample.sol.fitness());
//     }
//   }
//   return List::create(
//     Named("nodes") = List::create(
//       Named("solutions") = nodes,
//       Named("fitness") = fitness
//     ),
//     Named("edges") = DataFrame::create(
//       Named("from") = from_edges,
//       Named("to") = to_edges,
//       Named("weight") = weight
//     ),
//     Named("samples") = List::create(
//       Named("solutions") = sample_solutions,
//       Named("fitness") = sample_fitness,
//       Named("local_optima_idx") = local_optima_idx,
//       Named("no_steps") = sample_no_steps
//     )
//   );
// }
