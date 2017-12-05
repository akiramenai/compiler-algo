#include "MIR.h"
#include "gtest/gtest.h"
#include <iostream>
#include <sstream>

using namespace wyrm;
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

TEST(GlobalVariables3, BuildMIR) {
  Module TheModule{"bestiary"};
  MIRBuilder Builder{TheModule};
  auto expected = Builder.addGlobalVariable("wyrm");
  Builder.addGlobalVariable("dragon");
  auto wyrm = Builder.findGlobalVariable("wyrm");
  auto drake = Builder.findGlobalVariable("drake");
  EXPECT_TRUE(wyrm);
  EXPECT_EQ(expected, *wyrm);
  EXPECT_FALSE(drake);
}

TEST(Function1, BuildMIR) {
  Module TheModule{"my_module"};
  MIRBuilder Builder{TheModule};
  Builder.addFunction("func1", {"a", "b", "c"});
  Builder.addFunction("func2", {"d", "e", "f"});
  std::stringstream ss_expected{}, ss_actual{};
  ss_expected << "module my_module\n"
                 "function func1(a, b, c, ...) {\n"
                 "}\n"
                 "function func2(d, e, f, ...) {\n"
                 "}\n";
  ss_actual << TheModule;
  EXPECT_EQ(ss_expected.str(), ss_actual.str());
}

TEST(Function2, BuildMIR) {
  Module TheModule{"my_module"};
  MIRBuilder Builder{TheModule};
  Builder.addFunction("func1", {"a", "b", "c"});
  auto Func2Expected = Builder.addFunction("func2", {"d", "e", "f"});
  Builder.addFunction("func3", {"g", "h", "i"});
  Builder.addFunction("func42", {"w", "y", "r", "m"});
  auto Func2Actual = Builder.findFunction("func2");
  auto FuncNotFound = Builder.findFunction("make_code_faster");
  EXPECT_EQ(Func2Expected, Func2Actual);
  EXPECT_EQ(nullptr, FuncNotFound);
}
