#pragma once

#include <chrono>

#include <paradiseo/mo/mo>

template <class EOT = int, class TimeT = std::chrono::milliseconds>
class myTimeStat : public moStat<EOT, int> {
  std::chrono::system_clock::time_point start{std::chrono::system_clock::now()};

 public:
  using moStat<EOT, int>::value;

  myTimeStat() : moStat<EOT, int>(0, "timer in miliseconds") {}

  void operator()(EOT&) final { update(); };

  void update() {
    auto now = std::chrono::system_clock::now();
    value() = std::chrono::duration_cast<TimeT>(now - start).count();
  }

  void reset() { start = std::chrono::system_clock::now(); }
};
