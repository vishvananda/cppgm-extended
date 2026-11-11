#include "wasm/object_reader.h"

#include <algorithm>
#include <fstream>
#include <iterator>

namespace wasm {

namespace {

uint32_t decimal(const std::string& text)
{
	uint64_t value = 0;
	bool digit = false, space = false;
	for (char byte : text)
	{
		if (byte == ' ')
			space = true;
		else if (byte < '0' || byte > '9' || space)
			throw WasmError("malformed Wasm archive decimal field");
		else
		{
			digit = true;
			value = value * 10 + byte - '0';
			if (value > UINT32_MAX)
				throw WasmError("Wasm archive decimal field overflows");
		}
	}
	if (!digit)
		throw WasmError("missing Wasm archive decimal field");
	return static_cast<uint32_t>(value);
}

void archive(const Bytes& bytes, const std::string& path, std::vector<Object>& objects)
{
	Cursor input(bytes);
	input.take(8);
	std::string long_names;
	while (!input.empty())
	{
		const Bytes header = input.take(60).bytes();
		if (header.data[58] != '`' || header.data[59] != '\n')
			throw WasmError("invalid Wasm archive member header");
		const std::string text(header.data.begin(), header.data.end());
		std::string name = text.substr(0, 16);
		while (!name.empty() && name.back() == ' ')
			name.pop_back();
		const uint32_t size = decimal(text.substr(48, 10));
		Cursor member = input.take(size);
		if ((size & 1) != 0 && input.byte() != '\n')
			throw WasmError("invalid Wasm archive alignment byte");
		if (name == "//")
		{
			const Bytes names = member.bytes();
			long_names.assign(names.data.begin(), names.data.end());
			continue;
		}
		if (name.compare(0, 3, "#1/") == 0)
		{
			const Bytes extended = member.take(decimal(name.substr(3))).bytes();
			name.assign(extended.data.begin(), extended.data.end());
			while (!name.empty() && name.back() == '\0')
				name.pop_back();
		}
		else if (name.size() > 1 && name[0] == '/' && name[1] >= '0' && name[1] <= '9')
		{
			const uint32_t offset = decimal(name.substr(1));
			if (offset >= long_names.size())
				throw WasmError("invalid Wasm archive long-name offset");
			const size_t end = long_names.find('\n', offset);
			if (end == std::string::npos)
				throw WasmError("unterminated Wasm archive long name");
			name = long_names.substr(offset, end - offset);
		}
		if (name == "/" || name == "/SYM64/" || name.compare(0, 9, "__.SYMDEF") == 0)
			continue;
		if (!name.empty() && name.back() == '/')
			name.pop_back();
		Object object;
		object.path = path + "(" + name + ")";
		object.archive_member = true;
		read_object_bytes(member.bytes(), object);
		objects.push_back(std::move(object));
	}
}

} // namespace

void read_link_inputs(const std::string& path, std::vector<Object>& objects)
{
	std::ifstream file(path, std::ios::binary);
	if (!file)
		throw WasmError("cannot read Wasm link input: " + path);
	Bytes bytes;
	bytes.data.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	const std::string magic(bytes.data.begin(), bytes.data.begin() + std::min<size_t>(8, bytes.data.size()));
	if (magic == "!<arch>\n")
		archive(bytes, path, objects);
	else
	{
		Object object;
		object.path = path;
		read_object_bytes(bytes, object);
		objects.push_back(std::move(object));
	}
}

} // namespace wasm
