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
#include "wyrm_traits.h"

#include <boost/container/stable_vector.hpp>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace wyrm {

class BasicBlock;
class Function;
class Module;

/// \brief Represent symbolic register (a variable in high level language).
class SymReg {
public:
  SymReg(Module &OwningModule) : OwningModule{&OwningModule} {}
  SymReg(Function &OwningFunction) : OwningFunction{&OwningFunction} {}
  SymReg(Module &OwningModule, bool HasName)
      : HasName{HasName}, OwningModule{&OwningModule} {}
  SymReg(Function &OwningFunction, bool HasName)
      : HasName{HasName}, OwningFunction{&OwningFunction} {}
  SymReg(const SymReg &) = delete;
  SymReg &operator=(SymReg) = delete;
  SymReg(SymReg &&) = default;
  SymReg &operator=(SymReg &&) = default;
  bool hasName() const { return HasName; }
  template <typename T,
            typename = std::enable_if<is_one_of_v<T, Module, Function>>>
  const T &parent() const {
    if constexpr (std::is_same_v<T, Module>) {
      assert(OwningModule && "Symbolic register doesn't belong to a module");
      return *OwningModule;
    }
    if constexpr (std::is_same_v<T, Function>) {
      assert(OwningFunction &&
             "Symbolic register doesn't belong to a function");
      return *OwningFunction;
    }
  }
  template <typename T,
            typename = std::enable_if<is_one_of_v<T, Module, Function>>>
  T &parent() {
    return const_cast<T &>(static_cast<const SymReg *>(this)->parent<T>());
  }
  friend std::ostream &operator<<(std::ostream &stream, const SymReg &symReg);

private:
  bool HasName{false};
  Module *OwningModule{nullptr};
  Function *OwningFunction{nullptr};
};

/// \brief Destination for GoTo or Branch instruction.
using Label = std::size_t;
/// \brief Type of constants.
/// \todo Add float point numbers and composites.
using Imm = int;
/// \brief Value is either constant or located in symbolic register.
using Value = variant<SymReg, Imm>;

namespace detail {
/// \brief Base class for an instruction.
/// Use Instruction class rather than InstBase for polymorphic code.
class InstBase {
public:
  /// \return Owning basic block
  /// \pre There must be non null owning basic block
  BasicBlock &parent() {
    assert(OwningBB);
    return *OwningBB;
  }
  InstBase(InstBase &&) = default;
  InstBase(const InstBase &) = delete;
  InstBase &operator=(InstBase) = delete;

protected:
  InstBase(BasicBlock &BB) : OwningBB{&BB} {}

private:
  BasicBlock *OwningBB;
};

/// \brief An instructuion which return or might return a value in a symbolic
/// register.
template <typename ReturnTy> class ReturningInstBase : public InstBase {
public:
  const ReturnTy outRegister() const { return OutValue; }

protected:
  ReturningInstBase(BasicBlock &BB, ReturnTy OutValue)
      : InstBase(BB), OutValue{OutValue} {}

private:
  ReturnTy OutValue;
};

} // namespace detail

/// \brief Represent receiving function argument.
/// Define function parameter as a symbolic register with unknown value.
class ReceiveInst final: public detail::ReturningInstBase<SymReg &> {
public:
  friend class MIRBuilder;
  friend std::ostream &operator<<(std::ostream &Stream,
                                  const ReceiveInst &Inst);

private:
  ReceiveInst(BasicBlock &BB, SymReg &RetReg)
      : detail::ReturningInstBase<SymReg &>(BB, RetReg) {}
};

/// \brief Unconditional branch.
struct GoToInst {
  Label Dest;
  friend std::ostream &operator<<(std::ostream &Stream, const GoToInst &Inst);
};

/// \brief Conditional branch.
struct BrInst {
  Value Operand;
  Label Dest;
  friend std::ostream &operator<<(std::ostream &Stream, const BrInst &Inst);
};

/// \brief Return value from a function.
struct RetInst {
  Value Operand;
  friend std::ostream &operator<<(std::ostream &Stream, const RetInst &Inst);
};

struct CallInst {
  optional<SymReg &> Dest;
  std::vector<Value> Arguments;
  auto args_begin() { return std::begin(Arguments); }
  auto args_end() { return std::end(Arguments); }
  auto args_begin() const { return std::cbegin(Arguments); }
  auto args_end() const { return std::cend(Arguments); }
  friend std::ostream &operator<<(std::ostream &Stream, const CallInst &Inst);
};

/// \brief Instruction of form a = b.
struct AssignInst {
  SymReg Dest;
  Value Operand;
  friend std::ostream &operator<<(std::ostream &Stream, const AssignInst &Inst);
};

enum class UnOpKind { Neg, Not };

/// \brief Instruction of form a = op b.
struct UnOpInst {
  SymReg Dest;
  Value Operand;
  UnOpKind Kind;
  friend std::ostream &operator<<(std::ostream &Stream, const UnOpInst &Inst);
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
  friend std::ostream &operator<<(std::ostream &Stream, const BinOpInst &Inst);
};

using Instruction = variant<ReceiveInst, RetInst, GoToInst, BrInst, CallInst,
                            UnOpInst, BinOpInst, AssignInst>;
std::ostream &operator<<(std::ostream &Stream, const Instruction &Inst);

class BasicBlock {
public:
  auto begin() { return std::begin(Instructions); }
  auto end() { return std::end(Instructions); }
  auto begin() const { return std::cbegin(Instructions); }
  auto end() const { return std::cend(Instructions); }
  Instruction &operator[](size_t index) { return Instructions[index]; }
  Function &parent() { return OwningFunction; }
  const Function &parent() const { return OwningFunction; }
  bool hasLabel() const { return HasLabel; }
  BasicBlock(const BasicBlock &) = delete;
  BasicBlock &operator=(BasicBlock) = delete;
  BasicBlock(BasicBlock &&) = default;
  BasicBlock &operator=(BasicBlock &&) = default;
  friend class MIRBuilder;
  friend std::ostream &operator<<(std::ostream &Stream, const BasicBlock &BB);

private:
  BasicBlock(Function &Parent) : OwningFunction{Parent} {}
  Function &OwningFunction;
  boost::container::stable_vector<Instruction> Instructions{};
  bool HasLabel{};
};

class Function {
public:
  auto begin() { return std::begin(BasicBlocks); }
  auto end() { return std::end(BasicBlocks); }
  auto begin() const { return std::cbegin(BasicBlocks); }
  auto end() const { return std::cend(BasicBlocks); }
  BasicBlock &operator[](size_t index) { return BasicBlocks[index]; }
  Function(const Function &) = delete;
  Function &operator=(Function) = delete;
  Function(Function &&) = default;
  Function &operator=(Function &&) = default;
  Module &parent() { return OwningModule; }
  const Module &parent() const { return OwningModule; }
  const boost::container::stable_vector<SymReg> &symbolicRegisters() const {
    return SymbolicRegisters;
  }
  const string_view Name;
  friend class MIRBuilder;
  friend std::ostream &operator<<(std::ostream &stream,
                                  const Function &function);

private:
  Module &OwningModule;
  std::vector<string_view> ArgNames;
  Function(Module &Parent, std::string &&Name,
           std::vector<string_view> &&ArgNames)
      : Name{internedName(std::move(Name))},
        OwningModule{Parent}, ArgNames{std::move(ArgNames)} {}
  boost::container::stable_vector<BasicBlock> BasicBlocks{};
  boost::container::stable_vector<SymReg> SymbolicRegisters{};
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
  BasicBlock &createBasicBlock(Function &Func, std::string &&Label = "");
  /// \brief Create a new function in the module if there is no function with \p
  /// name in the module. \return Address of added function, nullptr if no
  /// function was added.
  Function *createFunction(std::string &&Name,
                           std::vector<std::string> &&NamedParameters = {});
  /// \brief Find function with p \name in the module and return its address.
  Function *findFunction(string_view name) const;
  /// \brief Create new global variable with specified \p name.
  /// If \p name is already defined create new global variable with
  /// name = \p name.unique_numeric_suffix.
  SymReg &createGlobalVariable(std::string &&name);
  /// \brief Find global variable with \p name in the module and return
  /// corresponding symbolic register.
  SymReg *findGlobalVariable(string_view name) const;
  Instruction &createReceiveInst(std::string &&Name = "");
  Function *currentFuction() {
    return (CurrentBB == nullptr) ? nullptr : &CurrentBB->parent();
  }
  MIRBuilder(Module &module) : TheModule{module} {}

private:
  /// \brief Find existing register with Name or create it in curruent fuction
  /// scope. If Name is empty create new unnamed register and return it.
  SymReg &symReg(std::string &&Name, Function *Func = nullptr);
  Module &TheModule;
  BasicBlock *CurrentBB{nullptr};
};

} // namespace wyrm
#endif // MIR_H
