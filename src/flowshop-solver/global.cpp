#include "global.hpp"

#include <paradiseo/eo/utils/eoRNG.h>

#include <cstring>
#include <sstream>
#include <vector>

std::mt19937_64 RNG::engine;
std::random_device RNG::true_rand_engine;
long RNG::saved_seed = 0l;
bool RNG::is_saved = false;
std::unordered_map<int, long> RNG::SeedPool::seeds;

auto RNG::seed(long s) -> long {
  // my own RNG
  engine.seed(s);
  // C RNG
  srand(unsigned(s));
  // ParadisEO RNG
  rng.reseed(uint32_t(s));
  return s;
}

auto getNextLineAndSplitIntoTokens(std::istream& str)
    -> std::vector<std::string> {
  std::vector<std::string> result;
  std::string line;
  std::getline(str, line);

  std::stringstream lineStream(line);
  std::string cell;

  while (std::getline(lineStream, cell, ',')) {
    result.push_back(cell);
  }
  // This checks for a trailing comma with no data after it.
  if (!lineStream && cell.empty()) {
    // If there was a trailing comma then add an empty element.
    result.emplace_back("");
  }
  return result;
}

auto operator>>(std::istream& str, CSVRow& data) -> std::istream& {
  data.readNexthrow(str);
  return str;
}

