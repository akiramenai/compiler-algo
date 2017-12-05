#include "MIR.h"
#include <cassert>
#include <iostream>
#include <limits>

namespace wyrm {
std::ostream &operator<<(std::ostream &stream, const Module &module) {
  stream << "module " << module.Name << "\n";
  for (size_t index : module.GlobalVariables) {
    stream << "global %";
    if (module.SymRegToName.count(index))
      stream << module.SymRegToName.at(index);
    else
      stream << std::to_string(index);
    stream << "\n";
  }
  for (const auto &F : module.Functions)
    stream << F;
  return stream;
}

std::ostream &operator<<(std::ostream &stream, const Function &function) {
  stream << "function " << function.Name << "(";
  for (const auto &ArgName : function.ArgNames)
    stream << ArgName << ", ";
  stream << "...) {\n";
  stream << "}\n";
  return stream;
}

SymReg MIRBuilder::addGlobalVariable(const std::string &name) {
  assert(TheModule.SymRegNum < std::numeric_limits<std::size_t>::max());
  TheModule.SymRegNum++;
  int i = 1;
  auto NewName = [name](int i) { return name + "." + std::to_string(i); };
  bool NeedRename{};
  if (TheModule.NameToSymReg.count(name) != 0u) {
    NeedRename = true;
    while (TheModule.NameToSymReg.count(NewName(i)) != 0u) {
      ++i;
    }
  }
  TheModule.NameToSymReg[NeedRename ? NewName(i) : name] = TheModule.SymRegNum;
  TheModule.SymRegToName[TheModule.SymRegNum] =
      (NeedRename ? NewName(i) : name);
  TheModule.GlobalVariables.insert(TheModule.SymRegNum);
  return TheModule.SymRegNum;
}

optional<SymReg> MIRBuilder::findGlobalVariable(const std::string &name) const {
  if (TheModule.NameToSymReg.count(name) == 0u)
    return {};
  SymReg Result = TheModule.NameToSymReg.at(name);
  if (TheModule.GlobalVariables.count(Result) == 0u)
    return {};
  return Result;
}

Function *MIRBuilder::addFunction(std::string &&name,
                                  std::vector<std::string> &&namedParameters) {
  if (TheModule.NameToFunction.count(name) != 0u)
    return nullptr;
  // TODO: private constructor might be called from emplace_back
  // see:
  // https://stackoverflow.com/questions/17007977/vectoremplace-back-for-objects-with-a-private-constructor
  Function F(std::move(name), std::move(namedParameters));
  TheModule.Functions.emplace_back(std::move(F));
  auto NameAsCstr = TheModule.Functions.back().Name.c_str();
  return TheModule.NameToFunction[NameAsCstr] = &TheModule.Functions.back();
}

Function *MIRBuilder::findFunction(string_view name) {
  if (TheModule.NameToFunction.count(name) == 0u)
    return nullptr;
  return TheModule.NameToFunction[name];
}
} // namespace wyrm
