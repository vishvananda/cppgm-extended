#include "lowir/text_reader.h"

#include <chrono>
#include <cstdio>

#include "lowir/text_parser.h"

namespace lowir {

namespace {

// Reads a whole file into `buffer`; false if it cannot be read.
bool read_file(const std::string& path, std::string& buffer)
{
	std::FILE* file = std::fopen(path.c_str(), "rb");
	if (file == nullptr)
		return false;
	// Sized from the file, so the buffer is allocated once.
	buffer.clear();
	if (std::fseek(file, 0, SEEK_END) == 0)
	{
		const long size = std::ftell(file);
		if (size > 0)
			buffer.reserve(static_cast<size_t>(size));
		std::rewind(file);
	}
	char chunk[65536];
	size_t count;
	while ((count = std::fread(chunk, 1, sizeof chunk, file)) != 0)
		buffer.append(chunk, count);
	const bool ok = std::ferror(file) == 0;
	std::fclose(file);
	return ok;
}

} // namespace

bool read_program_files(
	Program& program, const std::vector<std::string>& paths, std::string& error, ReadStatistics* statistics)
{
	// One buffer at a time: tokens refer to it only while it is parsed, and
	// every retained spelling is interned.
	ReadStatistics ignored;
	ReadStatistics& work = statistics != nullptr ? *statistics : ignored;
	const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	text::Parser parser(program);
	std::string buffer;
	try
	{
		for (const std::string& path : paths)
		{
			if (!read_file(path, buffer))
			{
				error = "cannot read " + path;
				return false;
			}
			work.source_bytes += buffer.size();
			parser.parse(buffer.data(), buffer.data() + buffer.size(), path);
		}
		const std::chrono::steady_clock::time_point parsed = std::chrono::steady_clock::now();
		work.parse_ms = std::chrono::duration<double, std::milli>(parsed - start).count();
		parser.finish();
		work.validate_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - parsed).count();
	}
	catch (const text::Failure& failure)
	{
		error = failure.message;
		return false;
	}
	return true;
}

bool read_program_text(Program& program, const std::string& text, const std::string& source_name, std::string& error)
{
	text::Parser parser(program);
	try
	{
		parser.parse(text.data(), text.data() + text.size(), source_name);
		parser.finish();
	}
	catch (const text::Failure& failure)
	{
		error = failure.message;
		return false;
	}
	return true;
}

} // namespace lowir
