#include "native/object/macho_reader.h"

namespace native {
namespace object {
namespace macho {

uint32_t Reader::section_symbol(uint32_t section, uint64_t address, int64_t& addend)
{
	const InputSection& source = sections_[section];
	if (address < source.address || address - source.address > source.size)
		return 0;
	const uint64_t offset = address - source.address;
	const uint32_t index = atom(section, offset);
	const uint32_t output = source.atoms[index];
	addend = static_cast<int64_t>(offset - source.boundaries[index]);
	if (section_symbols_[output] == 0)
	{
		Symbol symbol;
		symbol.section = output;
		symbol.type = SymbolType::Section;
		section_symbols_[output] = object_.add_symbol(symbol);
	}
	return section_symbols_[output];
}

bool Reader::relocation(uint32_t s, uint32_t& index)
{
	const InputSection& source = sections_[s];
	const uint64_t at = source.relocations + static_cast<uint64_t>(index) * 8;
	const uint64_t offset = get(at, 4);
	const uint32_t bits = static_cast<uint32_t>(get(at + 4, 4));
	const uint32_t type = bits >> 28;
	const unsigned width = 1u << ((bits >> 25) & 3);
	const bool external = (bits & (1u << 27)) != 0;
	const bool relative = (bits & (1u << 24)) != 0;
	const uint32_t target = bits & 0xffffff;
	if (offset > 0x7fffffffu || offset > source.size || width > source.size - offset ||
		!room(static_cast<uint64_t>(source.offset) + offset, width))
		return fail("relocation outside section");
	const uint32_t part = atom(s, offset);
	Section& section = object_.sections[source.atoms[part]];
	if (width > section.size - (offset - source.boundaries[part]))
		return fail("relocation crosses an atom boundary");
	uint64_t value = get(static_cast<uint64_t>(source.offset) + offset, width);
	if (width < 8 && (value & (1ull << (width * 8 - 1))) != 0)
		value |= ~((1ull << (width * 8)) - 1);
	Relocation r;
	r.offset = offset - source.boundaries[part];
	r.addend = static_cast<int64_t>(value);
	if (external)
	{
		if (target >= symbols_.size() || symbols_[target].output == 0)
			return fail("invalid relocation symbol");
		r.symbol = symbols_[target].output;
	}
	else
	{
		if (target == 0 || target > sections_.size())
			return fail("invalid relocation section");
		const uint64_t address = value + (relative ? source.address + offset + 4 : 0);
		r.symbol = section_symbol(target - 1, address, r.addend);
		if (r.symbol == 0)
			return fail("local relocation outside its target section");
	}
	if (type == 5)
	{
		if (relative || !external || ++index >= source.count)
			return fail("invalid subtractor relocation pair");
		const uint32_t next = static_cast<uint32_t>(get(at + 12, 4));
		const uint32_t next_symbol = next & 0xffffff;
		const InputSymbol& base = symbols_[target];
		if (get(at + 8, 4) != offset || (next & 0xff000000u) != ((bits & 0x06000000u) | 0x08000000u) ||
			next_symbol >= symbols_.size() || symbols_[next_symbol].output == 0 || base.section != s + 1 ||
			atom(s, base.value - source.address) != part)
			return fail("unsupported subtractor expression");
		r.symbol = symbols_[next_symbol].output;
		r.addend += static_cast<int64_t>(source.address + offset - base.value);
		r.type = width == 8 ? R_X86_64_PC64 : R_X86_64_PC32;
	}
	else if (type == 0 && !relative)
		r.type = width == 8 ? R_X86_64_64 : width == 4 ? R_X86_64_32 : width == 2 ? R_X86_64_16 : R_X86_64_8;
	else if (relative && width == 4)
	{
		if (type == 1 || (type >= 6 && type <= 8))
			r.type = R_X86_64_PC32;
		else if (type == 2)
			r.type = R_X86_64_PLT32;
		else if (type == 3)
			r.type = R_X86_64_REX_GOTPCRELX;
		else if (type == 4)
			r.type = R_X86_64_GOTPCREL;
		else if (type == 9)
			r.type = R_MACHO_TLV;
		else
			return fail("unsupported PC-relative relocation");
		r.addend -= 4;
		if (type >= 6 && type <= 8)
			r.addend -= 1u << (type - 6);
	}
	else
		return fail("unsupported relocation");
	section.relocations.push_back(r);
	return true;
}

bool Reader::relocations()
{
	for (uint32_t s = 0; s < sections_.size(); ++s)
	{
		if (sections_[s].kind == SectionKind::Null)
			continue;
		for (uint32_t i = 0; i < sections_[s].count; ++i)
			if (!relocation(s, i))
				return false;
	}
	return true;
}

} // namespace macho
} // namespace object
} // namespace native
