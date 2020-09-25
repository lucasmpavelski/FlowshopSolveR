#include "flowshop-solver/global.hpp"

std::mt19937_64 RNG::engine;
std::random_device RNG::true_rand_engine;
long RNG::saved_seed = 0l;
bool RNG::is_saved = false;
std::unordered_map<int, long> RNG::SeedPool::seeds;
