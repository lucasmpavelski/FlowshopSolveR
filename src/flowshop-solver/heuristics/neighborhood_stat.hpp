#pragma once

#include "paradiseo/eo/eoFunctor.h"
#include "paradiseo/mo/continuator/moStatBase.h"

template <class EOT>
class NeigborhoodStatBase : public moStatBase<EOT> {
 public:
  virtual void initNeighborhood(EOT&) {}
  virtual void neighborCall(EOT& ) {}
  virtual void lastNeighborhoodCall(EOT&) {}
  virtual void operator()(EOT&) override {}
};

template <class EOT, class T>
class NeigborhoodStat : public eoValueParam<T>,
                        public NeigborhoodStatBase<EOT> {
 public:
  NeigborhoodStat(T _value, std::string _description)
      : eoValueParam<T>(_value, _description) {}
};
