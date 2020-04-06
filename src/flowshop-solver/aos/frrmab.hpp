#ifndef FRRMAB_H
#define FRRMAB_H

#include <algorithm>
#include <vector>

#include "adaptive_operator_selection.hpp"
#include "global.hpp"

template <class T>
struct SlidingWindow {
  using size_type = typename std::vector<T>::size_type;
  using difference_type = typename std::vector<T>::difference_type;
  using value_type = T;

 private:
  std::vector<T> data;
  int curr_begin;

 public:
  template <class Itr, class ValT>
  class iterator {
    Itr begin_itr;
    Itr end_itr;
    Itr curr;
    bool done;

   public:
    using self_type = iterator;
    using reference = T&;
    using value_type = ValT;
    using iterator_category = std::forward_iterator_tag;
    using difference_type = int;

    iterator(Itr begin, Itr end, int curr_pos, bool finish)
        : begin_itr(begin),
          end_itr(end),
          curr(begin + curr_pos),
          done(finish) {}

    auto begin() -> self_type { return begin_itr; }
    auto end() -> self_type { return end_itr; }

    auto operator++() -> self_type {
      curr++;
      if (curr == end_itr) {
        curr = begin_itr;
        done = true;
      }
      return *this;
    }

    auto operator++(int) -> self_type { return operator++(); }

    auto operator==(const self_type& rhs) const -> bool {
      return curr == rhs.curr && done == rhs.done;
    }

    auto operator!=(const self_type& rhs) const -> bool {
      return curr != rhs.curr || done != rhs.done;
    }

    auto operator-> () -> Itr { return curr; }
    auto operator*() -> reference { return *curr; }
  };

  SlidingWindow(const int size, const T& init)
      : data(size, init), curr_begin(0) {}

  SlidingWindow(const std::initializer_list<T>& init)
      : data(init), curr_begin(0) {}

  using it_type = typename std::vector<T>::iterator;

  auto begin() -> iterator<it_type, T> {
    return iterator<it_type, T>(data.begin(), data.end(), curr_begin, false);
  }
  auto end() -> iterator<it_type, T> {
    return iterator<it_type, T>(data.begin(), data.end(), curr_begin, true);
  }

  auto begin() const -> iterator<it_type, const T> {
    return iterator<it_type, const T>(data.begin(), data.end(), curr_begin,
                                      false);
  }
  auto end() const -> iterator<it_type, const T> {
    return iterator<it_type, const T>(data.begin(), data.end(), curr_begin,
                                      true);
  }

  auto size() const -> std::size_t { return data.size(); }

  auto operator[](const size_type& index) -> T& { return data[index]; }
  auto operator[](const size_type& index) const -> const T& {
    return data[index];
  }

  void append(T el) {
    data[curr_begin] = el;
    curr_begin = (curr_begin + 1) % data.size();
  }

  void clear(T el) { std::fill(data.begin(), data.end(), el); }

  auto operator<<(T& el) -> SlidingWindow& {
    append(el);
    return *this;
  }
};

template <typename T>
class Indexed {
 public:
  T val;
  int idx;

  Indexed(const T& val, int idx = 0) : val(val), idx(idx) {}
};

template <typename T>
auto operator==(const Indexed<T>& a, const Indexed<T>& b) -> bool {
  return a.idx == b.idx && a.val == b.val;
}

template <typename T>
auto operator<(const Indexed<T>& a, const Indexed<T>& b) -> bool {
  return a.idx == b.idx && a.val == b.val;
}

template <typename T>
auto makeIndexed(const T& val, int idx = 0) -> Indexed<T> {
  return Indexed<T>(val, idx);
}

template <typename OpT>
class FRRMAB : public OperatorSelection<OpT> {
 public:
  using OperatorSelection<OpT>::doAdapt;
  using OperatorSelection<OpT>::noOperators;

  FRRMAB(const std::vector<OpT>& strategies,
         const int window_size = 5,
         // const double scale = 5.0, const double decay = 1.0 //1st conf
         // const double scale = 1.0, const double decay = 0.5 //2nd conf
         const double scale = 5.0,
         const double decay = 0.5  // 3rd conf
         //  const double scale = 1.0, const double decay = 1.0 //4th conf
         )
      : OperatorSelection<OpT>(strategies),
        scale(scale),
        decay(decay),
        last_op(-1),
        fir_records(window_size, Indexed<double>(-1, 0.0)),
        fir(noOperators()),
        frr(noOperators()),
        reward(noOperators()),
        num(noOperators()),
        ranks(noOperators()),
        not_selected(noOperators()),
        unused_operators_exist(true) {
    reset(0.0);
  };

  void reset(double d) final;

  void update() final { std::fill(fir.begin(), fir.end(), 0.0); };
  auto selectOperator() -> OpT& final;
  void feedback(const double cf, const double pf) final {
    if (cf < pf)
      fir[last_op] += (pf - cf) / pf;
  };

  auto printOn(std::ostream& os) -> std::ostream& final {
    os << "  strategy: FRRMAB\n"
       << "  window_size: " << fir_records.size() << '\n'
       << "  scale: " << scale << '\n'
       << "  decay: " << decay << '\n';
    return os;
  }

 private:
  void assignCredits();

  auto intpow(double base, int exp) -> double {
    double result = 1.0;
    while (exp != 0) {
      if ((exp & 1) == 1)
        result *= base;
      exp >>= 1;
      base *= base;
    }

    return result;
  }

  const double scale;
  const double decay;
  int last_op;

  SlidingWindow<Indexed<double>> fir_records;

  using real_vec = std::vector<double>;
  using int_vec = std::vector<int>;
  using bool_vec = std::vector<bool>;

  real_vec fir;
  real_vec frr;
  real_vec reward;
  int_vec num;
  int_vec ranks;

  bool_vec not_selected;
  bool unused_operators_exist;
};

template <typename OpT>
void FRRMAB<OpT>::reset(double) {
  last_op = -1;
  fir_records.clear(Indexed<double>(-1, 0.0));
  using std::fill;
  fill(fir.begin(), fir.end(), 0.0);
  fill(frr.begin(), frr.end(), 0.0);
  fill(reward.begin(), reward.end(), 0.0);
  fill(num.begin(), num.end(), 0);
  fill(not_selected.begin(), not_selected.end(), true);
  unused_operators_exist = true;
};

template <typename OpT>
auto FRRMAB<OpT>::selectOperator() -> OpT& {
  if (last_op != -1)
    assignCredits();

  int idx = -1;

  if (unused_operators_exist) {
    idx = RNG::intUniform(0, noOperators() - 1);
    not_selected[idx] = false;
    unused_operators_exist =
        std::any_of(not_selected.begin(), not_selected.end(),
                    [](const bool b) { return b; });
  } else {
    double max_opt = -std::numeric_limits<double>::infinity();
    const double log_sum = 2.0 * log(sum(num.data(), num.size()));
    for (int i = 0; i < noOperators(); ++i) {
      if (num[i] > 0) {
        const double opt = frr[i] + scale * sqrt(log_sum / num[i]);
        if (opt > max_opt) {
          max_opt = opt;
          idx = i;
        }
      }
    }
    assert(idx != -1);
  }
  last_op = idx;
  return this->getOperator(idx);
}

template <typename OpT>
void FRRMAB<OpT>::assignCredits() {
  std::fill(reward.begin(), reward.end(), 0.0);
  std::fill(num.begin(), num.end(), 0);

  fir_records.append(Indexed<double>(last_op, fir[last_op]));
  for (const auto& el : fir_records) {
    if (el.idx >= 0) {
      reward[el.idx] += el.val;
      num[el.idx]++;
    }
  }

  std::iota(ranks.begin(), ranks.end(), 0);
  std::sort(ranks.begin(), ranks.end(), [this](const int& a, const int& b) {
    return this->reward[a] > this->reward[b];
  });

  last_op = -1;
  double sum = 0.0;
  for (int i = 0; i < noOperators(); ++i) {
    frr[i] = intpow(decay, ranks[i]) * reward[i];
    sum += frr[i];
  }
  if (sum > 0.0) {
    for (int i = 0; i < noOperators(); ++i)
      frr[i] = frr[i] / sum;
  }
}

template <typename OpT>
auto operator<<(std::ostream& os, FRRMAB<OpT> const& fm) -> std::ostream& {
  os << static_cast<OperatorSelection<OpT> const&>(fm) << "\n"
     << "  scale: " << fm.scale << "\n"
     << "  decay: " << fm.decay << "\n"
     << "  sliding_window_size: " << fm.fir_records.size();
  return os;
};

#endif  // FRRMAB
