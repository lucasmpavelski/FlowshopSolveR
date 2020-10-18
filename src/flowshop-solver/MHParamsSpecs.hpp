#pragma once

#include <cassert>
#include <cstdlib>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <paradiseo/eo/utils/eoRealVectorBounds.h>

#include "flowshop-solver/ParamSpec.hpp"
#include "flowshop-solver/global.hpp"

class MHParamsSpecs {
 public:
  std::string mh_name;
  std::vector<std::shared_ptr<ParamSpec>> params;

  using params_container = decltype(params);
  using iterator = params_container::iterator;
  using const_iterator = params_container::const_iterator;

  MHParamsSpecs(std::string mh_name = {}, params_container params = {})
      : mh_name(std::move(mh_name)), params(std::move(params)) {
    for (auto& par : params)
      addParamToMap(par);
  }

  auto noParams() const -> int { return int(params.size()); }

  auto begin() -> iterator { return params.begin(); }
  auto end() -> iterator { return params.end(); }

  auto begin() const -> const_iterator { return params.begin(); }
  auto end() const -> const_iterator { return params.end(); }

  auto getParam(int i) const -> const std::shared_ptr<ParamSpec>& { return params[i]; }
  void addParam(std::shared_ptr<ParamSpec> ps) {
    addParamToMap(ps);
    params.emplace_back(std::move(ps));
  }

  auto mhName() const -> std::string { return mh_name; }
  void setMHName(const std::string& mn) { mh_name = mn; }

  auto noParams(ParamSpec::Type tp) const -> int { return params_counts.at(char(tp)); }
  auto noCatParams() const -> int { return noParams(ParamSpec::Type::CAT); }
  auto noIntParams() const -> int { return noParams(ParamSpec::Type::INT); }
  auto noRealParams() const -> int { return noParams(ParamSpec::Type::REAL); }
  auto noNumParams() const -> int { return noIntParams() + noRealParams(); }

  auto getIdx(const std::string& s) const -> int {
    if (params_map.find(s) == params_map.end())
      throw std::runtime_error("Unknown parameter " + s + "\n");
    return params_map.at(s);
  }

  auto isType(int i, ParamSpec::Type tp) const -> bool { return params[i]->type == tp; }
  auto isCategoric(int i) const -> bool { return isType(i, ParamSpec::Type::CAT); }
  auto isInteger(int i) const -> bool { return isType(i, ParamSpec::Type::INT); }
  auto isReal(int i) const -> bool { return isType(i, ParamSpec::Type::REAL); }
  auto isNumeric(int i) const -> bool { return isInteger(i) || isReal(i); }

  auto isType(const std::string& s, ParamSpec::Type tp) const -> bool {
    return isType(getIdx(s), tp);
  }
  auto isCategoric(const std::string& s) const -> bool {
    return isCategoric(getIdx(s));
  }
  auto isInteger(const std::string& s) const -> bool { return isInteger(getIdx(s)); }
  auto isReal(const std::string& s) const -> bool { return isReal(getIdx(s)); }
  auto isNumeric(const std::string& s) const -> bool { return isNumeric(getIdx(s)); }

  auto getValue(const std::string& param, const std::string& value) const -> int {
    int idx = getIdx(param);
    assert(isCategoric(idx));
    return spec(idx)->fromStrValue(value);
  }

  auto spec(int idx) const -> const std::shared_ptr<ParamSpec>& {
    assert(inBounds(idx, 0, noParams()));
    return params[idx];
  }

  auto spec(const std::string& s) const -> const std::shared_ptr<ParamSpec>& {
    return spec(getIdx(s));
  }

  auto operator[](unsigned i) const -> const std::shared_ptr<ParamSpec>& {
    return spec(i);
  }
  auto operator[](const std::string& s) const -> const std::shared_ptr<ParamSpec>& {
    return spec(s);
  }

  auto getLowerBounds() const -> std::vector<double> {
    std::vector<double> lbs;
    lbs.reserve(noParams());
    for (auto& param : params)
      lbs.emplace_back(param->lowerBound());
    return lbs;
  }

  auto getUpperBounds() const -> std::vector<double> {
    std::vector<double> ubs;
    ubs.reserve(noParams());
    for (auto& param : params)
      ubs.emplace_back(param->upperBound());
    return ubs;
  }

  auto getBounds() const -> eoRealVectorBounds {
    return eoRealVectorBounds(getLowerBounds(), getUpperBounds());
  }

  void print(std::ostream& o) const {
    o << "# " << mh_name << " params:\n";
    for (auto& param : params) {
      o << *param << "\n";
    }
  }

  static void read(std::istream& in, MHParamsSpecs& sps) {
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
    const auto idx = static_cast<int>(params_map.size());
    params_map[param->name] = idx;
    auto pt = char(param->type);
    params_counts[pt] = params_counts.find(pt) == params_counts.end()
                            ? 1
                            : params_counts[pt] + 1;
  }

  static auto str2Param(const std::string& str) -> std::shared_ptr<ParamSpec> {
    std::vector<std::string> splits = tokenize(str, std::string("\"()|#"));
    std::string name = trim(splits[0]);
    ParamSpec::Type type = ParamSpec::Type(trim(splits[2])[0]);
    std::vector<std::string> bounds_strs = tokenize(splits[3], ',');
    std::transform(bounds_strs.begin(), bounds_strs.end(), bounds_strs.begin(),
                   trim);
    if (type == ParamSpec::Type::CAT || type == ParamSpec::Type::ORD) {
      assert(bounds_strs.size() >= 1);
      return std::make_shared<CatParamSpec>(name, bounds_strs);
    } else {
      assert(bounds_strs.size() == 2);
      auto low = std::strtod(bounds_strs[0].c_str(), nullptr);
      auto up = std::strtod(bounds_strs[1].c_str(), nullptr);
      return std::make_shared<ParamSpec>(name, type, low, up);
    }
  }

  static auto inBounds(int x, int a, int b) -> bool { return x >= a && x < b; }

  friend auto operator<<(std::ostream& o, const MHParamsSpecs& sps) -> std::ostream& {
    sps.print(o);
    return o;
  }

  friend auto operator>>(std::istream& i, MHParamsSpecs& sps) -> std::istream& {
    MHParamsSpecs::read(i, sps);
    return i;
  }
};
