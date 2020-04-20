#pragma once

#include <chrono>

#include <paradiseo/mo/mo>

template <class EOT, class TimeT = std::chrono::milliseconds>
class myTimeStat : public moStat<EOT, int> {
  std::chrono::system_clock::time_point start;

 public:
  using moStat<EOT, int>::value;

  myTimeStat()
      : moStat<EOT, int>(0, "timer in miliseconds"),
        start{std::chrono::system_clock::now()} {}

  void operator()(EOT&) final {
    auto now = std::chrono::system_clock::now();
    value() = std::chrono::duration_cast<TimeT>(now - start).count();
  };
};
