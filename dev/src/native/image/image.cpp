#include "native/image/image.h"

#include <cstring>

#include "native/image/executable_file.h"

namespace native {

namespace {

uint64_t align_up(uint64_t value, uint64_t align)
{
	return (value + align - 1) & ~(align - 1);
}

void patch(std::vector<uint8_t>& out, uint64_t offset, uint64_t value, unsigned bytes)
{
	for (unsigned k = 0; k < bytes; ++k)
		out[offset + k] = static_cast<uint8_t>(value >> (8 * k));
}

} // namespace

Image::Image()
{}

uint32_t Image::add_symbol()
{
	locals_.push_back(ImageSymbol());
	return kLocalSymbols + static_cast<uint32_t>(locals_.size() - 1);
}

void Image::define(uint32_t symbol, Section section, uint64_t offset)
{
	ImageSymbol* defined = nullptr;
	if (symbol >= kLocalSymbols)
		defined = &locals_[symbol - kLocalSymbols];
	else
	{
		if (symbol >= symbols_.size())
			symbols_.resize(symbol + 1);
		defined = &symbols_[symbol];
	}
	defined->section = section;
	defined->offset = offset;
}

uint64_t Image::reserve(Section section, uint64_t bytes, uint64_t align)
{
	if (align == 0)
		align = 1;
	std::vector<uint8_t>* buffer = section == Section::Text
		? &text_
		: (section == Section::Rodata ? &rodata_ : (section == Section::Data ? &data_ : nullptr));
	if (buffer == nullptr)
	{
		bss_size_ = align_up(bss_size_, align);
		const uint64_t offset = bss_size_;
		bss_size_ += bytes;
		return offset;
	}
	const uint64_t offset = align_up(buffer->size(), align);
	buffer->resize(offset + bytes, section == Section::Text ? 0xcc : 0);
	return offset;
}

uint32_t Image::constant(const void* bytes, uint32_t size, uint32_t align)
{
	std::string key(static_cast<const char*>(bytes), size);
	key += static_cast<char>(align);
	std::unordered_map<std::string, uint32_t>::const_iterator found = constants_.find(key);
	if (found != constants_.end())
		return found->second;
	const uint64_t offset = reserve(Section::Rodata, size, align);
	std::memcpy(rodata_.data() + offset, bytes, size);
	const uint32_t symbol = add_symbol();
	define(symbol, Section::Rodata, offset);
	constants_[key] = symbol;
	return symbol;
}

void Image::add_code_fixups(const std::vector<CodeFixup>& fixups, uint64_t base)
{
	for (size_t i = 0; i < fixups.size(); ++i)
	{
		CodeFixup fixup = fixups[i];
		fixup.offset += static_cast<uint32_t>(base);
		code_fixups_.push_back(fixup);
	}
}

void Image::add_data_fixup(Section section, uint64_t offset, uint32_t target, int64_t addend)
{
	DataFixup fixup;
	fixup.section = section;
	fixup.offset = offset;
	fixup.target = target;
	fixup.addend = addend;
	data_fixups_.push_back(fixup);
}

uint64_t Image::address_of(uint32_t symbol) const
{
	const ImageSymbol& s = this->symbol(symbol);
	switch (s.section)
	{
	case Section::Text:
		return text_address_ + s.offset;
	case Section::Rodata:
		return rodata_address_ + s.offset;
	case Section::Data:
		return data_address_ + s.offset;
	case Section::Bss:
		return bss_address_ + s.offset;
	default:
		return 0;
	}
}

bool Image::write_executable(const std::string& path, uint32_t entry, std::string& error, uint32_t& undefined)
{
	const ExecutableLayout layout = layout_executable(text_.size(), rodata_.size(), data_.size(), bss_size_);
	text_address_ = layout.text_address;
	rodata_address_ = layout.rodata_address;
	data_address_ = layout.data_address;
	bss_address_ = layout.bss_address;
	for (size_t i = 0; i < code_fixups_.size(); ++i)
	{
		const CodeFixup& fixup = code_fixups_[i];
		if (fixup.kind != FixupKind::Address)
		{
			error = "a GOT or thread-pointer reference in a native image";
			return false;
		}
		if (symbol(fixup.target).section == Section::Undefined)
		{
			error = "undefined native symbol";
			undefined = fixup.target;
			return false;
		}
		const uint64_t target = address_of(fixup.target) + static_cast<int64_t>(fixup.addend);
		if (fixup.absolute)
		{
			patch(text_, fixup.offset, target, 8);
			continue;
		}
		const int64_t value = static_cast<int64_t>(target) -
			static_cast<int64_t>(text_address_ + fixup.offset + fixup.end);
		if (value < INT32_MIN || value > INT32_MAX)
		{
			error = "PC-relative reference out of range";
			return false;
		}
		patch(text_, fixup.offset, static_cast<uint64_t>(value), 4);
	}
	for (size_t i = 0; i < data_fixups_.size(); ++i)
	{
		const DataFixup& fixup = data_fixups_[i];
		if (symbol(fixup.target).section == Section::Undefined)
		{
			error = "undefined native symbol";
			undefined = fixup.target;
			return false;
		}
		const uint64_t value = address_of(fixup.target) + static_cast<uint64_t>(fixup.addend);
		patch(fixup.section == Section::Rodata ? rodata_ : data_, fixup.offset, value, 8);
	}
	return write_executable_file(path, layout, text_, rodata_, data_, bss_size_, address_of(entry), error);
}

} // namespace native
