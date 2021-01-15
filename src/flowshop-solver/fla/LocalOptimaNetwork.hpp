#pragma once

#include <algorithm>
#include <vector>
#include <ostream>

template <class EOT>
struct LocalOptimaNetwork {
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
        [b_idx](LocalOptimaNetwork<EOT>::edge& edge) { return edge.node_idx == b_idx; });
    if (edge_it == edges.at(a_idx).end())
      return nullptr;
    return &*edge_it;
  }

  void addEdge(const EOT& a, const EOT& b, int weight) {
    unsigned a_idx = getIndex(a);
    unsigned b_idx = getIndex(b);
    assert(a_idx != nodes.size() && b_idx != nodes.size());
    edges.at(a_idx).push_back(LocalOptimaNetwork<EOT>::edge(b_idx, weight));
  }

  void print(std::ostream& out) {
    for (unsigned i = 0; i < edges.size(); i++) {
      out << i << ": " << nodes[i] << '\n';
      for (auto e : edges[i]) {
        out << " (" << e.node_idx << "," << e.weight << ")";
      }
      out << '\n';
    }
    out << '\n';
  }
};
