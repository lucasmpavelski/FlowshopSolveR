#pragma once

#include <paradiseo/eo/eo>

class RunOptions {
 public:
  bool printBestFitness = false;
  bool printFitnessReward = false;
  bool printDestructionChoices = false;

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
                               .value()},
        printDestructionChoices{parser
                               .createParam(false,
                                            "printDestructionChoices",
                                            "print destruction AOS choices")
                               .value()} {}
};
