#include "native/image/executable_file.h"

#include <algorithm>
#include <cstdio>
#include <sys/stat.h>

// The v3 Mach-O executable layout, extended to separate read-only data,
// initialized data and zero-filled storage. No system assembler or linker
// is needed for a standalone course executable.
namespace native {
namespace {

const uint64_t kBase = 0x100000000ull;
const uint64_t kPage = 4096;

uint64_t align_up(uint64_t value, uint64_t align)
{
	return (value + align - 1) & ~(align - 1);
}

void put(std::vector<uint8_t>& out, uint64_t value, unsigned width)
{
	for (unsigned i = 0; i < width; ++i)
		out.push_back(static_cast<uint8_t>(value >> (8 * i)));
}

void name(std::vector<uint8_t>& out, const char* text)
{
	for (unsigned i = 0; i < 16; ++i)
	{
		out.push_back(static_cast<uint8_t>(*text));
		if (*text != 0)
			++text;
	}
}

void segment(std::vector<uint8_t>& out, const char* name_text, uint64_t address, uint64_t memory, uint64_t offset,
	uint64_t bytes, uint32_t protection, uint32_t sections)
{
	put(out, 0x19, 4);
	put(out, 72 + sections * 80, 4);
	name(out, name_text);
	put(out, address, 8);
	put(out, memory, 8);
	put(out, offset, 8);
	put(out, bytes, 8);
	put(out, protection, 4);
	put(out, protection, 4);
	put(out, sections, 4);
	put(out, 0, 4);
}

void section(std::vector<uint8_t>& out, const char* name_text, const char* segment_text, uint64_t address,
	uint64_t bytes, uint64_t offset, uint32_t flags)
{
	name(out, name_text);
	name(out, segment_text);
	put(out, address, 8);
	put(out, bytes, 8);
	put(out, offset, 4);
	put(out, 0, 4); // addresses already honor the largest input alignment
	put(out, 0, 4);
	put(out, 0, 4);
	put(out, flags, 4);
	put(out, 0, 4);
	put(out, 0, 4);
	put(out, 0, 4);
}

} // namespace

ExecutableLayout layout_macho_executable(uint64_t text_bytes, uint64_t rodata_bytes, uint64_t data_bytes,
	uint64_t bss_bytes, uint64_t text_align, uint64_t rodata_align, uint64_t data_align, uint64_t bss_align)
{
	ExecutableLayout layout;
	layout.target = Target::MacOS;
	layout.has_rodata = rodata_bytes != 0;
	layout.has_data = data_bytes != 0 || bss_bytes != 0;
	layout.text_offset = align_up(kPage, std::max<uint64_t>(text_align, 16));
	layout.text_address = kBase + layout.text_offset;
	layout.rodata_offset = align_up(layout.text_offset + text_bytes, std::max(kPage, rodata_align));
	layout.rodata_address = kBase + layout.rodata_offset;
	layout.data_offset = align_up(
		layout.has_rodata ? layout.rodata_offset + rodata_bytes : layout.text_offset + text_bytes,
		std::max(kPage, data_align));
	layout.data_address = kBase + layout.data_offset;
	layout.bss_address = align_up(layout.data_address + data_bytes, std::max<uint64_t>(64, bss_align));
	return layout;
}

bool write_macho_executable(const std::string& path, const ExecutableLayout& layout, const std::vector<uint8_t>& text,
	const std::vector<uint8_t>& rodata, const std::vector<uint8_t>& data, uint64_t bss_bytes, uint64_t entry,
	std::string& error)
{
	std::vector<uint8_t> out;
	const uint32_t commands = 72 + 152 + (layout.has_rodata ? 152 : 0) + (layout.has_data ? 232 : 0) + 184 + 24;
	put(out, 0xfeedfacf, 4);
	put(out, 0x01000007, 4);
	put(out, 3, 4);
	put(out, 2, 4); // MH_EXECUTE
	put(out, 4 + (layout.has_rodata ? 1 : 0) + (layout.has_data ? 1 : 0), 4);
	put(out, commands, 4);
	put(out, 1, 4); // MH_NOUNDEFS
	put(out, 0, 4);
	segment(out, "__PAGEZERO", 0, kBase, 0, 0, 0, 0);
	segment(out, "__TEXT", kBase, align_up(layout.text_offset + text.size(), kPage), 0,
		layout.text_offset + text.size(), 5, 1);
	section(out, "__text", "__TEXT", layout.text_address, text.size(), layout.text_offset, 0x80000400);
	if (layout.has_rodata)
	{
		segment(out, "__RODATA", layout.rodata_address, align_up(rodata.size(), kPage), layout.rodata_offset,
			rodata.size(), 1, 1);
		section(out, "__const", "__RODATA", layout.rodata_address, rodata.size(), layout.rodata_offset, 0);
	}
	uint64_t initialized = data.size();
	while (initialized != 0 && data[initialized - 1] == 0)
		--initialized;
	if (layout.has_data)
	{
		segment(out, "__DATA", layout.data_address,
			align_up(layout.bss_address + bss_bytes - layout.data_address, kPage), layout.data_offset, initialized, 3,
			2);
		section(out, "__data", "__DATA", layout.data_address, initialized, layout.data_offset, 0);
		section(out, "__bss", "__DATA", layout.data_address + initialized,
			layout.bss_address + bss_bytes - layout.data_address - initialized, 0, 1);
	}
	put(out, 5, 4); // LC_UNIXTHREAD, x86_THREAD_STATE64
	put(out, 184, 4);
	put(out, 4, 4);
	put(out, 42, 4);
	for (unsigned i = 0; i < 21; ++i)
		put(out, i == 16 ? entry : i == 17 ? 0x202 : i == 18 ? 0x2b : 0, 8);
	put(out, 0x32, 4); // LC_BUILD_VERSION
	put(out, 24, 4);
	put(out, 1, 4);
	put(out, 0x000a0f00, 4);
	put(out, 0, 4);
	put(out, 0, 4);
	out.resize(layout.text_offset, 0);
	out.insert(out.end(), text.begin(), text.end());
	if (layout.has_rodata)
	{
		out.resize(layout.rodata_offset, 0);
		out.insert(out.end(), rodata.begin(), rodata.end());
	}
	if (layout.has_data)
	{
		out.resize(layout.data_offset, 0);
		out.insert(out.end(), data.begin(), data.begin() + initialized);
	}
	std::FILE* file = std::fopen(path.c_str(), "wb");
	if (file == nullptr)
	{
		error = "cannot write " + path;
		return false;
	}
	const bool written = std::fwrite(out.data(), 1, out.size(), file) == out.size();
	const bool closed = std::fclose(file) == 0;
	if (!written || !closed || chmod(path.c_str(), 0755) != 0)
	{
		error = "cannot write executable " + path;
		return false;
	}
	return true;
}

} // namespace native
