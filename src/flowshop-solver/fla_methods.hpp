#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "fspproblemfactory.hpp"
#include "heuristics/FSPOrderHeuristics.hpp"
#include "heuristics/IGexplorer.hpp"
#include "heuristics/InsertionStrategy.hpp"
#include "heuristics/NEHInit.hpp"
#include "heuristics/OpPerturbDestConst.hpp"
#include "heuristics/ilsKickOp.hpp"

#include "heuristics/ig.hpp"

std::vector<FSPProblem::EOT> adaptiveWalk(
    std::unordered_map<std::string, std::string> prob_params,
    std::unordered_map<std::string, std::string> sampling_params,
    unsigned seed) {
  rng.reseed(seed);
  FSPProblem problem = FSPProblemFactory::get(prob_params);
  using EOT = FSPProblem::EOT;
  using Ngh = FSPProblem::Ngh;

  // sampling params
  using namespace std::string_literals;
  auto init_strat = getWithDef(sampling_params, "Init.Strat"s, "RANDOM"s);
  auto sampling_strat = getWithDef(sampling_params, "Sampling.Strat"s, "FI"s);

  auto order = totalProcTimes(problem.getData());
  auto& eval = problem.eval();
  auto& neighborEval = problem.neighborEval();

  eoInitPermutation<EOT> init0(problem.size());
  NEHInitOrdered<EOT> init1(eval, order);
  NEHInitRandom<EOT> init2(eval, problem.size());
  eoInit<EOT>* init = nullptr;
  if (init_strat == "RANDOM")
    init = &init0;
  else if (init_strat == "NEH")
    init = &init1;
  else if (init_strat == "RANDOM_NEH")
    init = &init2;
  else
    assert(false);

  const int nh_size = std::pow(problem.size() - 1, 2);
  moRndWithoutReplNeighborhood<Ngh> neighborhood(nh_size);

  moTrueContinuator<Ngh> tc;
  moCheckpoint<Ngh> checkpoint(tc);

  moSolutionStat<EOT> solutionStat;
  moVectorMonitor<EOT> solutionMonitor(solutionStat);
  checkpoint.add(solutionStat);
  checkpoint.add(solutionMonitor);

  moFirstImprHC<Ngh> fi(neighborhood, eval, neighborEval, checkpoint);
  moSimpleHC<Ngh> hc(neighborhood, eval, neighborEval, checkpoint);

  moSolComparator<EOT> comparator;
  IGexplorer<Ngh> igExplorer(eval, problem.size(0), comparator);
  moLocalSearch<Ngh> ig(igExplorer, checkpoint, eval);

  moLocalSearch<Ngh>* localSearch = nullptr;
  if (sampling_strat == "FI")
    localSearch = &fi;
  else if (sampling_strat == "HC")
    localSearch = &hc;
  else if (sampling_strat == "IG")
    localSearch = &ig;
  else
    throw std::runtime_error("Unknown sampling strat: " + sampling_strat);

  EOT sol;
  (*init)(sol);
  solutionStat.init(sol);
  (*localSearch)(sol);

  std::cerr << "no_evals" << problem.noEvals() << "\n";

  auto res = solutionMonitor.getSolutions();
  // remove duplicated last solution
  res.pop_back();
  return res;
}

double adaptiveWalkLength(
    const std::unordered_map<std::string, std::string>& prob_params,
    const std::unordered_map<std::string, std::string>& sampling_params,
    unsigned seed) {
  rng.reseed(seed);
  FSPProblem problem = FSPProblemFactory::get(prob_params);
  using EOT = FSPProblem::EOT;
  using Ngh = FSPProblem::Ngh;

  // sampling params
  using namespace std::string_literals;
  auto init_strat = getWithDef(sampling_params, "Init.Strat"s, "RANDOM"s);
  auto sampling_strat = getWithDef(sampling_params, "Sampling.Strat"s, "FI"s);

  auto order = totalProcTimes(problem.getData());
  auto& eval = problem.eval();
  auto& neighborEval = problem.neighborEval();

  eoInitPermutation<EOT> init0(problem.size());
  NEHInitOrdered<EOT> init1(eval, order);
  NEHInitRandom<EOT> init2(eval, problem.size());
  eoInit<EOT>* init = nullptr;
  if (init_strat == "RANDOM")
    init = &init0;
  else if (init_strat == "NEH")
    init = &init1;
  else if (init_strat == "RANDOM_NEH")
    init = &init2;
  else
    assert(false);

  const int nh_size = std::pow(problem.size() - 1, 2);
  moRndWithoutReplNeighborhood<Ngh> neighborhood(nh_size);

  moSampling<Ngh>* sampling = nullptr;
  moAdaptiveWalkSampling<Ngh> adaptive(*init, neighborhood, eval, neighborEval,
                                       1);
  moHillClimberSampling<Ngh> hc(*init, neighborhood, eval, neighborEval, 1);

  if (sampling_strat == "FI_LENGTH")
    sampling = &adaptive;
  else if (sampling_strat == "HC_LENGTH")
    sampling = &hc;
  sampling->operator()();
  std::cerr << "no_evals" << problem.noEvals() << "\n";
  sampling->fileExport("teste.txt");
  return sampling->getValues(0).at(0);
}

std::vector<double> randomWalk(
    std::unordered_map<std::string, std::string> prob_params,
    std::unordered_map<std::string, std::string> sampling_params,
    unsigned seed) {
  rng.reseed(seed);
  FSPProblem problem = FSPProblemFactory::get(prob_params);
  using EOT = FSPProblem::EOT;
  using Ngh = FSPProblem::Ngh;

  // sampling params
  using namespace std::string_literals;
  auto init_strat = getWithDef(sampling_params, "Init.Strat"s, "RANDOM"s);
  auto sampling_strat =
      getWithDef(sampling_params, "Sampling.Strat"s, "RANDOM"s);
  auto no_steps = std::stoi(getWithDef(sampling_params, "No.Steps"s, "1000"s));

  auto order = totalProcTimes(problem.getData());
  auto& eval = problem.eval();
  auto& neighborEval = problem.neighborEval();

  eoInitPermutation<EOT> init0(problem.size());
  NEHInitOrdered<EOT> init1(eval, order);
  NEHInitRandom<EOT> init2(eval, problem.size());
  eoInit<EOT>* init = nullptr;
  if (init_strat == "RANDOM")
    init = &init0;
  else if (init_strat == "NEH")
    init = &init1;
  else if (init_strat == "RANDOM_NEH")
    init = &init2;
  else
    assert(false);

  const int nh_size = std::pow(problem.size() - 1, 2);
  moRndWithoutReplNeighborhood<Ngh> neighborhood(nh_size);

  moAutocorrelationSampling<Ngh> random(*init, neighborhood, eval, neighborEval,
                                        no_steps);
  moMHRndFitnessCloudSampling<Ngh> mh(*init, neighborhood, eval, neighborEval,
                                      no_steps);

  moSampling<Ngh>* sampling = nullptr;
  if (sampling_strat == "RANDOM")
    sampling = &random;
  else if (sampling_strat == "MH")
    sampling = &mh;
  else
    assert(false);
  sampling->operator()();

  return sampling->getValues(0);
}

double walkSamplingAutocorr(
    const std::unordered_map<std::string, std::string>& prob_params,
    const std::unordered_map<std::string, std::string>& sampling_params,
    unsigned seed) {
  auto fits = randomWalk(prob_params, sampling_params, seed);
  double fits_mean = std::accumulate(begin(fits), end(fits), 0.0) / fits.size();

  using namespace std::string_literals;
  auto delay = getWithDef(sampling_params, "Delay"s, "1"s);
  int k = std::stoi(delay);
  double num_sum = 0.0;
  double den_sum = 0.0;
  for (unsigned i = 0; i < fits.size() - k; i++) {
    num_sum += (fits[i] - fits_mean) * (fits[i + k] - fits_mean);
    den_sum += std::pow((fits[i] - fits_mean), 2);
  }
  for (unsigned i = fits.size() - k; i < fits.size(); i++)
    den_sum += std::pow((fits[i] - fits_mean), 2);

  return num_sum / den_sum;
}

using ivec = FSPData::ivec;

int upw(int s, const ivec& ns) {
  return std::count_if(ns.begin(), ns.end(), [s](int n) { return n > s; });
}

int sidew(int s, const ivec& ns) {
  return std::count_if(ns.begin(), ns.end(), [s](int n) { return n == s; });
}

int downw(int s, const ivec& ns) {
  return std::count_if(ns.begin(), ns.end(), [s](int n) { return n < s; });
}

struct SolutionStatisticsResult {
  int up = 0, down = 0, side = 0, slmin = 0, lmin = 0, iplat = 0, ledge = 0,
      slope = 0, lmax = 0, slmax = 0;
};

SolutionStatisticsResult solutionStatistics(
    const std::unordered_map<std::string, std::string>& prob_params,
    const std::unordered_map<std::string, std::string>& sampling_params,
    unsigned seed) {
  rng.reseed(seed);
  using ProblemTp = FSPProblem;
  ProblemTp problem = FSPProblemFactory::get(prob_params);
  auto& neighborEval = problem.neighborEval();
  auto& fullEval = problem.eval();
  using EOT = ProblemTp::EOT;
  using Ngh = ProblemTp::Ngh;

  EOT sol;
  const unsigned nh_size = (problem.size() - 1) * (problem.size() - 1);
  moOrderNeighborhood<Ngh> neighborhood(nh_size);
  std::vector<int> fitness;
  fitness.reserve(nh_size);
  Ngh neighbor;
  eoInitPermutation<EOT> init0(problem.size());

  SolutionStatisticsResult res;

  const int no_samples = std::atoi(sampling_params.at("No.Samples").c_str());

  for (int i = 0; i < no_samples; i++) {
    init0(sol);
    fullEval(sol);
    // neighbor.size = sol.size();
    neighborhood.init(sol, neighbor);
    neighborEval(sol, neighbor);
    double fit = neighbor.fitness();
    fitness.push_back(fit);
    while (neighborhood.cont(sol)) {
      neighborhood.next(sol, neighbor);
      neighborEval(sol, neighbor);
      double fit = neighbor.fitness();
      fitness.push_back(fit);
    }

    int up = upw(sol.fitness(), fitness);
    int down = downw(sol.fitness(), fitness);
    int side = sidew(sol.fitness(), fitness);

    res.up += up;
    res.down += down;
    res.side += side;
    res.slmin += (down == 0) && (side == 0);
    res.lmin += (down == 0) && (side > 0) && (up > 0);
    res.iplat += (down == 0) && (up == 0);
    res.ledge += (down > 0) && (side > 0) && (up > 0);
    res.slope += (down > 0) && (side == 0) && (up > 0);
    res.lmax += (down > 0) && (side == 0) && (up == 0);
    res.slmax += (side == 0) && (up == 0);
  }

  return res;
}

std::vector<double> enumerateAll(
    const std::unordered_map<std::string, std::string>& prob_params) {
  std::vector<double> res;

  using ProblemTp = FSPProblem;
  ProblemTp problem = FSPProblemFactory::get(prob_params);
  auto& fullEval = problem.eval();
  using EOT = ProblemTp::EOT;

  int n = problem.size();
  EOT sol(n);
  std::iota(sol.begin(), sol.end(), 0);
  long no_solutions = factorial(n);
  res.reserve(no_solutions);

  for (int i = 0; i < no_solutions; i++) {
    fullEval(sol);
    res.emplace_back(sol.fitness());
    std::next_permutation(sol.begin(), sol.end());
    sol.invalidate();
  }

  return res;
}

template <class Ngh, class EOT = typename Ngh::EOT>
std::vector<EOT> enumerateAllSolutions(Problem<Ngh>& problem) {
  const int n = problem.size();
  const long no_solutions = factorial(n);
  auto& fullEval = problem.eval();

  std::vector<EOT> solutions;
  solutions.reserve(no_solutions);

  EOT sol(n);
  std::iota(sol.begin(), sol.end(), 0);
  for (int i = 0; i < no_solutions; i++) {
    fullEval(sol);
    solutions.emplace_back(sol);
    std::next_permutation(sol.begin(), sol.end());
    sol.invalidate();
  }

  return solutions;
}

template <class EOT>
struct graph {
  struct edge {
    int node_idx, weight;

    edge(int a, int b) : node_idx{a}, weight{b} {};
  };

  struct lo_sample {
    EOT sol;
    int no_steps;
    lo_sample(EOT sol, int no_steps)
        : sol{std::move(sol)}, no_steps{no_steps} {};
  };

  std::vector<EOT> nodes;
  std::vector<std::vector<lo_sample>> samples;
  std::vector<std::vector<edge>> edges;

  int addNode(EOT n, EOT sample, int no_steps) {
    unsigned idx = getIndex(n);
    if (idx == nodes.size()) {
      nodes.emplace_back(n);
      samples.resize(samples.size() + 1);
      edges.resize(edges.size() + 1);
    }
    samples[idx].emplace_back(lo_sample{sample, no_steps});
    return idx;
  }

  int getIndex(const EOT& n) {
    auto it = std::find(nodes.begin(), nodes.end(), n);
    return std::distance(nodes.begin(), it);
  }

  bool contains(const EOT& n) { return getIndex(n) != nodes.size(); }

  edge* getEdge(const EOT& a, const EOT& b) {
    int a_idx = getIndex(a);
    int b_idx = getIndex(b);
    auto edge_it = std::find_if(
        edges.at(a_idx).begin(), edges.at(a_idx).end(),
        [b_idx](graph<EOT>::edge& edge) { return edge.node_idx == b_idx; });
    if (edge_it == edges.at(a_idx).end())
      return nullptr;
    return &*edge_it;
  }

  void addEdge(const EOT& a, const EOT& b, int weight) {
    unsigned a_idx = getIndex(a);
    unsigned b_idx = getIndex(b);
    assert(a_idx != nodes.size() && b_idx != nodes.size());
    edges.at(a_idx).push_back(graph<EOT>::edge(b_idx, weight));
  }

  void print(std::ostream& out) {
    for (unsigned i = 0; i < nodes.size(); i++) {
      out << i << ": " << nodes[i] << '\n';
    }
    for (unsigned i = 0; i < edges.size(); i++) {
      out << i << ": ";
      for (auto e : edges[i]) {
        out << " (" << e.node_idx << "," << e.weight << ")";
      }
      out << '\n';
    }
    out << '\n';
  }
};

template <class Ngh, class EOT = typename Ngh::EOT>
void snowball(int d,
              int m,
              const EOT& x,
              moLocalSearch<Ngh>& localSearch,
              moCounterStat<EOT>& counter,
              moPerturbation<Ngh>& op,
              graph<EOT>& lon) {
  if (d > 0) {
    for (int j = 0; j < m; j++) {
      EOT x_l = x;
      op(x_l);
      EOT x0 = x_l;
      localSearch(x_l);
      lon.addNode(x_l, x0, counter.value());
      auto edge = lon.getEdge(x, x_l);
      if (edge != nullptr) {
        edge->weight++;
      } else {
        lon.addEdge(x, x_l, 1);
        snowball(d - 1, m, x_l, localSearch, counter, op, lon);
      }
    }
  }
}

template <class T>
bool contains(const std::vector<T>& vec, const T& obj) {
  return std::find(vec.begin(), vec.end(), obj) != vec.end();
}

template <class Ngh, class EOT = typename Ngh::EOT>
EOT randomWalkStep(EOT& sol,
                   graph<EOT>& lon,
                   moLocalSearch<Ngh>& localSearch,
                   moCounterStat<EOT>& counter,
                   eoEvalFunc<EOT>& eval,
                   const std::vector<EOT>& walk) {
  for (auto& edge : lon.edges[lon.getIndex(sol)]) {
    if (!contains(walk, lon.nodes[edge.node_idx])) {
      return lon.nodes[edge.node_idx];
    }
  }
  eoInitPermutation<EOT> init(sol.size());
  EOT new_sol = sol;
  init(new_sol);
  eval(new_sol);
  EOT x0 = new_sol;
  localSearch(new_sol);
  lon.addNode(new_sol, x0, counter.value());
  return new_sol;
}

graph<FSPProblem::EOT> sampleLON(
    const std::unordered_map<std::string, std::string>& prob_params,
    std::unordered_map<std::string, std::string> sampling_params,
    unsigned seed) {
  rng.reseed(seed);
  FSPProblem problem = FSPProblemFactory::get(prob_params);
  using EOT = FSPProblem::EOT;
  using Ngh = FSPProblem::Ngh;

  // sampling params
  using namespace std::string_literals;
  auto init_strat = getWithDef(sampling_params, "Init.Strat"s, "RANDOM"s);
  auto sampling_strat =
      getWithDef(sampling_params, "Sampling.Strat"s, "RANDOM_BEST"s);
  auto d = stoi(getWithDef(sampling_params, "Depth"s, "2"s));
  auto m = stoi(getWithDef(sampling_params, "No.Edges"s, "15"s));
  auto l = stoi(getWithDef(sampling_params, "Walk.Lengh"s, "50"s));
  auto perturb_strat =
      getWithDef(sampling_params, "Perturbation.Operator"s, "IG"s);
  auto strength = stoi(getWithDef(sampling_params, "Strength"s, "1"s));

  // full IG params defaults
  sampling_params["IG.Init.Strat"] = "2";
  sampling_params["IG.Comp.Strat"] = "0";
  sampling_params["IG.Neighborhood.Size"] = "6.3774";
  sampling_params["IG.Neighborhood.Strat"] = "1";
  sampling_params["IG.Local.Search"] = "0";
  sampling_params["IG.Accept"] = "1";
  sampling_params["IG.Accept.Temperature"] = "1";
  sampling_params["IG.Algo"] = "1";
  sampling_params["IG.Destruction.Size"] = "0.2335";
  sampling_params["IG.LS.Single.Step"] = "0";
  sampling_params["IG.LSPS.Local.Search"] = "1";
  sampling_params["IG.LSPS.Single.Step"] = "1";

  auto order = totalProcTimes(problem.getData());
  auto& eval = problem.eval();
  auto& neighborEval = problem.neighborEval();

  eoInitPermutation<EOT> init0(problem.size());
  NEHInitOrdered<EOT> init1(eval, order);
  NEHInitRandom<EOT> init2(eval, problem.size());
  eoInit<EOT>* init = nullptr;
  if (init_strat == "RANDOM")
    init = &init0;
  else if (init_strat == "NEH")
    init = &init1;
  else if (init_strat == "RANDOM_NEH")
    init = &init2;
  else
    assert(false);

  const int nh_size = std::pow(problem.size() - 1, 2);
  moRndWithoutReplNeighborhood<Ngh> neighborhood(nh_size);

  moTrueContinuator<Ngh> tc;
  moCounterStat<EOT> counter;
  moCheckpoint<Ngh> checkpoint(tc);
  checkpoint.add(counter);

  // moSolutionStat<EOT> solutionStat;
  // moVectorMonitor<EOT> solutionMonitor(solutionStat);
  // checkpoint.add(solutionStat);
  // checkpoint.add(solutionMonitor);

  moSolNeighborComparator<Ngh> solNeighborEqComp;
  moEqualNeighborComparator<Ngh> neighborEqComp;
  moFirstImprHC<Ngh> fi(neighborhood, eval, neighborEval, checkpoint,
                        neighborEqComp, solNeighborEqComp);
  moSimpleHC<Ngh> hc(neighborhood, eval, neighborEval, checkpoint,
                     neighborEqComp, solNeighborEqComp);
  moRandomBestHC<Ngh> randomBestHC(neighborhood, eval, neighborEval, checkpoint,
                                   neighborEqComp, solNeighborEqComp);

  moSolComparator<EOT> comparator;
  IGexplorer<Ngh> igExplorer(eval, problem.size(0), comparator);
  moLocalSearch<Ngh> ig(igExplorer, checkpoint, eval);

  std::unordered_map<std::string, double> fullIgParams;
  for (const auto& k : sampling_params) {
    if (k.first.rfind("IG.", 0) == 0) {
      fullIgParams[k.first] = std::stod(k.second);
    }
  }

  MHParamsSpecs specs = MHParamsSpecsFactory::get("IG");
  MHParamsValues params(&specs);
  params.readValues(fullIgParams);

  const int N = problem.size(0);
  const int M = problem.size(1);
  const int max_nh_size = pow(N - 1, 2);
  const std::string mh = params.mhName();
  const double max_ct = problem.upperBound();

  // continuator
  eoEvalFunc<EOT>& fullEval = problem.eval();
  moEval<Ngh>& evalN = problem.neighborEval();
  moIterContinuator<Ngh> globalContinuator(N, false);

  moTrueContinuator<Ngh> localContinuator;
  moCheckpoint<Ngh> localCheckpoint(localContinuator);

  moCheckpoint<Ngh> igCheckpoint(globalContinuator);
  igCheckpoint.add(counter);

  // neighborhood size
  const int min_nh_size = (N >= 20) ? 11 : 2;
  const int nh_interval = (N >= 20) ? 10 : 1;
  const int no_nh_sizes = (max_nh_size - min_nh_size) / nh_interval;
  const int scale =
      int((no_nh_sizes + 1) * params.real("IG.Neighborhood.Size") / 10.0);
  const int ig_nh_size =
      std::min(max_nh_size, min_nh_size + scale * nh_interval);

  moOrderNeighborhood<Ngh> neighborhood0(ig_nh_size);
  moRndWithoutReplNeighborhood<Ngh> neighborhood1(ig_nh_size);
  moNeighborhood<Ngh>* igNeighborhood = nullptr;
  switch (params.categorical("IG.Neighborhood.Strat")) {
    case 0:
      igNeighborhood = &neighborhood0;
      break;
    case 1:
      igNeighborhood = &neighborhood1;
      break;
    default:
      assert(false);
      break;
  }

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
  switch (params.categorical("IG.Comp.Strat")) {
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

  // algos xxHC
  moFirstImprHC<Ngh> algo0(*igNeighborhood, fullEval, evalN, localCheckpoint,
                           *compNN,
                           *compSN);  // FIHC
  moSimpleHC<Ngh> algo1(*igNeighborhood, fullEval, evalN, localCheckpoint,
                        *compNN,
                        *compSN);  // BestHC
  moRandomBestHC<Ngh> algo2(*igNeighborhood, fullEval, evalN, localCheckpoint,
                            *compNN,
                            *compSN);  // rndBestHC

  // IG (Ruiz+Stuetzle)
  // iterative greedy improvement without replacement (IG)
  // FastIGexplorer igexplorer(evalN, *compNN, *compSN);
  IGexplorer<Ngh> igexplorer(fullEval, N, *compSS);
  moLocalSearch<Ngh> algo3(igexplorer, localCheckpoint, fullEval);
  // IGexplorerWithRepl<Ngh> igWithReplexplorer(fullEval, N, *compSS); //
  // iterative greedy improvement with replacement moLocalSearch<Ngh>
  // algo4(igWithReplexplorer, checkpoint, fullEval);
  moLocalSearch<Ngh>* algo;
  switch (params.categorical("IG.Local.Search")) {
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
    // case 4: algo=&algo4; break;
    default:
      assert(false);
      break;
  }
  moCombinedContinuator<Ngh> singleStepContinuator(localCheckpoint);
  falseContinuator<Ngh> falseCont;
  singleStepContinuator.add(falseCont);
  if (params.categorical("IG.LS.Single.Step")) {
    algo->setContinuator(singleStepContinuator);
  }

  moAlwaysAcceptCrit<Ngh> accept0;
  moBetterAcceptCrit<Ngh> accept1(
      compSS0);  // no interest to accept equal solution here !
  // IG accept criterion based on temperature
  const double temperature =
      params.real("IG.Accept.Temperature") * max_ct / (N * M * 10);
  acceptCritTemperature<Ngh> accept2(temperature);

  moAcceptanceCriterion<Ngh>* accept;
  switch (params.categorical("IG.Accept")) {
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
  *** Perturb
  ****/
  const int destruction_size = N * params.real("IG.Destruction.Size");
  OpPerturbDestConst<EOT> igOpPerturb(fullEval, destruction_size);
  moMonOpPerturb<Ngh> igPerturb0(igOpPerturb, fullEval);

  const int N_lsps = N - destruction_size;
  const int nh_size_lsps =
      getNhSize(N_lsps, params.real("IG.Neighborhood.Size"));

  moOrderNeighborhood<Ngh> neighborhood0_lsps(nh_size_lsps);
  moRndWithoutReplNeighborhood<Ngh> neighborhood1_lsps(nh_size_lsps);
  moNeighborhood<Ngh>* neighborhood_lsps = nullptr;
  switch (params.categorical("IG.Neighborhood.Strat")) {
    case 0:
      neighborhood_lsps = &neighborhood0_lsps;
      break;
    case 1:
      neighborhood_lsps = &neighborhood1_lsps;
      break;
    default:
      assert(false);
      break;
  }

  // algos xxHC_lsps
  moFirstImprHC<Ngh> algo0_lsps(*neighborhood_lsps, fullEval, evalN,
                                localContinuator, *compNN,
                                *compSN);  // FIHC
  moSimpleHC<Ngh> algo1_lsps(*neighborhood_lsps, fullEval, evalN,
                             localContinuator, *compNN,
                             *compSN);  // BestHC
  moRandomBestHC<Ngh> algo2_lsps(*neighborhood_lsps, fullEval, evalN,
                                 localContinuator, *compNN,
                                 *compSN);  // rndBestHC
  IGexplorer<Ngh> igexplorer_lsps(fullEval, N_lsps, *compSS);
  moLocalSearch<Ngh> algo3_lsps(igexplorer_lsps, localContinuator, fullEval);

  moLocalSearch<Ngh>* igLSPSLocalSearh = nullptr;
  switch (params.categorical("IG.LSPS.Local.Search")) {
    case 0:
      igLSPSLocalSearh = &algo0_lsps;
      break;
    case 1:
      igLSPSLocalSearh = &algo1_lsps;
      break;
    case 2:
      igLSPSLocalSearh = &algo2_lsps;
      break;
    case 3:
      igLSPSLocalSearh = &algo3_lsps;
      break;
    default:
      throw std::runtime_error(
          "Unknown IG LSPS local search: " +
          std::to_string(params.categorical("IG.LSPS.Local.Search")));
      break;
  }

  moTrueContinuator<Ngh> trueCont;
  if (params.categorical("IG.LSPS.Single.Step")) {
    igLSPSLocalSearh->setContinuator(singleStepContinuator);
  }
  auto& neEval = problem.neighborEval();
  InsertFirstBest<Ngh> insert(neEval);
  IGLocalSearchPartialSolution<Ngh> igLSPS(insert, *igLSPSLocalSearh,
                                           destruction_size, *compSS);
  moMonOpPerturb<Ngh> igPerturb1(igLSPS, fullEval);

  moPerturbation<Ngh>* perturb;
  switch (params.categorical("IG.Algo")) {
    case 0:
      perturb = &igPerturb0;
      break;
    case 1:
      perturb = &igPerturb1;
      break;
    default:
      assert(false);
      break;
  }

  /****
  *** ILS
  ****/
  moILS<Ngh, Ngh> fullIg(*algo, fullEval, igCheckpoint, *perturb, *accept);

  moLocalSearch<Ngh>* localSearch = nullptr;
  if (sampling_strat == "FI")
    localSearch = &fi;
  else if (sampling_strat == "HC")
    localSearch = &hc;
  else if (sampling_strat == "RANDOM_BEST")
    localSearch = &randomBestHC;
  else if (sampling_strat == "IG")
    localSearch = &ig;
  else if (sampling_strat == "FULL_IG")
    localSearch = &fullIg;
  else
    throw std::runtime_error("Unknown sampling strat: " + sampling_strat);

  OpPerturbDestConst<EOT> OpPerturb(eval, strength);
  // eoSwapMutation<EOT> swapPerturb(strength);
  // eoShiftMutation<EOT> shiftPerturb;
  moMonOpPerturb<Ngh> perturb0(OpPerturb, eval);
  // moMonOpPerturb<Ngh> perturb1(swapPerturb, eval);
  // moMonOpPerturb<Ngh> perturb2(shiftPerturb, eval);

  moPerturbation<Ngh>* op = nullptr;
  if (perturb_strat == "IG")
    op = &perturb0;
  else if (perturb_strat == "SWAP")
    op = nullptr;  //&perturb1;
  else
    throw std::runtime_error("Unknown perturb_strat: " + perturb_strat);

  graph<EOT> lon;

  EOT sol;
  (*init)(sol);
  EOT x0 = sol;
  eval(x0);
  (*localSearch)(sol);
  lon.addNode(sol, x0, counter.value());

  std::vector<EOT> walk;
  walk.reserve(l);
  walk.push_back(sol);
  for (int i = 0; i <= l - 1; i++) {
    snowball(d, m, sol, *localSearch, counter, *op, lon);
    sol = randomWalkStep(sol, lon, *localSearch, counter, eval, walk);
    walk.push_back(sol);
  }

  std::cerr << "no_evals" << problem.noEvals() << "\n";
  return lon;
}
