#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <stdexcept>
#include <string>

// An immutable physical source file image. Source offsets are byte offsets
// into this image; tokens and diagnostics refer to it instead of copying text.
//
// Construction applies the two whole-file rules of translation phases 1-2:
// a leading UTF-8 byte order mark is not part of the source text, and a
// non-empty file that does not end in a new-line, or ends in a new-line
// preceded by a backslash (possibly spelled `??/`), is processed as if one
// more new-line were appended ([lex.phases] 2.2p1).
class SourceBuffer
{
public:
	explicit SourceBuffer(std::string bytes);

	static SourceBuffer ReadFrom(std::istream& in);

	// Token spellings to be lexed again, as pasted tokens are: no byte order
	// mark is recognized, and a new-line always ends the text.
	static SourceBuffer ForTokenText(std::string text);

	const char* data() const { return bytes_.data(); }
	uint32_t size() const { return static_cast<uint32_t>(bytes_.size()); }

	// Offset of the first source character (after any byte order mark).
	uint32_t begin_offset() const { return begin_; }

private:
	struct TokenTextTag
	{};
	SourceBuffer(std::string bytes, TokenTextTag);

	bool ends_with_spliced_new_line() const;

	std::string bytes_;
	uint32_t begin_;
};

struct SourcePosition
{
	std::size_t line;
	std::size_t column;
};

// Maps non-decreasing offsets (from begin_offset()) to 1-based physical lines
// and byte columns in time proportional to the bytes passed over.
class SourceLineCursor
{
public:
	explicit SourceLineCursor(const SourceBuffer& source);

	SourcePosition advance_to(uint32_t offset);

private:
	const SourceBuffer& source_;
	uint32_t offset_;
	uint32_t line_start_;
	std::size_t line_;
};

// A fatal lexical error at a source offset.
class SourceError : public std::runtime_error
{
public:
	SourceError(uint32_t offset, const std::string& message) : std::runtime_error(message), offset_(offset) {}

	uint32_t offset() const { return offset_; }

private:
	uint32_t offset_;
};

// Renders `line:column: message` for a diagnostic about `source`.
std::string FormatSourceError(const SourceBuffer& source, const SourceError& error);
