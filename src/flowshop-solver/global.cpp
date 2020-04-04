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

long RNG::seed(long s) {
  // my own RNG
  engine.seed(s);
  // C RNG
  srand(unsigned(s));
  // ParadisEO RNG
  rng.reseed(uint32_t(s));
  return s;
}

std::vector<std::string> getNextLineAndSplitIntoTokens(std::istream& str) {
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
    result.push_back("");
  }
  return result;
}

std::istream& operator>>(std::istream& str, CSVRow& data) {
  data.readNexthrow(str);
  return str;
}

long factorial(unsigned n) {
  long ret = 1;
  while (n >= 2)
    ret *= n--;
  return ret;
}

AssertionFailureException::AssertionFailureException(const char* expression,
                                                     const char* file,
                                                     const int line,
                                                     const std::string& message)
    : expression(expression), file(file), line(line), message(message) {
  std::ostringstream outputStream;
  if (!message.empty())
    outputStream << message << ": ";

  if (std::strcmp(expression, "false") == 0 || std::strcmp(expression, "0") ||
      std::strcmp(expression, "FALSE"))
    outputStream << "Unreachable code assertion";
  else
    outputStream << "Assertion '" << expression << "'";

  outputStream << " failed in file '" << file << "' line " << line;
  report = outputStream.str();
}