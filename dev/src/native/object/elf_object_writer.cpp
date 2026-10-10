#include "native/object/elf_object.h"

#include <cstdio>
#include <cstring>

#include "native/object/elf_format.h"
#include "support/flat_map.h"

namespace native {
namespace object {

namespace {

struct OutputSection
{
	uint32_t name; // offset in .shstrtab
	uint32_t type;
	uint64_t flags;
	uint64_t offset;
	uint64_t size;
	uint32_t link;
	uint32_t info;
	uint64_t align;
	uint64_t entry_size;
	// The contents: an input section's bytes, else `built` (NOBITS: none).
	const std::vector<uint8_t>* bytes;
	std::vector<uint8_t> built;
};

uint32_t section_type(SectionKind kind)
{
	switch (kind)
	{
	case SectionKind::Bss:
	case SectionKind::TlsBss:
		return kShtNobits;
	case SectionKind::InitArray:
		return kShtInitArray;
	case SectionKind::FiniArray:
		return kShtFiniArray;
	default:
		return kShtProgbits;
	}
}

uint64_t section_flags(SectionKind kind)
{
	switch (kind)
	{
	case SectionKind::Text:
		return kShfAlloc | kShfExecinstr;
	case SectionKind::Rodata:
		return kShfAlloc;
	case SectionKind::Null:
		return 0;
	case SectionKind::TlsData:
	case SectionKind::TlsBss:
		return kShfAlloc | kShfWrite | kShfTls;
	default:
		return kShfAlloc | kShfWrite;
	}
}

uint8_t binding_code(Binding binding)
{
	return binding == Binding::Local ? kStbLocal : (binding == Binding::Weak ? kStbWeak : kStbGlobal);
}

uint8_t type_code(SymbolType type)
{
	switch (type)
	{
	case SymbolType::Object:
		return kSttObject;
	case SymbolType::Function:
		return kSttFunc;
	case SymbolType::Section:
		return kSttSection;
	case SymbolType::File:
		return kSttFile;
	case SymbolType::Tls:
		return kSttTls;
	default:
		return kSttNotype;
	}
}

struct WordHash
{
	size_t operator()(uint64_t key) const { return static_cast<size_t>(key); }
};

// One encoding: the section order is null, the groups, the object's
// sections, their RELA tables, .symtab, .strtab, .symtab_shndx (extended
// numbering only), .shstrtab and .note.GNU-stack.
class Encoder
{
public:
	explicit Encoder(const ObjectFile& object) : object_(object), names_(1, '\0') {}

	void encode(std::vector<uint8_t>& out);

private:
	void number();
	void add_groups();
	void add_contents();
	void add_relocations();
	void add_symbols();
	void write(std::vector<uint8_t>& out);
	uint32_t name(const char* prefix, const char* text);
	uint32_t section_name(bool rela, uint32_t name);

	const ObjectFile& object_;
	std::vector<OutputSection> sections_;
	std::vector<char> names_;
	std::vector<uint32_t> content_index_;
	std::vector<uint32_t> rela_index_;
	std::vector<uint32_t> symbol_index_;
	// (RELA table?, object string offset) -> .shstrtab offset.
	FlatMap<uint64_t, uint32_t, WordHash> section_names_;
	uint32_t group_name_ = 0;
	uint32_t symbol_count_ = 1;
	uint32_t first_global_ = 1;
	uint32_t symtab_ = 0;
	uint32_t strtab_ = 0;
	uint32_t shstrtab_ = 0;
	bool extended_ = false;
};

uint32_t Encoder::section_name(bool rela, uint32_t name)
{
	// Sections sharing a name share its .shstrtab entry.
	const uint64_t key = (static_cast<uint64_t>(rela) << 32) | name;
	if (const uint32_t* known = section_names_.find(key))
		return *known;
	const uint32_t offset = this->name(rela ? ".rela" : "", object_.string(name));
	section_names_.insert(key, offset);
	return offset;
}

uint32_t Encoder::name(const char* prefix, const char* text)
{
	const uint32_t offset = static_cast<uint32_t>(names_.size());
	names_.insert(names_.end(), prefix, prefix + std::strlen(prefix));
	names_.insert(names_.end(), text, text + std::strlen(text));
	names_.push_back('\0');
	return offset;
}

void Encoder::number()
{
	content_index_.assign(object_.sections.size(), 0);
	rela_index_.assign(object_.sections.size(), 0);
	uint32_t next = 1 + static_cast<uint32_t>(object_.groups.size());
	for (size_t s = 1; s < object_.sections.size(); ++s)
		content_index_[s] = next++;
	for (size_t s = 1; s < object_.sections.size(); ++s)
	{
		if (!object_.sections[s].relocations.empty())
			rela_index_[s] = next++;
	}
	// Symbols naming sections from SHN_LORESERVE on keep their indexes in
	// SHT_SYMTAB_SHNDX (the extended numbering).
	extended_ = next > kShnLoreserve;
	symtab_ = next++;
	strtab_ = next++;
	if (extended_)
		++next;
	shstrtab_ = next;
	// Symbols: the null symbol, the locals, then the others (sh_info).
	symbol_index_.assign(object_.symbols.size(), 0);
	for (size_t i = 1; i < object_.symbols.size(); ++i)
	{
		if (object_.symbols[i].binding == Binding::Local)
			symbol_index_[i] = symbol_count_++;
	}
	first_global_ = symbol_count_;
	for (size_t i = 1; i < object_.symbols.size(); ++i)
	{
		if (object_.symbols[i].binding != Binding::Local)
			symbol_index_[i] = symbol_count_++;
	}
}

void Encoder::add_groups()
{
	for (const Group& group : object_.groups)
	{
		OutputSection section = OutputSection();
		if (group_name_ == 0)
			group_name_ = name(".group", "");
		section.name = group_name_;
		section.type = kShtGroup;
		section.link = symtab_;
		section.info = symbol_index_[group.signature];
		section.align = 4;
		section.entry_size = 4;
		put32(section.built, kGrpComdat);
		// A member's relocation table belongs to the group too.
		for (const uint32_t member : group.sections)
		{
			put32(section.built, content_index_[member]);
			if (rela_index_[member] != 0)
				put32(section.built, rela_index_[member]);
		}
		sections_.push_back(std::move(section));
	}
}

void Encoder::add_contents()
{
	for (size_t s = 1; s < object_.sections.size(); ++s)
	{
		const Section& from = object_.sections[s];
		OutputSection section = OutputSection();
		section.name = section_name(false, from.name);
		section.type = section_type(from.kind);
		section.flags = section_flags(from.kind) | (from.group != kNoGroup ? kShfGroup : 0);
		const bool nobits = from.kind == SectionKind::Bss || from.kind == SectionKind::TlsBss;
		section.size = nobits ? from.size : from.bytes.size();
		section.align = from.align;
		section.entry_size = from.kind == SectionKind::InitArray || from.kind == SectionKind::FiniArray ? 8 : 0;
		section.bytes = nobits ? nullptr : &from.bytes;
		sections_.push_back(std::move(section));
	}
}

void Encoder::add_relocations()
{
	for (size_t s = 1; s < object_.sections.size(); ++s)
	{
		const Section& from = object_.sections[s];
		if (from.relocations.empty())
			continue;
		OutputSection section = OutputSection();
		section.name = section_name(true, from.name);
		section.type = kShtRela;
		section.flags = kShfInfoLink | (from.group != kNoGroup ? kShfGroup : 0);
		section.link = symtab_;
		section.info = content_index_[s];
		section.align = 8;
		section.entry_size = 24;
		section.built.reserve(from.relocations.size() * 24);
		for (const Relocation& relocation : from.relocations)
		{
			put64(section.built, relocation.offset);
			put64(section.built, (static_cast<uint64_t>(symbol_index_[relocation.symbol]) << 32) | relocation.type);
			put64(section.built, static_cast<uint64_t>(relocation.addend));
		}
		sections_.push_back(std::move(section));
	}
}

void Encoder::add_symbols()
{
	OutputSection table = OutputSection();
	table.name = name(".symtab", "");
	table.type = kShtSymtab;
	table.link = strtab_;
	table.info = first_global_;
	table.align = 8;
	table.entry_size = 24;
	OutputSection indexes = OutputSection();
	std::vector<uint32_t> order(symbol_count_, 0);
	for (size_t i = 1; i < object_.symbols.size(); ++i)
		order[symbol_index_[i]] = static_cast<uint32_t>(i);
	table.built.reserve(24 * symbol_count_);
	table.built.resize(24, 0);
	if (extended_)
		put32(indexes.built, 0);
	for (uint32_t k = 1; k < symbol_count_; ++k)
	{
		const Symbol& symbol = object_.symbols[order[k]];
		uint32_t index = symbol.section;
		bool escaped = false;
		if (index == kAbsoluteSection || index == kCommonSection)
			index = index == kAbsoluteSection ? kShnAbs : kShnCommon;
		else if (index != kUndefinedSection)
		{
			index = content_index_[index];
			escaped = index >= kShnLoreserve;
		}
		put32(table.built, symbol.name);
		table.built.push_back(static_cast<uint8_t>((binding_code(symbol.binding) << 4) | type_code(symbol.type)));
		table.built.push_back(0);
		put16(table.built, escaped ? kShnXindex : index);
		put64(table.built, symbol.value);
		put64(table.built, symbol.size);
		if (extended_)
			put32(indexes.built, escaped ? index : 0);
	}
	sections_.push_back(std::move(table));
	OutputSection strings = OutputSection();
	strings.name = name(".strtab", "");
	strings.type = kShtStrtab;
	strings.align = 1;
	strings.built.assign(object_.strings.begin(), object_.strings.end());
	sections_.push_back(std::move(strings));
	if (extended_)
	{
		indexes.name = name(".symtab_shndx", "");
		indexes.type = kShtSymtabShndx;
		indexes.link = symtab_;
		indexes.align = 4;
		indexes.entry_size = 4;
		sections_.push_back(std::move(indexes));
	}
}

void Encoder::encode(std::vector<uint8_t>& out)
{
	number();
	sections_.push_back(OutputSection());
	add_groups();
	add_contents();
	add_relocations();
	add_symbols();
	OutputSection shstrtab = OutputSection();
	shstrtab.name = name(".shstrtab", "");
	shstrtab.type = kShtStrtab;
	shstrtab.align = 1;
	OutputSection stack = OutputSection();
	stack.name = name(".note.GNU-stack", "");
	stack.type = kShtProgbits;
	stack.align = 1;
	shstrtab.built.assign(names_.begin(), names_.end());
	sections_.push_back(std::move(shstrtab));
	sections_.push_back(std::move(stack));
	write(out);
}

void Encoder::write(std::vector<uint8_t>& out)
{
	// The header, each section's contents, then the section header table.
	uint64_t position = kElfHeaderBytes;
	for (size_t s = 1; s < sections_.size(); ++s)
	{
		OutputSection& section = sections_[s];
		if (section.bytes == nullptr && section.type != kShtNobits)
			section.size = section.built.size();
		const uint64_t align = section.align == 0 ? 1 : section.align;
		position = (position + align - 1) & ~(align - 1);
		section.offset = position;
		if (section.type != kShtNobits)
			position += section.size;
	}
	const uint64_t header_table = (position + 7) & ~static_cast<uint64_t>(7);
	const uint32_t count = static_cast<uint32_t>(sections_.size());
	if (count >= kShnLoreserve)
		sections_[0].size = count;
	if (shstrtab_ >= kShnLoreserve)
		sections_[0].link = shstrtab_;
	out.clear();
	out.reserve(header_table + kSectionHeaderBytes * count);
	put_elf_ident(out);
	put16(out, kEtRel);
	put16(out, kEmX86_64);
	put32(out, 1);
	put64(out, 0);
	put64(out, 0);
	put64(out, header_table);
	put32(out, 0);
	put16(out, kElfHeaderBytes);
	put16(out, 0);
	put16(out, 0);
	put16(out, kSectionHeaderBytes);
	put16(out, count >= kShnLoreserve ? 0 : count);
	put16(out, shstrtab_ >= kShnLoreserve ? kShnXindex : shstrtab_);
	for (size_t s = 1; s < sections_.size(); ++s)
	{
		const OutputSection& section = sections_[s];
		if (section.type == kShtNobits)
			continue;
		out.resize(section.offset, 0);
		const std::vector<uint8_t>& bytes = section.bytes != nullptr ? *section.bytes : section.built;
		out.insert(out.end(), bytes.begin(), bytes.end());
	}
	out.resize(header_table, 0);
	for (const OutputSection& section : sections_)
	{
		put32(out, section.name);
		put32(out, section.type);
		put64(out, section.flags);
		put64(out, 0);
		put64(out, section.offset);
		put64(out, section.size);
		put32(out, section.link);
		put32(out, section.info);
		put64(out, section.align);
		put64(out, section.entry_size);
	}
}

} // namespace

void encode_elf_object(const ObjectFile& object, std::vector<uint8_t>& out)
{
	Encoder encoder(object);
	encoder.encode(out);
}

bool write_elf_object(const ObjectFile& object, const std::string& path, std::string& error)
{
	std::vector<uint8_t> image;
	encode_elf_object(object, image);
	std::FILE* file = std::fopen(path.c_str(), "wb");
	if (file == nullptr)
	{
		error = "cannot write " + path;
		return false;
	}
	const bool written = std::fwrite(image.data(), 1, image.size(), file) == image.size();
	const bool closed = std::fclose(file) == 0;
	if (!written || !closed)
	{
		error = "cannot write " + path;
		return false;
	}
	return true;
}

} // namespace object
} // namespace native
