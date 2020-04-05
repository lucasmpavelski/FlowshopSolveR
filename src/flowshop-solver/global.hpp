#pragma once

// Std includes
#include <algorithm>
#include <cassert>
#include <chrono>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// Paradiseo includes
#include "paradiseo/eo/utils/eoRNG.h"
#include "paradiseo/eo/utils/eoStdoutMonitor.h"

template <class EnumT, class Number>
EnumT fromNumeric(Number n) {
  return static_cast<EnumT>(n);  //(std::underlying_type<EnumT>(n));
}

template <class EnumT>
int asInt(EnumT e) {
  return static_cast<int>(e);
}

struct EnumClassHash {
  template <typename T>
  std::size_t operator()(T t) const {
    return asInt(t);
  }
};

template <class T>
using EnumToStrT = std::unordered_map<T, std::string, EnumClassHash>;

template <class T>
using StrToEnumT = std::unordered_map<std::string, T>;

template <class T>
StrToEnumT<T> StrToEnumMap();

template <class T>
EnumToStrT<T> EnumToStrMap() {
  return flipEnumMap(StrToEnumMap<T>());
};

template <class T>
EnumToStrT<T> flipEnumMap(const StrToEnumT<T>& map) {
  EnumToStrT<T> reversed;
  for (auto p : map)
    reversed[p.second] = p.first;
  return reversed;
}

template <class T>
T strToEnum(const std::string& s) {
  return StrToEnumMap<T>().at(s);
};

template <class T>
std::string enumToStr(const T& e) {
  return EnumToStrMap<T>().at(e);
};

class prefixedPrinter : public eoStdoutMonitor {
 public:
  prefixedPrinter(std::string _prefix,
                  std::string _delim = ",",
                  unsigned int _width = 20,
                  char _fill = ' ')
      : eoStdoutMonitor{_delim, _width, _fill}, prefix{_prefix, "prefix"} {
    // add(prefix);
  }

  eoValueParam<std::string> prefix;
};

template <class Itr, class Val = typename std::iterator_traits<Itr>::value_type>
inline Val sum(Itr b, Itr e) {
  return std::accumulate(b, e, Val(0));
}

template <typename T>
inline T sum(const T x[], const size_t n) {
  return std::accumulate(x, x + n, T(0));
}

template <typename T>
inline T mean(const T x[], const size_t n) {
  T s = sum(x, n);
  return s / T(n);
}

template <typename Itr1, typename Itr2>
class printableSeq {
  Itr1 beg, end;
  const std::string sep;

 public:
  printableSeq(Itr1 beg, Itr2 end, const std::string& sep = " ")
      : beg(beg), end(end), sep(sep) {}

  std::ostream& operator()(std::ostream& os = std::cout) const {
    return os << *this;
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  const printableSeq<Itr1, Itr2>& pa) {
    if (pa.beg != pa.end)
      os << *pa.beg;
    Itr1 p = pa.beg;
    while (++p != pa.end)
      os << pa.sep << *p;
    return os;
  }
};

template <typename Container,
          class Itr1 = typename Container::const_iterator,
          class Itr2 = typename Container::const_iterator>
printableSeq<Itr1, Itr2> printSeq(const Container& c,
                                  const std::string& sep = " ") {
  return printableSeq<Itr1, Itr2>(c.cbegin(), c.cend(), sep);
}

template <typename Itr1, typename Itr2>
printableSeq<Itr1, Itr2> printSeq(Itr1 beg,
                                  Itr2 end,
                                  const std::string& sep = " ") {
  return printableSeq<Itr1, Itr2>(beg, end, sep);
}

template <typename Itr1>
printableSeq<Itr1, Itr1> printSeq_n(Itr1 beg,
                                    const size_t n,
                                    const std::string& sep = " ") {
  return printableSeq<Itr1, Itr1>(beg, beg + n, sep);
}

template <typename TimeT = std::chrono::milliseconds>
struct Measure {
  template <typename F>
  static typename TimeT::rep execution(F const& func) {
    auto start = std::chrono::system_clock::now();
    func();
    auto duration = std::chrono::duration_cast<TimeT>(
        std::chrono::system_clock::now() - start);
    return duration.count();
  }
};

template <class T = uint32_t>
struct ParadiseoRNGFunctor {
  using result_type = T;
  T operator()() { return rng.rand(); }
  T min() { return 0; }
  T max() { return rng.rand_max(); }
};

struct RNG {
  static std::mt19937_64 engine;
  static std::random_device true_rand_engine;

  static long seed(long s = true_rand_engine());

  inline static long trueRandom() { return true_rand_engine(); }

  inline static long pseudoRandom() { return engine(); };

  template <typename RealT = double>
  inline static RealT realUniform(const RealT& from = 0.0,
                                  const RealT& to = 1.0) {
    std::uniform_real_distribution<RealT> uniform_dist(from, to);
    return uniform_dist(engine);
  }

  inline static int intUniform(int from, int to) {
    std::uniform_int_distribution<int> uniform_dist(from, to);
    return uniform_dist(engine);
  }

  inline static int intUniform(const int until = 100) {
    return intUniform(0, until);
  }

  inline static bool flipCoin() {
    static std::bernoulli_distribution dist(0.5);
    return dist(engine);
  }

  static long saved_seed;
  static bool is_saved;

  static inline void saveRNGState() {
    if (is_saved) {
      throw std::logic_error(
          "Trying to save another seed when the previous was "
          "not restored!");
    }
    saved_seed = engine();
    is_saved = true;
  }

  static inline void restoreRNGState() {
    if (!is_saved) {
      throw std::logic_error("Trying restore when the seed is not saved!");
    }
    seed(saved_seed);
    is_saved = false;
  }

  struct Guard {
    Guard() { saveRNGState(); }
    ~Guard() { restoreRNGState(); }
  };

  struct SeedPool {
    static std::unordered_map<int, long> seeds;
    static long get(int i) {
      if (seeds.count(i) == 0)
        seeds[i] = engine();
      return seeds[i];
    }
  };
};

template <class T = std::string>
std::vector<T> tokenize(const T& s, char delim = ',') {
  std::stringstream ss(s);
  std::vector<T> result;
  while (ss.good()) {
    std::string substr;
    std::getline(ss, substr, delim);
    result.push_back(substr);
  }
  return result;
}

template <class T = std::string>
std::vector<T> tokenize(const T& s, const T& delims) {
  std::vector<T> res = tokenize(s, delims[0]);
  for (char delim : delims) {
    std::vector<T> temp = res;
    res.clear();
    for (const auto& s : temp) {
      auto tokens = tokenize(s, delim);
      std::copy(tokens.begin(), tokens.end(), std::back_inserter(res));
    }
  }
  return res;
}

// template <typename K, typename V>
// V getWithDef(const std::unordered_map<K,V> & m, const K & key, const V &
// defval ) {
//  auto it = m.find(key);
//  return it == m.end() ? defval : it->second;
//}

template <typename Map,
          typename Key = typename Map::key_type,
          typename Val = typename Map::mapped_type>
Val getWithDef(const Map& m, const Key& key, const Val& defval) {
  auto it = m.find(key);
  return it == m.end() ? defval : it->second;
}

std::vector<std::string> getNextLineAndSplitIntoTokens(std::istream& str);

class CSVRow {
 public:
  std::string const& operator[](std::size_t index) const {
    return m_data[index];
  }
  std::size_t size() const { return m_data.size(); }
  void readNexthrow(std::istream& str) {
    std::string line;
    std::getline(str, line);

    std::stringstream lineStream(line);
    std::string cell;

    m_data.clear();
    while (std::getline(lineStream, cell, ',')) {
      m_data.push_back(cell);
    }
    // This checks for a trailing comma with no data after it.
    if (!lineStream && cell.empty()) {
      // If there was a trailing comma then add an empty element.
      m_data.push_back("");
    }
  }

 private:
  std::vector<std::string> m_data;
};

std::istream& operator>>(std::istream& str, CSVRow& data);

// trim from start (in place)
static inline void ltrim(std::string& s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                  [](int ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](int ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

// trim from both ends
static inline std::string trim(std::string s) {
  ltrim(s);
  rtrim(s);
  return s;
}

template <class T, class Compare>
constexpr const T& clamp(const T& v, const T& lo, const T& hi, Compare comp) {
  return assert(!comp(hi, lo)), comp(v, lo) ? lo : comp(hi, v) ? hi : v;
}

template <class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
  return (v < lo) ? lo : (v > hi) ? hi : v;
  // clamp(v, lo, hi, std::less<T>());
}

long factorial(unsigned n);

struct AssertionFailureException : public std::exception {
  using string = std::string;

 private:
  const char* expression;
  const char* file;
  int line;
  string message;
  string report;

 public:
  AssertionFailureException(const char* expression,
                            const char* file,
                            const int line,
                            const std::string& message);

  virtual const char* what() const noexcept { return report.c_str(); }
  const char* getExpression() const noexcept { return expression; }
  const char* getFile() const noexcept { return file; }
  int getLine() const noexcept { return line; }
  const char* Message() const noexcept { return message.c_str(); }
};

/// Assert that EXPRESSION evaluates to true, otherwise raise
/// AssertionFailureException with associated MESSAGE (which may use C++
/// stream-style message formatting)
#define throw_assert(EXPRESSION, MESSAGE)                                 \
  if (EXPRESSION) {                                                       \
  } else                                                                  \
    throw AssertionFailureException(                                      \
        #EXPRESSION, __FILE__, __LINE__,                                  \
        static_cast<std::ostringstream&>(std::ostringstream() << MESSAGE) \
            .str())
