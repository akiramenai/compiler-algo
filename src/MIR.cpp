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

std::ostream &operator<<(std::ostream &stream, const Module &module) {
  stream << "module " << module.Name << "\n";
  for (auto &symReg : module.GlobalVariables)
    stream << "global " << symReg << "\n";
  for (const auto &F : module.Functions)
    stream << F;
  return stream;
}

std::ostream &operator<<(std::ostream &stream, const Function &function) {
  stream << "function " << function.Name << "(";
  for (auto ArgName : function.ArgNames)
    stream << ArgName << ", ";
  stream << "...) {\n";
  stream << "}\n";
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
MIRBuilder::createFunction(std::string &&name,
                           std::vector<std::string> &&namedParameters) {
  auto &FunctionNames = GlobalContext.ModuleSymbols[&TheModule].Functions;
  if (FunctionNames.count(name) != 0u)
    return nullptr;
  // TODO: private constructor might be called from emplace_back
  // see:
  // https://stackoverflow.com/questions/17007977/vectoremplace-back-for-objects-with-a-private-constructor
  Function F(std::move(name), std::move(namedParameters));
  TheModule.Functions.emplace_back(std::move(F));
  auto Name = TheModule.Functions.back().Name;
  return FunctionNames[Name] = &TheModule.Functions.back();
}

Function *MIRBuilder::findFunction(string_view name) const {
  auto &FunctionNames = GlobalContext.ModuleSymbols[&TheModule].Functions;
  if (FunctionNames.count(name) == 0u)
    return nullptr;
  return FunctionNames[name];
}

BasicBlock &MIRBuilder::createBasicBlock(Function &func, string_view label) {
  assert(func.LabelToBasicBlock.count(label) == 0u && "Label must be unique");
  // TODO: private constructor might be called from emplace_back
  BasicBlock BB;
  func.BasicBlocks.emplace_back(std::move(BB));
  auto &BBRef = func.BasicBlocks.back();
  if (!label.empty())
    func.LabelToBasicBlock[label] = &BBRef;
  return BBRef;
}

void MIRBuilder::setBasicBlock(BasicBlock &bb) { CurrentBB = &bb; }

} // namespace wyrm
