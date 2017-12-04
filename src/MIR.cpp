#include "MIR.h"
#include <cassert>
#include <iostream>
#include <limits>

namespace ca {
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
  TheModule.GlobalVariables.push_back(TheModule.SymRegNum);
  return TheModule.SymRegNum;
}
} // namespace ca
