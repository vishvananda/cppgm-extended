#include "native/object/object_file.h"

namespace native {
namespace object {

ObjectFile::ObjectFile()
{
	strings.push_back('\0');
	sections.push_back(Section());
	symbols.push_back(Symbol());
}

uint32_t ObjectFile::add_string(const char* text, size_t length)
{
	if (length == 0)
		return 0;
	const uint32_t offset = static_cast<uint32_t>(strings.size());
	strings.insert(strings.end(), text, text + length);
	strings.push_back('\0');
	return offset;
}

uint32_t ObjectFile::add_section(const char* name, SectionKind kind, uint64_t align)
{
	Section section;
	section.name = add_string(name, std::char_traits<char>::length(name));
	section.kind = kind;
	section.align = align;
	sections.push_back(std::move(section));
	return static_cast<uint32_t>(sections.size() - 1);
}

uint32_t ObjectFile::add_section(uint32_t name, SectionKind kind, uint64_t align)
{
	Section section;
	section.name = name;
	section.kind = kind;
	section.align = align;
	sections.push_back(std::move(section));
	return static_cast<uint32_t>(sections.size() - 1);
}

uint32_t ObjectFile::add_symbol(const Symbol& symbol)
{
	symbols.push_back(symbol);
	return static_cast<uint32_t>(symbols.size() - 1);
}

} // namespace object
} // namespace native
