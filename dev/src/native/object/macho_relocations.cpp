#include "native/object/macho_encoder.h"

#include <algorithm>

namespace native {
namespace object {
namespace macho {

void Encoder::fixup(
	uint32_t section, uint32_t at, uint32_t symbol, uint32_t type, unsigned width, bool pcrel, bool external)
{
	const unsigned length = width == 8 ? 3 : width == 4 ? 2 : width == 2 ? 1 : 0;
	const uint32_t bits = symbol | (pcrel ? 1u << 24 : 0) | (length << 25) | (external ? 1u << 27 : 0) | (type << 28);
	sections_[section].fixups.push_back(std::vector<Reloc>{ Reloc{ at, bits } });
}

uint32_t Encoder::base_symbol(uint32_t section, uint64_t offset)
{
	// Mach-O linkers atomize __eh_frame at CIE/FDE boundaries and refuse
	// symbols inside a record. Base its expression at the record's start.
	if (sections_[section].name == "__eh_frame")
	{
		Section& frame = sections_[section];
		if (frame.records.empty())
		{
			const std::vector<uint8_t>& bytes = frame.bytes;
			uint64_t record = 0;
			while (record + 4 <= bytes.size())
			{
				frame.records.push_back(record);
				uint32_t length = 0;
				for (unsigned i = 0; i < 4; ++i)
					length |= static_cast<uint32_t>(bytes[record + i]) << (8 * i);
				if (length == 0 || length > bytes.size() - record - 4)
					break;
				record += 4 + length;
			}
		}
		const auto record = std::upper_bound(frame.records.begin(), frame.records.end(), offset);
		if (record != frame.records.begin())
			offset = *(record - 1);
	}
	return symbol(
		"Lcppgm_pcrel_" + std::to_string(symbols_.size()), 0x0e, static_cast<uint8_t>(section + 1), 0, offset);
}

bool Encoder::relocation(uint32_t source, const Relocation& r)
{
	const Placement p = placements_[source];
	Section& section = sections_[p.section];
	const uint64_t at = p.base + r.offset;
	if (r.symbol >= symbol_index_.size() || at > 0x7fffffffu)
		return fail("relocation index or offset out of range");
	unsigned width = 4;
	uint32_t type = 0;
	bool relative = false;
	bool difference = false;
	int64_t addend = r.addend;
	switch (r.type)
	{
	case R_X86_64_NONE:
		return true;
	case R_X86_64_64:
		width = 8;
		break;
	case R_X86_64_32:
	case R_X86_64_32S:
		break;
	case R_X86_64_16:
		width = 2;
		break;
	case R_X86_64_8:
		width = 1;
		break;
	case R_X86_64_PC64:
		width = 8;
		difference = true;
		break;
	case R_X86_64_PC32:
		if (input_.sections[source].kind != SectionKind::Text)
			difference = true;
		else
		{
			type = 1; // SIGNED: Mach-O's PC base is four bytes past the field
			relative = true;
			addend += 4;
		}
		break;
	case R_X86_64_PLT32:
		type = 2; // BRANCH
		relative = true;
		addend += 4;
		break;
	case R_X86_64_GOTPCREL:
		type = 4; // GOT
		relative = true;
		addend += 4;
		break;
	case R_X86_64_GOTPCRELX:
	case R_X86_64_REX_GOTPCRELX:
		type = 3; // GOT_LOAD
		relative = true;
		addend += 4;
		break;
	case R_MACHO_TLV:
		type = 9; // TLV
		relative = true;
		addend += 4;
		break;
	default:
		return fail("unsupported relocation type " + std::to_string(r.type));
	}
	if (at > section.bytes.size() || width > section.bytes.size() - at)
		return fail("relocation outside section bytes");
	patch(section.bytes, at, static_cast<uint64_t>(addend), width);
	const Symbol& target = symbols_[symbol_index_[r.symbol]];
	const SectionKind source_kind = input_.sections[source].kind;
	if ((source_kind == SectionKind::Metadata || source_kind == SectionKind::MetadataStrings) && !relative &&
		!difference && target.section != 0)
	{
		// Mach-O DWARF contains object addresses in place, with section
		// relocations. DW_FORM_strp/sec_offset instead stay section-relative.
		patch(section.bytes, at, target.value + r.addend, width);
		if (sections_[target.section - 1].segment != "__DWARF")
			fixup(p.section, static_cast<uint32_t>(at), target.section, 0, width, false, false);
		return true;
	}
	if (difference)
	{
		// A data PC-relative value is target - field, expressed with the
		// x86-64 SUBTRACTOR/UNSIGNED pair, never an instruction relocation.
		const uint32_t base = base_symbol(p.section, at);
		const Symbol target = symbols_[symbol_index_[r.symbol]];
		if (target.section == 0)
			return fail("data PC-relative relocation against an undefined symbol");
		const uint64_t address = target.value + r.addend;
		const auto found = address_symbols_.find(std::make_pair(target.section - 1, address));
		const uint32_t named = found != address_symbols_.end()
			? found->second
			: symbol("Lcppgm_address_" + std::to_string(symbols_.size()), 0x0e, target.section, 0, address);
		patch(section.bytes, at, symbols_[base].value - at, width);
		fixup(p.section, static_cast<uint32_t>(at), base, 5, width, false);
		fixup(p.section, static_cast<uint32_t>(at), named, 0, width, false);
		auto& fixups = sections_[p.section].fixups;
		fixups[fixups.size() - 2].push_back(fixups.back()[0]);
		fixups.pop_back();
	}
	else
		fixup(p.section, static_cast<uint32_t>(at), symbol_index_[r.symbol], type, width, relative);
	return true;
}

bool Encoder::relocations()
{
	for (uint32_t s = 1; s < input_.sections.size(); ++s)
		for (const Relocation& r : input_.sections[s].relocations)
			if (!relocation(s, r))
				return false;
	return symbols_.size() < (1u << 24) || fail("too many relocation symbols");
}

} // namespace macho
} // namespace object
} // namespace native
