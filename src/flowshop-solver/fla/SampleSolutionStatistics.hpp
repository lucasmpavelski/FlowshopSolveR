#pragma once

#include <vector>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

template <class Ngh, class EOT = typename Ngh::EOT>
class SampleSolutionStatistics {
  using ivec = std::vector<int>;
  const int solSize;
  eoInitPermutation<EOT> init;
  eoEvalFunc<EOT>& fullEval;
  moEval<Ngh>& neighborEval;
  moOrderNeighborhood<Ngh> neighborhood;
  std::vector<int> fitness;

  auto upw(int s, const ivec& ns) -> unsigned {
    return std::count_if(ns.begin(), ns.end(), [s](int n) { return n > s; });
  }

  auto sidew(int s, const ivec& ns) -> unsigned {
    return std::count_if(ns.begin(), ns.end(), [s](int n) { return n == s; });
  }

  auto downw(int s, const ivec& ns) -> unsigned {
    return std::count_if(ns.begin(), ns.end(), [s](int n) { return n < s; });
  }

  struct SolutionStatisticsResult {
    double up = 0, down = 0, side = 0, slmin = 0, lmin = 0, iplat = 0,
           ledge = 0, slope = 0, lmax = 0, slmax = 0;
  };

 public:
  SampleSolutionStatistics(int solSize,
                           eoEvalFunc<EOT>& fullEval,
                           moEval<Ngh>& neighborEval)
      : solSize(solSize),
        init(solSize),
        fullEval(fullEval),
        neighborEval(neighborEval),
        neighborhood((solSize - 1) * (solSize - 1)),
        fitness((solSize - 1) * (solSize - 1)) {}

  auto sample(const unsigned noSamples) -> SolutionStatisticsResult {
    EOT sol;
    Ngh neighbor;
    SolutionStatisticsResult res;
    const double scale = 1.0 / noSamples;
    for (int i = 0; i < noSamples; i++) {
      init(sol);
      fullEval(sol);
      // neighbor.size = sol.size();
      neighborhood.init(sol, neighbor);
      neighborEval(sol, neighbor);
      fitness[0] = neighbor.fitness();
      for (int no_neighbors = 1; neighborhood.cont(sol); no_neighbors++) {
        neighborhood.next(sol, neighbor);
        neighborEval(sol, neighbor);
        fitness[no_neighbors] = neighbor.fitness();
      }
      const double nScale = 1.0 / neighborhood.getNeighborhoodSize();

      unsigned up = upw(sol.fitness(), fitness);
      unsigned down = downw(sol.fitness(), fitness);
      unsigned side = sidew(sol.fitness(), fitness);

      res.up += scale * nScale * up;
      res.down += scale * nScale * down;
      res.side += scale * nScale * side;
      res.slmin += scale * ((down == 0) && (side == 0));
      res.lmin += scale * ((down == 0) && (side > 0) && (up > 0));
      res.iplat += scale * ((down == 0) && (up == 0));
      res.ledge += scale * ((down > 0) && (side > 0) && (up > 0));
      res.slope += scale * ((down > 0) && (side == 0) && (up > 0));
      res.lmax += scale * ((down > 0) && (side == 0) && (up == 0));
      res.slmax += scale * ((side == 0) && (up == 0));
    }

    return res;
  }
};