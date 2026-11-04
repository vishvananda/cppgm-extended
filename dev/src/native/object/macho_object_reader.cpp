#include "native/object/macho_reader.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>

namespace native {
namespace object {
namespace macho {

bool Reader::fail(const char* message)
{
	error_ = "Mach-O: " + std::string(message);
	return false;
}

bool Reader::room(uint64_t at, uint64_t count) const
{
	return at <= bytes_.size() && count <= bytes_.size() - at;
}

uint64_t Reader::get(uint64_t at, unsigned width) const
{
	uint64_t result = 0;
	for (unsigned i = 0; i < width; ++i)
		result |= static_cast<uint64_t>(bytes_[at + i]) << (8 * i);
	return result;
}

std::string Reader::name(uint64_t at) const
{
	size_t length = 0;
	while (length < 16 && bytes_[at + length] != 0)
		++length;
	return std::string(reinterpret_cast<const char*>(bytes_.data() + at), length);
}

namespace {

SectionKind kind(const std::string& segment, const std::string& name, uint32_t flags)
{
	if (segment == "__DWARF" || name == "__eh_frame" || name == "__compact_unwind")
		return SectionKind::Null;
	if ((flags & 0x80000400) != 0)
		return SectionKind::Text;
	switch (flags & 0xff)
	{
	case 1:
		return SectionKind::Bss;
	case 9:
		return SectionKind::InitArray;
	case 10:
		return SectionKind::FiniArray;
	case 0x11:
		return SectionKind::TlsData;
	case 0x12:
		return SectionKind::TlsBss;
	default:
		return segment == "__TEXT" ? SectionKind::Rodata : SectionKind::Data;
	}
}

} // namespace

bool Reader::commands()
{
	if (!room(0, 32) || get(0, 4) != 0xfeedfacf || get(4, 4) != 0x01000007 || get(12, 4) != 1)
		return fail("not an x86-64 relocatable object");
	const uint64_t end = 32 + get(20, 4);
	if (!room(32, end - 32))
		return fail("truncated load commands");
	uint64_t at = 32;
	for (uint64_t i = 0; i < get(16, 4); ++i)
	{
		if (at > end || end - at < 8)
			return fail("truncated load command");
		const uint64_t size = get(at + 4, 4);
		if (size < 8 || size > end - at)
			return fail("invalid load command size");
		if (get(at, 4) == 0x19)
		{
			if (size < 72 || get(at + 64, 4) > (size - 72) / 80)
				return fail("truncated section table");
			for (uint64_t j = 0; j < get(at + 64, 4); ++j)
			{
				const uint64_t s = at + 72 + j * 80;
				const uint32_t flags = static_cast<uint32_t>(get(s + 64, 4));
				InputSection section;
				section.name = name(s);
				section.kind = kind(name(s + 16), section.name, flags);
				section.address = get(s + 32, 8);
				section.size = get(s + 40, 8);
				section.offset = static_cast<uint32_t>(get(s + 48, 4));
				const uint64_t power = get(s + 52, 4);
				if (power > 24 || section.address > UINT64_MAX - section.size)
					return fail("invalid section extent or alignment");
				section.align = 1u << power;
				section.relocations = static_cast<uint32_t>(get(s + 56, 4));
				section.count = static_cast<uint32_t>(get(s + 60, 4));
				if (!room(section.relocations, static_cast<uint64_t>(section.count) * 8))
					return fail("truncated relocations");
				if (section.kind != SectionKind::Bss && section.kind != SectionKind::TlsBss &&
					section.kind != SectionKind::Null && !room(section.offset, section.size))
					return fail("truncated section bytes");
				sections_.push_back(std::move(section));
			}
		}
		else if (get(at, 4) == 2)
		{
			if (size != 24 || symtab_ != 0)
				return fail("invalid symbol table command");
			symtab_ = static_cast<uint32_t>(get(at + 8, 4));
			symbol_count_ = static_cast<uint32_t>(get(at + 12, 4));
			strtab_ = static_cast<uint32_t>(get(at + 16, 4));
			strings_ = static_cast<uint32_t>(get(at + 20, 4));
		}
		at += size;
	}
	if (at != end || sections_.size() > 255 || !room(symtab_, static_cast<uint64_t>(symbol_count_) * 16) ||
		!room(strtab_, strings_))
		return fail("invalid object tables");
	return true;
}

bool Reader::symbols()
{
	symbols_.reserve(symbol_count_);
	for (uint32_t i = 0; i < symbol_count_; ++i)
	{
		const uint64_t at = symtab_ + static_cast<uint64_t>(i) * 16;
		const uint64_t offset = get(at, 4);
		if (offset >= strings_)
			return fail("symbol string outside table");
		const char* start = reinterpret_cast<const char*>(bytes_.data() + strtab_ + offset);
		const char* end = static_cast<const char*>(std::memchr(start, 0, strings_ - offset));
		if (end == nullptr)
			return fail("unterminated symbol name");
		InputSymbol symbol;
		symbol.name.assign(start, end);
		symbol.type = static_cast<uint8_t>(get(at + 4, 1));
		symbol.section = static_cast<uint8_t>(get(at + 5, 1));
		symbol.desc = static_cast<uint16_t>(get(at + 6, 2));
		symbol.value = get(at + 8, 8);
		if ((symbol.type & 0xe0) == 0 && (symbol.type & 0x0e) == 0x0e)
		{
			if (symbol.section == 0 || symbol.section > sections_.size())
				return fail("invalid symbol section");
			InputSection& section = sections_[symbol.section - 1];
			if (symbol.value < section.address || symbol.value - section.address > section.size)
				return fail("symbol outside section");
			if (!symbol.name.empty() && symbol.name[0] != 'L' && symbol.value - section.address < section.size)
				section.boundaries.push_back(symbol.value - section.address);
		}
		symbols_.push_back(std::move(symbol));
	}
	return true;
}

uint32_t Reader::atom(uint32_t section, uint64_t offset) const
{
	const InputSection& s = sections_[section];
	const auto at = std::upper_bound(s.boundaries.begin(), s.boundaries.end(), offset);
	return static_cast<uint32_t>(at - s.boundaries.begin() - 1);
}

bool Reader::atoms()
{
	for (InputSection& source : sections_)
	{
		source.boundaries.push_back(0);
		std::sort(source.boundaries.begin(), source.boundaries.end());
		source.boundaries.erase(
			std::unique(source.boundaries.begin(), source.boundaries.end()), source.boundaries.end());
		for (size_t i = 0; i < source.boundaries.size(); ++i)
		{
			const uint64_t base = source.boundaries[i];
			const uint64_t end = i + 1 < source.boundaries.size() ? source.boundaries[i + 1] : source.size;
			uint64_t alignment = source.align;
			while (base % alignment != 0)
				alignment >>= 1;
			const uint32_t section = object_.add_section(source.name.c_str(), source.kind, alignment);
			Section& output = object_.sections[section];
			output.size = end - base;
			if (source.kind != SectionKind::Bss && source.kind != SectionKind::TlsBss &&
				source.kind != SectionKind::Null)
				output.bytes.assign(bytes_.begin() + source.offset + base, bytes_.begin() + source.offset + end);
			source.atoms.push_back(section);
		}
	}
	for (InputSymbol& source : symbols_)
	{
		if ((source.type & 0xe0) != 0)
			continue; // STABS are debug records, not link symbols
		Symbol symbol;
		const bool external = (source.type & 1) != 0;
		symbol.binding = !external ? Binding::Local : (source.desc & 0xc0) != 0 ? Binding::Weak : Binding::Global;
		symbol.visibility = (source.type & 0x10) != 0 ? Visibility::Hidden : Visibility::Default;
		symbol.value = source.value;
		const std::string name = !source.name.empty() && source.name[0] == '_' ? source.name.substr(1) : source.name;
		symbol.name = object_.add_string(name);
		switch (source.type & 0x0e)
		{
		case 0:
			if (source.value != 0)
			{
				symbol.section = kCommonSection;
				symbol.size = source.value;
				symbol.value = 1ull << ((source.desc >> 8) & 15);
			}
			break;
		case 2:
			symbol.section = kAbsoluteSection;
			break;
		case 0x0e:
		{
			const InputSection& section = sections_[source.section - 1];
			const uint64_t offset = source.value - section.address;
			const uint32_t index = atom(source.section - 1, offset);
			symbol.section = section.atoms[index];
			symbol.value = offset - section.boundaries[index];
			symbol.type = section.kind == SectionKind::Text ? SymbolType::Function : SymbolType::Object;
			break;
		}
		default:
			return fail("unsupported symbol type");
		}
		source.output = object_.add_symbol(symbol);
		if (external && symbol.binding == Binding::Weak && symbol.section < object_.sections.size() &&
			symbol.section != 0 && object_.sections[symbol.section].group == kNoGroup)
		{
			Group group;
			group.signature = source.output;
			group.sections.push_back(symbol.section);
			object_.sections[symbol.section].group = static_cast<uint32_t>(object_.groups.size());
			object_.groups.push_back(std::move(group));
		}
	}
	section_symbols_.resize(object_.sections.size(), 0);
	return true;
}

bool Reader::read()
{
	return commands() && symbols() && atoms() && relocations();
}

} // namespace macho

bool read_macho_object(const std::string& path, ObjectFile& object, std::string& error)
{
	std::ifstream file(path, std::ios::binary);
	if (!file)
	{
		error = "cannot read " + path;
		return false;
	}
	const std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	object.path = path;
	macho::Reader reader(bytes, object, error);
	return reader.read();
}

} // namespace object
} // namespace native
