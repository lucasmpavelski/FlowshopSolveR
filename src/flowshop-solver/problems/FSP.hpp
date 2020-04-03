#pragma once

#include "paradiseo/eo/eoInt.h"
#include "paradiseo/eo/eoScalarFitness.h"
#include "paradiseo/mo/problems/permutation/moShiftNeighbor.h"

template <class EOT, class Fitness = typename EOT::Fitness>
class myShiftNeighbor : public moIndexNeighbor<EOT, Fitness> {
 public:
  using moIndexNeighbor<EOT, Fitness>::key;

  myShiftNeighbor() = default;
  myShiftNeighbor(int from, int to, int size) : myShiftNeighbor{} {
    set(from, to, size);
  }

  /**
   * Apply move on a solution regarding a key
   * @param _sol the solution to move
   */
  virtual void move(EOT& _sol) {
    if (static_cast<int>(key) >= 0) {
      unsigned int tmp;
      size = _sol.size();
      translate(key + 1);
      // keep the first component to change
      tmp = _sol[first];
      // shift
      if (first < second) {
        for (unsigned int i = first; i < second - 1; i++)
          _sol[i] = _sol[i + 1];
        // shift the first component
        _sol[second - 1] = tmp;
      } else { /* first > second*/
        for (unsigned int i = first; i > second; i--)
          _sol[i] = _sol[i - 1];
        // shift the first component
        _sol[second] = tmp;
      }
      _sol.invalidate();
    } else {
      auto begin = _sol.begin();
      if (first == second) {
        return;
      } else if (first < second) {
        std::rotate(begin + first, begin + first + 1, begin + second);
      } else {
        std::rotate(begin + second, begin + first, begin + first + 1);
      }
      _sol.invalidate();
    }
  }

  using moIndexNeighbor<EOT, Fitness>::index;

  void set(unsigned first, unsigned second, unsigned size) {
    this->first = first;
    this->second = second;
    this->size = size;
    this->key = static_cast<unsigned>(-1);
  }

  std::pair<unsigned, unsigned> firstSecond(EOT& _sol) {
    if (static_cast<int>(key) >= 0) {
      size = _sol.size();
      translate(key + 1);
      // if (first < second)
      //   second--;
    }
    return {first, second};
  }

  /**
   * fix two indexes regarding a key
   * @param _key the key allowing to compute the two indexes for the shift
   */
  void translate(unsigned int _key) {
    int step;
    int val = _key;
    int tmpSize = size * (size - 1) / 2;
    // moves from left to right
    if (val <= tmpSize) {
      step = size - 1;
      first = 0;
      while ((val - step) > 0) {
        val = val - step;
        step--;
        first++;
      }
      second = first + val + 1;
    }
    // moves from right to left (equivalent moves are avoided)
    else { /* val > tmpSize */
      val = val - tmpSize;
      step = size - 2;
      second = 0;
      while ((val - step) > 0) {
        val = val - step;
        step--;
        second++;
      }
      first = second + val + 1;
    }
  }

  void print() {
    std::cout << key << ": [" << first << ", " << second << "] -> "
              << (*this).fitness() << std::endl;
  }

 private:
  unsigned int first;
  unsigned int second;
  unsigned int size;
};

using FSPMax = eoInt<eoMaximizingFitness>;
using FSPMin = eoInt<eoMinimizingFitness>;
using FSP = FSPMin;
using FSPNeighbor = myShiftNeighbor<FSP>;
