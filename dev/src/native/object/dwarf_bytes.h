#pragma once

#include <cstdint>
#include <vector>

namespace native {

// Little-endian DWARF fields and its signed/unsigned LEB128 operands.
class DwarfBytes
{
public:
	explicit DwarfBytes(std::vector<uint8_t>& bytes) : bytes_(bytes) {}
	uint32_t size() const;
	void fixed(uint64_t value, unsigned width);
	void uleb(uint64_t value);
	void sleb(int64_t value);
	uint32_t text(const char* value);
	void patch32(uint32_t offset, uint32_t value);

private:
	std::vector<uint8_t>& bytes_;
};

} // namespace native
