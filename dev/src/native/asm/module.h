#pragma once

#include "asm/module_text.h"
#include "lowir/program.h"
#include "native/asm/module_symbols.h"
#include "native/asm/module_unwind.h"

namespace native {

class AssemblyEnvironment;

class ModuleAssembler
{
public:
	ModuleAssembler(object::ObjectFile& out, Target target);
	void begin(asm_model::Dialect dialect);
	void module_start();
	void begin_raw(uint32_t symbol, const std::string& name, bool comdat, bool initial);
	void end_raw();
	void declare(const assembly::ModuleLine& line);
	void append(const assembly::ModuleLine& line);
	void finish();

private:
	void flush();
	bool directive(const std::string& name, assembly::ModuleCursor& cursor);
	void section(const std::string& name, object::SectionKind kind);
	void data(assembly::ModuleCursor& cursor, unsigned bytes);
	void strings(assembly::ModuleCursor& cursor, bool terminate);
	void alignment(assembly::ModuleCursor& cursor, bool power);
	void size(assembly::ModuleCursor& cursor);
	void type(assembly::ModuleCursor& cursor);
	void reserve(uint64_t count, uint8_t fill);
	uint64_t offset() const;
	uint32_t symbol(assembly::ModuleCursor& cursor) { return symbols_.token(cursor); }
	object::ObjectFile& out_;
	Target target_;
	ModuleSymbols symbols_;
	ModuleUnwind unwind_;
	asm_model::Dialect dialect_ = asm_model::Dialect::Att;
	uint32_t section_ = 0;
	uint32_t previous_ = 0;
	std::vector<uint32_t> section_stack_;
	std::string code_;
	std::vector<assembly::SymbolUse> uses_;
	std::vector<uint32_t> code_lines_;
	uint32_t raw_symbol_ = 0;
	uint32_t raw_section_ = 0;
	uint32_t raw_first_ = 0;
	bool raw_initial_ = false;
};

void emit_assembly_items(object::ObjectFile& out, const lowir::Program& program, Target target,
	const std::vector<uint32_t>& symbols, AssemblyEnvironment& environment);

} // namespace native
