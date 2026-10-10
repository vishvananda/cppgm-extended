#include "native/image/executable_file.h"

#include <cstdio>
#include <sys/stat.h>

#include "native/object/elf_format.h"

namespace native {

namespace {

const uint64_t kBase = 0x400000;
const uint64_t kPage = 0x1000;

uint64_t align_up(uint64_t value, uint64_t align)
{
	return (value + align - 1) & ~(align - 1);
}

void program_header(std::vector<uint8_t>& out, uint32_t type, uint32_t flags, uint64_t offset, uint64_t address,
	uint64_t file_bytes, uint64_t memory_bytes, uint64_t align)
{
	object::put32(out, type);
	object::put32(out, flags);
	object::put64(out, offset);
	object::put64(out, address);
	object::put64(out, address);
	object::put64(out, file_bytes);
	object::put64(out, memory_bytes);
	object::put64(out, align);
}

} // namespace

ExecutableLayout layout_executable(uint64_t text_bytes, uint64_t rodata_bytes, uint64_t data_bytes, uint64_t bss_bytes,
	uint64_t text_align, uint64_t rodata_align, uint64_t data_align, uint64_t bss_align)
{
	// Addresses equal kBase plus file offsets (kBase is aligned beyond any
	// section alignment the linker accepts), so aligning an offset aligns
	// its address.
	ExecutableLayout layout;
	layout.has_rodata = rodata_bytes != 0;
	layout.has_data = data_bytes != 0 || bss_bytes != 0;
	layout.headers = 2 + (layout.has_rodata ? 1 : 0) + (layout.has_data ? 1 : 0);
	layout.text_offset = align_up(
		object::kElfHeaderBytes + object::kProgramHeaderBytes * layout.headers, text_align > 16 ? text_align : 16);
	const uint64_t text_end = layout.text_offset + text_bytes;
	layout.rodata_offset = align_up(text_end, rodata_align > kPage ? rodata_align : kPage);
	const uint64_t rodata_end = layout.rodata_offset + rodata_bytes;
	layout.data_offset = align_up(layout.has_rodata ? rodata_end : text_end, data_align > kPage ? data_align : kPage);
	layout.text_address = kBase + layout.text_offset;
	layout.rodata_address = kBase + layout.rodata_offset;
	layout.data_address = kBase + layout.data_offset;
	layout.bss_address = align_up(layout.data_address + data_bytes, bss_align > 64 ? bss_align : 64);
	return layout;
}

bool write_executable_file(const std::string& path, const ExecutableLayout& layout, const std::vector<uint8_t>& text,
	const std::vector<uint8_t>& rodata, const std::vector<uint8_t>& data, uint64_t bss_bytes, uint64_t entry,
	std::string& error)
{
	const uint64_t text_end = layout.text_offset + text.size();
	std::vector<uint8_t> out;
	out.reserve(layout.text_offset);
	object::put_elf_ident(out);
	object::put16(out, object::kEtExec);
	object::put16(out, object::kEmX86_64);
	object::put32(out, 1);
	object::put64(out, entry);
	object::put64(out, object::kElfHeaderBytes);
	object::put64(out, 0);
	object::put32(out, 0);
	object::put16(out, object::kElfHeaderBytes);
	object::put16(out, object::kProgramHeaderBytes);
	object::put16(out, layout.headers);
	object::put16(out, 0);
	object::put16(out, 0);
	object::put16(out, 0);
	program_header(out, 1, 5, 0, kBase, text_end, text_end, kPage);
	if (layout.has_rodata)
		program_header(out, 1, 4, layout.rodata_offset, layout.rodata_address, rodata.size(), rodata.size(), kPage);
	uint64_t data_file_bytes = data.size();
	while (data_file_bytes > 0 && data[data_file_bytes - 1] == 0)
		--data_file_bytes;
	if (layout.has_data)
		program_header(out, 1, 6, layout.data_offset, layout.data_address, data_file_bytes,
			layout.bss_address + bss_bytes - layout.data_address, kPage);
	program_header(out, 0x6474e551, 6, 0, 0, 0, 0, 16); // PT_GNU_STACK
	out.resize(layout.text_offset, 0);
	std::FILE* file = std::fopen(path.c_str(), "wb");
	if (file == nullptr)
	{
		error = "cannot write " + path;
		return false;
	}
	uint64_t position = 0;
	bool written = true;
	auto write = [&](const uint8_t* bytes, uint64_t count) {
		written = written && std::fwrite(bytes, 1, count, file) == count;
		position += count;
	};
	auto pad_to = [&](uint64_t offset) {
		static const uint8_t zeros[256] = { 0 };
		while (position < offset)
			write(zeros, offset - position < sizeof zeros ? offset - position : sizeof zeros);
	};
	write(out.data(), out.size());
	write(text.data(), text.size());
	if (layout.has_rodata)
	{
		pad_to(layout.rodata_offset);
		write(rodata.data(), rodata.size());
	}
	if (layout.has_data)
	{
		pad_to(layout.data_offset);
		write(data.data(), data_file_bytes);
	}
	const bool closed = std::fclose(file) == 0;
	if (!written || !closed)
	{
		error = "cannot write " + path;
		return false;
	}
	chmod(path.c_str(), 0755);
	return true;
}

} // namespace native
