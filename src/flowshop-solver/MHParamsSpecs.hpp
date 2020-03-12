#pragma once

#include <cassert>
#include <cstdlib>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "paradiseo/eo/utils/eoRealVectorBounds.h"

#include "ParamSpec.hpp"
#include "global.hpp"

class MHParamsSpecs {
public:
  std::string mh_name;
  std::vector<std::shared_ptr<ParamSpec>> params;

  using params_container = decltype(params);
  using iterator         = params_container::iterator;
  using const_iterator   = params_container::const_iterator;

  MHParamsSpecs(std::string mh_name = {}, params_container params = {})
      : mh_name(std::move(mh_name)), params(std::move(params)) {
    for (auto &par : params)
      addParamToMap(par);
  }

  int noParams() const { return int(params.size()); }

  iterator begin() { return params.begin(); }
  iterator end() { return params.end(); }

  const_iterator begin() const { return params.begin(); }
  const_iterator end() const { return params.end(); }

  const std::shared_ptr<ParamSpec> &getParam(int i) const { 
    return params[i]; 
  }
  void addParam(std::shared_ptr<ParamSpec> ps) {
    addParamToMap(ps);
    params.emplace_back(std::move(ps));
  }

  std::string mhName() const { return mh_name; }
  void setMHName(std::string mn) { mh_name = mn; }

  int noParams(ParamSpec::Type tp) const { return params_counts.at(char(tp)); }
  int noCatParams() const { return noParams(ParamSpec::Type::CAT); }
  int noIntParams() const { return noParams(ParamSpec::Type::INT); }
  int noRealParams() const { return noParams(ParamSpec::Type::REAL); }
  int noNumParams() const { return noIntParams() + noRealParams(); }

  int getIdx(const std::string &s) const {
    if (params_map.find(s) == params_map.end())
      throw std::runtime_error("Unknown parameter " + s + "\n");
    return params_map.at(s);
  }

  bool isType(int i, ParamSpec::Type tp) const { return params[i]->type == tp; }
  bool isCategoric(int i) const { return isType(i, ParamSpec::Type::CAT); }
  bool isInteger(int i) const { return isType(i, ParamSpec::Type::INT); }
  bool isReal(int i) const { return isType(i, ParamSpec::Type::REAL); }
  bool isNumeric(int i) const { return isInteger(i) || isReal(i); }

  bool isType(const std::string &s, ParamSpec::Type tp) const {
    return isType(getIdx(s), tp);
  }
  bool isCategoric(const std::string &s) const {
    return isCategoric(getIdx(s));
  }
  bool isInteger(const std::string &s) const { return isInteger(getIdx(s)); }
  bool isReal(const std::string &s) const { return isReal(getIdx(s)); }
  bool isNumeric(const std::string &s) const { return isNumeric(getIdx(s)); }

  int getValue(const std::string &param, const std::string& value) const {
    int idx = getIdx(param);
    assert(isCategoric(idx));
    return spec(idx)->fromStrValue(value);
  }

  const std::shared_ptr<ParamSpec> &spec(int idx) const {
    assert(inBounds(idx, 0, noParams()));
    return params[idx];
  }

  const std::shared_ptr<ParamSpec> &spec(const std::string &s) const {
    return spec(getIdx(s));
  }

  const std::shared_ptr<ParamSpec> &operator[](int i) const { return spec(i); }
  const std::shared_ptr<ParamSpec> &operator[](const std::string &s) const {
    return spec(s);
  }

  std::vector<double> getLowerBounds() const {
    std::vector<double> lbs;
    lbs.reserve(noParams());
    for (auto &param : params)
      lbs.emplace_back(param->lowerBound());
    return lbs;
  }

  std::vector<double> getUpperBounds() const {
    std::vector<double> ubs;
    ubs.reserve(noParams());
    for (auto &param : params)
      ubs.emplace_back(param->upperBound());
    return ubs;
  }

  eoRealVectorBounds getBounds() const {
    return eoRealVectorBounds(getLowerBounds(), getUpperBounds());
  }

  void print(std::ostream &o) const {
    o << "# " << mh_name << " params:\n";
    for (auto &param : params) {
      o << *param << "\n";
    }
  }

  static void read(std::istream &in, MHParamsSpecs &sps) {
    while (in.good()) {
      std::string line;
      std::getline(in, line, '\n');
      line = trim(line);
      if (line.empty() || line[0] == '#')
        continue;
      sps.addParam(str2Param(line));
    }
  }

private:
  std::unordered_map<std::string, int> params_map;
  std::unordered_map<char, int> params_counts;

  void addParamToMap(const std::shared_ptr<ParamSpec>& param) {
    const int idx = int(params_map.size());
    params_map[param->name] = idx;
    auto pt                 = char(param->type);
    params_counts[pt]       = params_counts.find(pt) == params_counts.end()
                            ? 1
                            : params_counts[pt] + 1;
  }

  static std::shared_ptr<ParamSpec> str2Param(const std::string &str) {
    std::vector<std::string> splits = tokenize(str, std::string("\"()|#"));
    // example: ILS.Accept.Temperature  "" r (0.0, 0.5)  | ILS.Accept == 2 # hello
    // split[0] == name
    // split[1] == par_switch (unused)
    // split[2] == type
    // split[3] == bounds or values
    // split[4] == conditionals (unused)
    // split[5] == commentary (unused)
    std::string name = trim(splits[0]);
    ParamSpec::Type type = ParamSpec::Type(trim(splits[2])[0]);
    std::vector<std::string> bounds_strs = tokenize(splits[3], ',');
    std::transform(bounds_strs.begin(), bounds_strs.end(), bounds_strs.begin(), trim);
    if (type == ParamSpec::Type::CAT) {
      assert(bounds_strs.size() >= 1);
      return std::make_shared<CatParamSpec>(name, bounds_strs);
    } else {
      assert(bounds_strs.size() == 2);
      auto low = std::strtod(bounds_strs[0].c_str(), nullptr);
      auto up  = std::strtod(bounds_strs[1].c_str(), nullptr);
      return std::make_shared<ParamSpec>(name, type, low, up);
    }
    /*std::istringstream ss(str);
    std::string name, par_switch, type_str;
    std::getline(ss, name, ' ');
    while (ss.good() && ss.peek() != '\"')
      ss.get();
    std::getline(ss, par_switch, '\"'); // unused
    ss.get();
    while (ss.good() && ss.peek() == ' ')
      ss.get();
    std::getline(ss, type_str, ' ');
    auto type          = ParamSpec::Type(type_str[0]);
    std::string bounds = ss.str();
    auto beg           = bounds.find('(');
    auto end           = bounds.find(')');
    bounds             = bounds.substr(beg + 1, end - beg - 1);
    auto bounds_strs   = tokenize(bounds, ',');
    */
  }

  static bool inBounds(int x, int a, int b) { return x >= a && x < b; }

  friend std::ostream &operator<<(std::ostream &o, const MHParamsSpecs &sps) {
    sps.print(o);
    return o;
  }

  friend std::istream &operator>>(std::istream &i, MHParamsSpecs &sps) {
    MHParamsSpecs::read(i, sps);
    return i;
  }
};
