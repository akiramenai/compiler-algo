/// \file
/// \brief Define MIR for optimizations.
/// MIR operates with symbolic registers or variables and immediate values or
/// constants of 32-bit integer type.
/// All names are in the global scope and should not clash.
// TODO: Add other types support.
// TODO: Add memory operations.
#ifndef MIR_H
#define MIR_H

#include <boost/variant.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace ca {

/// \brief Represent symbolic register (a variable in high level language).
using SymReg = std::size_t;
/// \brief Destination for GoTo or Branch instruction.
using Label = std::size_t;
/// \brief Type of constants.
/// \todo Add float point numbers and composites.
using Imm = int;
/// \brief Value is either constant or located in symbolic register.
using Value = boost::variant<SymReg, Imm>;

/// \brief Begin of function body.
struct BeginInst {};
/// \brief End of function body.
struct EndInst {};

/// \brief Represent receiving function argument.
/// Define function parameter as a symbolic register with unknown value.
struct ReceiveInst {
  SymReg VarIdx;
};

/// \brief Unconditional branch.
struct GoToInst {
  Label Dest;
};

/// \brief Conditional branch.
struct BrInst {
  Value Operand;
  Label Dest;
};

/// \brief Return value from a function.
struct RetInst {
  Value Operand;
};

struct CallInst {
  SymReg Dest;
  std::vector<Value> Arguments;
  auto args_begin() { return std::begin(Arguments); }
  auto args_end() { return std::end(Arguments); }
  auto args_begin() const { return std::cbegin(Arguments); }
  auto args_end() const { return std::cend(Arguments); }
};

/// \brief Instruction of form a = b.
struct AssignInst {
  SymReg Dest;
  Value Operand;
};

enum class UnOpKind { Neg, Not };

/// \brief Instruction of form a = op b.
struct UnOpInst {
  SymReg Dest;
  Value Operand;
  UnOpKind Kind;
};

enum class BinOpKind {
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  Min,
  Max,
  Shl,
  Shr,
  Shra,
  And,
  Or,
  Xor,
  Eq,
  Neq,
  Less,
  Leq,
  Greater,
  Geq
};

/// \brief Instruction of form a = b op c.
struct BinOpInst {
  SymReg Dest;
  Value Operand1;
  Value Operand2;
  BinOpKind Kind;
};

using Instruction =
    boost::variant<BeginInst, EndInst, ReceiveInst, RetInst, GoToInst, BrInst,
                   CallInst, UnOpInst, BinOpInst, AssignInst>;

class BasicBlock {
public:
  auto begin() { return std::begin(Instructions); }
  auto end() const { return std::end(Instructions); }
  auto cbegin() { return std::cbegin(Instructions); }
  auto cend() const { return std::cend(Instructions); }
  Instruction &operator[](size_t index) { return Instructions[index]; }
  BasicBlock(const BasicBlock &) = delete;
  BasicBlock &operator=(BasicBlock) = delete;
  friend class MIRBuilder;

private:
  std::vector<Instruction> Instructions;
};

class Function {
public:
  auto begin() { return std::begin(BasicBlocks); }
  auto end() const { return std::end(BasicBlocks); }
  auto cbegin() { return std::cbegin(BasicBlocks); }
  auto cend() const { return std::cend(BasicBlocks); }
  BasicBlock &operator[](size_t index) { return BasicBlocks[index]; }
  Function(const Function &) = delete;
  Function &operator=(Function) = delete;
  friend class MIRBuilder;

private:
  std::vector<BasicBlock> BasicBlocks;
  std::vector<std::string> ArgNames;
};

class Module {
public:
  auto begin() { return std::begin(Functions); }
  auto end() const { return std::end(Functions); }
  auto cbegin() { return std::cbegin(Functions); }
  auto cend() const { return std::cend(Functions); }
  Function &operator[](size_t index) { return Functions[index]; }
  std::string &Name{ModuleName};
  Module(std::string_view name) : ModuleName{name} {}
  Module(const Module &) = delete;
  Module &operator=(Module) = delete;
  friend class MIRBuilder;
  friend std::ostream &operator<<(std::ostream &stream, const Module &module);

private:
  std::string ModuleName;
  std::vector<Function> Functions;
  std::vector<size_t> GlobalVariables;
  size_t SymRegNum;
  std::unordered_map<std::string, size_t> NameToSymReg;
  std::unordered_map<size_t, std::string> SymRegToName;
};

/// \brief Helper for building IR.
class MIRBuilder {
public:
  /// \brief Add \p BB to the module's basic block list, create a new if \p BB
  /// is nullptr.
  //  void addBasicBlock(BasicBlock *bb);
  /// \brief Add instruction to the end of current basic block.
  /// If there is no current basic block create a new one and make it current.
  //  void addInstruction();
  /// \brief Set \p BB as the current basic block.
  /// Precondition: \p must belong to the module.
  //  void setBasicBlock(BasicBlock &bb);
  /// \brief Create new global variable with specified \p name.
  /// If \p name is already defined create new global variable with
  /// name = \p name.unique_numeric_suffix.
  SymReg addGlobalVariable(const std::string &name);
  MIRBuilder(Module &module) : TheModule{module} {}

private:
  [[maybe_unused]] Module &TheModule;
  [[maybe_unused]] BasicBlock *BB;
};

} // namespace ca
#endif // MIR_H
