#pragma once

#include "paradiseo/eo/eoFunctor.h"
#include "paradiseo/mo/continuator/moStatBase.h"

template <class EOT>
class NeigborhoodStatBase : public moStatBase<EOT> {
 public:
  virtual void initNeighborhood(EOT& sol) {}
  virtual void neighborCall(EOT& sol) {}
  virtual void lastNeighborhoodCall(EOT& sol) {}
  virtual void operator()(EOT& sol) override {}
};

template <class EOT, class T>
class NeigborhoodStat : public eoValueParam<T>,
                        public NeigborhoodStatBase<EOT> {
 public:
  NeigborhoodStat(T _value, std::string _description)
      : eoValueParam<T>(_value, _description) {}
};
