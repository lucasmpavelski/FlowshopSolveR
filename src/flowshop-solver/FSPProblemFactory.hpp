#pragma once

#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "problems/FSPProblem.hpp"

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
    return {
      "problem",
      "type",
      "instance",
      "objective",
      "budget",
      "stopping_criterion"
    };
  }

  static auto instFolder() -> std::string {
    return data_folder + "/instances/flowshop/";
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

  static auto get(
      const std::unordered_map<std::string, std::string>& prob_data) -> FSPProblem {
    assert(prob_data.at("problem") == "FSP");
    const auto instance = prob_data.at("instance");
    if (!cache.count(instance)) {
      cache[instance] = FSPData(FSPProblemFactory::instFolder() + prob_data.at("instance"));
    }
    FSPData data(cache.at(instance));
    const std::string type = prob_data.at("type");
    const std::string objective = prob_data.at("objective");
    const std::string stopping_criterion = prob_data.at("stopping_criterion");
    unsigned lower_bound = getLowerBound(prob_data.at("instance"), objective);
    return FSPProblem(std::move(data), type, objective, prob_data.at("budget"),
                      stopping_criterion, lower_bound);
  }
};
