#pragma once

#include <neighborhood/moIndexNeighbor.h>

/**
 * Indexed Shift Neighbor
 */
template <class EOT, class Fitness = typename EOT::Fitness>
class fspShiftNeighbor : public moIndexNeighbor<EOT, Fitness> {
 public:
  using moIndexNeighbor<EOT, Fitness>::key;

  fspShiftNeighbor<EOT, Fitness>& operator=(
      const fspShiftNeighbor<EOT, Fitness>& _source) {
    moIndexNeighbor<EOT, Fitness>::operator=(_source);
    this->first = _source.first;
    this->second = _source.second;
    this->size = _source.size;
    return *this;
  }

  /**
   * Apply move on a solution regarding a key
   * @param _sol the solution to move
   */
  void move(EOT& _sol) override {
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
  }

  /**
   * fix two indexes regarding a key
   * @param _key the key allowing to compute the two indexes for the shift
   */
  void translate(unsigned int _key) {
    for (int a = 0; a < size; a++) {
      for (int b = 0; b < size; b++) {
        if (b == a || b == a - 1)
          continue;
        if (--_key == 0) {
          first = a;
          second = b;
        }
      }
    }
    /* int step;
     int val = _key;
     int tmpSize = size * (size-1) / 2;
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
     else {  // val > tmpSize
         val = val - tmpSize;
         step = size - 2;
         second = 0;
         while ((val - step) > 0) {
             val = val - step;
             step--;
             second++;
         }
         first = second + val + 1;
     }*/
  }

  void print() {
    std::cout << key << ": [" << first << ", " << second << "] -> "
              << (*this).fitness() << std::endl;
  }

  /**
   * Getter
   * @return index of the IndexNeighbor
   */
  inline unsigned int index() const { return key; }

  /**
   * Setter :
   * Only set the index which not depends on the current solution
   *
   * @param _key index of the IndexNeighbor
   */
  void index(unsigned int _key) { key = _key; }

  void index(EOT& _solution, unsigned int _key) final {
    key = _key;
    size = _solution.size();
  }

  void setSize(unsigned size) { this->size = size; }

  void setPositions(int first, int second) {
    this->first = first;
    this->second = second;
    this->key = 0;
    for (int a = 0; a < size; a++) {
      for (int b = 0; b < size; b++) {
        if (b == a || b == a - 1)
          continue;
        if (a == first && b == second) {
          return;
        }
        this->key++;
      }
    }
  }

  unsigned getFirst() const { return first; }

  unsigned getSecond() const { return second; }

 private:
  unsigned int first;
  unsigned int second;
  unsigned int size;
};
