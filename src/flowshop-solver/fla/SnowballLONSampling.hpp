#pragma once

#include <algorithm>
#include <vector>
#include <iostream>

#include <paradiseo/mo/mo>
#include <paradiseo/eo/eo>

#include "flowshop-solver/FSPProblemFactory.hpp"
#include "flowshop-solver/MHParamsSpecsFactory.hpp"
#include "flowshop-solver/eoFSPFactory.hpp"
#include "flowshop-solver/fla_methods.hpp"
#include "flowshop-solver/problems/FSPProblem.hpp"
#include "flowshop-solver/global.hpp"


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

  auto addNode(EOT n, EOT sample, int no_steps) -> int {
    int idx = getIndex(n);
    if (static_cast<unsigned>(idx) == nodes.size()) {
      nodes.emplace_back(n);
      samples.resize(samples.size() + 1);
      edges.resize(edges.size() + 1);
    }
    samples[idx].emplace_back(lo_sample{sample, no_steps});
    return idx;
  }

  auto getIndex(const EOT& n) -> int {
    auto it = std::find(nodes.begin(), nodes.end(), n);
    return std::distance(nodes.begin(), it);
  }

  auto contains(const EOT& n) -> bool { return getIndex(n) != nodes.size(); }

  auto getEdge(const EOT& a, const EOT& b) -> edge* {
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

  MHParamsSpecs specs = MHParamsSpecsFactory::get("Snowball.txt");
  MHParamsValues params(&specs);
  params.readValues(sampling_params);

  eoFSPFactory factory(params, problem);

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
 

  eoInit<FSP>* init = factory.buildInit();
  moIndexNeighborhood<FSPNeighbor>* neighborhood = factory.buildNeighborhood();
  moLocalSearch<Ngh>* localSearch = factory.buildLocalSearch();
  moPerturbation<Ngh>* perturbation = factory.buildPerturb();

  moTrueContinuator<Ngh> tc;
  moCounterStat<EOT> counter;
  moCheckpoint<Ngh> checkpoint(tc);
  checkpoint.add(counter);

  // continuator
  moIterContinuator<Ngh> globalContinuator(problem.size(0), false);

  moTrueContinuator<Ngh> localContinuator;
  moCheckpoint<Ngh> localCheckpoint(localContinuator);

  moCheckpoint<Ngh> igCheckpoint(globalContinuator);
  igCheckpoint.add(counter);

  graph<EOT> lon;

  EOT sol;
  (*init)(sol);
  EOT x0 = sol;
  problem.eval(x0);
  (*localSearch)(sol);
  lon.addNode(sol, x0, counter.value());

  std::vector<EOT> walk;
  walk.reserve(l);
  walk.push_back(sol);
  for (int i = 0; i <= l - 1; i++) {
    snowball(d, m, sol, *localSearch, counter, *perturbation, lon);
    sol = randomWalkStep(sol, lon, *localSearch, counter, problem.eval(), walk);
    walk.push_back(sol);
  }

  std::cerr << "no_evals" << problem.noEvals() << "\n";
  return lon;
}
