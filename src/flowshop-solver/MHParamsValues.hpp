#pragma once

#include <string>

#include <es/eoReal.h>
#include <eo>

#include "MHParamsSpecs.hpp"

class MHParamsValues : public eoReal<eoMaximizingFitness> {
 public:
  using value_type = eoReal<eoMaximizingFitness>::value_type;
  using eoReal<eoMaximizingFitness>::operator[];
  using eoReal<eoMaximizingFitness>::begin;
  using eoReal<eoMaximizingFitness>::end;

  const MHParamsSpecs* specs;

  MHParamsValues() : eoReal<eoMaximizingFitness>(unsigned(0)), specs(nullptr) {}
  MHParamsValues(const MHParamsSpecs* specs)
      : eoReal<eoMaximizingFitness>(unsigned(specs->noParams())),
        specs(specs) {}

  double& operator[](const std::string& s) { return (*this)[specs->getIdx(s)]; }
  double operator[](const std::string& s) const {
    return (*this)[specs->getIdx(s)];
  }

  int categorical(const std::string& s) const {
    if (!specs->isCategoric(s)) {
      throw std::runtime_error("Parameter " + s + " is not categoric");
    }
    return static_cast<int>((*this)[s]);
  }

  std::string categoricalName(const std::string& s) const {
    int index = specs->getIdx(s);
    auto paramSpec = specs->getParam(index);
    return paramSpec->toStrValue(categorical(s));
  }

  int integer(const std::string& s) const {
    if (!specs->isInteger(s))
      throw std::runtime_error("Parameter " + s + " is not integer");
    return static_cast<int>((*this)[s]);
  }
  float real(const std::string& s) const {
    if (!specs->isReal(s))
      throw std::runtime_error("Parameter " + s + " is not real");
    return (*this)[s];
  }

  std::string mhName() const { return specs->mhName(); }

  template <class RNG>
  void randomizeValues(RNG& rng) {
    for (unsigned i = 0; i < size(); i++) {
      float lb = (*specs)[i]->lowerBound();
      float ub = (*specs)[i]->upperBound();
      std::uniform_real_distribution<float> dist(lb, ub);
      this->at(i) = dist(rng);
    }
  }

  std::ostream& printValues(std::ostream& out) const {
    for (unsigned i = 0; i < size(); i++) {
      auto spec = (*specs)[i];
      spec->strValue(out, operator[](i)) << '\t';
    }
    return out;
  }

  void readValues(std::unordered_map<std::string, double> values) {
    for (auto ps : *specs) {
      if (values.find(ps->name) == values.end())
        throw std::runtime_error("Parameter " + ps->name + " needs a value!");
      this->operator[](ps->name) = values.at(ps->name);
    }
  }

  std::unordered_map<std::string, double> toMap() const {
    std::unordered_map<std::string, double> values;
    for (auto ps : *specs)
      values[ps->name] = this->operator[](ps->name);
    return values;
  }
};
