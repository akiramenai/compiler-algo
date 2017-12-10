/// \file
/// \brief Define MIR for optimizations.
/// MIR operates with symbolic registers or variables and immediate values or
/// constants of 32-bit integer type.
/// All names are in the global scope and should not clash.
// TODO: Add other types support.
// TODO: Add memory operations.
#ifndef MIR_H
#define MIR_H

#include "context.h"

#include <boost/container/stable_vector.hpp>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace wyrm {

class BasicBlock;
class Function;
class Module;

/// \brief Represent symbolic register (a variable in high level language).
struct SymReg {
  bool HasName{false};
  friend std::ostream &operator<<(std::ostream &stream, const SymReg &symReg);
};

/// \brief Destination for GoTo or Branch instruction.
using Label = std::size_t;
/// \brief Type of constants.
/// \todo Add float point numbers and composites.
using Imm = int;
/// \brief Value is either constant or located in symbolic register.
using Value = variant<SymReg, Imm>;

struct InstBase {};
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
  optional<SymReg> Dest;
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

using Instruction = variant<ReceiveInst, RetInst, GoToInst, BrInst, CallInst,
                            UnOpInst, BinOpInst, AssignInst>;

class BasicBlock {
public:
  auto begin() { return std::begin(Instructions); }
  auto end() const { return std::end(Instructions); }
  auto cbegin() { return std::cbegin(Instructions); }
  auto cend() const { return std::cend(Instructions); }
  Instruction &operator[](size_t index) { return Instructions[index]; }
  BasicBlock(const BasicBlock &) = delete;
  BasicBlock &operator=(BasicBlock) = delete;
  BasicBlock(BasicBlock &&) = default;
  BasicBlock &operator=(BasicBlock &&) = default;
  friend class MIRBuilder;

private:
  BasicBlock() {}
  std::vector<Instruction> Instructions{};
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
  Function(Function &&) = default;
  Function &operator=(Function &&) = default;
  Module *parent() {return OwningModule;}
  const string_view Name;
  friend class MIRBuilder;
  friend std::ostream &operator<<(std::ostream &stream,
                                  const Function &function);

private:
  Module *OwningModule;
  std::vector<string_view> ArgNames;
  Function(Module *Parent, std::string &&Name,
           std::vector<string_view> &&ArgNames)
      // clang-format off
      : Name{internedName(std::move(Name))},
        OwningModule{Parent}, ArgNames{std::move(ArgNames)}
  // clang-format on
  {
    assert(Parent && "Parent must be non-null");
  }
  boost::container::stable_vector<BasicBlock> BasicBlocks{};
  std::unordered_map<string_view, BasicBlock *> LabelToBasicBlock{};
};

class Module {
public:
  auto begin() { return std::begin(Functions); }
  auto end() const { return std::end(Functions); }
  auto cbegin() { return std::cbegin(Functions); }
  auto cend() const { return std::cend(Functions); }
  Function &operator[](size_t index) { return Functions[index]; }
  const string_view Name;
  Module(std::string &&name) : Name{internedName(std::move(name))} {}
  Module(const Module &) = delete;
  Module &operator=(Module) = delete;
  Module(Module &&) = default;
  Module &operator=(Module &&) = default;
  friend class MIRBuilder;
  friend std::ostream &operator<<(std::ostream &stream, const Module &module);

private:
  boost::container::stable_vector<Function> Functions{};
  boost::container::stable_vector<SymReg> GlobalVariables{};
};

/// \brief Helper for building IR.
class MIRBuilder {
public:
  /// \brief Add instruction to the end of current basic block.
  /// \pre CurrentBB != nullptr
  template <typename InstType, typename... ArgTypes>
  Instruction createInstruction(ArgTypes &&... args) {
    assert(CurrentBB != nullptr);
    return InstType(std::forward<ArgTypes...>(args...));
  }
  /// \brief Set \p BB as the current basic block.
  void setBasicBlock(BasicBlock &bb) { CurrentBB = &bb; }
  /// \brief Add a new basic block to \p func's basic block list.
  /// \param label Optional label for the block for GoTo instructions. Emptry
  /// string means no label.
  /// \pre \p label must be unique withing a BasicBlock.
  BasicBlock &createBasicBlock(Function &func, string_view label);
  /// \brief Create a new function in the module if there is no function with \p
  /// name in the module. \return Address of added function, nullptr if no
  /// function was added.
  Function *createFunction(std::string &&Name,
                           std::vector<std::string> &&NamedParameters);
  /// \brief Find function with p \name in the module and return its address.
  Function *findFunction(string_view name) const;
  /// \brief Create new global variable with specified \p name.
  /// If \p name is already defined create new global variable with
  /// name = \p name.unique_numeric_suffix.
  SymReg &createGlobalVariable(std::string &&name);
  /// \brief Find global variable with \p name in the module and return
  /// corresponding symbolic register.
  SymReg *findGlobalVariable(string_view name) const;
  MIRBuilder(Module &module) : TheModule{module} {}

private:
  Module &TheModule;
  BasicBlock *CurrentBB{nullptr};
};

} // namespace wyrm
#endif // MIR_H
