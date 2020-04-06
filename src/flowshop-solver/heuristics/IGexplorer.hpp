#pragma once

#include <algorithm>

#include "../global.hpp"
#include "flowshop-solver/heuristics/neighborhood_checkpoint.hpp"
#include "flowshop-solver/problems/FSPEvalFunc.hpp"
#include "paradiseo/eo/utils/eoRNG.h"
#include "paradiseo/mo/comparator/moNeighborComparator.h"
#include "paradiseo/mo/comparator/moSolComparator.h"
#include "paradiseo/mo/comparator/moSolNeighborComparator.h"
#include "paradiseo/mo/explorer/moNeighborhoodExplorer.h"
#include "paradiseo/mo/neighborhood/moNeighborhood.h"

/**
 * Explorer for a simple Hill-climbing
 */
template <class Neighbor, class EOT = typename Neighbor::EOT>
class IGexplorer : public moNeighborhoodExplorer<Neighbor> {
  eoEvalFunc<EOT>& eval;
  int size;
  moSolComparator<EOT>& solComparator;
  NeigborhoodCheckpoint<Neighbor> neighborhoodCheckpoint;
  // true if the solution has changed
  bool improve;
  bool LO;
  std::vector<int> RandJOB;
  // EOT solTMP, best;
  unsigned k;

 public:
  IGexplorer(eoEvalFunc<EOT>& _fullEval,
             int size,
             moSolComparator<EOT>& _solComparator,
             NeigborhoodCheckpoint<Neighbor> neighborhoodCheckpoint =
                 NeigborhoodCheckpoint<Neighbor>())
      : moNeighborhoodExplorer<Neighbor>(),
        eval(_fullEval),
        size(size),
        solComparator(_solComparator),
        neighborhoodCheckpoint{std::move(neighborhoodCheckpoint)} {}

  /**
   * initParam: NOTHING TO DO
   * @param _solution unused solution
   */
  void initParam(EOT& _solution) override {
    improve = false;
    LO = false;
    RandJOB.resize(size);
    std::copy(_solution.begin(), _solution.end(), RandJOB.begin());
    std::shuffle(RandJOB.begin(), RandJOB.end(),
                 ParadiseoRNGFunctor<unsigned int>());
    k = 0;
    neighborhoodCheckpoint.init(_solution);
  }

  /**
   * updateParam: NOTHING TO DO
   * @param _solution unused solution
   */
  void updateParam(EOT&) override {
    if (k < RandJOB.size() - 1)
      k++;
    else {
      k = 0;
      std::shuffle(RandJOB.begin(), RandJOB.end(),
                   ParadiseoRNGFunctor<unsigned int>());
    }
    if (k == 0 && !improve) {
      LO = true;
    }
    //		improve=!improve;
    improve = false;
  }

  /**
   * terminate: NOTHING TO DO
   * @param _solution unused solution
   */
  void terminate(EOT&) override {}

  /**
   * Explore the neighborhood of a solution
   * @param _solution the current solution
   */
  void operator()(EOT& _solution) override {
    neighborhoodCheckpoint.initNeighborhood(_solution);
    int j = 0;
    while (_solution[j] != RandJOB[k]) {
      j++;
    }
    int position = 0;
    EOT solTMP = _solution;
    solTMP.erase(solTMP.begin() + j);
    solTMP.insert(solTMP.begin() + position, RandJOB[k]);
    solTMP.invalidate();
    eval(solTMP);
    EOT best = solTMP;
    std::vector<int> ties;
    for (; position < static_cast<int>(_solution.size()); position++) {
      if (position == j)
        continue;
      solTMP = _solution;
      solTMP.invalidate();
      solTMP.erase(solTMP.begin() + j);
      solTMP.insert(solTMP.begin() + position, RandJOB[k]);
      eval(solTMP);
      neighborhoodCheckpoint.neighborCall(solTMP);
      if (solComparator(best, solTMP)) {
        best = solTMP;
        ties = {position};
      } else if (solComparator.equals(best, solTMP)) {
        ties.push_back(position);
      }
    }
    int chosen = RNG::intUniform(ties.size() - 1);
    if (solComparator(_solution, best)) {
      solTMP = _solution;
      solTMP.invalidate();
      solTMP.erase(solTMP.begin() + j);
      solTMP.insert(solTMP.begin() + ties[chosen], RandJOB[k]);
      _solution = solTMP;
      _solution.fitness(best.fitness());
      improve = true;
    }
    neighborhoodCheckpoint.lastNeighborhoodCall(_solution);
  }

  /**
   * continue if a move is accepted
   * @param _solution the solution
   * @return true if an ameliorated neighbor was be found
   */
  bool isContinue(EOT&) override { return !LO; }

  /**
   * move the solution with the best neighbor
   * @param _solution the solution to move
   */
  void move(EOT&) override {}

  /**
   * accept test if an amelirated neighbor was be found
   * @param _solution the solution
   * @return true if the best neighbor ameliorate the fitness
   */
  bool accept(EOT&) override { return false; }
};
