#include "native/object/elf_object.h"

#include <cstdio>
#include <cstring>
#include <sys/stat.h>

#include "native/object/elf_format.h"

namespace native {
namespace object {

namespace {

const uint32_t kShtX86_64Unwind = 0x70000001;

// Unwind tables and the exception tables only they reference: GNU as
// gives `.eh_frame` SHT_PROGBITS rather than SHT_X86_64_UNWIND, and its
// relocations name COMDAT members the linker may discard.
bool unwind_section(const char* name)
{
	return std::strcmp(name, ".eh_frame") == 0 || std::strncmp(name, ".gcc_except_table", 17) == 0;
}

struct SectionHeader
{
	uint32_t name;
	uint32_t type;
	uint64_t flags;
	uint64_t offset;
	uint64_t size;
	uint32_t link;
	uint32_t info;
	uint64_t align;
	uint64_t entry_size;
};

// One decoding. Object symbols keep the ELF symbol table's numbering;
// every section that is not an ELF structure (symbol, string, relocation
// and group tables) becomes an object section, loaded or not.
class Decoder
{
public:
	Decoder(const std::vector<uint8_t>& image, ObjectFile& object, std::string& error)
		: image_(image), object_(object), error_(error)
	{}

	bool decode();

private:
	bool fail(const char* message)
	{
		error_ = object_.path + ": " + message;
		return false;
	}
	bool read_headers();
	bool read_strings();
	bool add_sections();
	bool read_sections();
	bool read_symbols();
	bool read_groups();
	bool read_relocations();
	bool contents(const SectionHeader& header) const
	{
		return header.type == kShtNobits ||
			(header.offset <= image_.size() && header.size <= image_.size() - header.offset);
	}
	const uint8_t* at(uint64_t offset) const { return image_.data() + offset; }

	const std::vector<uint8_t>& image_;
	ObjectFile& object_;
	std::string& error_;
	std::vector<SectionHeader> headers_;
	std::vector<uint32_t> section_of_; // ELF index -> object section (0: none)
	uint32_t names_ = 0;
	uint32_t symtab_ = 0;
	uint32_t symtab_shndx_ = 0;
};

bool Decoder::decode()
{
	if (!is_elf_image(image_) || image_.size() < kElfHeaderBytes)
		return fail("not an ELF object");
	if (image_[4] != 2 || image_[5] != 1)
		return fail("not a little-endian ELF64 object");
	if (get_le(at(16), 2) != kEtRel)
		return fail("not a relocatable object");
	if (get_le(at(18), 2) != kEmX86_64)
		return fail("not an x86-64 object");
	return read_headers() && read_strings() && add_sections() && read_sections() && read_symbols() && read_groups() &&
		read_relocations();
}

bool Decoder::read_headers()
{
	const uint64_t table = get_le(at(40), 8);
	const uint64_t entry = get_le(at(58), 2);
	uint64_t count = get_le(at(60), 2);
	uint64_t names = get_le(at(62), 2);
	if (table == 0)
		return true;
	if (entry != kSectionHeaderBytes || table > image_.size() || image_.size() - table < kSectionHeaderBytes)
		return fail("malformed section header table");
	// Extended numbering keeps the counts in section 0.
	if (count == 0)
		count = get_le(at(table + 32), 8);
	if (names == kShnXindex)
		names = get_le(at(table + 40), 4);
	if (count > (image_.size() - table) / kSectionHeaderBytes)
		return fail("malformed section header table");
	headers_.resize(count);
	for (uint64_t i = 0; i < count; ++i)
	{
		const uint8_t* h = at(table + i * kSectionHeaderBytes);
		SectionHeader& header = headers_[i];
		header.name = static_cast<uint32_t>(get_le(h, 4));
		header.type = static_cast<uint32_t>(get_le(h + 4, 4));
		header.flags = get_le(h + 8, 8);
		header.offset = get_le(h + 24, 8);
		header.size = get_le(h + 32, 8);
		header.link = static_cast<uint32_t>(get_le(h + 40, 4));
		header.info = static_cast<uint32_t>(get_le(h + 44, 4));
		header.align = get_le(h + 48, 8);
		header.entry_size = get_le(h + 56, 8);
		if (i != 0 && !contents(header))
			return fail("section extends past the end of the file");
	}
	if (names >= count || headers_[names].type != kShtStrtab)
		return fail("missing section name table");
	names_ = static_cast<uint32_t>(names);
	for (uint64_t i = 1; i < count; ++i)
	{
		switch (headers_[i].type)
		{
		case kShtSymtab:
			if (symtab_ != 0)
				return fail("more than one symbol table");
			symtab_ = static_cast<uint32_t>(i);
			break;
		case kShtSymtabShndx:
			symtab_shndx_ = static_cast<uint32_t>(i);
			break;
		case kShtRel:
			return fail("REL relocation tables are not used on x86-64");
		default:
			break;
		}
	}
	return true;
}

bool Decoder::read_strings()
{
	// Symbol names keep their string table offsets; section names follow.
	if (symtab_ == 0)
		return true;
	const SectionHeader& table = headers_[symtab_];
	if (table.entry_size != kSymbolBytes || table.link >= headers_.size() || headers_[table.link].type != kShtStrtab)
		return fail("malformed symbol table");
	const SectionHeader& names = headers_[table.link];
	if (names.size == 0 || *at(names.offset) != '\0')
		return fail("malformed string table");
	object_.strings.assign(at(names.offset), at(names.offset) + names.size);
	object_.strings.push_back('\0');
	return true;
}

bool Decoder::add_sections()
{
	if (headers_.empty())
		return true;
	const SectionHeader& table = headers_[names_];
	section_of_.assign(headers_.size(), 0);
	for (size_t i = 1; i < headers_.size(); ++i)
	{
		const SectionHeader& header = headers_[i];
		if (header.type == kShtSymtab || header.type == kShtSymtabShndx || header.type == kShtStrtab ||
			header.type == kShtRela || header.type == kShtGroup)
			continue;
		if (header.name >= table.size)
			return fail("malformed section name");
		const char* name = reinterpret_cast<const char*>(at(table.offset + header.name));
		section_of_[i] = object_.add_section("", SectionKind::Null, 1);
		object_.sections[section_of_[i]].name = object_.add_string(name, strnlen(name, table.size - header.name));
	}
	return true;
}

bool Decoder::read_sections()
{
	for (size_t i = 1; i < headers_.size(); ++i)
	{
		if (section_of_[i] == 0)
			continue;
		const SectionHeader& header = headers_[i];
		Section& section = object_.sections[section_of_[i]];
		if ((header.flags & kShfAlloc) == 0 || header.type == kShtNote || header.type == kShtX86_64Unwind ||
			unwind_section(object_.string(section.name)))
			continue;
		if ((header.flags & kShfTls) != 0)
			return fail("thread-local storage sections are not supported");
		switch (header.type)
		{
		case kShtProgbits:
			section.kind = (header.flags & kShfExecinstr) != 0
				? SectionKind::Text
				: ((header.flags & kShfWrite) != 0 ? SectionKind::Data : SectionKind::Rodata);
			break;
		case kShtNobits:
			section.kind = SectionKind::Bss;
			break;
		case kShtInitArray:
		case kShtPreinitArray:
			section.kind = SectionKind::InitArray;
			break;
		case kShtFiniArray:
			section.kind = SectionKind::FiniArray;
			break;
		default:
			continue;
		}
		section.align = header.align == 0 ? 1 : header.align;
		if ((section.align & (section.align - 1)) != 0)
			return fail("section alignment is not a power of two");
		section.size = header.size;
		if (section.kind != SectionKind::Bss)
			section.bytes.assign(at(header.offset), at(header.offset) + header.size);
	}
	return true;
}

bool Decoder::read_symbols()
{
	if (symtab_ == 0)
		return true;
	const SectionHeader& table = headers_[symtab_];
	const SectionHeader& names = headers_[table.link];
	const uint8_t* indexes = nullptr;
	const uint64_t count = table.size / kSymbolBytes;
	if (symtab_shndx_ != 0 && headers_[symtab_shndx_].size >= count * 4)
		indexes = at(headers_[symtab_shndx_].offset);
	object_.symbols.resize(count == 0 ? 1 : count);
	for (uint64_t i = 1; i < count; ++i)
	{
		const uint8_t* entry = at(table.offset + i * kSymbolBytes);
		Symbol& symbol = object_.symbols[i];
		symbol.name = static_cast<uint32_t>(get_le(entry, 4));
		if (symbol.name >= names.size)
			return fail("malformed symbol name");
		const uint8_t info = entry[4];
		const uint8_t bind = info >> 4;
		const uint8_t type = info & 0xf;
		uint32_t index = static_cast<uint32_t>(get_le(entry + 6, 2));
		symbol.value = get_le(entry + 8, 8);
		symbol.size = get_le(entry + 16, 8);
		if (type == kSttTls)
			return fail("thread-local symbols are not supported");
		if (type == kSttGnuIfunc)
			return fail("indirect function symbols are not supported");
		symbol.binding = bind == kStbLocal ? Binding::Local : (bind == kStbWeak ? Binding::Weak : Binding::Global);
		symbol.type = type == kSttFunc                 ? SymbolType::Function
			: type == kSttObject || type == kSttCommon ? SymbolType::Object
			: type == kSttSection                      ? SymbolType::Section
			: type == kSttFile                         ? SymbolType::File
													   : SymbolType::None;
		if (index == kShnXindex)
		{
			if (indexes == nullptr)
				return fail("missing extended section indexes");
			index = static_cast<uint32_t>(get_le(indexes + 4 * i, 4));
		}
		else if (index >= kShnLoreserve)
		{
			if (index == kShnAbs || index == kShnCommon)
			{
				symbol.section = index == kShnAbs ? kAbsoluteSection : kCommonSection;
				continue;
			}
			return fail("unsupported special section index");
		}
		if (index >= headers_.size())
			return fail("symbol section index out of range");
		symbol.section = index == 0 ? kUndefinedSection : section_of_[index];
		// A local symbol of a structural section (a relocatable link's
		// .group section symbols) is never a relocation target here.
		if (index != 0 && symbol.section == 0 && symbol.binding != Binding::Local)
			return fail("symbol defined in a non-content section");
	}
	return true;
}

bool Decoder::read_groups()
{
	for (size_t i = 1; i < headers_.size(); ++i)
	{
		const SectionHeader& header = headers_[i];
		if (header.type != kShtGroup)
			continue;
		if (header.size < 4 || header.link != symtab_ || header.info == 0 || header.info >= object_.symbols.size())
			return fail("malformed section group");
		if ((get_le(at(header.offset), 4) & kGrpComdat) == 0)
			continue;
		Group group;
		group.signature = header.info;
		for (uint64_t k = 4; k + 4 <= header.size; k += 4)
		{
			const uint64_t member = get_le(at(header.offset + k), 4);
			if (member == 0 || member >= headers_.size())
				return fail("malformed section group member");
			if (section_of_[member] != 0)
				group.sections.push_back(section_of_[member]);
		}
		const uint32_t index = static_cast<uint32_t>(object_.groups.size());
		for (const uint32_t member : group.sections)
			object_.sections[member].group = index;
		object_.groups.push_back(std::move(group));
	}
	return true;
}

bool Decoder::read_relocations()
{
	for (size_t i = 1; i < headers_.size(); ++i)
	{
		const SectionHeader& header = headers_[i];
		if (header.type != kShtRela)
			continue;
		if (header.info == 0 || header.info >= headers_.size() || header.entry_size != kRelaBytes)
			return fail("malformed relocation table");
		const uint32_t target = section_of_[header.info];
		if (target == 0 || object_.sections[target].kind == SectionKind::Null)
			continue;
		if (header.link != symtab_)
			return fail("relocation table names another symbol table");
		Section& section = object_.sections[target];
		const uint64_t count = header.size / kRelaBytes;
		section.relocations.reserve(count);
		for (uint64_t k = 0; k < count; ++k)
		{
			const uint8_t* entry = at(header.offset + k * kRelaBytes);
			Relocation relocation;
			relocation.offset = get_le(entry, 8);
			const uint64_t info = get_le(entry + 8, 8);
			relocation.symbol = static_cast<uint32_t>(info >> 32);
			relocation.type = static_cast<uint32_t>(info);
			relocation.addend = static_cast<int64_t>(get_le(entry + 16, 8));
			if (relocation.symbol >= object_.symbols.size() || relocation.offset > section.size)
				return fail("relocation out of range");
			section.relocations.push_back(relocation);
		}
	}
	return true;
}

} // namespace

bool is_elf_image(const std::vector<uint8_t>& image)
{
	return image.size() >= 4 && image[0] == 0x7f && image[1] == 'E' && image[2] == 'L' && image[3] == 'F';
}

bool decode_elf_object(const std::vector<uint8_t>& image, ObjectFile& object, std::string& error)
{
	Decoder decoder(image, object, error);
	return decoder.decode();
}

bool read_elf_object(const std::string& path, ObjectFile& object, std::string& error)
{
	std::FILE* file = std::fopen(path.c_str(), "rb");
	if (file == nullptr)
	{
		error = "cannot read " + path;
		return false;
	}
	// The file is read into a buffer of its size (a FIFO or a file that
	// grows meanwhile is read to its end in chunks).
	std::vector<uint8_t> image;
	struct stat status;
	if (fstat(fileno(file), &status) == 0 && S_ISREG(status.st_mode))
		image.reserve(static_cast<size_t>(status.st_size));
	uint8_t buffer[65536];
	size_t count;
	while ((count = std::fread(buffer, 1, sizeof buffer, file)) != 0)
		image.insert(image.end(), buffer, buffer + count);
	const bool failed = std::ferror(file) != 0;
	std::fclose(file);
	if (failed)
	{
		error = "cannot read " + path;
		return false;
	}
	object.path = path;
	return decode_elf_object(image, object, error);
}

} // namespace object
} // namespace native
