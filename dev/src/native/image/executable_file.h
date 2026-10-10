#pragma once

#include <cstdint>
#include <string>
#include <vector>

// The static Linux x86-64 executable both native outputs write
// (lowir2native's image and link mode's linker): the ELF and program
// headers, then one PT_LOAD per protection class (code r-x, constants r--,
// data and zero-filled storage rw-) and PT_GNU_STACK. No section headers.
namespace native {

struct ExecutableLayout
{
	uint64_t text_offset = 0;
	uint64_t text_address = 0;
	uint64_t rodata_offset = 0;
	uint64_t rodata_address = 0;
	uint64_t data_offset = 0;
	uint64_t data_address = 0;
	uint64_t bss_address = 0;
	unsigned headers = 0;
	bool has_rodata = false;
	bool has_data = false;
};

// The file offsets and addresses of segments of these sizes, each aligned
// to its contents' largest alignment (the text at least 16, the
// zero-filled storage at least 64; the others start on pages).
ExecutableLayout layout_executable(uint64_t text_bytes, uint64_t rodata_bytes, uint64_t data_bytes, uint64_t bss_bytes,
	uint64_t text_align = 16, uint64_t rodata_align = 1, uint64_t data_align = 1, uint64_t bss_align = 64);

// Writes the executable (mode 0755). Trailing zero bytes of the data take
// no file space: the segment's memory size covers them.
bool write_executable_file(const std::string& path, const ExecutableLayout& layout, const std::vector<uint8_t>& text,
	const std::vector<uint8_t>& rodata, const std::vector<uint8_t>& data, uint64_t bss_bytes, uint64_t entry,
	std::string& error);

} // namespace native
