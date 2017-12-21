#include "MIR.h"

#include <cassert>
#include <iostream>
#include <limits>

namespace wyrm {

static size_t bbNumber(const BasicBlock &BB) {
  if (BB.hasLabel())
    return 0;
  size_t BBNum{1};
  auto &Func = BB.parent();
  for (const auto &CurrBB : Func) {
    if (&BB == &CurrBB)
      break;
    BBNum += !CurrBB.hasLabel();
  }
  return BBNum;
}

std::ostream &operator<<(std::ostream &Stream, const ReceiveInst &Inst) {
  Stream << "  " << Inst.outRegister() << " = receive\n";
  return Stream;
}

static void dumpLabel(std::ostream &Stream, const BasicBlock &BB) {
  if (BB.hasLabel())
    Stream << GlobalContext.Names.at(&BB);
  else
    Stream << "BB" << std::to_string(bbNumber(BB));
}

std::ostream &operator<<(std::ostream &Stream, const GoToInst &Inst) {
  const BasicBlock &Successor{Inst.successor()};
  Stream << "  goto ";
  dumpLabel(Stream, Successor);
  Stream << "\n";
  return Stream;
}

std::ostream &operator<<(std::ostream &Stream, const BrInst &Inst) {
  auto Cond = Inst.condition();
  auto &TrueSuccessor = Inst.trueSuccessor();
  auto &FalseSuccessor = Inst.falseSuccessor();
  Stream << "  br " << Cond << ", ";
  dumpLabel(Stream, TrueSuccessor);
  Stream << ", ";
  dumpLabel(Stream, FalseSuccessor);
  Stream << "\n";
  return Stream;
}

std::ostream &operator<<(std::ostream &Stream, const RetInst &Inst) {
  auto Val = Inst.operand();
  Stream << "  ret " << Val << "\n";
  return Stream;
}

std::ostream &operator<<(std::ostream &Stream, const CallInst &Inst) {
  auto *RetVal = Inst.outRegister();
  Stream << "  ";
  if (RetVal)
    Stream << *RetVal << " = call ";
  Stream << Inst.callee().Name << "(";
  auto It = std::cbegin(Inst);
  if (It != std::cend(Inst)) {
    for (auto E = std::prev(std::cend(Inst)); It != E; ++It)
      Stream << *It << ", ";
    Stream << *It;
  }
  Stream << ")\n";
  return Stream;
}

std::ostream &operator<<(std::ostream &Stream, const UnOpInst &Inst) {
  auto &RetVal = Inst.outRegister();
  Stream << "  " << RetVal << " = ";
  std::unordered_map<UnOpKind, std::string> InstNames = {
      {UnOpKind::Assign, ""}, {UnOpKind::Neg, "neg "}, {UnOpKind::Not, "not "}};
  Stream << InstNames[Inst.kind()] << Inst.operand() << "\n";
  return Stream;
}

std::ostream &operator<<(std::ostream &Stream, const BinOpInst &Inst) {
  auto &RetVal = Inst.outRegister();
  Stream << "  " << RetVal << " = ";
  std::unordered_map<BinOpKind, std::string> InstNames = {
      {BinOpKind::Add, "add"},     {BinOpKind::Sub, "sub"},
      {BinOpKind::Mul, "mul"},     {BinOpKind::Div, "div"},
      {BinOpKind::Mod, "mod"},     {BinOpKind::Min, "min"},
      {BinOpKind::Max, "max"},     {BinOpKind::Shl, "shl"},
      {BinOpKind::Shr, "shr"},     {BinOpKind::Shra, "shra"},
      {BinOpKind::And, "and"},     {BinOpKind::Or, "or"},
      {BinOpKind::Xor, "xor"},     {BinOpKind::Eq, "cmp eq"},
      {BinOpKind::Neq, "cmp neq"}, {BinOpKind::Less, "cmp lt"},
      {BinOpKind::Leq, "cmp leq"}, {BinOpKind::Greater, "cmp gt"},
      {BinOpKind::Geq, "cmp ge"}};
  Stream << InstNames[Inst.kind()] << " " << Inst.operand1() << ", "
         << Inst.operand2() << "\n";
  return Stream;
}

std::ostream &operator<<(std::ostream &Stream, const Instruction &Inst) {
  visit([&Stream](auto &&Arg) { Stream << Arg; }, Inst);
  return Stream;
}

std::ostream &operator<<(std::ostream &stream, const SymReg &symReg) {
  if (symReg.HasName)
    stream << "%" << GlobalContext.Names.at(&symReg);
  else {
    auto &Func = symReg.parent<Function>();
    auto It =
        std::find_if(std::cbegin(Func.symbolicRegisters()),
                     std::cend(Func.symbolicRegisters()),
                     [&symReg](const SymReg &Reg) { return &symReg == &Reg; });
    size_t Number = It - std::cbegin(Func.symbolicRegisters()) + 1;
    stream << "%" << Number;
  }
  return stream;
}

std::ostream &operator<<(std::ostream &Stream, const BasicBlock &BB) {
  if (BB.hasLabel())
    Stream << GlobalContext.Names.at(&BB) << ":\n";
  else {
    auto &Func = BB.parent();
    size_t BBNum{1};
    for (const auto &CurrBB : Func) {
      if (&BB == &CurrBB)
        break;
      BBNum += !CurrBB.hasLabel();
    }
    Stream << "BB" << BBNum << ":\n";
    for (const Instruction &Inst : BB)
      Stream << Inst;
  }
  return Stream;
}

std::ostream &operator<<(std::ostream &stream, const Function &function) {
  stream << "function " << function.Name << "(";
  for (auto ArgName : function.ArgNames)
    stream << ArgName << ", ";
  stream << "...) {\n";
  for (const auto &BB : function)
    stream << BB;
  stream << "}\n";
  return stream;
}

std::ostream &operator<<(std::ostream &stream, const Module &module) {
  stream << "module " << module.Name << "\n";
  for (auto &symReg : module.GlobalVariables)
    stream << "global " << symReg << "\n";
  for (const auto &F : module.Functions)
    stream << F;
  return stream;
}

SymReg &MIRBuilder::createGlobalVariable(std::string &&name) {
  TheModule.GlobalVariables.emplace_back(TheModule, true);
  SymReg &Result = TheModule.GlobalVariables.back();
  auto &GlobalNames = GlobalContext.ModuleSymbols[&TheModule].GlobalVariables;
  size_t i = 1;
  auto NewName = [name](size_t i) { return name + "." + std::to_string(i); };
  bool NeedRename{};
  if (GlobalNames.count(name) != 0u) {
    NeedRename = true;
    while (GlobalNames.count(NewName(i)) != 0u) {
      ++i;
    }
  }
  auto InternedName = internedName(NeedRename ? NewName(i) : name);
  GlobalNames[InternedName] = &Result;
  GlobalContext.NameTable[&Result] = InternedName;
  return Result;
}

SymReg *MIRBuilder::findGlobalVariable(string_view name) const {
  auto &GlobalNames = GlobalContext.ModuleSymbols[&TheModule].GlobalVariables;
  if (GlobalNames.count(name) == 0u)
    return nullptr;
  return GlobalNames[name];
}

Function *
MIRBuilder::createFunction(std::string &&Name,
                           std::vector<std::string> &&NamedParameters) {
  auto &FunctionNames = GlobalContext.ModuleSymbols[&TheModule].Functions;
  if (FunctionNames.count(Name) != 0u)
    return nullptr;
  // TODO: private constructor might be called from emplace_back
  // see:
  // https://stackoverflow.com/questions/17007977/vectoremplace-back-for-objects-with-a-private-constructor
  std::vector<string_view> InternedParameters;
  InternedParameters.reserve(NamedParameters.size());
  for (auto &ParName : NamedParameters)
    InternedParameters.push_back(internedName(std::move(ParName)));
  Function F(TheModule, std::move(Name), std::move(InternedParameters));
  TheModule.Functions.emplace_back(std::move(F));
  auto FuncName = TheModule.Functions.back().Name;
  return FunctionNames[FuncName] = &TheModule.Functions.back();
}

Function *MIRBuilder::findFunction(string_view name) const {
  auto &FunctionNames = GlobalContext.ModuleSymbols[&TheModule].Functions;
  if (FunctionNames.count(name) == 0u)
    return nullptr;
  return FunctionNames[name];
}

BasicBlock &MIRBuilder::createBasicBlock(Function &Func, std::string &&Label) {
  assert((Label.empty() ||
          GlobalContext.FunctionSymbols[&Func].Labels.count(Label) == 0u) &&
         "Label must be unique");
  BasicBlock BB(Func);
  BB.HasLabel = !Label.empty();
  // TODO: private constructor might be called from emplace_back
  Func.BasicBlocks.emplace_back(std::move(BB));
  auto &BBRef = Func.BasicBlocks.back();
  if (Label.empty())
    return BBRef;
  string_view InternedLabel = internedName(std::move(Label));
  GlobalContext.NameTable[&BBRef] = InternedLabel;
  GlobalContext.FunctionSymbols[&Func].Labels[InternedLabel] = &BBRef;
  return BBRef;
}

SymReg &MIRBuilder::symReg(std::string &&Name, Function *Func) {
  Func = Func ? Func : currentFuction();
  assert(Func && "Symbolic register must belong to a function");
  auto &SymRegs = Func->SymbolicRegisters;
  if (Name.empty()) {
    SymRegs.emplace_back(*Func, false);
    return SymRegs.back();
  }
  string_view InternedName = internedName(std::move(Name));
  auto &NameToSymReg = GlobalContext.FunctionSymbols[Func].LocalVariables;
  if (NameToSymReg.count(InternedName))
    return *NameToSymReg.at(InternedName);
  SymRegs.emplace_back(*Func, true);
  SymReg &Result = SymRegs.back();
  GlobalContext.NameTable[&Result] = InternedName;
  NameToSymReg[InternedName] = &Result;
  return Result;
}

Instruction &MIRBuilder::createReceiveInst(std::string &&Name) {
  SymReg &Register = symReg(std::move(Name));
  return createInst<ReceiveInst>(Register);
}

Instruction &MIRBuilder::createGoToInst(BasicBlock &Destination) {
  return createInst<GoToInst>(Destination);
}

Instruction &MIRBuilder::createBrInst(Value Condition,
                                      BasicBlock &TrueDestination,
                                      BasicBlock &FalseDestination) {
  return createInst<BrInst>(Condition, TrueDestination, FalseDestination);
}

Instruction &MIRBuilder::createRetInst(Value ReturnValue) {
  return createInst<RetInst>(ReturnValue);
}

Instruction &MIRBuilder::createCallInst(bool ReturnValue, Function &Callee,
                                        std::vector<Value> &&Arguments,
                                        std::string &&Name) {
  SymReg *RetReg = ReturnValue ? &symReg(std::move(Name)) : nullptr;
  return createInst<CallInst>(RetReg, Callee, std::move(Arguments));
}

Instruction &MIRBuilder::createUnOpInst(UnOpKind Kind, Value Operand,
                                        std::string &&Name) {
  SymReg &RetReg = symReg(std::move(Name));
  return createInst<UnOpInst>(RetReg, Kind, Operand);
}

Instruction &MIRBuilder::createBinOpInst(BinOpKind Kind, Value Operand1,
                                         Value Operand2, std::string &&Name) {
  SymReg &RetReg = symReg(std::move(Name));
  return createInst<BinOpInst>(RetReg, Kind, Operand1, Operand2);
}
} // namespace wyrm
