#pragma once

#include <vector>

#include <paradiseo/eo/eo>

template <class VecT>
class PositionSelector : public eoFunctorBase {
public:
  virtual void init(const VecT&) {}
  virtual auto select(const VecT&) -> int = 0;
  virtual void feedback(const double) {};
};