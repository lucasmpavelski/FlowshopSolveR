#pragma once

#include <string>
#include <unordered_map>

#include "flowshop-solver/fla/LocalOptimaNetwork.hpp"
#include "flowshop-solver/problems/FSPProblem.hpp"

class LONSampling {
 public:
  LONSampling() = default;
  virtual auto sampleLON(
      const std::unordered_map<std::string, std::string>& prob_params,
      const std::unordered_map<std::string, std::string>& sampling_params,
      unsigned seed) -> LocalOptimaNetwork<FSPProblem::EOT> = 0;
};