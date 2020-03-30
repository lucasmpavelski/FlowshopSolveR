#pragma once

#include <acceptCrit/moAcceptanceCriterion.h>
#include <comparator/moSolComparator.h>
#include <memory/moDummyMemory.h>
#include <utils/eoRNG.h>

#include <cmath>

/**
 * Acceptance Criterion for extreme intensification : accept if the new solution
 * is better than previous one
 */
template <class Neighbor>
class acceptCritTemperature : public moAcceptanceCriterion<Neighbor>,
                              public moDummyMemory<Neighbor> {
  double threshold;

 public:
  typedef typename Neighbor::EOT EOT;

  /*
    default constructor:
    compare the fitness value: accept if the fitness is higher
  */
  acceptCritTemperature(double _threshold) : threshold(_threshold) {}

  /**
   * Accept if the new solution is better than previous one according to the
   * comparator
   * @param _sol1 the previous solution
   * @param _sol2 the new solution after local search
   * @return true if the new solution is better than previous one
   */
  bool operator()(EOT& _sol1, EOT& _sol2) {
    if (_sol2.fitness() < _sol1.fitness()) {
      return true;
    } else {
      return rng.uniform() <=
             std::exp((_sol1.fitness() - _sol2.fitness()) / threshold);
    }
  }
};
