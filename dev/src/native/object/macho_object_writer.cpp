#include "native/object/macho_encoder.h"

#include <algorithm>
#include <cstdio>
#include <limits>

namespace native {
namespace object {
namespace macho {

void put(std::vector<uint8_t>& bytes, uint64_t value, unsigned width)
{
	for (unsigned i = 0; i < width; ++i)
		bytes.push_back(static_cast<uint8_t>(value >> (i * 8)));
}

void patch(std::vector<uint8_t>& bytes, uint64_t at, uint64_t value, unsigned width)
{
	for (unsigned i = 0; i < width; ++i)
		bytes[at + i] = static_cast<uint8_t>(value >> (i * 8));
}

void name(std::vector<uint8_t>& bytes, const std::string& value)
{
	for (unsigned i = 0; i < 16; ++i)
		bytes.push_back(i < value.size() ? static_cast<uint8_t>(value[i]) : 0);
}

namespace {

unsigned power(uint64_t alignment)
{
	unsigned result = 0;
	while ((1ull << result) < alignment)
		++result;
	return result;
}

unsigned category(const Symbol& symbol)
{
	return (symbol.type & 1) == 0 ? 0 : (symbol.type & 0x0e) == 0 ? 2 : 1;
}

bool zerofill(const Section& section)
{
	return (section.flags & 0xff) == 1 || (section.flags & 0xff) == 0x12;
}

void header(std::vector<uint8_t>& out, uint32_t commands, uint32_t flags)
{
	put(out, 0xfeedfacf, 4);
	put(out, 0x01000007, 4); // CPU_TYPE_X86_64
	put(out, 3, 4);
	put(out, 1, 4); // MH_OBJECT
	put(out, 4, 4);
	put(out, commands, 4);
	put(out, flags, 4);
	put(out, 0, 4);
}

} // namespace

void Encoder::write(std::vector<uint8_t>& out)
{
	std::vector<uint32_t> order, indexes(symbols_.size());
	uint32_t counts[3] = {};
	std::vector<uint8_t> strings(1, 0);
	for (unsigned kind = 0; kind < 3; ++kind)
		for (uint32_t i = 0; i < symbols_.size(); ++i)
			if (category(symbols_[i]) == kind)
			{
				indexes[i] = static_cast<uint32_t>(order.size());
				order.push_back(i);
				++counts[kind];
				Symbol& symbol = symbols_[i];
				symbol.string = static_cast<uint32_t>(strings.size());
				strings.insert(strings.end(), symbol.name.begin(), symbol.name.end());
				strings.push_back(0);
			}
	const uint32_t segment_bytes = 72 + static_cast<uint32_t>(sections_.size()) * 80;
	const uint32_t commands = segment_bytes + 24 + 80 + 24;
	const uint64_t contents = align_up(32 + commands, 16);
	uint64_t file = contents, address = 0;
	for (Section& section : sections_)
	{
		address = align_up(address, section.align);
		section.address = address;
		address += section.size;
		if (!zerofill(section))
		{
			file = align_up(file, section.align);
			section.offset = static_cast<uint32_t>(file);
			file += section.bytes.size();
		}
	}
	const uint64_t contents_end = file;
	for (Section& section : sections_)
		for (const auto& group : section.fixups)
			for (const Reloc& r : group)
				if ((r.bits & (1u << 27)) == 0)
				{
					const unsigned width = 1u << ((r.bits >> 25) & 3);
					uint64_t addend = 0;
					for (unsigned k = 0; k < width; ++k)
						addend |= static_cast<uint64_t>(section.bytes[r.at + k]) << (8 * k);
					patch(section.bytes, r.at, addend + sections_[(r.bits & 0xffffffu) - 1].address, width);
				}
	file = align_up(file, 8);
	for (Section& section : sections_)
	{
		section.relocations = section.fixups.empty() ? 0 : static_cast<uint32_t>(file);
		std::stable_sort(section.fixups.begin(), section.fixups.end(),
			[](const std::vector<Reloc>& a, const std::vector<Reloc>& b) { return a[0].at > b[0].at; });
		for (const auto& group : section.fixups)
			file += group.size() * 8;
	}
	const uint32_t symtab = static_cast<uint32_t>(file);
	const uint32_t strtab = symtab + static_cast<uint32_t>(symbols_.size()) * 16;
	out.clear();
	out.reserve(strtab + strings.size());
	header(out, commands, 0x2000 | (has_tlv_ ? 0x800000 : 0)); // SUBSECTIONS_VIA_SYMBOLS, HAS_TLV_DESCRIPTORS
	put(out, 0x19, 4);                                         // LC_SEGMENT_64
	put(out, segment_bytes, 4);
	name(out, "");
	put(out, 0, 8);
	put(out, address, 8);
	put(out, contents, 8);
	put(out, contents_end - contents, 8);
	put(out, 7, 4);
	put(out, 7, 4);
	put(out, sections_.size(), 4);
	put(out, 0, 4);
	for (const Section& section : sections_)
	{
		name(out, section.name);
		name(out, section.segment);
		put(out, section.address, 8);
		put(out, section.size, 8);
		put(out, section.offset, 4);
		put(out, power(section.align), 4);
		put(out, section.relocations, 4);
		uint32_t count = 0;
		for (const auto& group : section.fixups)
			count += static_cast<uint32_t>(group.size());
		put(out, count, 4);
		put(out, section.flags, 4);
		put(out, 0, 4);
		put(out, 0, 4);
		put(out, 0, 4);
	}
	put(out, 2, 4); // LC_SYMTAB
	put(out, 24, 4);
	put(out, symtab, 4);
	put(out, symbols_.size(), 4);
	put(out, strtab, 4);
	put(out, strings.size(), 4);
	put(out, 0xb, 4); // LC_DYSYMTAB
	put(out, 80, 4);
	uint32_t first = 0;
	for (unsigned kind = 0; kind < 3; ++kind)
	{
		put(out, first, 4);
		put(out, counts[kind], 4);
		first += counts[kind];
	}
	for (unsigned i = 0; i < 12; ++i)
		put(out, 0, 4);
	put(out, 0x32, 4); // LC_BUILD_VERSION: macOS 10.15, no SDK dependency in an object
	put(out, 24, 4);
	put(out, 1, 4);
	put(out, 0x000a0f00, 4);
	put(out, 0, 4);
	put(out, 0, 4);
	for (const Section& section : sections_)
		if (!zerofill(section))
		{
			out.resize(section.offset, 0);
			out.insert(out.end(), section.bytes.begin(), section.bytes.end());
		}
	for (const Section& section : sections_)
		if (!section.fixups.empty())
		{
			out.resize(section.relocations, 0);
			for (const auto& group : section.fixups)
				for (const Reloc& r : group)
				{
					put(out, r.at, 4);
					put(out, (r.bits & (1u << 27)) != 0 ? (r.bits & 0xff000000u) | indexes[r.bits & 0xffffffu] : r.bits,
						4);
				}
		}
	out.resize(symtab, 0);
	for (uint32_t i : order)
	{
		const Symbol& symbol = symbols_[i];
		put(out, symbol.string, 4);
		put(out, symbol.type, 1);
		put(out, symbol.section, 1);
		put(out, symbol.desc, 2);
		put(out, symbol.value + (symbol.section != 0 ? sections_[symbol.section - 1].address : 0), 8);
	}
	out.insert(out.end(), strings.begin(), strings.end());
}

bool Encoder::encode(std::vector<uint8_t>& bytes)
{
	if (!sections() || !symbols() || !relocations())
		return false;
	uint64_t size = 4096 + symbols_.size() * 64;
	for (const Section& section : sections_)
	{
		size += section.bytes.size() + section.align + section.fixups.size() * 16;
		if (size > std::numeric_limits<uint32_t>::max())
			return fail("object exceeds 4 GiB");
	}
	write(bytes);
	return true;
}

} // namespace macho

bool encode_macho_object(const ObjectFile& object, std::vector<uint8_t>& bytes, std::string& error)
{
	macho::Encoder encoder(object, error);
	return encoder.encode(bytes);
}

bool write_macho_object(const ObjectFile& object, const std::string& path, std::string& error)
{
	std::vector<uint8_t> bytes;
	if (!encode_macho_object(object, bytes, error))
		return false;
	std::FILE* file = std::fopen(path.c_str(), "wb");
	if (file == nullptr)
	{
		error = "cannot write " + path;
		return false;
	}
	const bool written = std::fwrite(bytes.data(), 1, bytes.size(), file) == bytes.size();
	const bool closed = std::fclose(file) == 0;
	if (!written || !closed)
		error = "cannot write " + path;
	return written && closed;
}

} // namespace object
} // namespace native
