#pragma once

#include <cmath>

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

/**
 * Acceptance Criterion for extreme intensification : accept if the new solution
 * is better than previous one
 */
template <class Neighbor>
class acceptCritTemperature : public moAcceptanceCriterion<Neighbor>,
                              public moDummyMemory<Neighbor> {
  double threshold;
  moSolComparator<typename Neighbor::EOT> compare;

 public:
  using EOT = typename Neighbor::EOT;

  acceptCritTemperature(double _threshold) : threshold(_threshold) {}

  /**
   * Accept if the new solution is better than previous one according to the
   * comparator
   * @param _sol1 the previous solution
   * @param _sol2 the new solution after local search
   * @return true if the new solution is better than previous one
   */
  auto operator()(EOT& _sol1, EOT& _sol2) -> bool override {
    if (compare(_sol1, _sol2)) {
      return true;
    } else {
      double prob = std::exp((_sol1.fitness() - _sol2.fitness()) / threshold);
      bool accept = rng.uniform() <= prob;
      return accept;
    }
  }
};
