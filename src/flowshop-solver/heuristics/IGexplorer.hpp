#pragma once

#include <algorithm>

#include <comparator/moNeighborComparator.h>
#include <comparator/moSolComparator.h>
#include <comparator/moSolNeighborComparator.h>
#include <explorer/moNeighborhoodExplorer.h>
#include <neighborhood/moNeighborhood.h>
#include <utils/eoRNG.h>

#include "global.hpp"
#include "problems/FSPEvalFunc.hpp"
#include "heuristics/neighborhood_checkpoint.hpp"

/**
 * Explorer for a simple Hill-climbing
 */
template <class Neighbor>
class IGexplorer : public moNeighborhoodExplorer<Neighbor> {
 public:
  typedef typename Neighbor::EOT EOT;

  /**
   * Constructor
   * @param _neighborhood the neighborhood
   * @param _eval the evaluation function
   * @param _neighborComparator a neighbor comparator
   * @param _solNeighborComparator solution vs neighbor comparator
   */
  IGexplorer(eoEvalFunc<EOT>& _fullEval,
             int size,
             moSolComparator<EOT>& _solComparator,
             NeigborhoodCheckpoint<Neighbor> neighborhoodCheckpoint =
                 NeigborhoodCheckpoint<Neighbor>())
      : moNeighborhoodExplorer<Neighbor>(),
        eval(_fullEval),
        size(size),
        solComparator(_solComparator),
        neighborhoodCheckpoint{neighborhoodCheckpoint} {}

  /**
   * initParam: NOTHING TO DO
   * @param _solution unused solution
   */
  virtual void initParam(EOT& _solution) {
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
  virtual void updateParam(EOT& _solution) {
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
  virtual void terminate(EOT& _solution) {}

  /**
   * Explore the neighborhood of a solution
   * @param _solution the current solution
   */
  virtual void operator()(EOT& _solution) {
    // std::cerr << "explore!" << '\n';
    neighborhoodCheckpoint.initNeighborhood(_solution);
    int j = 0;
    while (_solution[j] != RandJOB[k]) {
      j++;
    }
    int position = 0;
    /*if (j == position)
      position++;*/
    EOT solTMP = _solution;
    solTMP.erase(solTMP.begin() + j);
    solTMP.insert(solTMP.begin() + position, RandJOB[k]);
    solTMP.invalidate();
    eval(solTMP);
    EOT best = solTMP;
    for (; position < _solution.size(); position++) {
      if (position == j)
        continue;
      solTMP = _solution;
      solTMP.invalidate();
      solTMP.erase(solTMP.begin() + j);
      solTMP.insert(solTMP.begin() + position, RandJOB[k]);
      eval(solTMP);
      neighborhoodCheckpoint.neighborCall(solTMP);
      if (solComparator(best, solTMP))
        best = solTMP;
    }
    if (solComparator(_solution, best)) {
      _solution = best;
      improve = true;
    }
    neighborhoodCheckpoint.lastNeighborhoodCall(_solution);
  }

  /**
   * continue if a move is accepted
   * @param _solution the solution
   * @return true if an ameliorated neighbor was be found
   */
  virtual bool isContinue(EOT& _solution) { return !LO; }

  /**
   * move the solution with the best neighbor
   * @param _solution the solution to move
   */
  virtual void move(EOT& _solution) {}

  /**
   * accept test if an amelirated neighbor was be found
   * @param _solution the solution
   * @return true if the best neighbor ameliorate the fitness
   */
  virtual bool accept(EOT& _solution) { return false; }

  /**
   * Return the class Name
   * @return the class name as a std::string
   */
  virtual std::string className() const { return "IGexplorer"; }

 private:
  moSolComparator<EOT>& solComparator;
  NeigborhoodCheckpoint<Neighbor> neighborhoodCheckpoint;
  // true if the solution has changed
  bool improve;
  bool LO;
  std::vector<int> RandJOB;
  // EOT solTMP, best;
  eoEvalFunc<EOT>& eval;
  int size;
  int k;
};
