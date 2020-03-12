#include "../heuristics.hpp"
#include "aco.hpp"
#include "hc.hpp"
#include "ig.hpp"
#include "ihc.hpp"
#include "ils.hpp"
#include "isa.hpp"
#include "sa.hpp"
#include "ts.hpp"

Result solveWithAll(std::unordered_map<std::string, std::string> prob,
                    std::unordered_map<std::string, double> params_values);

Result solveWith(std::string mh,
                 std::unordered_map<std::string, std::string> prob,
                 std::unordered_map<std::string, double> params) {
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
  else if (mh == "all")
    return solveWithAll(prob, params);
  else
    throw std::runtime_error("Unknown MH: " + mh);
  return {};
}

Result solveWithAll(std::unordered_map<std::string, std::string> prob,
                    std::unordered_map<std::string, double> params_values) {
  const MHParamsSpecs specs = MHParamsSpecsFactory::get("all");
  MHParamsValues params(&specs);
  params.readValues(params_values);
  return solveWith(params.categoricalName("MH"), prob, params_values);
}