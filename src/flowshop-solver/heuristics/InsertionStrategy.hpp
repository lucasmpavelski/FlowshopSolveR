#pragma once

#include <memory>
#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

#include "flowshop-solver/global.hpp"
#include "flowshop-solver/problems/FSP.hpp"
#include "flowshop-solver/problems/FSPData.hpp"

template <class Ngh, class EOT = typename Ngh::EOT>
class InsertionStrategy : public eoBF<EOT&, int, void> {
 public:
  moEval<Ngh>& neighborEval;

  InsertionStrategy(moEval<Ngh>& neighborEval) : neighborEval{neighborEval} {}

  void insertJob(EOT& sol, int jobToInsert) {
    sol.emplace_back(jobToInsert);
    insert(sol, sol.size() - 1);
  }

  virtual void insert(EOT& sol, int positionToInsert) = 0;

  void operator()(EOT& sol, int positionToInsert) override {
    insert(sol, positionToInsert);
  }
};

template <class Ngh, class EOT = typename Ngh::EOT>
class InsertBest : public InsertionStrategy<Ngh> {
  moNeighborComparator<Ngh>& neighborComparator;

 public:
  InsertBest(moEval<Ngh>& neighborEval,
             moNeighborComparator<Ngh>& neighborComparator)
      : InsertionStrategy<Ngh>{neighborEval},
        neighborComparator{neighborComparator} {}

  using InsertionStrategy<Ngh>::neighborEval;

  void insert(EOT& sol, int positionToInsert) override {
    if (sol.size() == 1)
      return;
    Ngh neighbor, bestNeighbor;
    for (unsigned position = 0; position < sol.size(); position++) {
      neighbor.set(positionToInsert, position, sol.size());
      neighbor.invalidate();
      neighborEval(sol, neighbor);
      if (bestNeighbor.invalid() ||
          neighborComparator(bestNeighbor, neighbor)) {
        bestNeighbor = neighbor;
      }
      if (positionToInsert == static_cast<int>(position)) {
        sol.fitness(neighbor.fitness());
      }
    }
    bestNeighbor.move(sol);
  }
};

template <class Ngh, class EOT = typename Ngh::EOT>
class InsertFirstBest : public InsertBest<Ngh> {
  moNeighborComparator<Ngh> neighborComparator;

 public:
  InsertFirstBest(moEval<Ngh>& eval)
      : InsertBest<Ngh>{eval, neighborComparator} {}
};

template <class Ngh>
class InsertLastBest : public InsertBest<Ngh> {
  moEqualNeighborComparator<Ngh> neighborComparator;

 public:
  InsertLastBest(moEval<Ngh>& eval)
      : InsertBest<Ngh>{eval, neighborComparator} {}
};

template <class Ngh>
class myRandomNeighborComparator : public moNeighborComparator<Ngh> {
  double r{};

 public:
  myRandomNeighborComparator() : r{RNG::realUniform<double>()} {}

  auto operator()(const Ngh& _neighbor1, const Ngh& _neighbor2)
      -> bool override {
    /**
     * Simple reservoir sampling from:
     * Fan, C.; Muller, M.E.; Rezucha, I. (1962). "Development of sampling plans
     * by using sequential (item by item) selection techniques and digital
     * computers". Journal of the American Statistical Association. 57 (298):
     * 387-402.
     */
    if (_neighbor1.fitness() < _neighbor2.fitness()) {
      r = RNG::realUniform<double>();
      return true;
    } else if (_neighbor1.fitness() == _neighbor2.fitness()) {
      const auto r2 = RNG::realUniform<double>();
      if (r2 >= r) {
        r = r2;
        return true;
      }
      return false;
    } else {
      return false;
    }
  }

  void reset() { r = 0.0; }
};

template <class Ngh, class EOT = typename Ngh::EOT>
class InsertRandomBest : public InsertBest<Ngh> {
  myRandomNeighborComparator<Ngh> comparator;

 public:
  InsertRandomBest(moEval<Ngh>& eval) : InsertBest<Ngh>{eval, comparator} {}

  void insert(EOT& sol, int positionToInsert) override {
    InsertBest<Ngh>::insert(sol, positionToInsert);
    comparator.reset();
  }
};

template <class Ngh, class EOT = typename Ngh::EOT>
class InsertBestTieBreaking : public InsertionStrategy<Ngh> {
  moNeighborComparator<Ngh> better;
  moEqualNeighborComparator<Ngh> equal;

 public:
  InsertBestTieBreaking(moEval<Ngh>& eval) : InsertionStrategy<Ngh>{eval} {}

  using InsertionStrategy<Ngh>::neighborEval;

  void insert(EOT& sol, int positionToInsert) override {
    if (sol.size() == 1)
      return;
    Ngh neighbor;
    std::vector<Ngh> bestNeighbors;
    for (unsigned position = 0; position < sol.size(); position++) {
      neighbor.set(positionToInsert, position, sol.size());
      neighbor.invalidate();
      neighborEval(sol, neighbor);
      if (bestNeighbors.empty() || better(bestNeighbors[0], neighbor)) {
        bestNeighbors.clear();
        bestNeighbors.push_back(neighbor);
      } else if (equal(bestNeighbors[0], neighbor)) {
        bestNeighbors.push_back(neighbor);
      }
      if (positionToInsert == static_cast<int>(position)) {
        sol.fitness(neighbor.fitness());
      }
    }
    neighbor.set(positionToInsert, positionToInsert, sol.size());
    neighbor.fitness(sol.fitness());
    Ngh bestNeighbor = tieBreak(sol, bestNeighbors);
    bestNeighbor.move(sol);
    sol.fitness(bestNeighbor.fitness());
  }

 protected:
  virtual auto tieBreak(EOT& sol, std::vector<Ngh> bestPositions) -> Ngh = 0;
};

/**
 * Introduced on An improved NEH heuristic to minimize makespan in
 * permutationflow shops by Pawel J. Kalczynski, Jerzy Kamburowski
 */
class InsertNM1 : public InsertBestTieBreaking<FSPNeighbor> {
  moNeighborComparator<FSPNeighbor> neighborComparator;
  const FSPData& fspData;
  std::vector<int> ar, br;

 public:
  InsertNM1(moEval<FSPNeighbor>& eval, const FSPData& fspData)
      : InsertBestTieBreaking<FSPNeighbor>{eval},
        fspData{fspData},
        ar(fspData.noJobs()),
        br(fspData.noJobs()) {
    init();
  }

  using InsertBestTieBreaking<FSPNeighbor>::insert;

 private:
  void init() {
    const auto noJobs = fspData.noJobs();
    const auto noMachines = fspData.noMachines();
    for (int r = 0; r < noJobs; r++) {
      ar[r] = 0;
      br[r] = 0;
      for (int i = 0; i < noMachines; i++) {
        ar[r] += fspData.pt(r, i) - fspData.pt(r, noMachines - 1);
        br[r] += fspData.pt(r, i) - fspData.pt(r, 0);
      }
    }
  }

 protected:
  auto tieBreak(FSP& sol, std::vector<FSPNeighbor> bestPositions)
      -> FSPNeighbor override {
    int cmax = sol.fitness();
    int ap = cmax, bp = cmax;
    for (const auto& j : sol) {
      ap -= fspData.pt(j, fspData.noMachines() - 1);
      bp -= fspData.pt(j, 0);
    }
    for (FSPNeighbor neighbor : bestPositions) {
      auto move = neighbor.firstSecond(sol);
      int arn = ar[move.second];
      int brn = br[move.second];
      if (std::min(ap, brn) >= std::min(arn, bp)) {
        return neighbor;
      }
    }
    return bestPositions[bestPositions.size() - 1];
  };
};

/**
 * Introduced on An improved NEH heuristic to minimize makespan in
 * permutationflow shops by Pawel J. Kalczynski, Jerzy Kamburowski
 */
class InsertKK1 : public InsertBestTieBreaking<FSPNeighbor> {
  moNeighborComparator<FSPNeighbor> neighborComparator;
  const FSPData& fspData;
  std::vector<int> ar, br;

 public:
  InsertKK1(moEval<FSPNeighbor>& eval, const FSPData& fspData)
      : InsertBestTieBreaking<FSPNeighbor>{eval},
        fspData{fspData},
        ar(fspData.noJobs()),
        br(fspData.noJobs()) {
    init();
  }

  using InsertBestTieBreaking<FSPNeighbor>::insert;

 private:
  void init() {
    const auto noJobs = fspData.noJobs();
    const auto noMachines = fspData.noMachines();
    int mw = (noMachines - 1) * (noMachines - 2) / 2;
    for (int j = 0; j < noJobs; j++) {
      ar[j] = 0;
      br[j] = 0;
      for (int i = 0; i < noMachines; i++) {
        int p_ij = fspData.pt(j, i);
        ar[j] += (mw + noMachines - i - 1) * p_ij;
        br[j] += (mw + i) * p_ij;
      }
    }
  }

 protected:
  auto tieBreak(FSP& sol, std::vector<FSPNeighbor> bestPositions)
      -> FSPNeighbor override {
    for (FSPNeighbor neighbor : bestPositions) {
      auto move = neighbor.firstSecond(sol);
      int arn = ar[move.second];
      int brn = br[move.second];
      if (arn <= brn) {
        return neighbor;
      }
    }
    return bestPositions[bestPositions.size() - 1];
  };
};

/**
 * Kalczynski, P. J., & Kamburowski, J. (2009). An empirical analysis of the
 * optimality rate of flow shop heuristics. European Journal of Operational
 * Research, 198(1), 93?101. doi:10.1016/j.ejor.2008.08.021
 */
class InsertKK2 : public InsertBestTieBreaking<FSPNeighbor> {
  moNeighborComparator<FSPNeighbor> neighborComparator;
  const FSPData& fspData;
  std::vector<double> u;

 public:
  InsertKK2(moEval<FSPNeighbor>& eval, const FSPData& fspData)
      : InsertBestTieBreaking<FSPNeighbor>{eval},
        fspData{fspData},
        u(fspData.noJobs()) {
    init();
  }

  using InsertBestTieBreaking<FSPNeighbor>::insert;

 private:
  void init() {
    auto noJobs = fspData.noJobs();
    auto noMachines = fspData.noMachines();
    const int s = std::floor(noMachines / 2.0);
    const int t = std::ceil(noMachines / 2.0);
    const double eps = 1e-6;
    std::vector<double> order(noJobs);
    for (int j = 0; j < noJobs; j++) {
      u[j] = 0.0;
      for (int h = 1; h <= s; h++) {
        const int hi = h - 1;
        const double w = (h - 0.75) / (s - 0.75) - eps;
        const int p_sp1mh_j = fspData.pt(j, s + 1 - hi);
        const int p_tph_j = fspData.pt(j, t + hi);
        u[j] += w * (p_sp1mh_j - p_tph_j);
      }
    }
  }

 protected:
  auto tieBreak(FSP& sol, std::vector<FSPNeighbor> bestPositions)
      -> FSPNeighbor override {
    for (FSPNeighbor neighbor : bestPositions) {
      auto move = neighbor.firstSecond(sol);
      if (u[move.second] <= 0) {
        return neighbor;
      }
    }
    return bestPositions[bestPositions.size() - 1];
  };
};

template <class Ngh>
auto buildInsertionStrategy(const std::string& name, moEval<Ngh>& eval)
    -> std::unique_ptr<InsertionStrategy<Ngh>> {
  if (name == "first_best")
    return std::make_unique<InsertFirstBest<Ngh>>(eval);
  if (name == "last_best")
    return std::make_unique<InsertLastBest<Ngh>>(eval);
  if (name == "random_best")
    return std::make_unique<InsertRandomBest<Ngh>>(eval);
  return nullptr;
}

auto buildInsertionStrategyFSP(const std::string& name,
                               moEval<FSPNeighbor>& eval,
                               const FSPData& fspData)
    -> std::unique_ptr<InsertionStrategy<FSPNeighbor>> {
  if (name == "nm1")
    return std::make_unique<InsertNM1>(eval, fspData);
  if (name == "kk1")
    return std::make_unique<InsertKK1>(eval, fspData);
  if (name == "kk2")
    return std::make_unique<InsertKK1>(eval, fspData);
  return nullptr;
}
