#include "fastfspeval.hpp"

int positionPairToKey(int first, int second, int n) {
  if (first - 1 == second) {
    int aux = first;
    first = second;
    second = aux;
  }
  if (first == second) {
    return -first - 2;
  }
  if (first < second) {
    int t = (first + 1) * (first + 2) / 2;
    return first * n + (second - t);
  }
  int t = (second + 1) * (second + 2) / 2;
  int res = (n - 1) * n / 2 + second * (n - 1) + first - t;
  return res;
}

std::pair<int, int> keyToPositionPair(int val, int n) {
  if (val < 0) {
    return {-val - 1, -val - 1};
  }
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