#pragma once

#include <cstdint>
#include <initializer_list>

#include "lowir/builder.h"

// Structured construction of the runtime library's LowIR routines
// (native/link/runtime_library.h): a program of declarations and
// definitions named by their object symbols, and per routine a thin layer
// over lowir::FunctionBuilder with byte-offset addressing, typed memory
// access, slots for mutable locals and blocks placed as they are entered.
namespace native {

typedef lowir::TypeKind Kind;

// One 8-byte item of a runtime global: `symbol + value`, or the integer
// `value` when `symbol` is kNoId.
struct DataWord
{
	lowir::SymbolId symbol;
	int64_t value;
};

class RuntimeProgram
{
public:
	explicit RuntimeProgram(lowir::Program& program) : program_(program) {}

	lowir::Program& program() { return program_; }
	lowir::NameId name(const char* spelling);
	// Whether the program defines a function or global of this name.
	bool defined(const char* spelling);
	lowir::SignatureId signature(Kind result, std::initializer_list<Kind> parameters);
	// A function definition or declaration named by its object symbol.
	lowir::SymbolId function(const char* name, lowir::SignatureId signature, bool define);
	// A global: a definition of `bytes` zero bytes (8-aligned) or a
	// declaration (bytes == 0).
	lowir::SymbolId global(const char* name, uint64_t bytes);
	// A read-only global of 8-byte items, or of a string's bytes and NUL.
	lowir::SymbolId words(const char* name, std::initializer_list<DataWord> items);
	lowir::SymbolId string(const char* name, const char* text);
	// `alias object name = target`.
	void alias(const char* name, lowir::SymbolId target);

private:
	lowir::Program& program_;
};

class RoutineBuilder
{
public:
	RoutineBuilder(lowir::Program& program, lowir::SymbolId function);
	~RoutineBuilder() { builder_.finish(); }

	RoutineBuilder(const RoutineBuilder&) = delete;
	RoutineBuilder& operator=(const RoutineBuilder&) = delete;

	lowir::Value param(uint32_t index) const { return lowir::Value::temp(index); }
	lowir::Value integer(int64_t value) { return builder_.integer(value); }
	// A floating literal of `kind` (rounded to it).
	lowir::Value floating(Kind kind, long double value);

	// Memory at `base + offset` bytes.
	lowir::Value at(lowir::Value base, int64_t offset);
	lowir::Value at(lowir::Value base, lowir::Value offset);
	lowir::Value load(Kind kind, lowir::Value address, int64_t offset = 0);
	void store(Kind kind, lowir::Value value, lowir::Value address, int64_t offset = 0);
	lowir::Value address(lowir::SymbolId symbol);
	lowir::Value slot_address(lowir::SlotId slot);

	lowir::Value binary(lowir::BinaryOp op, Kind kind, lowir::Value a, lowir::Value b);
	lowir::Value add(Kind kind, lowir::Value a, lowir::Value b) { return binary(lowir::BinaryOp::Add, kind, a, b); }
	lowir::Value compare(lowir::ComparePredicate predicate, Kind kind, lowir::Value a, lowir::Value b);
	lowir::Value equal(Kind kind, lowir::Value a, lowir::Value b)
	{
		return compare(lowir::ComparePredicate::Eq, kind, a, b);
	}
	lowir::Value convert(lowir::ConvertOp op, Kind to, Kind from, lowir::Value value);

	lowir::Value call(Kind result, lowir::SymbolId callee, std::initializer_list<lowir::Value> arguments);
	lowir::Value call_indirect(
		Kind result, lowir::Value callee, lowir::SignatureId signature, std::initializer_list<lowir::Value> arguments);

	// A mutable local in a slot.
	lowir::SlotId local(Kind kind);
	lowir::Value read(Kind kind, lowir::SlotId slot);
	void write(Kind kind, lowir::SlotId slot, lowir::Value value);

	lowir::BlockId block();
	// Places `block` after the blocks placed so far and appends to it.
	void enter(lowir::BlockId block);
	void jump(lowir::BlockId target) { builder_.jump(target); }
	void branch(lowir::Value condition, lowir::BlockId if_true, lowir::BlockId if_false)
	{
		builder_.branch(condition, if_true, if_false);
	}
	void ret(Kind kind, lowir::Value value);
	void ret();
	void unreachable();

private:
	lowir::Value value_of(lowir::Instruction inst, Kind result);

	lowir::Program& program_;
	lowir::FunctionBuilder builder_;
};

} // namespace native
