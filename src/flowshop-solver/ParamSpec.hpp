#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

/***
 * Abstract parameter specification
 */
class ParamSpec {
 public:
  float lower_bound, upper_bound;
  std::string name;
  enum class Type : char { CAT = 'c', INT = 'i', REAL = 'r', ORD = 'o' } type;

  ParamSpec(std::string _name,
            Type _type,
            float _lower_bound,
            float _upper_bound)
      : lower_bound(_lower_bound),
        upper_bound(_upper_bound),
        name(std::move(_name)),
        type(_type) {}

  virtual ~ParamSpec() = default;

  void print(std::ostream& o) const {
    o << name << "\t";
    o << "\"\"\t";
    o << char(type) << "\t";
    printRange(o);
  }

  virtual void printRange(std::ostream& o) const {
    o << "(" << lowerBound() << "," << upperBound() << ")";
  }

  [[nodiscard]] auto lowerBound() const -> float { return lower_bound; }
  [[nodiscard]] auto upperBound() const -> float { return upper_bound; }

  auto strValue(std::ostream& o, float num) const -> std::ostream& {
    return o << toStrValue(num);
  }

  [[nodiscard]] virtual auto toStrValue(float num) const -> std::string {
    switch (type) {
      case Type::REAL:
        return std::to_string(num);
      case Type::INT:
      case Type::CAT:
      case Type::ORD:
        return std::to_string(int(num));
    }
    return "";
  }

  [[nodiscard]] virtual auto fromStrValue(const std::string& s) const -> float {
    try {
      if (s == "NA")
        return std::numeric_limits<float>::quiet_NaN();
      return std::stof(s.c_str());
    } catch (std::invalid_argument e) {
      std::cerr << "Value " << s
                << " cannot be converted to values for paramater " << name;
      throw std::move(e);
    }
  }

  friend auto operator<<(std::ostream& o, const ParamSpec& ps)
      -> std::ostream& {
    ps.print(o);
    return o;
  }

  friend auto operator==(const ParamSpec& a, const ParamSpec& b) -> bool {
    return a.name == b.name && a.type == b.type &&
           a.lower_bound == b.lower_bound && a.upper_bound == b.upper_bound;
  }

  friend auto operator!=(const ParamSpec& a, const ParamSpec& b) -> bool {
    return !(a == b);
  }
};

class CatParamSpec : public ParamSpec {
 public:
  std::vector<std::string> cats;

  CatParamSpec(std::string name, const std::vector<std::string>& cats)
      : ParamSpec(std::move(name),
                  ParamSpec::Type::CAT,
                  0.0f,
                  cats.size() - 1e-6f),
        cats(cats) {}

  void printRange(std::ostream& o) const override {
    o << "(";
    for (int i = 0; i < int(cats.size()) - 1; i++)
      o << cats[i] << ", ";
    o << cats[cats.size() - 1] << ")";
  }

  [[nodiscard]] auto toStrValue(float num) const -> std::string override {
    if (num < 0)
      throw std::runtime_error("Parameter value not found for " + this->name);
    return cats[int(num)];
  }

  [[nodiscard]] auto fromStrValue(const std::string& s) const
      -> float override {
    auto r = std::find(cats.begin(), cats.end(), s);
    if (r != cats.end())
      return std::distance(cats.begin(), r);
    return -1.0;
  }
};
