#pragma once

#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "flowshop-solver/problems/FSPData.hpp"
#include "flowshop-solver/problems/FSPProblem.hpp"

class FSPProblemFactory {
  static std::string data_folder;
  static std::unordered_map<std::string, FSPData> cache;
  static std::vector<std::unordered_map<std::string, std::string>>
      lower_bounds_data;

 public:
  static void init(const std::string& data_folder) {
    FSPProblemFactory::data_folder = data_folder;
    loadLowerBoundsData();
  }

  static auto names() -> std::vector<std::string> {
    return {"problem",   "type",   "instance",
            "objective", "budget", "stopping_criterion"};
  }

  static auto instPath(const std::string& problem, const std::string& inst) {
    return data_folder + "/instances/" + problem + "/" + inst;
  }

  static auto lowerBoundsFile() -> std::string {
    return data_folder + "/lower_bounds_data.csv";
  }

  static void loadLowerBoundsData() {
    std::ifstream fl(lowerBoundsFile());
    if (!fl.good())
      throw std::runtime_error("lower bounds data file not found in " +
                               lowerBoundsFile());
    std::vector<std::string> header = getNextLineAndSplitIntoTokens(fl);
    lower_bounds_data.clear();
    do {
      std::vector<std::string> vline = getNextLineAndSplitIntoTokens(fl);
      if (!fl)
        continue;
      if (vline.size() != header.size()) {
        throw std::runtime_error("Error while reading lower bounds data!");
      }
      std::unordered_map<std::string, std::string> line;
      for (unsigned i = 0; i < vline.size(); i++) {
        line[header[i]] = vline[i];
      }
      lower_bounds_data.emplace_back(line);
    } while (fl);
  }

  static auto getLowerBound(const std::string& instance,
                            const std::string& objective) -> unsigned {
    auto l = std::find_if(
        lower_bounds_data.begin(), lower_bounds_data.end(),
        [&](const std::unordered_map<std::string, std::string>& line) {
          return line.at("instance") == instance &&
                 line.at("objective") == objective;
        });
    if (l == lower_bounds_data.end())
      return 0;
    return std::atol(l->at("best_bound").c_str());
  }

  static auto cachedInstance(const std::string& problem,
                             const std::string& inst) -> const FSPData& {
    if (cache.find(inst) == cache.end()) {
      cache.emplace(inst, instPath(problem, inst));
    }
    return cache.at(inst);
  }

  static auto get(const std::unordered_map<std::string, std::string>& prob_data)
      -> FSPProblem {
    for (const auto& name : names()) {
      if (prob_data.count(name) != 1)
        throw std::runtime_error("Missing problem attribute " + name);
    }

    auto problem = prob_data.at("problem");
    auto instance = prob_data.at("instance");
    auto type = prob_data.at("type");
    auto stoppingCriterion = prob_data.at("stopping_criterion");
    auto objective = prob_data.at("objective");
    auto budget = prob_data.at("budget");

    auto inst = cachedInstance(problem, instance);
    auto lowerBound = getLowerBound(instance, objective);

    return FSPProblem(inst, type, objective, budget, stoppingCriterion,
                      lowerBound);
  }
};
