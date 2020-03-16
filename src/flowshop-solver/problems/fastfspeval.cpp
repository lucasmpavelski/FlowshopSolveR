#include "fastfspeval.hpp"

int positionPairToKey(int first, int second, int n) {
  if (first < second) {
    int t = (first + 1) * (first + 2) / 2;
    return first * n + (second - t);
  }
  int t = (second + 1) * (second + 2) / 2;
  return (n - 1) * n / 2 + second * (n - 1) + first - t;
}

std::pair<int, int> keyToPositionPair(int val, int n) {
  int step, first, second;
  int tmpSize = n * (n - 1) / 2;
  // moves from left to right
  if (val <= tmpSize) {
    step = n - 1;
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
    step = n - 2;
    second = 0;
    while ((val - step) > 0) {
      val = val - step;
      step--;
      second++;
    }
    first = second + val + 1;
  }
  return {first, second};
}