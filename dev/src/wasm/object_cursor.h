#pragma once

#include <cstring>
#include "wasm/bytes.h"
#include "wasm/mir.h"

namespace wasm {

class Cursor
{
public:
	Cursor(const uint8_t* data, size_t bytes) : data_(data), size_(bytes) {}
	explicit Cursor(const Bytes& bytes) : Cursor(bytes.data.data(), bytes.data.size()) {}
	uint8_t byte()
	{
		if (position_ == size_)
			throw WasmError("truncated Wasm object");
		return data_[position_++];
	}
	uint32_t u32()
	{
		uint32_t value = 0;
		for (unsigned i = 0; i < 5; ++i)
		{
			const uint8_t part = byte();
			if (i == 4 && (part & 0xf0) != 0)
				throw WasmError("Wasm object has an overflowing unsigned LEB");
			value |= uint32_t(part & 0x7f) << (i * 7);
			if ((part & 0x80) == 0)
				return value;
		}
		throw WasmError("Wasm object has an unterminated unsigned LEB");
	}
	int64_t signed_leb(unsigned bits = 32)
	{
		uint64_t value = 0;
		unsigned shift = 0;
		for (unsigned i = 0; i < (bits + 6) / 7; ++i)
		{
			const uint8_t part = byte();
			if (i == (bits + 6) / 7 - 1)
			{
				const unsigned significant = bits - i * 7;
				const uint8_t extra = static_cast<uint8_t>((0x7f << significant) & 0x7f);
				const uint8_t expected = (part & (1u << (significant - 1))) != 0 ? extra : 0;
				if ((part & extra) != expected)
					throw WasmError("Wasm object has an overflowing signed LEB");
			}
			value |= uint64_t(part & 0x7f) << shift;
			shift += 7;
			if ((part & 0x80) == 0)
			{
				if (shift < 64 && (part & 0x40) != 0)
					value |= ~uint64_t(0) << shift;
				const int64_t result = static_cast<int64_t>(value);
				return result;
			}
		}
		throw WasmError("Wasm object has an unterminated signed LEB");
	}
	Cursor take(size_t bytes)
	{
		if (bytes > size_ - position_)
			throw WasmError("Wasm object length exceeds its enclosing section");
		Cursor result(data_ + position_, bytes);
		position_ += bytes;
		return result;
	}
	std::string string()
	{
		const uint32_t bytes = u32();
		Cursor text = take(bytes);
		validate_name(text.data_, bytes);
		return std::string(reinterpret_cast<const char*>(text.data_), bytes);
	}
	Bytes bytes()
	{
		Bytes result;
		result.data.assign(data_ + position_, data_ + size_);
		position_ = size_;
		return result;
	}
	Bytes consumed(uint32_t start) const
	{
		if (start > position_)
			throw WasmError("invalid Wasm byte range");
		Bytes result;
		result.data.assign(data_ + start, data_ + position_);
		return result;
	}
	void done() const
	{
		if (!empty())
			throw WasmError("unexpected trailing data in Wasm object section");
	}
	bool empty() const { return position_ == size_; }
	uint32_t position() const { return static_cast<uint32_t>(position_); }

private:
	const uint8_t* data_;
	size_t size_;
	size_t position_ = 0;
};

} // namespace wasm
