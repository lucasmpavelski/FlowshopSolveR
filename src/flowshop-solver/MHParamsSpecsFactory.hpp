#pragma once

#include <exception>
#include <fstream>
#include <string>
#include <unordered_map>

#include "flowshop-solver/MHParamsSpecs.hpp"

class MHParamsSpecsFactory {
  static std::string specsFolder;
  static std::unordered_map<std::string, MHParamsSpecs> cache;

 public:
  static void init(const std::string& folder, bool quiet = false) {
    specsFolder = folder;
  }

  static const MHParamsSpecs& get(const std::string& mh) {
    if (cache.find(mh) == cache.end()) {
      auto path = specsFolder + '/' + mh + ".txt";
      std::ifstream fl(path);
      MHParamsSpecs specs;
      fl >> specs;
      specs.setMHName(mh);
      cache[mh] = specs;
    }
    return cache[mh];
  }
};
