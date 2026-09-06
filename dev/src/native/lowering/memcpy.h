#pragma once

#include "lowir/model/program.h"

#include <vector>

namespace lowir_native {
namespace memcpy_detail {

inline lowir_model::SymbolId builtin_symbol_named(
    const lowir_model::LowirProgram & program, const char * object_symbol)
{
  for(std::size_t i = 0; i < program.function_declarations.size(); ++i) {
    const lowir_model::FunctionDeclaration & declaration =
      program.function_declarations[i];
    if(declaration.metadata.object_symbol.valid() &&
       program.strings.get(declaration.metadata.object_symbol) ==
         object_symbol)
      return declaration.symbol;
  }
  for(std::size_t i = 0; i < program.functions.size(); ++i) {
    const lowir_model::LowirFunction & function = program.functions[i];
    if(function.metadata.object_symbol.valid() &&
       program.strings.get(function.metadata.object_symbol) == object_symbol)
      return function.symbol;
  }
  return lowir_model::SymbolId();
}

inline lowir_model::SymbolId builtin_symbol(
    const lowir_model::LowirProgram & program)
{
  return builtin_symbol_named(program, "cppgm_builtin_memcpy");
}

inline bool is_inline_unused_call(
    const lowir_model::Instruction & instruction,
    const std::vector<std::size_t> & uses,
    lowir_model::SymbolId memcpy_symbol)
{
  return memcpy_symbol.valid() &&
    instruction.kind == lowir_model::Instruction::IK_CALL &&
    instruction.first.kind == lowir_model::Operand::OP_GLOBAL &&
    instruction.first.symbol == memcpy_symbol &&
    instruction.args.size() == 3 && instruction.dest.valid() &&
    uses[instruction.dest] == 0;
}

}  // namespace memcpy_detail

// The optimizer's dynamic fill, `call @__builtin_fill_bytes(dst, byte,
// count)` on the object symbol cppgm_builtin_fill_bytes.  It is never a real
// call: with its result unused it lowers to `rep stosb` from the SysV
// argument registers, so a program that carries one links with no runtime,
// hosted or not.
namespace fill_detail {

inline lowir_model::SymbolId builtin_symbol(
    const lowir_model::LowirProgram & program)
{
  return memcpy_detail::builtin_symbol_named(
    program, "cppgm_builtin_fill_bytes");
}

inline bool is_inline_unused_call(
    const lowir_model::Instruction & instruction,
    const std::vector<std::size_t> & uses,
    lowir_model::SymbolId fill_symbol)
{
  return memcpy_detail::is_inline_unused_call(instruction, uses, fill_symbol);
}

// The unit fill, `call @__builtin_fill_units(dst, value, count, unit)` on
// cppgm_builtin_fill_units: rdi, rsi, rdx as for the byte fill, and the
// unit -- 2, 4 or 8 -- an integer literal the optimizer wrote, which
// selects `rep stosw`, `rep stosd` or `rep stosq`.
inline lowir_model::SymbolId units_builtin_symbol(
    const lowir_model::LowirProgram & program)
{
  return memcpy_detail::builtin_symbol_named(
    program, "cppgm_builtin_fill_units");
}

inline std::size_t inline_unused_units_call_width(
    const lowir_model::Instruction & instruction,
    const std::vector<std::size_t> & uses,
    lowir_model::SymbolId fill_units_symbol)
{
  if(!fill_units_symbol.valid() ||
     instruction.kind != lowir_model::Instruction::IK_CALL ||
     instruction.first.kind != lowir_model::Operand::OP_GLOBAL ||
     instruction.first.symbol != fill_units_symbol ||
     instruction.args.size() != 4 || !instruction.dest.valid() ||
     uses[instruction.dest] != 0) return 0;
  const lowir_model::Operand & unit = instruction.args[3];
  if(unit.kind != lowir_model::Operand::OP_INTEGER || !unit.has_int_value)
    return 0;
  return unit.int_value == 2 || unit.int_value == 4 || unit.int_value == 8 ?
    static_cast<std::size_t>(unit.int_value) : 0;
}

}  // namespace fill_detail
}  // namespace lowir_native
