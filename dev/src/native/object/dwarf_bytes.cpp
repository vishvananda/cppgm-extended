#include "native/object/dwarf_bytes.h"

namespace native {

uint32_t DwarfBytes::size() const
{
	return static_cast<uint32_t>(bytes_.size());
}

void DwarfBytes::fixed(uint64_t value, unsigned width)
{
	for (unsigned i = 0; i < width; ++i)
	{
		bytes_.push_back(static_cast<uint8_t>(value));
		value >>= 8;
	}
}

void DwarfBytes::uleb(uint64_t value)
{
	do
	{
		uint8_t byte = static_cast<uint8_t>(value & 0x7f);
		value >>= 7;
		bytes_.push_back(byte | (value != 0 ? 0x80 : 0));
	} while (value != 0);
}

void DwarfBytes::sleb(int64_t value)
{
	for (;;)
	{
		const uint8_t byte = static_cast<uint8_t>(value & 0x7f);
		value >>= 7;
		const bool done = (value == 0 && (byte & 0x40) == 0) || (value == -1 && (byte & 0x40) != 0);
		bytes_.push_back(byte | (done ? 0 : 0x80));
		if (done)
			return;
	}
}

uint32_t DwarfBytes::text(const char* value)
{
	const uint32_t result = size();
	do
	{
		bytes_.push_back(static_cast<uint8_t>(*value));
	} while (*value++ != 0);
	return result;
}

void DwarfBytes::patch32(uint32_t offset, uint32_t value)
{
	for (unsigned i = 0; i < 4; ++i)
	{
		bytes_[offset + i] = static_cast<uint8_t>(value);
		value >>= 8;
	}
}

} // namespace native
