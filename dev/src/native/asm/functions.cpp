#include "native/asm/functions.h"

#include <algorithm>

#include "asm/module_expand.h"
#include "native/asm/binding.h"
#include "native/asm/environment.h"
#include "native/asm/module.h"
#include "native/backend_error.h"

namespace native {
void ModuleAssembler::module_start()
{
	section(".text", object::SectionKind::Text);
	previous_ = 0;
}
void ModuleAssembler::begin_raw(uint32_t id, const std::string& name, bool comdat, bool initial)
{
	flush();
	unwind_.finish();
	if (comdat)
	{
		section_ = out_.add_section(name.c_str(), object::SectionKind::Text, 16);
		object::Group group;
		group.signature = id;
		group.sections.push_back(section_);
		out_.sections[section_].group = static_cast<uint32_t>(out_.groups.size());
		out_.groups.push_back(std::move(group));
	}
	else
		section(name, object::SectionKind::Text);
	auto& text = out_.sections[section_];
	text.align = std::max<uint64_t>(text.align, 16);
	while (text.bytes.size() % 16 != 0)
		text.bytes.push_back(0xcc);
	text.size = text.bytes.size();
	symbols_.bind(id, out_.symbols[id].binding);
	symbols_.define(id, section_, text.size);
	out_.symbols[id].type = object::SymbolType::Function;
	raw_symbol_ = id;
	raw_section_ = section_;
	raw_first_ = static_cast<uint32_t>(text.size);
	raw_initial_ = initial;
	if (initial)
		unwind_.begin(static_cast<uint32_t>(code_lines_.size() + 1));
}
void ModuleAssembler::end_raw()
{
	if (raw_initial_)
		unwind_.end(static_cast<uint32_t>(code_lines_.size() + 1));
	flush();
	unwind_.finish();
	if (section_ != raw_section_ || !section_stack_.empty())
		throw BackendError("assembly function must restore its code section");
	auto& text = out_.sections[section_];
	text.bytes.push_back(0x0f);
	text.bytes.push_back(0x0b);
	text.size = text.bytes.size();
	out_.symbols[raw_symbol_].size = text.size - raw_first_;
	raw_symbol_ = 0;
}
void emit_assembly_function(ModuleAssembler& assembler, const lowir::Program& program,
	const lowir::AssemblyFunction& function, const std::vector<uint32_t>& mapping, AssemblyEnvironment& environment)
{
	const auto& body = *function.body;
	struct Source
	{
		asm_model::Dialect dialect;
		std::vector<assembly::ModuleLine> lines;
	};
	std::vector<Source> sources;
	uint32_t prefix = UINT32_MAX;
	assembly::ModuleExpander expander = environment.fork(0);
	for (const lowir::Instruction& instruction : body.instructions)
	{
		if (instruction.op != lowir::Opcode::Asm)
			continue;
		const std::vector<lowir::Value> arguments(body.operands.begin() + instruction.extra_first,
			body.operands.begin() + instruction.extra_first + instruction.extra_count);
		std::string text;
		std::vector<assembly::SymbolUse> symbols;
		std::vector<assembly::LabelUse> labels;
		prepare_assembly_source(
			*program.assembly, static_cast<uint32_t>(instruction.immediate), body, arguments, text, symbols, labels);
		const auto& definition = program.assembly->templates[instruction.immediate];
		if (prefix != definition.environment)
		{
			prefix = definition.environment;
			expander = environment.fork(prefix);
		}
		Source source;
		source.dialect = program.assembly->templates[instruction.immediate].dialect;
		source.lines = expander.expand(assembly::module_lines(text, symbols, labels));
		for (auto& line : source.lines)
			remap_assembly_symbols(line.symbols, mapping);
		for (const auto& line : source.lines)
			assembler.declare(line);
		sources.push_back(std::move(source));
	}
	const auto& metadata = program.symbols[function.symbol].metadata;
	const bool comdat = metadata.binding == ir_model::SBM_WEAK && (metadata.flags & lowir::SF_REPLACEABLE) == 0;
	assembler.begin_raw(mapping[function.symbol], metadata.section != 0 ? program.spelling(metadata.section) : ".text",
		comdat, function.unwind == asm_model::FunctionUnwind::Initial);
	for (const Source& source : sources)
	{
		assembler.begin(source.dialect);
		for (const auto& line : source.lines)
			assembler.append(line);
	}
	assembler.end_raw();
}
} // namespace native
