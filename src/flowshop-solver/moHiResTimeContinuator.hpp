#pragma once

#include <chrono>

#include <paradiseo/mo/continuator/moContinuator.h>

/**
 * Termination condition until a running time is reached.
 */
template <class Neighbor, class TimeT = std::chrono::milliseconds>
class moHighResTimeContinuator : public moContinuator<Neighbor> {
 public:
  typedef typename Neighbor::EOT EOT;

  /**
   * Constructor
   * @param _max maximum running time (in second)
   * @param _verbose verbose mode true/false -> on/off
   */
  moHighResTimeContinuator(typename TimeT::rep _max,
                           bool _verbose = true,
                           bool resetFirstInit = true)
      : max(_max), verbose(_verbose), resetFirstInit(resetFirstInit) {
    external = false;
    start = std::chrono::system_clock::now();
  }

  /**
   * Synchronize the whole time with an external starting time
   * @param _externalStart external starting time
   */
  void setStartingTime(std::chrono::system_clock::time_point _externalStart) {
    external = true;
    start = _externalStart;
  }

  /**
   * To get the starting time
   * @return starting time
   */
  std::chrono::system_clock::time_point getStartingTime() const {
    return start;
  }

  /**
   * To set the maximum running time
   *
   * @param _maxTime maximum running time
   */
  void maxTime(typename TimeT::rep _maxTime) { max = _maxTime; }

  /**
   * Returns false when the running time is reached.
   * @param _sol the current solution
   */
  bool operator()(EOT& _sol) final override {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<TimeT>(now - start);
    bool res = (duration.count() < max);
    if (!res && verbose)
      std::cout << "STOP in moTimeContinuator: Reached maximum time ["
                << duration.count() << "/" << max << "]" << std::endl;
    return res;
  }

  /**
   * reset the start time
   * @param _solution a solution
   */
  void init(EOT&) final override {
    if (resetFirstInit) {
      setStartingTime(std::chrono::system_clock::now());
      resetFirstInit = false;
      return;
    }
    if (!external)
      start = std::chrono::system_clock::now();
  }

  /**
   * Class name
   */
  virtual std::string className(void) const {
    return "moHighResTimeContinuator";
  }

 private:
  std::chrono::system_clock::time_point start;
  typename TimeT::rep max;
  bool verbose, external, resetFirstInit;
};