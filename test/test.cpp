#include "MIR.h"
#include "gtest/gtest.h"
#include <iostream>
#include <sstream>

using namespace ca;
TEST(TrivialModule, BuildMIR) {
  Module TheModule{"my_module"};
  std::stringstream ss_expected{}, ss_actual{};
  ss_expected << "module my_module\n";
  ss_actual << TheModule;
  EXPECT_EQ(ss_expected.str(), ss_actual.str());
}

TEST(GlobalVariables1, BuildMIR) {
  Module TheModule{"my_module"};
  MIRBuilder Builder{TheModule};
  Builder.addGlobalVariable("var1");
  Builder.addGlobalVariable("var2");
  std::stringstream ss_expected{}, ss_actual{};
  ss_expected << "module my_module\n"
                 "global %var1\n"
                 "global %var2\n";
  ss_actual << TheModule;
  EXPECT_EQ(ss_expected.str(), ss_actual.str());
}

TEST(GlobalVariables2, BuildMIR) {
  Module TheModule{"my_module"};
  MIRBuilder Builder{TheModule};
  Builder.addGlobalVariable("var");
  Builder.addGlobalVariable("var");
  std::stringstream ss_expected{}, ss_actual{};
  ss_expected << "module my_module\n"
                 "global %var\n"
                 "global %var.1\n";
  ss_actual << TheModule;
  EXPECT_EQ(ss_expected.str(), ss_actual.str());
}
