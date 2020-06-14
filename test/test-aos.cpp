#include <iostream>
#include <array>
#include <random>

#include <gtest/gtest.h>

#include "aos/adaptive_operator_selection.hpp"
#include "flowshop-solver/aos/probability_matching.hpp"
#include "flowshop-solver/aos/lin_ucb.hpp"
#include "flowshop-solver/aos/frrmab.hpp"
#include "flowshop-solver/aos/thompson_sampling.hpp"
#include "flowshop-solver/aos/probability_matching.hpp"

auto timeAverage(std::vector<double> vals, int size) -> std::vector<double> {
  unsigned bs = vals.size() / size;
  std::vector<double> avgs(size, 0.0);
  for (unsigned i = 0; i < vals.size(); i++) 
    avgs[i / bs] += vals[i] / bs;
  return avgs;
}

TEST(AOSStaticDistributions, AOSTests)
{
  const int gens = 500;
  std::default_random_engine rng;
  std::vector<int> choices = {0, 1};

  std::map<std::string, OperatorSelection<int>*> aoss;
  FRRMAB<int> mab(choices, 50, 5.0, 0.5);
  aoss["FRRMAB"] = &mab;
  ThompsonSampling<int> ts(choices);
  aoss["TS"] = &ts;
  ProbabilityMatching<int> pm(choices);
  aoss["PM"] = &pm;

  for (const auto& kv : aoss) {
    std::vector<std::normal_distribution<float>> dists = {
      std::normal_distribution<float>(-10, 1),
      std::normal_distribution<float>(10, 1)
    };

    std::cout << kv.first << '\n';
    auto& aos = *kv.second;
    std::vector<double> choices;
    for (int i = 0; i < gens; i++) {
      int sel = aos.selectOperator();
      float val = dists[sel](rng);
      choices.push_back(sel);
      aos.feedback(val);
      aos.update();
    }
    auto ta = timeAverage(choices, 10);
    for (const auto& el : ta)
      std::cerr << el << ' ';
    std::cerr << '\n';
  }

  for (const auto& kv : aoss) {

    std::vector<std::normal_distribution<float>> dists = {
      std::normal_distribution<float>(10, 1),
      std::normal_distribution<float>(-10, 1)
    };

    auto& aos = *kv.second;
    aos.reset(0);
    std::vector<double> choices;
    for (int i = 0; i < gens; i++) {
      if (i == gens / 2)
        std::swap(dists[0], dists[1]);
      int sel = aos.selectOperator();
      float val = dists[sel](rng);
      choices.push_back(sel);
      aos.feedback(val);
      aos.update();
    }
    auto ta = timeAverage(choices, 10);
    std::cerr << kv.first << ": ";
    for (const auto& el : ta)
      std::cerr << el << '\t';
    std::cerr << '\n';
  }
}





auto main(int argc, char **argv) -> int
{
  argc = 2;
  // char* argvv[] = {"", "--gtest_filter=FLA.*"};
  // testing::InitGoogleTest(&argc, argvv);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
