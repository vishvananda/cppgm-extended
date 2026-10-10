#pragma once

#include <cstdio>
#include <string>

#include "lowir/program.h"

namespace lowir {

// Serializes a program as canonical LowIR text (lowir.md "Program
// Structure"): declared globals, declared functions, global definitions,
// function definitions and object aliases, each phase in declaration
// order; metadata in one bracket per item, in a fixed key order; literals
// by value. Unnamed temporaries, slots and blocks print the presentation
// names their kinds give them (lowir/local_names.h).
void write_program(const Program& program, std::string& out);

// The same text streamed to a file through a bounded buffer. Returns false
// if the file cannot be written.
bool write_program_file(const Program& program, const std::string& path);

// The function-definition phase written as each definition completes, so
// its body can be released at once: a bounded buffer that spills to an
// anonymous temporary file. write_program_file with a stream writes the
// other phases around it in canonical order.
class DefinitionStream : public DefinitionSink
{
public:
	DefinitionStream() : spill_(nullptr), spilled_(0), failed_(false) {}
	~DefinitionStream();

	DefinitionStream(const DefinitionStream&) = delete;
	DefinitionStream& operator=(const DefinitionStream&) = delete;

	// Appends the definition's text (the program's order of definitions
	// is the order of appends unless reordered).
	void append(const Program& program, SymbolId function) override;
	void reorder(const std::vector<uint32_t>& appends) override { order_ = appends; }
	bool empty() const { return buffer_.empty() && spill_ == nullptr; }
	// Copies the section to `file`; false on a write failure.
	bool copy_to(std::FILE* file);

private:
	bool copy_range(std::FILE* file, uint64_t first, uint64_t last);

	std::string buffer_;
	std::FILE* spill_;
	// Bytes in the spill file (the buffer follows them), each append's
	// first byte, and the order to copy the appends in (empty: as
	// appended).
	uint64_t spilled_;
	std::vector<uint64_t> starts_;
	std::vector<uint32_t> order_;
	bool failed_;
};

bool write_program_file(const Program& program, DefinitionStream& definitions, const std::string& path);

} // namespace lowir
