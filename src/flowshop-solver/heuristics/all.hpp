#pragma once

#include <unordered_map>

#include "flowshop-solver/heuristics.hpp"
#include "flowshop-solver/heuristics/aco.hpp"
#include "flowshop-solver/heuristics/hc.hpp"
#include "flowshop-solver/heuristics/ig.hpp"
#include "flowshop-solver/heuristics/ihc.hpp"
#include "flowshop-solver/heuristics/ils.hpp"
#include "flowshop-solver/heuristics/isa.hpp"
#include "flowshop-solver/heuristics/sa.hpp"
#include "flowshop-solver/heuristics/ts.hpp"

template <class ParamType = double>
auto solveWith(
    std::string mh,
    const std::unordered_map<std::string, std::string>& problem_specs,
    const std::unordered_map<std::string, ParamType>& params_values) -> Result {
  FSPProblem prob = FSPProblemFactory::get(problem_specs);
  MHParamsSpecs specs = MHParamsSpecsFactory::get(mh);
  MHParamsValues params(&specs);
  params.readValues(params_values);

  if (mh == "all")
    mh = params.categoricalName("MH");

  if (mh == "HC")
    return solveWithHC(prob, params);
  else if (mh == "SA")
    return solveWithSA(prob, params);
  else if (mh == "IHC")
    return solveWithIHC(prob, params);
  else if (mh == "ISA")
    return solveWithISA(prob, params);
  else if (mh == "TS")
    return solveWithTS(prob, params);
  else if (mh == "IG")
    return solveWithIG(prob, params);
  else if (mh == "ILS")
    return solveWithILS(prob, params);
  else if (mh == "ACO")
    return solveWithACO(prob, params);
  else
    throw std::runtime_error("Unknown MH: " + mh);
  return {};
}