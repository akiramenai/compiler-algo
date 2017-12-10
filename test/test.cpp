#include "MIR.h"
#include "gtest/gtest.h"
#include <iostream>
#include <sstream>

using namespace wyrm;
TEST(MIRBuilder, TrivialModule) {
  Module TheModule{"my_module"};
  std::stringstream ss_expected{}, ss_actual{};
  ss_expected << "module my_module\n";
  ss_actual << TheModule;
  EXPECT_EQ(ss_expected.str(), ss_actual.str());
}

TEST(MIRBuilder, GlobalVariables1) {
  Module TheModule{"my_module"};
  MIRBuilder Builder{TheModule};
  Builder.createGlobalVariable("var1");
  Builder.createGlobalVariable("var2");
  std::stringstream ss_expected{}, ss_actual{};
  ss_expected << "module my_module\n"
                 "global %var1\n"
                 "global %var2\n";
  ss_actual << TheModule;
  EXPECT_EQ(ss_expected.str(), ss_actual.str());
}

TEST(MIRBuilder, GlobalVariables2) {
  Module TheModule{"my_module"};
  MIRBuilder Builder{TheModule};
  Builder.createGlobalVariable("var");
  Builder.createGlobalVariable("var");
  std::stringstream ss_expected{}, ss_actual{};
  ss_expected << "module my_module\n"
                 "global %var\n"
                 "global %var.1\n";
  ss_actual << TheModule;
  EXPECT_EQ(ss_expected.str(), ss_actual.str());
}

TEST(MIRBuilder, GlobalVariables3) {
  Module TheModule{"bestiary"};
  MIRBuilder Builder{TheModule};
  auto &expected = Builder.createGlobalVariable("wyrm");
  Builder.createGlobalVariable("dragon");
  auto *wyrm = Builder.findGlobalVariable("wyrm");
  auto *drake = Builder.findGlobalVariable("drake");
  EXPECT_TRUE(wyrm);
  EXPECT_EQ(&expected, wyrm);
  EXPECT_FALSE(drake);
}

TEST(MIRBuilder, Function1) {
  Module TheModule{"my_module"};
  MIRBuilder Builder{TheModule};
  Builder.createFunction("func1", {"a", "b", "c"});
  Builder.createFunction("func2", {"d", "e", "f"});
  std::stringstream ss_expected{}, ss_actual{};
  ss_expected << "module my_module\n"
                 "function func1(a, b, c, ...) {\n"
                 "}\n"
                 "function func2(d, e, f, ...) {\n"
                 "}\n";
  ss_actual << TheModule;
  EXPECT_EQ(ss_expected.str(), ss_actual.str());
}

TEST(MIRBuilder, Function2) {
  Module TheModule{"my_module"};
  MIRBuilder Builder{TheModule};
  Builder.createFunction("func1", {"a", "b", "c"});
  auto Func2Expected = Builder.createFunction("func2", {"d", "e", "f"});
  Builder.createFunction("func3", {"g", "h", "i"});
  Builder.createFunction("func42", {"w", "y", "r", "m"});
  auto Func2Actual = Builder.findFunction("func2");
  auto FuncNotFound = Builder.findFunction("make_code_faster");
  auto Func2Duplicate = Builder.createFunction("func2", {});
  EXPECT_EQ(Func2Expected, Func2Actual);
  EXPECT_EQ(nullptr, FuncNotFound);
  EXPECT_EQ(nullptr, Func2Duplicate);
}
