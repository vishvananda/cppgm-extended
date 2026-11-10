#include "native/asm/module.h"

#include <climits>
#include "native/backend_error.h"

namespace native {
namespace {
using object::SectionKind;
const uint64_t kModuleByteLimit = 64u * 1024u * 1024u;
}
void ModuleAssembler::reserve(uint64_t count, uint8_t fill)
{
	object::Section& section = out_.sections[section_];
	if (count > kModuleByteLimit || section.size > kModuleByteLimit - count)
		throw BackendError("module section exceeds the byte limit");
	section.size += count;
	if (section.kind != SectionKind::Bss)
		section.bytes.resize(section.size, fill);
	else if (fill != 0)
		throw BackendError("nonzero initialization in module bss section");
}
void ModuleAssembler::data(assembly::ModuleCursor& cursor, unsigned bytes)
{
	do
	{
		uint64_t bits = 0;
		uint32_t target = lowir::kNoId;
		if (cursor.numeric())
			bits = cursor.number();
		else
		{
			target = symbol(cursor);
			if (cursor.take('+'))
				bits = cursor.number();
			else if (cursor.take('-'))
				bits = uint64_t(0) - cursor.number();
		}
		const uint64_t first = offset();
		reserve(bytes, 0);
		if (target != lowir::kNoId)
		{
			if (bytes != 8 || out_.sections[section_].kind == SectionKind::Bss)
				cursor.fail("module symbol data requires an eight-byte address");
			out_.sections[section_].relocations.push_back(
				object::Relocation{ first, object::R_X86_64_64, target, static_cast<int64_t>(bits) });
		}
		else if (out_.sections[section_].kind == SectionKind::Bss)
		{
			if (bits != 0)
				cursor.fail("nonzero module bss initializer");
		}
		else
			for (unsigned i = 0; i < bytes; ++i)
				out_.sections[section_].bytes[first + i] = static_cast<uint8_t>(bits >> (8 * i));
	} while (cursor.take(','));
}
void ModuleAssembler::strings(assembly::ModuleCursor& cursor, bool terminate)
{
	do
	{
		const std::string value = cursor.quoted();
		const uint64_t first = offset();
		reserve(value.size() + terminate, 0);
		if (out_.sections[section_].kind == SectionKind::Bss)
			cursor.fail("string in module bss");
		std::copy(value.begin(), value.end(), out_.sections[section_].bytes.begin() + first);
	} while (cursor.take(','));
}
void ModuleAssembler::alignment(assembly::ModuleCursor& cursor, bool power)
{
	uint64_t alignment = cursor.number();
	if (power)
	{
		if (alignment > 20)
			cursor.fail("module alignment exponent exceeds limit");
		alignment = uint64_t(1) << alignment;
	}
	if (alignment == 0 || alignment > (1u << 20) || (alignment & (alignment - 1)) != 0)
		cursor.fail("module alignment requires a bounded power of two");
	uint8_t fill = out_.sections[section_].kind == SectionKind::Text ? 0x90 : 0;
	uint64_t maximum = UINT64_MAX;
	if (cursor.take(','))
	{
		if (cursor.numeric())
			fill = static_cast<uint8_t>(cursor.number());
		if (cursor.take(','))
			maximum = cursor.number();
	}
	out_.sections[section_].align = std::max(out_.sections[section_].align, alignment);
	const uint64_t bytes = (uint64_t(0) - offset()) & (alignment - 1);
	if (bytes <= maximum)
		reserve(bytes, fill);
}
void ModuleAssembler::size(assembly::ModuleCursor& cursor)
{
	const uint32_t id = symbol(cursor);
	if (!cursor.take(','))
		cursor.fail("module size requires an expression");
	uint64_t bytes = 0;
	if (cursor.numeric())
		bytes = cursor.number();
	else
	{
		if (cursor.word() != "." || !cursor.take('-') || symbol(cursor) != id || out_.symbols[id].section != section_ ||
			out_.symbols[id].value > offset())
			cursor.fail("unsupported module size expression");
		bytes = offset() - out_.symbols[id].value;
	}
	out_.symbols[id].size = bytes;
}
void ModuleAssembler::type(assembly::ModuleCursor& cursor)
{
	const uint32_t id = symbol(cursor);
	if (!cursor.take(','))
		cursor.fail("module type requires its kind");
	cursor.take('@');
	cursor.take('%');
	const std::string kind = cursor.word();
	if (kind == "function" || kind == "gnu_indirect_function")
	{
		if (kind != "function")
			cursor.fail("indirect function module definition is not implemented");
		out_.symbols[id].type = object::SymbolType::Function;
	}
	else if (kind == "object")
		out_.symbols[id].type = object::SymbolType::Object;
	else
		cursor.fail("unsupported module symbol type");
}
bool ModuleAssembler::directive(const std::string& name, assembly::ModuleCursor& cursor)
{
	if (name == ".text" || name == ".data" || name == ".bss" || name == ".rodata")
		section(name,
			name == ".text"         ? SectionKind::Text
				: name == ".bss"    ? SectionKind::Bss
				: name == ".rodata" ? SectionKind::Rodata
									: SectionKind::Data);
	else if (name == ".globl" || name == ".global" || name == ".weak" || name == ".weak_definition" ||
		name == ".local" || name == ".hidden" || name == ".private_extern")
	{
		do
		{
			const uint32_t id = symbol(cursor);
			if (name == ".hidden" || name == ".private_extern")
				out_.symbols[id].visibility = object::Visibility::Hidden;
			else
				symbols_.bind(id,
					name == ".local"                                    ? object::Binding::Local
						: name == ".weak" || name == ".weak_definition" ? object::Binding::Weak
																		: object::Binding::Global);
		} while (cursor.take(','));
	}
	else if (name == ".type")
		type(cursor);
	else if (name == ".size")
		size(cursor);
	else if (name == ".byte" || name == ".word" || name == ".short" || name == ".long" || name == ".int" ||
		name == ".quad")
		data(cursor, name == ".byte" ? 1 : name == ".word" || name == ".short" ? 2 : name == ".quad" ? 8 : 4);
	else if (name == ".ascii" || name == ".asciz" || name == ".string")
		strings(cursor, name != ".ascii");
	else if (name == ".zero" || name == ".space" || name == ".skip")
	{
		const uint64_t bytes = cursor.number();
		const uint8_t fill = cursor.take(',') ? static_cast<uint8_t>(cursor.number()) : 0;
		reserve(bytes, fill);
	}
	else if (name == ".p2align" || name == ".balign" || name == ".align")
		alignment(cursor, name == ".p2align" || (name == ".align" && target_ == Target::MacOS));
	else if (name == ".code64" || name == ".subsections_via_symbols")
	{
	}
	else if (name == ".set" || name == ".equ")
	{
		const uint32_t id = symbol(cursor);
		if (!cursor.take(','))
			cursor.fail("module alias requires its target");
		const uint32_t target = symbol(cursor);
		int64_t addend = 0;
		if (cursor.take('+'))
			addend = static_cast<int64_t>(cursor.number());
		else if (cursor.take('-'))
			addend = static_cast<int64_t>(uint64_t(0) - cursor.number());
		symbols_.alias(id, target, addend);
	}
	else if (name == ".previous")
	{
		if (previous_ == 0)
			cursor.fail("no previous module section");
		std::swap(section_, previous_);
	}
	else if (name == ".popsection")
	{
		if (section_stack_.empty())
			cursor.fail("empty module section stack");
		previous_ = section_;
		section_ = section_stack_.back();
		section_stack_.pop_back();
	}
	else if (name == ".section" || name == ".pushsection")
	{
		if (name == ".pushsection")
			section_stack_.push_back(section_);
		std::string section_name = cursor.quoted_start() ? cursor.quoted() : cursor.word();
		if (section_name.empty())
			cursor.fail("module section name cannot be empty");
		SectionKind kind = SectionKind::Data;
		if (cursor.take(','))
		{
			if (target_ == Target::MacOS && section_name.compare(0, 2, "__") == 0)
			{
				const std::string item = cursor.word();
				kind = section_name == "__TEXT" ? (item == "__text" ? SectionKind::Text : SectionKind::Rodata)
					: item == "__bss"           ? SectionKind::Bss
												: SectionKind::Data;
				section_name = item == "__text" ? ".text"
					: item == "__const"         ? ".rodata"
					: item == "__data"          ? ".data"
					: item == "__bss"           ? ".bss"
												: item;
				if (cursor.take(','))
					cursor.fail("Mach-O section attributes are not implemented yet");
			}
			else
			{
				const std::string flags = cursor.quoted();
				if (flags.find_first_not_of("awxMS") != std::string::npos)
					cursor.fail("module section flags are not implemented");
				kind = flags.find('x') != std::string::npos ? SectionKind::Text
					: flags.find('w') != std::string::npos  ? SectionKind::Data
					: flags.find('a') != std::string::npos  ? SectionKind::Rodata
															: SectionKind::Metadata;
				const bool strings = flags.find('M') != std::string::npos || flags.find('S') != std::string::npos;
				if (strings)
				{
					if (target_ != Target::Linux || flags.find_first_not_of("MS") != std::string::npos ||
						flags.find('M') == std::string::npos || flags.find('S') == std::string::npos)
						cursor.fail("merged string metadata requires nonallocated ELF MS flags");
					kind = SectionKind::MetadataStrings;
				}
				if (cursor.take(','))
				{
					cursor.take('@');
					cursor.take('%');
					const std::string type = cursor.word();
					if (type == "nobits")
						kind = SectionKind::Bss;
					else if (type != "progbits")
						cursor.fail("unsupported module section type");
					if (strings && (type != "progbits" || !cursor.take(',') || cursor.number() != 1))
						cursor.fail("merged string metadata requires byte-sized progbits entries");
				}
			}
		}
		if (section_name == ".text")
			kind = SectionKind::Text;
		else if (section_name == ".rodata")
			kind = SectionKind::Rodata;
		else if (section_name == ".bss")
			kind = SectionKind::Bss;
		section(section_name, kind);
	}
	else
		return false;
	return true;
}
} // namespace native
