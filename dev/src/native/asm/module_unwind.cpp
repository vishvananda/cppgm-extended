#include "native/asm/module_unwind.h"

#include <cstring>

namespace native {
namespace {
bool initial_rule(AssemblyCfiOp op)
{
	switch (op)
	{
	case AssemblyCfiOp::DefCfa:
	case AssemblyCfiOp::DefRegister:
	case AssemblyCfiOp::DefOffset:
	case AssemblyCfiOp::Adjust:
	case AssemblyCfiOp::Offset:
	case AssemblyCfiOp::RelOffset:
	case AssemblyCfiOp::ValOffset:
	case AssemblyCfiOp::Register:
	case AssemblyCfiOp::Restore:
	case AssemblyCfiOp::Undefined:
	case AssemblyCfiOp::Same:
		return true;
	default:
		return false;
	}
}
bool same(const CieDescription& a, const CieDescription& b)
{
	return a.personality == b.personality && a.initial == b.initial && a.signal == b.signal &&
		a.return_column == b.return_column && a.personality_encoding == b.personality_encoding &&
		a.lsda_encoding == b.lsda_encoding && a.initial_rules == b.initial_rules;
}
}
struct ModuleUnwind::Procedure
{
	uint32_t section;
	uint32_t first;
	CieDescription description;
	asm_model::SymbolReference personality;
	asm_model::SymbolReference lsda;
	AssemblyCfiRules rules;
	uint32_t initial_bytes = 0;
	bool initial_open = true;
	Procedure(uint32_t section, uint32_t first, bool simple) : section(section), first(first), rules(simple)
	{
		description.initial = !simple;
	}
};
ModuleUnwind::ModuleUnwind(object::ObjectFile& out, Target target) : out_(out), target_(target)
{}
ModuleUnwind::~ModuleUnwind() = default;
void ModuleUnwind::begin(uint32_t anchor)
{
	AssemblyCfiEvent event;
	event.op = AssemblyCfiOp::Start;
	event.anchor = anchor;
	events_.push_back(event);
}
void ModuleUnwind::end(uint32_t anchor)
{
	AssemblyCfiEvent event;
	event.op = AssemblyCfiOp::End;
	event.anchor = anchor;
	events_.push_back(event);
}
void ModuleUnwind::record(const assembly::ModuleLine& line, uint32_t anchor, assembly::SymbolLookup* symbols)
{
	if (events_.size() >= 1000000)
		throw assembly::AssemblyError{ "CFI directive count exceeds limit", line.line, 1 };
	events_.push_back(parse_assembly_cfi(line, anchor, symbols));
}
void ModuleUnwind::flush(
	uint32_t section, const std::vector<uint32_t>& lines, const std::vector<uint32_t>& positions, uint32_t end)
{
	size_t index = 0;
	for (const AssemblyCfiEvent& event : events_)
	{
		while (index < lines.size() && lines[index] < event.anchor)
			++index;
		apply(event, section, index < positions.size() ? positions[index] : end);
	}
	events_.clear();
}
void ModuleUnwind::apply(const AssemblyCfiEvent& event, uint32_t section, uint32_t position)
{
	if (event.op == AssemblyCfiOp::Sections)
		return;
	if (event.op == AssemblyCfiOp::Start)
	{
		if (current_)
			throw assembly::AssemblyError{ "nested CFI procedures", event.line, 1 };
		current_.reset(new Procedure(section, position, event.simple));
		return;
	}
	if (!current_)
		throw assembly::AssemblyError{ "CFI directive outside a procedure", event.line, 1 };
	if (section != current_->section || position < current_->first)
		throw assembly::AssemblyError{ "CFI procedure changed its code section", event.line, 1 };
	if (event.op == AssemblyCfiOp::End)
	{
		emit(position);
		current_.reset();
	}
	else if (event.op == AssemblyCfiOp::Signal)
		current_->description.signal = true;
	else if (event.op == AssemblyCfiOp::ReturnColumn)
		current_->description.return_column = event.reg;
	else if (event.op == AssemblyCfiOp::Personality)
	{
		current_->description.personality = event.encoding != 0xff;
		current_->description.personality_encoding = event.encoding;
		current_->personality = event.reference;
	}
	else if (event.op == AssemblyCfiOp::Lsda)
	{
		current_->description.lsda_encoding = event.encoding;
		current_->lsda = event.reference;
	}
	else
	{
		current_->rules.apply(event, position - current_->first);
		if (position == current_->first && current_->initial_open && initial_rule(event.op))
			current_->initial_bytes = static_cast<uint32_t>(current_->rules.bytes().size());
		else
			current_->initial_open = false;
	}
}
uint32_t ModuleUnwind::section_symbol(uint32_t section)
{
	for (uint32_t i = 1; i < out_.symbols.size(); ++i)
		if (out_.symbols[i].type == object::SymbolType::Section && out_.symbols[i].section == section)
			return i;
	object::Symbol symbol;
	symbol.type = object::SymbolType::Section;
	symbol.section = section;
	return out_.add_symbol(symbol);
}
void ModuleUnwind::pointer_relocation(
	uint32_t section, uint32_t offset, uint8_t encoding, const asm_model::SymbolReference& reference, bool personality)
{
	const unsigned bytes = unwind_pointer_bytes(encoding);
	const bool relative = (encoding & 0x70) == 0x10;
	uint32_t type = relative    ? (bytes == 8 ? object::R_X86_64_PC64 : object::R_X86_64_PC32)
		: bytes == 8            ? object::R_X86_64_64
		: (encoding & 15) == 11 ? object::R_X86_64_32S
								: object::R_X86_64_32;
	if (target_ == Target::MacOS && personality && encoding == 0x9b)
		type = object::R_X86_64_GOTPCREL;
	out_.sections[section].relocations.push_back(
		object::Relocation{ offset, type, reference.symbol, reference.addend });
}
void ModuleUnwind::emit(uint32_t end)
{
	if (frame_ == 0)
	{
		for (uint32_t i = 1; i < out_.sections.size(); ++i)
			if (std::strcmp(out_.string(out_.sections[i].name), ".eh_frame") == 0 &&
				out_.sections[i].group == object::kNoGroup && out_.sections[i].kind == object::SectionKind::Rodata)
			{
				frame_ = i;
				break;
			}
		if (frame_ == 0)
			frame_ = out_.add_section(".eh_frame", object::SectionKind::Rodata, 8);
	}
	Procedure& procedure = *current_;
	const auto& instructions = procedure.rules.bytes();
	procedure.description.initial_rules.assign(instructions.begin(), instructions.begin() + procedure.initial_bytes);
	auto& section = out_.sections[frame_];
	uint32_t cie = 0;
	bool found = false;
	for (const Cie& item : cies_)
		if (same(item.description, procedure.description) && item.personality == procedure.personality.symbol &&
			item.addend == procedure.personality.addend)
		{
			cie = item.offset;
			found = true;
			break;
		}
	if (!found)
	{
		cie = static_cast<uint32_t>(section.bytes.size());
		const uint32_t field = encode_cie(procedure.description, section.bytes);
		if (procedure.description.personality)
			pointer_relocation(frame_, field, procedure.description.personality_encoding, procedure.personality, true);
		Cie entry;
		entry.description = procedure.description;
		entry.personality = procedure.personality.symbol;
		entry.addend = procedure.personality.addend;
		entry.offset = cie;
		cies_.push_back(entry);
	}
	uint32_t pc_field = 0, lsda_field = 0;
	const auto& rules = procedure.rules.bytes();
	const uint8_t empty = 0;
	FdeDescription description;
	description.cie = cie;
	description.code_bytes = end - procedure.first;
	description.lsda_encoding = procedure.description.lsda_encoding;
	encode_fde(description, rules.size() == procedure.initial_bytes ? &empty : rules.data() + procedure.initial_bytes,
		rules.size() - procedure.initial_bytes, section.bytes, pc_field, lsda_field);
	section.size = section.bytes.size();
	section.relocations.push_back(
		object::Relocation{ pc_field, object::R_X86_64_PC32, section_symbol(procedure.section), procedure.first });
	if (description.lsda_encoding != 0xff)
		pointer_relocation(frame_, lsda_field, description.lsda_encoding, procedure.lsda, false);
}
void ModuleUnwind::finish()
{
	if (current_)
		throw assembly::AssemblyError{ "unterminated CFI procedure", 1, 1 };
}
} // namespace native
