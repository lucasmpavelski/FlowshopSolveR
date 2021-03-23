#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

template <class Ngh, class EOT = typename Ngh::EOT>
class InitLocalSearch : public eoInit<EOT> {
  eoInit<EOT>& init;
  moLocalSearch<Ngh>& localSearch;

 public:
  InitLocalSearch(eoInit<EOT>& init, moLocalSearch<Ngh>& localSearch)
      : init(init), localSearch(localSearch) {}

  void operator()(EOT& sol) {
    init(sol);
    localSearch(sol);
  }
};