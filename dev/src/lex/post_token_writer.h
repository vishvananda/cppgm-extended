// Output format of DebugPostTokenOutputStream from the CPPGM PA2 starter code
// (C) 2013 CPPGM Foundation www.cppgm.org. See NOTICE.

#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>

#include "lex/post_token.h"

// Writes tokens in the PA2 posttoken text format, one line per token. This
// is a tool view: the compiler consumes PostToken directly. Output collects
// in a buffer that is written out when it grows large, on flush() and on
// destruction.
class PostTokenWriter
{
public:
	explicit PostTokenWriter(std::ostream& out);
	~PostTokenWriter();

	void write(const PostToken& token);
	// Appends a line of other text, such as a dump's header records.
	void write_line(const std::string& text);
	void flush();

private:
	void append(const char* text);
	void append(const char* text, uint32_t length);
	void append(const TokenSpelling& spelling) { append(spelling.text, spelling.length); }
	void append_type_and_value(const PostToken& token);
	void append_ud_prefix(const PostToken& token);

	std::ostream& out_;
	std::string buffer_;
};
