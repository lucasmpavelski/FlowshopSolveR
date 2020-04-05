#pragma once

#include <coolingSchedule/moCoolingSchedule.h>

#include "problems/FSPData.hpp"

template <class EOT>
class opCoolingSchedule : public moCoolingSchedule<EOT> {
 public:
  /**
   * Constructor
   * @param _initT initial temperature
   * @param _alpha factor of decreasing
   * @param _span number of iteration with equal temperature
   * @param _finalT final temperature, threshold of the stopping criteria
   */
  opCoolingSchedule(const FSPData& fspData,
                    double T,
                    double _finalT,
                    double _beta) {
    initT =
        T * fspData.maxCT() / (10 * fspData.noJobs() * fspData.noMachines());
    finalT = _finalT;
    beta = _beta;
  }

  /**
   * Getter on the initial temperature
   * @param _solution initial solution
   * @return the initial temperature
   */
  virtual double init(EOT&) { return initT; }

  /**
   * update the temperature by a factor
   * @param _temp current temperature to update
   * @param _acceptedMove true when the move is accepted, false otherwise
   */
  virtual void update(double& _temp, bool) {
    _temp = _temp / (1 + beta * _temp);
  }

  /**
   * compare the temperature to the threshold
   * @param _temp current temperature
   * @return true if the current temperature is over the threshold (final
   * temperature)
   */
  virtual bool operator()(double _temp) { return _temp > finalT; }

 private:
  // initial temperature
  double initT;
  // coefficient of decrease
  double beta;
  // maximum number of iterations at the same temperature
  double finalT;
};
