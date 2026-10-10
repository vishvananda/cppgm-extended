#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "lex/text_literal.h"
#include "lex/token_spelling.h"
#include "types/fundamental_type.h"

// Translation phase 6 for one maximal sequence of adjacent string-literal and
// user-defined-string-literal tokens ([lex.string] 2.14.5p13, [lex.ext]
// 2.14.8p8), under the course rules: the pieces may carry at most one
// distinct encoding-prefix, which then applies to the whole sequence, and at
// most one distinct ud-suffix. The encoding is known only after the last
// piece, so the pieces' spellings are copied, space-separated, and encoded
// together.
//
// Buffers are reused from one sequence to the next.
class StringLiteralSequence
{
public:
	// Starts an empty sequence.
	void clear();
	// Accept reserved ud-suffixes (the compiler's dialect, see
	// IsUserDefinedSuffix).
	void set_reserved_suffixes(bool on) { reserved_suffixes_ = on; }

	// Appends the next piece. The spelling is copied.
	void add(const char* spelling, uint32_t length);

	// Concatenates the pieces into code units, including the terminating
	// null. False if the sequence is ill-formed; source() stays available.
	bool concatenate();

	// The pieces' spellings separated by single spaces, valid until the next
	// clear() or add().
	TokenSpelling source() const
	{
		return TokenSpelling{ spellings_.data(), static_cast<uint32_t>(spellings_.size()) };
	}

	// After a successful concatenate(): the array element type, the code
	// units (little-endian), the array bound and the ud-suffix (empty if
	// none).
	FundamentalType element_type() const { return StringElementType(prefix_); }
	const unsigned char* data() const { return reinterpret_cast<const unsigned char*>(units_.data()); }
	uint32_t size() const { return static_cast<uint32_t>(units_.size()); }
	uint32_t element_count() const { return size() / CodeUnitWidth(prefix_); }
	TokenSpelling ud_suffix() const { return suffix_; }

private:
	// Where a piece's spelling is in spellings_.
	struct Piece
	{
		uint32_t offset;
		uint32_t length;
	};

	bool combine_prefixes_and_suffixes();
	bool encode_body(const char* text, const TextLiteralParts& parts);
	void append_unit(uint32_t unit);
	void append_code_point(uint32_t code_point);

	std::string spellings_;
	std::vector<Piece> pieces_;
	std::string units_;
	EncodingPrefix prefix_ = EncodingPrefix::None;
	unsigned width_ = 1;
	TokenSpelling suffix_ = TokenSpelling{ nullptr, 0 };
	bool reserved_suffixes_ = false;
};
