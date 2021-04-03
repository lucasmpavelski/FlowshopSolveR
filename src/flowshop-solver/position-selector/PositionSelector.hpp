#pragma once

#include <vector>

#include <paradiseo/eo/eo>

class PositionSelector : public eoFunctorBase {
public:
  virtual void init(const std::vector<int>&) {}
  virtual auto select(const std::vector<int>&) -> int = 0;
  virtual void feedback(const double) {};
};