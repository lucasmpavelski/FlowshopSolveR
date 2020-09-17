#pragma once

#include <paradiseo/eo/eo>

class RunOptions {
 public:
  bool printBestFitness = false;
  bool printFitnessReward = false;
  bool printDestructionChoices = false;
  bool printLastFitness = false;

  RunOptions() = default;

  RunOptions(eoParser& parser)
      : printBestFitness{createParam(parser,
                                     false,
                                     "printBestFitness",
                                     "print best fitness")},
        printFitnessReward{createParam(parser,
                                       false,
                                       "printFitnessReward",
                                       "print AOS IG fitness rewards")},
        printDestructionChoices{createParam(parser,
                                            false,
                                            "printDestructionChoices",
                                            "print AOS IG destruction sizes")},
        printLastFitness{createParam(parser,
                                     false,
                                     "printLastFitness",
                                     "print final result")} {}

 private:
  template <class T>
  auto createParam(eoParser& parser,
                   T def,
                   const std::string& name,
                   const std::string& desc) -> T {
    return parser.createParam(def, name, desc).value();
  }
};
