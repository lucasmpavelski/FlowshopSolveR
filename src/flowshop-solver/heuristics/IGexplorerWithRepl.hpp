#pragma once

#include <algorithm>
#include <numeric>
#include <vector>

#include <comparator/moNeighborComparator.h>
#include <comparator/moSolComparator.h>
#include <comparator/moSolNeighborComparator.h>
#include <eoEvalFunc.h>
#include <explorer/moNeighborhoodExplorer.h>
#include <neighborhood/moNeighborhood.h>
#include <utils/rnd_generators.h>

/**
 * Iterated Greedy Explorer with replacement
 */
template <class Neighbor>
class IGexplorerWithRepl : public moNeighborhoodExplorer<Neighbor> {
 public:
  using EOT = typename Neighbor::EOT;

  /**
   * Constructor
   * @param eval the evaluation function
   * @param size the solution size
   * @param comp the solution comparator
   */
  IGexplorerWithRepl(eoEvalFunc<EOT>& eval, int size,
                     moSolComparator<EOT>& comp)
      : moNeighborhoodExplorer<Neighbor>(),
        eval(eval),
        size(size),
        comp(comp) {}

  /**
   * initParam: NOTHING TO DO
   * @param _solution unused solution
   */
  virtual void initParam(EOT&) {
    cpt = 0;
    rand_job.resize(size);
    std::iota(std::begin(rand_job), std::end(rand_job), 0);
    k = rng.uniform(0, size);
  }

  /**
   * updateParam: NOTHING TO DO
   * @param _solution unused solution
   */
  virtual void updateParam(EOT& _solution) {
    if (comp(_solution, sol_tmp)) {
      _solution = sol_tmp;
      cpt = 0;
    } else
      cpt++;
    k = rng.uniform(0, size);
  }

  /**
   * terminate: NOTHING TO DO
   * @param _solution unused solution
   */
  virtual void terminate(EOT&){}

  /**
   * Explore the neighborhood of a solution
   * @param _solution the current solution
   */
  /**
   * Explore the neighborhood of a solution
   * @param _solution the current solution
   */
  virtual void operator()(EOT& _solution) {
    int j;
    j = 0;
    while (_solution[j] != rand_job[k]) {
      j++;
    }
    sol_tmp = _solution;
    sol_tmp.erase(sol_tmp.begin() + j);
    sol_tmp.insert(sol_tmp.begin(), rand_job[k]);
    sol_tmp.invalidate();
    eval(sol_tmp);
    best = sol_tmp;
    for (int position = 1; position < _solution.size(); position++) {
      sol_tmp = _solution;
      sol_tmp.invalidate();
      sol_tmp.erase(sol_tmp.begin() + j);
      sol_tmp.insert(sol_tmp.begin() + position, rand_job[k]);
      eval(sol_tmp);
      if (comp(best, sol_tmp)) best = sol_tmp;
    }
  }

  /**
   * continue if a move is accepted
   * @param _solution the solution
   * @return true if an ameliorated neighbor was be found
   */
  virtual bool isContinue(EOT&) { return cpt < 2 * size; }

  /**
   * move the solution with the best neighbor
   * @param _solution the solution to move
   */
  virtual void move(EOT&){}

  /**
   * accept test if an amelirated neighbor was be found
   * @param _solution the solution
   * @return true if the best neighbor ameliorate the fitness
   */
  virtual bool accept(EOT&) { return false; }

  /**
   * Return the class Name
   * @return the class name as a std::string
   */
  virtual std::string className() const { return "IGexplorerWithRepl"; }

 private:
  eoEvalFunc<EOT>& eval;
  int size;
  moSolComparator<EOT> comp;
  bool improve;  // true if the solution has changed
  std::vector<int> rand_job;
  EOT sol_tmp, best;
  int k, cpt;
};
