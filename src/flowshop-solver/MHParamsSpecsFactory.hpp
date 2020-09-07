#pragma once

#include <exception>
#include <fstream>
#include <string>
#include <unordered_map>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "MHParamsSpecs.hpp"

class MHParamsSpecsFactory {
  static std::unordered_map<std::string, MHParamsSpecs> cache;

 public:
  static void init(const std::string& specs_folder, bool quiet = false) {
    if (!fs::exists(specs_folder)) {
      throw std::runtime_error(specs_folder + " not found!");
    }
    for (auto& fn : fs::directory_iterator(specs_folder)) {
      auto path = fn.path();
      std::ifstream fl(path);
      MHParamsSpecs specs;
      fl >> specs;
      std::string mh_name = path.replace_extension("").filename();
      specs.setMHName(mh_name);
      cache[mh_name] = specs;
      if (!quiet)
        std::cerr << "Cached MH:\n" << specs << '\n';
    }
  }

  static MHParamsSpecs get(const std::string& mh) {
    if (cache.find(mh) == cache.end())
      throw std::runtime_error("Unknown metaheuristic: '" + mh + "'");
    return cache[mh];
  }
};
