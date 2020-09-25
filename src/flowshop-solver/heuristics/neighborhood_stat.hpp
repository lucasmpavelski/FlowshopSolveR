#pragma once

#include <paradiseo/mo/mo>
#include <paradiseo/eo/eo>

template <class Ngh, class EOT = typename Ngh::EOT>
class NeigborhoodStatBase : public moStatBase<EOT> {
 public:
  virtual void initNeighborhood(EOT&) {}
  virtual void neighborCall(Ngh&) {}
  virtual void lastNeighborhoodCall(EOT&) {}
  void operator()(EOT&) override {}
};

template <class EOT, class T>
class NeigborhoodStat : public eoValueParam<T>,
                        public NeigborhoodStatBase<EOT> {
 public:
  NeigborhoodStat(T _value, const std::string& _description)
      : eoValueParam<T>(_value, _description) {}
};
