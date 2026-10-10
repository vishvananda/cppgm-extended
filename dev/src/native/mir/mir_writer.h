#pragma once

#include <cstdio>
#include <string>

#include "lowir/program.h"
#include "native/mir/mir.h"

// The deterministic machine-IR dump: a view of the MIR native emission
// consumes. Sections are appended as the backend produces them (startup,
// globals, then one function at a time) and flushed to the file in
// bounded chunks.
namespace native {

class MirWriter
{
public:
	MirWriter(const lowir::Program& program, std::FILE* file);
	~MirWriter();

	void header(const Startup& startup);
	void global(lowir::SymbolId symbol);
	void function(const Function& function);
	// Flushes buffered text; false on a write error.
	bool finish();

private:
	void flush_if_large();
	void type(lowir::Type type);
	void reg(Reg reg);
	void operand(const Function* function, const Operand& op);
	void instruction(const Function& function, const Inst& inst);
	void call_facts(const Inst& inst);
	void symbol(uint32_t symbol);
	void label(const Function& function, uint32_t block);
	void block_name(const Function& function, const Block& block);

	const lowir::Program& program_;
	std::FILE* file_;
	std::string text_;
	bool ok_ = true;
};

// The canonical spelling of a floating literal in MIR: the shortest
// decimal that reads back to the same value, with `f` (f32) or `L` (f80).
void append_mir_float(std::string& out, lowir::TypeKind kind, uint64_t low, uint64_t high);

} // namespace native
