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
#include <utility>
#include <vector>

// Paradiseo includes
#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

template <class EnumT, class Number>
auto fromNumeric(Number n) -> EnumT {
  return static_cast<EnumT>(n);  //(std::underlying_type<EnumT>(n));
}

template <class EnumT>
auto asInt(EnumT e) -> int {
  return static_cast<int>(e);
}

struct EnumClassHash {
  template <typename T>
  auto operator()(T t) const -> std::size_t {
    return asInt(t);
  }
};

template <class T>
using EnumToStrT = std::unordered_map<T, std::string, EnumClassHash>;

template <class T>
using StrToEnumT = std::unordered_map<std::string, T>;

template <class T>
auto StrToEnumMap() -> StrToEnumT<T>;

template <class T>
auto EnumToStrMap() -> EnumToStrT<T> {
  return flipEnumMap(StrToEnumMap<T>());
};

template <class T>
auto flipEnumMap(const StrToEnumT<T>& map) -> EnumToStrT<T> {
  EnumToStrT<T> reversed;
  for (auto p : map)
    reversed[p.second] = p.first;
  return reversed;
}

template <class T>
auto strToEnum(const std::string& s) -> T {
  return StrToEnumMap<T>().at(s);
};

template <class T>
auto enumToStr(const T& e) -> std::string {
  return EnumToStrMap<T>().at(e);
};

class prefixedPrinter : public eoStdoutMonitor {
 public:
  prefixedPrinter(std::string _prefix,
                  std::string _delim = ",",
                  unsigned int _width = 20,
                  char _fill = ' ')
      : eoStdoutMonitor{std::move(_delim), _width, _fill},
        prefix{std::move(_prefix), "prefix"} {
    // add(prefix);
  }

  eoValueParam<std::string> prefix;
};

template <class Itr, class Val = typename std::iterator_traits<Itr>::value_type>
inline auto sum(Itr b, Itr e) -> Val {
  return std::accumulate(b, e, Val(0));
}

template <typename T>
inline auto sum(const T x[], const size_t n) -> T {
  return std::accumulate(x, x + n, T(0));
}

template <typename T>
inline auto mean(const T x[], const size_t n) -> T {
  T s = sum(x, n);
  return s / T(n);
}

template <typename Itr1, typename Itr2>
class printableSeq {
  Itr1 beg, end;
  const std::string sep;

 public:
  printableSeq(Itr1 beg, Itr2 end, std::string sep = " ")
      : beg(beg), end(end), sep(std::move(sep)) {}

  auto operator()(std::ostream& os = std::cout) const -> std::ostream& {
    return os << *this;
  }

  friend auto operator<<(std::ostream& os, const printableSeq<Itr1, Itr2>& pa)
      -> std::ostream& {
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
auto printSeq(const Container& c, const std::string& sep = " ")
    -> printableSeq<Itr1, Itr2> {
  return printableSeq<Itr1, Itr2>(c.cbegin(), c.cend(), sep);
}

template <typename Itr1, typename Itr2>
auto printSeq(Itr1 beg, Itr2 end, const std::string& sep = " ")
    -> printableSeq<Itr1, Itr2> {
  return printableSeq<Itr1, Itr2>(beg, end, sep);
}

template <typename Itr1>
auto printSeq_n(Itr1 beg, const size_t n, const std::string& sep = " ")
    -> printableSeq<Itr1, Itr1> {
  return printableSeq<Itr1, Itr1>(beg, beg + n, sep);
}

template <typename TimeT = std::chrono::milliseconds>
struct Measure {
  template <typename F>
  static auto execution(F const& func) -> typename TimeT::rep {
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
  auto operator()() -> T { return rng.rand(); }
  auto min() -> T { return 0; }
  auto max() -> T { return rng.rand_max(); }
};

struct RNG {
  static std::mt19937_64 engine;
  static std::random_device true_rand_engine;

  static auto seed(long s = true_rand_engine()) -> long {
    // my own RNG
    engine.seed(s);
    // C RNG
    srand(unsigned(s));
    // ParadisEO RNG
    rng.reseed(uint32_t(s));
    return s;
  };

  inline static auto trueRandom() -> long { return true_rand_engine(); }

  inline static auto pseudoRandom() -> long { return engine(); };

  template <typename RealT = double>
  inline static auto realUniform(const RealT& from = 0.0, const RealT& to = 1.0)
      -> RealT {
    std::uniform_real_distribution<RealT> uniform_dist(from, to);
    return uniform_dist(engine);
  }

  inline static auto intUniform(int from, int to) -> int {
    std::uniform_int_distribution<int> uniform_dist(from, to);
    return uniform_dist(engine);
  }

  inline static auto intUniform(const int until = 100) -> int {
    return intUniform(0, until);
  }

  inline static auto flipCoin() -> bool {
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
    ~Guard() noexcept {
      try {
        restoreRNGState();
      } catch (std::exception e) {
        std::cerr << e.what();
        std::exit(1);
      }
    }
  };

  struct SeedPool {
    static std::unordered_map<int, long> seeds;
    static auto get(int i) -> long {
      if (seeds.count(i) == 0)
        seeds[i] = engine();
      return seeds[i];
    }
  };
};

template <class T = std::string>
auto tokenize(const T& s, char delim = ',') -> std::vector<T> {
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
auto tokenize(const T& s, const T& delims) -> std::vector<T> {
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
auto getWithDef(const Map& m, const Key& key, const Val& defval) -> Val {
  auto it = m.find(key);
  return it == m.end() ? defval : it->second;
}

inline auto getNextLineAndSplitIntoTokens(std::istream& str)
    -> std::vector<std::string> {
  std::vector<std::string> result;
  std::string line;
  std::getline(str, line);

  std::stringstream lineStream(line);
  std::string cell;

  while (std::getline(lineStream, cell, ',')) {
    result.push_back(cell);
  }
  // This checks for a trailing comma with no data after it.
  if (!lineStream && cell.empty()) {
    // If there was a trailing comma then add an empty element.
    result.emplace_back("");
  }
  return result;
}

class CSVRow {
 public:
  auto operator[](std::size_t index) const -> std::string const& {
    return m_data[index];
  }
  auto size() const -> std::size_t { return m_data.size(); }
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
      m_data.emplace_back("");
    }
  }

  friend auto operator>>(std::istream& str, CSVRow& data) -> std::istream& {
    data.readNexthrow(str);
    return str;
  }

 private:
  std::vector<std::string> m_data;
};

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
[[maybe_unused]] static inline auto trim(std::string s) -> std::string {
  ltrim(s);
  rtrim(s);
  return s;
}

template <class T, class Compare>
constexpr auto clamp(const T& v, const T& lo, const T& hi, Compare comp)
    -> const T& {
  return assert(!comp(hi, lo)), comp(v, lo) ? lo : comp(hi, v) ? hi : v;
}

template <class T>
constexpr auto clamp(const T& v, const T& lo, const T& hi) -> const T& {
  return (v < lo) ? lo : (v > hi) ? hi : v;
  // clamp(v, lo, hi, std::less<T>());
}

inline auto factorial(unsigned n) -> long {
  long ret = 1;
  while (n >= 2)
    ret *= n--;
  return ret;
}

template <class T>
auto contains(const std::vector<T>& vec, const T& obj) -> bool {
  return std::find(vec.begin(), vec.end(), obj) != vec.end();
}