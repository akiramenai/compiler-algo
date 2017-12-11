#include "MIR.h"

#include <cassert>
#include <iostream>
#include <limits>

namespace wyrm {

std::ostream &operator<<(std::ostream &stream, const SymReg &symReg) {
  if (symReg.HasName)
    stream << "%" << GlobalContext.Names.at(&symReg);
  else
    stream << "%%";
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
  TheModule.GlobalVariables.emplace_back(SymReg{true});
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
          GlobalContext.FunctionSymbols[&Func].count(Label) == 0u) &&
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
  GlobalContext.FunctionSymbols[&Func].insert(InternedLabel);
  return BBRef;
}

} // namespace wyrm
