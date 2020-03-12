#pragma once

#include <paradiseo/mo/perturb/moPerturbation.h>

template <class Chrom>
class ilsKickOp : public eoMonOp<Chrom> {
 public:
  /// CTor
  ilsKickOp(const unsigned _howManySwaps = 1, const double strength = 0.1)
      : howManySwaps(_howManySwaps), strength(strength) {
    // consistency checks
    if (howManySwaps < 1)
      throw std::runtime_error("Invalid number of swaps in eoSwapMutation.");
    if (strength < 0 || strength > 1)
      throw std::runtime_error("Invalid strength value.");
  }

  /// The class name.
  virtual std::string className() const { return "eoSwapMutation"; }

  /**
   * Swap two components of the given chromosome.
   * @param chrom The cromosome which is going to be changed.
   */
  bool operator()(Chrom& chrom) {
    const unsigned n = chrom.size();

    for (unsigned int swap = 0; swap < howManySwaps; swap++) {
      // generate two different indices
      int i, j, diff = n * strength;
      do {
        i = eo::rng.random(n);
        j = eo::rng.random(diff * 2) - diff;
        if (j >= 0)
          j = i + j + 1;
        else
          j = i + j;
      } while (j < 0 || j >= n);

      // swap
      std::swap(chrom[i], chrom[j]);
    }
    return true;
  }

 private:
  const unsigned int howManySwaps;
  const double strength;
};