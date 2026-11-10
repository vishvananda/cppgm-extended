#include "native/asm/module.h"

#include <algorithm>

#include "lowir/assembly.h"
#include "asm/module_expand.h"
#include "native/asm/binding.h"
#include "native/asm/encoding.h"
#include "native/asm/environment.h"
#include "native/asm/functions.h"
#include "native/backend_error.h"

namespace native {

ModuleAssembler::ModuleAssembler(object::ObjectFile& out, Target target)
	: out_(out), target_(target), symbols_(out, target), unwind_(out, target)
{
	syntax_.power_alignment = target == Target::MacOS;
	section(".text", object::SectionKind::Text);
}
uint64_t ModuleAssembler::offset() const
{
	return out_.sections[section_].size;
}
void ModuleAssembler::section(const std::string& name, object::SectionKind kind)
{
	flush();
	previous_ = section_;
	for (uint32_t i = 1; i < out_.sections.size(); ++i)
		if (name == out_.string(out_.sections[i].name) && out_.sections[i].kind == kind &&
			out_.sections[i].group == object::kNoGroup)
		{
			section_ = i;
			return;
		}
	section_ = out_.add_section(name.c_str(), kind, 1);
}
void ModuleAssembler::flush()
{
	if (code_.empty())
	{
		if (unwind_.pending())
			unwind_.flush(section_, {}, {}, static_cast<uint32_t>(offset()));
		return;
	}
	object::Section& section = out_.sections[section_];
	if (section.kind != object::SectionKind::Text)
		throw BackendError("module instructions require a text section");
	std::vector<assembly::Instruction> instructions;
	try
	{
		instructions = assembly::parse(code_, syntax_.dialect, uses_, {}, &symbols_, &syntax_);
	}
	catch (const assembly::AssemblyError& error)
	{
		const uint32_t line = error.line != 0 && error.line <= code_lines_.size() ? code_lines_[error.line - 1]
																				  : error.line;
		throw BackendError("module assembly line " + std::to_string(line) + ": " + error.message);
	}
	std::vector<uint32_t> buffer_lines;
	for (const assembly::Instruction& instruction : instructions)
	{
		buffer_lines.push_back(instruction.line);
		section.align = std::max<uint64_t>(section.align, instruction.alignment);
	}
	for (assembly::Instruction& instruction : instructions)
		if (instruction.line != 0 && instruction.line <= code_lines_.size())
			instruction.line = code_lines_[instruction.line - 1];
	Assembler assembler(section.bytes);
	std::vector<uint32_t> positions;
	encode_assembly(assembler, instructions, RM(), &positions);
	if (!assembler.resolve_labels())
		throw BackendError("unresolved module assembly label");
	section.size = section.bytes.size();
	for (uint32_t i = 0; i < instructions.size(); ++i)
		if (instructions[i].label && instructions[i].mnemonic.find_first_not_of("0123456789") != std::string::npos)
			symbols_.define(instructions[i].symbol_definition != asm_model::kNoId
					? instructions[i].symbol_definition
					: symbols_.named(instructions[i].mnemonic),
				section_, positions[i]);
	for (const CodeFixup& fixup : assembler.fixups())
	{
		object::Relocation relocation;
		relocation.offset = fixup.offset;
		relocation.symbol = fixup.target;
		relocation.addend = fixup.addend - (fixup.absolute ? 0 : fixup.end);
		relocation.type = fixup.absolute        ? object::R_X86_64_64
			: fixup.kind == FixupKind::Got      ? object::R_X86_64_GOTPCREL
			: fixup.kind == FixupKind::GotTpOff ? object::R_X86_64_GOTTPOFF
			: fixup.kind == FixupKind::Tlv      ? object::R_MACHO_TLV
			: fixup.kind == FixupKind::TpOff    ? object::R_X86_64_TPOFF32
			: fixup.branch                      ? object::R_X86_64_PLT32
												: object::R_X86_64_PC32;
		section.relocations.push_back(relocation);
	}
	unwind_.flush(section_, buffer_lines, positions, static_cast<uint32_t>(section.size));
	code_.clear();
	uses_.clear();
	code_lines_.clear();
}
void ModuleAssembler::declare(const assembly::ModuleLine& line)
{
	assembly::ModuleCursor cursor(line);
	const std::string name = cursor.word();
	if (name == ".globl" || name == ".global" || name == ".weak" || name == ".weak_definition" || name == ".local" ||
		name == ".hidden" || name == ".private_extern" || name == ".type")
	{
		directive(name, cursor);
		cursor.finish();
	}
}
void ModuleAssembler::append(const assembly::ModuleLine& line)
{
	assembly::ModuleCursor cursor(line);
	const std::string name = cursor.word();
	const bool label = cursor.take(':');
	if (!label && name.compare(0, 5, ".cfi_") == 0)
	{
		if (out_.sections[section_].kind != object::SectionKind::Text)
			cursor.fail("CFI requires a code section");
		unwind_.record(line, static_cast<uint32_t>(code_lines_.size() + 1), &symbols_);
		return;
	}
	const bool syntax = !label && (name == ".intel_syntax" || name == ".att_syntax");
	if (syntax && out_.sections[section_].kind != object::SectionKind::Text)
	{
		assembly::parse(line.text, syntax_.dialect, line.symbols, line.labels, &symbols_, &syntax_);
		return;
	}
	const bool code_data = out_.sections[section_].kind == object::SectionKind::Text &&
		(name == ".p2align" || name == ".balign" || name == ".align" || name == ".zero" || name == ".space" ||
			name == ".skip" ||
			((name == ".byte" || name == ".short" || name == ".word" || name == ".long" || name == ".quad") &&
				cursor.numeric()));
	if (name[0] == '.' && !label && !syntax && !code_data)
	{
		flush();
		if (!directive(name, cursor))
			cursor.fail("module directive is not implemented: " + name);
		cursor.finish();
		return;
	}
	if (out_.sections[section_].kind != object::SectionKind::Text)
	{
		if (!label)
			cursor.fail("instructions require a text section");
		cursor.finish();
		assembly::ModuleCursor definition(line);
		symbols_.define(symbol(definition), section_, offset());
		return;
	}
	if (code_.size() + line.text.size() > 64u * 1024u * 1024u)
		cursor.fail("module code exceeds the byte limit");
	code_lines_.push_back(line.line);
	const uint32_t first = static_cast<uint32_t>(code_.size());
	code_ += line.text;
	code_ += '\n';
	for (assembly::SymbolUse use : line.symbols)
	{
		use.first += first;
		uses_.push_back(use);
	}
}
void ModuleAssembler::begin(asm_model::Dialect dialect)
{
	flush();
	syntax_.dialect = dialect;
	syntax_.register_prefix = dialect == asm_model::Dialect::Att;
}
void ModuleAssembler::finish()
{
	flush();
	unwind_.finish();
	if (!section_stack_.empty())
		throw BackendError("unbalanced module section stack");
	symbols_.finish();
}
void emit_assembly_items(object::ObjectFile& out, const lowir::Program& program, Target target,
	const std::vector<uint32_t>& mapping, AssemblyEnvironment& environment)
{
	if (!program.assembly || (program.assembly->globals.empty() && program.assembly->functions.empty()))
		return;
	try
	{
		auto sources = environment.globals();
		for (auto& source : sources)
			for (auto& line : source)
				remap_assembly_symbols(line.symbols, mapping);
		ModuleAssembler assembler(out, target);
		for (const std::vector<assembly::ModuleLine>& source : sources)
			for (const assembly::ModuleLine& line : source)
				assembler.declare(line);
		for (const lowir::AssemblyFunction& function : program.assembly->functions)
			emit_assembly_function(assembler, program, function, mapping, environment);
		assembler.module_start();
		for (uint32_t i = 0; i < sources.size(); ++i)
		{
			assembler.begin(program.assembly->templates[program.assembly->globals[i].definition].dialect);
			for (const assembly::ModuleLine& line : sources[i])
				assembler.append(line);
		}
		assembler.finish();
	}
	catch (const assembly::AssemblyError& error)
	{
		throw BackendError("module assembly line " + std::to_string(error.line) + ": " + error.message);
	}
}
} // namespace native
