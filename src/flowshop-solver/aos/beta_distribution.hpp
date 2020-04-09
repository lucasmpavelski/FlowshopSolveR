#pragma once

#include <ostream>
#include <random>

// This implementation of Beta distribution is from
//  http://stackoverflow.com/questions/15165202/random-number-generator-with-beta-distribution

template <typename RealType = double>
class beta_distribution {
 public:
  using result_type = RealType;

  class param_type {
   public:
    using distribution_type = beta_distribution;

    explicit param_type(RealType a = 2.0, RealType b = 2.0)
        : a_param(a), b_param(b) {}

    auto a() const -> RealType { return a_param; }
    auto b() const -> RealType { return b_param; }

    auto operator==(const param_type& other) const -> bool {
      return (a_param == other.a_param && b_param == other.b_param);
    }

    auto operator!=(const param_type& other) const -> bool {
      return !(*this == other);
    }

   private:
    RealType a_param, b_param;
  };

  explicit beta_distribution(RealType a = 2.0, RealType b = 2.0)
      : a_gamma(a), b_gamma(b) {}
  explicit beta_distribution(const param_type& param)
      : a_gamma(param.a()), b_gamma(param.b()) {}

  void reset() {}

  auto param() const -> param_type { return param_type(a(), b()); }

  void param(const param_type& param) {
    a_gamma = gamma_dist_type(param.a());
    b_gamma = gamma_dist_type(param.b());
  }

  template <typename URNG>
  auto operator()(URNG& engine) -> result_type {
    return generate(engine, a_gamma, b_gamma);
  }

  template <typename URNG>
  auto operator()(URNG& engine, const param_type& param) -> result_type {
    gamma_dist_type a_param_gamma(param.a()), b_param_gamma(param.b());
    return generate(engine, a_param_gamma, b_param_gamma);
  }

  auto min() const -> result_type { return 0.0; }
  auto max() const -> result_type { return 1.0; }

  auto a() const -> result_type { return a_gamma.alpha(); }
  auto b() const -> result_type { return b_gamma.alpha(); }

  auto operator==(const beta_distribution<result_type>& other) const -> bool {
    return (param() == other.param() && a_gamma == other.a_gamma &&
            b_gamma == other.b_gamma);
  }

  auto operator!=(const beta_distribution<result_type>& other) const -> bool {
    return !(*this == other);
  }

 private:
  using gamma_dist_type = std::gamma_distribution<result_type>;

  gamma_dist_type a_gamma, b_gamma;

  template <typename URNG>
  auto generate(URNG& engine,
                gamma_dist_type& x_gamma,
                gamma_dist_type& y_gamma) -> result_type {
    result_type x = x_gamma(engine);
    return x / (x + y_gamma(engine));
  }
};

template <typename CharT, typename RealType>
auto operator<<(std::basic_ostream<CharT>& os,
                const beta_distribution<RealType>& beta)
    -> std::basic_ostream<CharT>& {
  os << "~Beta(" << beta.a() << "," << beta.b() << ")";
  return os;
}

template <typename CharT, typename RealType>
auto operator>>(std::basic_istream<CharT>& is,
                beta_distribution<RealType>& beta)
    -> std::basic_istream<CharT>& {
  std::string str;
  RealType a, b;
  if (std::getline(is, str, '(') && str == "~Beta" && is >> a &&
      is.get() == ',' && is >> b && is.get() == ')') {
    beta = beta_distribution<RealType>(a, b);
  } else {
    is.setstate(std::ios::failbit);
  }
  return is;
}
