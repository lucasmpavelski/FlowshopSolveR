#pragma once

#include <paradiseo/eo/eo>

class RunOptions {
 public:
  bool printBestFitness = false;
  bool printFitnessReward = false;

  RunOptions() = default;

  RunOptions(eoParser& parser)
      : printBestFitness{parser
                             .createParam(false,
                                          "printBestFitness",
                                          "print best fitness")
                             .value()},
        printFitnessReward{parser
                               .createParam(false,
                                            "printFitnessReward",
                                            "print fitness rewards")
                               .value()} {}
};
