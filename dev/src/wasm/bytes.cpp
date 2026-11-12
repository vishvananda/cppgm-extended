#include "wasm/bytes.h"

#include "wasm/mir.h"
#include "support/unicode/utf8.h"

namespace wasm {

void validate_name(const uint8_t* data, size_t size)
{
	for (size_t offset = 0; offset < size;)
	{
		if (data[offset] < 0x80)
		{
			++offset;
			continue;
		}
		const Utf8Decoded decoded = DecodeUtf8(data + offset, size - offset);
		if (decoded.status != Utf8DecodeStatus::Ok)
			throw WasmError("invalid UTF-8 in Wasm name");
		offset += decoded.length;
	}
}

void Bytes::unsigned_leb(uint64_t value)
{
	do
	{
		const uint8_t part = value & 0x7f;
		value >>= 7;
		byte(part | (value != 0 ? 0x80 : 0));
	} while (value != 0);
}

void Bytes::signed_leb(int64_t value)
{
	bool more;
	do
	{
		const uint8_t part = value & 0x7f;
		value >>= 7;
		more = !((value == 0 && (part & 0x40) == 0) || (value == -1 && (part & 0x40) != 0));
		byte(part | (more ? 0x80 : 0));
	} while (more);
}

void Bytes::fixed(uint64_t value, unsigned bytes)
{
	for (unsigned i = 0; i < bytes; ++i)
		byte(static_cast<uint8_t>(value >> (8 * i)));
}

void Bytes::string(const std::string& text)
{
	validate_name(reinterpret_cast<const uint8_t*>(text.data()), text.size());
	unsigned_leb(text.size());
	data.insert(data.end(), text.begin(), text.end());
}

void Bytes::append(const Bytes& other)
{
	if (other.data.size() > UINT32_MAX || data.size() > UINT32_MAX - other.data.size())
		throw WasmError("Wasm section exceeds its 32-bit size limit");
	data.insert(data.end(), other.data.begin(), other.data.end());
}

void Bytes::section(uint8_t id, const Bytes& contents)
{
	if (contents.data.size() > UINT32_MAX)
		throw WasmError("Wasm section exceeds its 32-bit size limit");
	byte(id);
	unsigned_leb(contents.data.size());
	append(contents);
}

} // namespace wasm
