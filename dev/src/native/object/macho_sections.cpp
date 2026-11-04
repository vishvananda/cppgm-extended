#include "native/object/macho_encoder.h"

#include <algorithm>
#include <cstring>

namespace native {
namespace object {
namespace macho {

bool Encoder::fail(const std::string& message)
{
	error_ = "Mach-O: " + message;
	return false;
}

uint32_t Encoder::section(const char* segment, const std::string& name, uint32_t flags)
{
	for (uint32_t i = 0; i < sections_.size(); ++i)
		if (sections_[i].segment == segment && sections_[i].name == name && sections_[i].flags == flags)
			return i;
	Section entry;
	entry.segment = segment;
	entry.name = name;
	entry.flags = flags;
	sections_.push_back(std::move(entry));
	return static_cast<uint32_t>(sections_.size() - 1);
}

bool Encoder::sections()
{
	placements_.resize(input_.sections.size());
	for (uint32_t s = 1; s < input_.sections.size(); ++s)
	{
		const object::Section& source = input_.sections[s];
		const std::string spelling = input_.string(source.name);
		const char* segment = "__DATA";
		std::string name;
		uint32_t flags = 0;
		switch (source.kind)
		{
		case SectionKind::Text:
			segment = "__TEXT";
			name = "__text";
			flags = 0x80000400; // PURE_INSTRUCTIONS | SOME_INSTRUCTIONS
			break;
		case SectionKind::Rodata:
			segment = "__TEXT";
			name = spelling == ".eh_frame"        ? "__eh_frame"
				: spelling == ".gcc_except_table" ? "__gcc_except_tab"
												  : "__const";
			if (spelling == ".eh_frame")
				flags = 0x6800000b; // coalesced, no TOC, strip static symbols, live support
			break;
		case SectionKind::Bss:
			name = "__bss";
			flags = 1;
			break;
		case SectionKind::TlsData:
			name = "__thread_data";
			flags = 0x11;
			break;
		case SectionKind::TlsBss:
			name = "__thread_bss";
			flags = 0x12;
			break;
		case SectionKind::InitArray:
			name = "__mod_init_func";
			flags = 9;
			break;
		case SectionKind::FiniArray:
			name = "__mod_term_func";
			flags = 10;
			break;
		case SectionKind::Metadata:
		case SectionKind::MetadataStrings:
			segment = "__DWARF";
			name = spelling.empty() ? "__metadata" : "__" + spelling.substr(spelling[0] == '.' ? 1 : 0);
			flags = 0x02000000; // DEBUG
			break;
		default:
			name = spelling.compare(0, 12, ".data.rel.ro") == 0 ? "__const" : "__data";
			break;
		}
		// Preserve user section names; COMDAT's ELF suffix is an atom here,
		// not a Mach-O section (Mach-O permits only 255 sections per object).
		const bool standard = spelling.compare(0, 5, ".text") == 0 || spelling.compare(0, 5, ".data") == 0 ||
			spelling.compare(0, 7, ".rodata") == 0 || spelling.compare(0, 4, ".bss") == 0 ||
			spelling.compare(0, 6, ".tdata") == 0 || spelling.compare(0, 5, ".tbss") == 0 ||
			spelling == ".init_array" || spelling == ".fini_array" || spelling == ".eh_frame" ||
			spelling == ".gcc_except_table" || spelling.compare(0, 7, ".debug_") == 0;
		if (!standard && !spelling.empty())
		{
			const size_t comma = spelling.find(',');
			if (comma != std::string::npos)
			{
				// A native section attribute: segment,section (no assembler flags).
				const std::string custom_segment = spelling.substr(0, comma);
				name = spelling.substr(comma + 1);
				if (custom_segment != "__TEXT" && custom_segment != "__DATA")
					return fail("unsupported section segment: " + spelling);
				segment = custom_segment == "__TEXT" ? "__TEXT" : "__DATA";
			}
			else
				name = spelling;
		}
		if (name.size() > 16 || source.align == 0 || (source.align & (source.align - 1)) != 0 ||
			source.align > (1u << 24))
			return fail("invalid section name or alignment: " + spelling);
		const uint32_t index = section(segment, name, flags);
		Section& output = sections_[index];
		const uint64_t base = align_up(output.size, source.align);
		placements_[s] = Placement{ index, base };
		output.align = std::max(output.align, source.align);
		output.size = base + source.size;
		if (source.kind != SectionKind::Bss && source.kind != SectionKind::TlsBss)
		{
			if (source.bytes.size() != source.size)
				return fail("section size disagrees with its bytes");
			output.bytes.resize(base, 0);
			output.bytes.insert(output.bytes.end(), source.bytes.begin(), source.bytes.end());
		}
	}
	return sections_.size() <= 255 || fail("more than 255 sections");
}

uint32_t Encoder::symbol(const std::string& name, uint8_t type, uint8_t section, uint16_t desc, uint64_t value)
{
	symbols_.push_back(Symbol{ name, type, section, desc, value, 0 });
	const uint32_t index = static_cast<uint32_t>(symbols_.size() - 1);
	if (section != 0)
		address_symbols_.insert(std::make_pair(std::make_pair(section - 1, value), index));
	return index;
}

void Encoder::tlv(uint32_t original, const object::Symbol& source, Symbol& target)
{
	if (!has_tlv_)
	{
		has_tlv_ = true;
		tlv_section_ = section("__DATA", "__thread_vars", 0x13);
		sections_[tlv_section_].align = 8;
		bootstrap_ = symbol("___tlv_bootstrap", 1, 0, 0, 0);
	}
	const uint32_t storage = symbol("Lcppgm_tlv_" + std::to_string(original), 0x0e, target.section, 0, target.value);
	Section& variables = sections_[tlv_section_];
	const uint32_t at = static_cast<uint32_t>(variables.bytes.size());
	variables.bytes.resize(at + 24, 0);
	variables.size = variables.bytes.size();
	fixup(tlv_section_, at, bootstrap_, 0, 8, false);
	fixup(tlv_section_, at + 16, storage, 0, 8, false);
	target.section = static_cast<uint8_t>(tlv_section_ + 1);
	target.value = at;
	(void)source;
}

bool Encoder::symbols()
{
	symbol_index_.resize(input_.symbols.size(), 0);
	symbol("Lcppgm_absolute_zero", 2, 0, 0, 0);
	for (uint32_t i = 1; i < input_.symbols.size(); ++i)
	{
		const object::Symbol& source = input_.symbols[i];
		if (source.type == SymbolType::File)
			continue;
		const bool defined = source.section != kUndefinedSection;
		const bool external = source.binding != Binding::Local;
		const bool section_symbol = source.type == SymbolType::Section;
		Symbol target;
		target.name = section_symbol                ? "Lcppgm_section_" + std::to_string(i)
			: source.name == 0                      ? "Lcppgm_anonymous_" + std::to_string(i)
			: input_.string(source.name)[0] == '\1' ? std::string(input_.string(source.name) + 1)
													: std::string("_") + input_.string(source.name);
		target.type = external ? 1 : 0;
		target.section = 0;
		target.desc = source.binding == Binding::Weak ? (defined ? 0x80 : 0x40) : 0;
		target.value = source.value;
		if (source.visibility == Visibility::Hidden || source.visibility == Visibility::Internal)
			target.type |= 0x10; // private external
		if (source.section == kAbsoluteSection)
			target.type |= 2;
		else if (source.section == kCommonSection)
		{
			target.value = source.size;
			unsigned alignment = 0;
			while ((1ull << alignment) < source.value && alignment < 16)
				++alignment;
			if (alignment > 15)
				return fail("common symbol alignment exceeds Mach-O's limit");
			target.desc |= static_cast<uint16_t>(alignment << 8);
		}
		else if (defined)
		{
			if (source.section >= placements_.size())
				return fail("invalid symbol section");
			const Placement placement = placements_[source.section];
			target.type |= 0x0e;
			target.section = static_cast<uint8_t>(placement.section + 1);
			target.value += placement.base;
			if (source.type == SymbolType::Tls)
				tlv(i, source, target);
		}
		symbol_index_[i] = symbol(target.name, target.type, target.section, target.desc, target.value);
		if (source.type == SymbolType::Function && target.section != 0)
			address_symbols_[std::make_pair(target.section - 1, target.value)] = symbol_index_[i];
	}
	return sections_.size() <= 255 || fail("more than 255 sections");
}

} // namespace macho
} // namespace object
} // namespace native
