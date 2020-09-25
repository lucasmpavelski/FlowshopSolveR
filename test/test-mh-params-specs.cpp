#include <gtest/gtest.h>

#include "flowshop-solver/MHParamsSpecs.hpp"

TEST(ParamSpec, EqualOperator) {
  ParamSpec paramA("param1", ParamSpec::Type::CAT, 1.1, 2);
  ParamSpec paramB("param1", ParamSpec::Type::CAT, 1.1, 2);
  ParamSpec paramC("param1", ParamSpec::Type::CAT, 1 + 1.0/10, 2);
  ASSERT_TRUE(paramA == paramB);
  ASSERT_TRUE(paramA == paramC);
}

TEST(ParamSpec, NotEqualOperator) {
  ParamSpec paramA("param1", ParamSpec::Type::CAT, 1, 2);
  ParamSpec paramB("param1", ParamSpec::Type::REAL, 1, 2);
  ParamSpec paramC("param1", ParamSpec::Type::CAT, 1, 3);
  ASSERT_TRUE(paramA != paramB);
  ASSERT_TRUE(paramA != paramC);
}

TEST(ParamSpec, CategoricalValueFromStr) {
  CatParamSpec catparam("test", {"a", "b", "c"});
  ParamSpec* paramA = &catparam;
  ASSERT_FLOAT_EQ(paramA->fromStrValue("c"), 2.0f);
}

TEST(ParamSpec, NumericalValueFromStr) {
  ParamSpec numparam("test", ParamSpec::Type::REAL, 1.0f, 2.0f);
  ASSERT_FLOAT_EQ(numparam.fromStrValue("2.5"), 2.5f);
}

auto main(int argc, char **argv) -> int {
  argc = 2;
  // char* argvv[] = {"", "--gtest_filter=FLA.*"};
  // testing::InitGoogleTest(&argc, argvv);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}