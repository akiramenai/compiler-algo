#include "MIR.h"
#include "gtest/gtest.h"
#include <iostream>
#include <memory>
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

TEST(MIRBuilder, FunctionParent) {
  Module TheModule{"my_module"};
  MIRBuilder Builder{TheModule};
  auto Func = Builder.createFunction("func1", {});
  EXPECT_EQ(&TheModule, &Func->parent());
}

TEST(MIRBuilder, FunctionDump) {
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

TEST(MIRBuilder, BasicBlockDump) {
  Module TheModule{"my_module"};
  MIRBuilder Builder{TheModule};
  auto F = Builder.createFunction("func1");
  ASSERT_TRUE(F);
  Builder.createBasicBlock(*F);
  Builder.createBasicBlock(*F, "NamedBB");
  Builder.createBasicBlock(*F);
  Builder.createBasicBlock(*F);
  std::stringstream ss_expected{}, ss_actual{};
  ss_expected << "module my_module\n"
                 "function func1(...) {\n"
                 "BB1:\n"
                 "NamedBB:\n"
                 "BB2:\n"
                 "BB3:\n"
                 "}\n";
  ss_actual << TheModule;
  EXPECT_EQ(ss_expected.str(), ss_actual.str());
}

namespace {
struct InstContext {
  std::unique_ptr<Module> TheModule;
  std::unique_ptr<MIRBuilder> Builder;
};

static InstContext createInstContext() {
  auto TheModule = std::make_unique<Module>("my_module");
  auto Builder = std::make_unique<MIRBuilder>(*TheModule);
  auto *F = Builder->createFunction("func1");
  assert(F);
  Builder->setBasicBlock(Builder->createBasicBlock(*F));
  return {std::move(TheModule), std::move(Builder)};
}

TEST(MIRBuilder, ReceiveInstDump) {
  auto[TheModule, Builder] = createInstContext();
  auto &Inst = Builder->createReceiveInst();
  std::stringstream Expected{}, Actual{};
  Expected << "  %1 = receive\n";
  Actual << Inst;
  EXPECT_EQ(Expected.str(), Actual.str());
  Builder.release();
  TheModule.release();
}

TEST(MIRBuilder, GoToInstDump) {
  auto[TheModule, Builder] = createInstContext();
  ASSERT_TRUE(Builder->currentBasicBlock());
  auto &BB = Builder->createBasicBlock(Builder->currentBasicBlock()->parent());
  auto &Inst = Builder->createGoToInst(BB);
  std::stringstream Expected{}, Actual{};
  Expected << "  goto BB2\n";
  Actual << Inst;
  EXPECT_EQ(Expected.str(), Actual.str());
  Builder.release();
  TheModule.release();
}

TEST(MIRBuilder, BrInstDump) {
  auto[TheModule, Builder] = createInstContext();
  ASSERT_TRUE(Builder->currentBasicBlock());
  auto &BB1 = Builder->createBasicBlock(Builder->currentBasicBlock()->parent(),
                                        "TrueBB");
  auto &BB2 = Builder->createBasicBlock(Builder->currentBasicBlock()->parent(),
                                        "FalseBB");
  auto &Inst = Builder->createBrInst(1, BB1, BB2);
  std::stringstream Expected{}, Actual{};
  Expected << "  br 1, TrueBB, FalseBB\n";
  Actual << Inst;
  EXPECT_EQ(Expected.str(), Actual.str());
  Builder.release();
  TheModule.release();
}

TEST(MIRBuilder, RetInstDump) {
  auto[TheModule, Builder] = createInstContext();
  ASSERT_TRUE(Builder->currentBasicBlock());
  auto &Inst = Builder->createRetInst(1);
  std::stringstream Expected{}, Actual{};
  Expected << "  ret 1\n";
  Actual << Inst;
  EXPECT_EQ(Expected.str(), Actual.str());
  Builder.release();
  TheModule.release();
}

TEST(MIRBuilder, CallInstDump) {
  auto[TheModule, Builder] = createInstContext();
  ASSERT_TRUE(Builder->currentBasicBlock());
  auto *Callee = Builder->createFunction("sum", {"x", "y"});
  ASSERT_TRUE(Callee);
  auto &Inst = Builder->createCallInst(true, *Callee, {1, 2}, "add.res");
  std::stringstream Expected{}, Actual{};
  Expected << "  %add.res = call sum(1, 2)\n";
  Actual << Inst;
  EXPECT_EQ(Expected.str(), Actual.str());
  Builder.release();
  TheModule.release();
}

TEST(MIRBuilder, UnOpInstDump) {
  auto[TheModule, Builder] = createInstContext();
  ASSERT_TRUE(Builder->currentBasicBlock());
  auto &Inst1 = Builder->createUnOpInst(UnOpKind::Assign, 5);
  auto &Inst1Un = get<UnOpInst>(Inst1);
  auto &Inst2 = Builder->createUnOpInst(UnOpKind::Neg, Inst1Un.outRegister());
  auto &Inst2Un = get<UnOpInst>(Inst2);
  auto &Inst3 = Builder->createUnOpInst(UnOpKind::Not, Inst2Un.outRegister());
  std::stringstream Expected{}, Actual{};
  Expected << "  %1 = 5\n"
              "  %2 = neg %1\n"
              "  %3 = not %2\n";
  Actual << Inst1 << Inst2 << Inst3;
  EXPECT_EQ(Expected.str(), Actual.str());
  Builder.release();
  TheModule.release();
}

TEST(MIRBuilder, BinOpInstDump) {
  auto[TheModule, Builder] = createInstContext();
  ASSERT_TRUE(Builder->currentBasicBlock());
  std::stringstream Expected{}, Actual{};
  auto &Inst1 = Builder->createUnOpInst(UnOpKind::Assign, 5);
  auto &Inst1Un = get<UnOpInst>(Inst1);
  SymReg *OutRegPrev = &Inst1Un.outRegister();
  ASSERT_TRUE(OutRegPrev);
  for (size_t I = 0, E = static_cast<size_t>(BinOpKind::Geq); I <= E; ++I) {
    auto &Inst =
        Builder->createBinOpInst(static_cast<BinOpKind>(I), *OutRegPrev, I);
    auto &InstBin = get<BinOpInst>(Inst);
    OutRegPrev = &InstBin.outRegister();
    ASSERT_TRUE(OutRegPrev);
    Actual << Inst;
  }
  Expected << "  %2 = add %1, 0\n"
              "  %3 = sub %2, 1\n"
              "  %4 = mul %3, 2\n"
              "  %5 = div %4, 3\n"
              "  %6 = mod %5, 4\n"
              "  %7 = min %6, 5\n"
              "  %8 = max %7, 6\n"
              "  %9 = shl %8, 7\n"
              "  %10 = shr %9, 8\n"
              "  %11 = shra %10, 9\n"
              "  %12 = and %11, 10\n"
              "  %13 = or %12, 11\n"
              "  %14 = xor %13, 12\n"
              "  %15 = cmp eq %14, 13\n"
              "  %16 = cmp neq %15, 14\n"
              "  %17 = cmp lt %16, 15\n"
              "  %18 = cmp leq %17, 16\n"
              "  %19 = cmp gt %18, 17\n"
              "  %20 = cmp ge %19, 18\n";
  EXPECT_EQ(Expected.str(), Actual.str());
  Builder.release();
  TheModule.release();
}
} // namespace
