#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

/***
 * Abstract parameter specification
 */
class ParamSpec {
 public:
  float lower_bound, upper_bound;
  std::string name;
  enum class Type : char { CAT = 'c', INT = 'i', REAL = 'r' } type;

  ParamSpec(std::string _name,
            Type _type,
            float _lower_bound,
            float _upper_bound)
      : lower_bound(_lower_bound),
        upper_bound(_upper_bound),
        name(_name),
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

  float lowerBound() const { return lower_bound; }
  float upperBound() const { return upper_bound; }

  std::ostream& strValue(std::ostream& o, float num) const {
    return o << toStrValue(num);
  }

  virtual std::string toStrValue(float num) const {
    switch (type) {
      case Type::REAL:
        return std::to_string(num);
      case Type::INT:
      case Type::CAT:
        return std::to_string(int(num));
    }
    return "";
  }

  virtual float fromStrValue(std::string s) const {
    switch (type) {
      case Type::REAL:
        return std::stof(s.c_str());
      case Type::INT:
      case Type::CAT:
        return std::stoi(s.c_str());
    }
    return 0.0;
  }

  friend std::ostream& operator<<(std::ostream& o, const ParamSpec& ps) {
    ps.print(o);
    return o;
  }

  friend bool operator==(const ParamSpec& a, const ParamSpec& b) {
    return a.name == b.name && a.type == b.type &&
           a.lower_bound == b.lower_bound && a.upper_bound == b.upper_bound;
  }

  friend bool operator!=(const ParamSpec& a, const ParamSpec& b) {
    return !(a == b);
  }
};

/// std::ostream &operator<<(std::ostream &o, const ParamSpec::Type &t) {
///  switch (t) {
///  case ParamSpec::Type::CAT: o << 'c'; break;
///  case ParamSpec::Type::INT: o << 'i'; break;
///  case ParamSpec::Type::REAL: o << 'r'; break;
///  }
///  return o;
///}

class CatParamSpec : public ParamSpec {
 public:
  std::vector<std::string> cats;

  CatParamSpec(std::string name, std::vector<std::string> cats)
      : ParamSpec(name, ParamSpec::Type::CAT, 0, cats.size() - 1e-6),
        cats(cats) {}

  virtual ~CatParamSpec() = default;

  void printRange(std::ostream& o) const override {
    o << "(";
    for (int i = 0; i < int(cats.size()) - 1; i++)
      o << cats[i] << ", ";
    o << cats[cats.size() - 1] << ")";
  }

  std::string toStrValue(float num) const override { return cats[int(num)]; }

  float fromStrValue(std::string s) const override {
    auto r = std::find(cats.begin(), cats.end(), s);
    if (r != cats.end())
      return std::distance(cats.begin(), r);
    return -1.0;
  }
};
