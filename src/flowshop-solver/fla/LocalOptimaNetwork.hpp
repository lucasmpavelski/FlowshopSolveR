#pragma once

#include <algorithm>
#include <numeric>
#include <ostream>
#include <unordered_map>
#include <vector>

template <class EOT>
struct VectorHasher {
    auto operator()(const EOT &V) const -> int {
        int hash = V.size();
        for(auto &i : V) {
            hash ^= i + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

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
  std::unordered_map<EOT, int, VectorHasher<EOT>> indexMap;
  std::vector<std::vector<lo_sample>> samples;
  std::vector<std::vector<edge>> edges;

  auto addNode(EOT n, EOT sample, int no_steps) -> int {
    int idx = addNode(n);
    addSample(idx, sample, no_steps);
    return idx;
  }

  auto addNode(EOT n) -> int {
    int idx = getIndex(n);
    if (static_cast<unsigned>(idx) == nodes.size()) {
      nodes.emplace_back(n);
      samples.resize(samples.size() + 1);
      edges.resize(edges.size() + 1);
      indexMap[n] = idx;
    }
    return idx;
  }

  void addSamples(int optimaIdx, const std::vector<lo_sample>& newSamples) {
    for (const auto& sample : newSamples) {
      addSample(optimaIdx, sample.sol, sample.no_steps);
    }
  }

  void addSample(int optimaIdx, EOT sample, int no_steps) {
    samples[optimaIdx].emplace_back(lo_sample{sample, no_steps});
  }

  auto getIndex(const EOT& n) -> int {
    auto it = indexMap.find(n);
    return it == indexMap.end() ? nodes.size() : it->second;
  }

  auto contains(const EOT& n) -> bool { return getIndex(n) != nodes.size(); }
  auto containsEdge(const EOT& a, const EOT& b) -> bool {
    return getEdge(a, b) != nullptr;
  } 
  auto containsEdge(int a_idx, int b_idx) -> bool {
    return getEdge(a_idx, b_idx) != nullptr;
  }

  auto getEdge(const EOT& a, const EOT& b) -> edge* {
    return getEdge(getIndex(a), getIndex(b));
  }

  auto getEdge(int a_idx, int b_idx) -> edge* {
    auto edge_it = std::find_if(edges.at(a_idx).begin(), edges.at(a_idx).end(),
                                [b_idx](LocalOptimaNetwork<EOT>::edge& edge) {
                                  return edge.node_idx == b_idx;
                                });
    if (edge_it == edges.at(a_idx).end())
      return nullptr;
    return &*edge_it;
  }

  void addEdge(const EOT& a, const EOT& b, int weight) {
    unsigned a_idx = getIndex(a);
    unsigned b_idx = getIndex(b);
    addEdge(a_idx, b_idx, weight);
  }

  void addEdge(int a_idx, int b_idx, int weight) {
    assert(a_idx != nodes.size() && b_idx != nodes.size());
    edges[a_idx].push_back(LocalOptimaNetwork<EOT>::edge(b_idx, weight));
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

  [[nodiscard]] auto size() const -> std::size_t { return nodes.size(); }

  [[nodiscard]] auto noEdges() const -> int {
    return std::accumulate(
        edges.begin(), edges.end(), 0,
        [](int size, const std::vector<edge>& e) { return size + e.size(); });
  }

  void merge(LocalOptimaNetwork<EOT> other) {
    for (int i = 0; i < other.size(); i++) {
      const auto& node = other.nodes[i];
      const auto a_idx = addNode(node);
      // addSamples(a_idx, other.samples[i]);
      const auto& nodeEdges = other.edges[i];
      for (const auto& edge: nodeEdges) {
        const auto& nodeb = other.nodes[edge.node_idx];
        const auto b_idx = addNode(nodeb);
        // addSamples(b_idx, other.samples[edge.node_idx]);
        if (!containsEdge(a_idx, b_idx)) {
          addEdge(a_idx, b_idx, edge.weight);
        } else {
          getEdge(a_idx, b_idx)->weight += edge.weight;
        }
      }
    }
  }
};
