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

#include "flowshop-solver/fla/LocalOptimaNetwork.hpp"
#include "flowshop-solver/fla/LONSampling.hpp"


class SnowballLONSampling : public LONSampling {
  template <class Ngh, class EOT = typename Ngh::EOT>
  void snowball(int d,
                int m,
                const EOT& x,
                moLocalSearch<Ngh>& localSearch,
                moCounterStat<EOT>& counter,
                moPerturbation<Ngh>& op,
                LocalOptimaNetwork<EOT>& lon) {
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
  auto randomWalkStep(EOT& sol,
                    LocalOptimaNetwork<EOT>& lon,
                    moLocalSearch<Ngh>& localSearch,
                    moCounterStat<EOT>& counter,
                    eoEvalFunc<EOT>& eval,
                    const std::vector<EOT>& walk) -> EOT {
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
  public:

  SnowballLONSampling() = default;

  auto sampleLON(
      const std::unordered_map<std::string, std::string>& prob_params,
      const std::unordered_map<std::string, std::string>& sampling_params,
      unsigned seed) -> LocalOptimaNetwork<FSPProblem::EOT> override {
    rng.reseed(seed);
    FSPProblem problem = FSPProblemFactory::get(prob_params);
    using EOT = FSPProblem::EOT;
    using Ngh = FSPProblem::Ngh;

    MHParamsSpecs specs = MHParamsSpecsFactory::get("Snowball");
    MHParamsValues params(&specs);
    params.readValues(sampling_params);

    eoFSPFactory factory(params, problem);

    // sampling params
    using namespace std::string_literals;
    auto d = params.integer("Snowball.Depth");
    auto m = params.integer("Snowball.NoEdges");
    auto l = params.integer("Snowball.WalkLength");

    eoInit<FSP>* init = factory.buildInit();
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

    LocalOptimaNetwork<EOT> lon;

    EOT sol(problem.size(0));
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
};