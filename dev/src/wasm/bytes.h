#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace wasm {

void validate_name(const std::uint8_t* data, std::size_t size);

struct Bytes
{
	std::vector<std::uint8_t> data;
	void byte(std::uint8_t value) { data.push_back(value); }
	void unsigned_leb(std::uint64_t value);
	void signed_leb(std::int64_t value);
	void fixed(std::uint64_t value, unsigned bytes);
	void string(const std::string& text);
	void append(const Bytes& other);
	void section(std::uint8_t id, const Bytes& contents);
};

} // namespace wasm
