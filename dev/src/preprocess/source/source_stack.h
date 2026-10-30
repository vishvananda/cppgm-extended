#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "lex/identifier_table.h"
#include "preprocess/lexer/pp_lexer.h"
#include "preprocess/macro/macro_definition.h"
#include "preprocess/macro/macro_expander.h"
#include "preprocess/preprocessor_names.h"
#include "preprocess/source/source_files.h"
#include "support/memory/arena.h"
#include "support/telemetry.h"

// How much of a file read so far is a whole-file #ifndef guard
// (SourceFile::guard): nothing yet, inside `#ifndef guard`, after its
// #endif, or something else.
enum class GuardState : uint8_t
{
	Start,
	Open,
	Closed,
	None
};

// One file being read: an inclusion of a source file (its index in
// SourceFiles, which keeps the presumed name and line numbering) with its
// lexer.
struct SourceFrame
{
	SourceFile* file;
	std::unique_ptr<PPLexer> lexer;
	uint32_t inclusion;
	uint32_t base;
	// The number of open conditional groups when the file was entered; a
	// file shall close the groups it opens.
	std::size_t conditional_depth;
	// The include search directory the file was found in (where
	// #include_next continues), or -1.
	int32_t search_index;
	// The next token starts a line.
	bool at_line_start;
	// The guard candidate and the number of conditional groups open outside
	// its #ifndef.
	GuardState guard_state;
	Identifier* guard;
	std::size_t guard_depth;
};

// Told when an included file ends, before the includer continues.
class SourceFrameObserver
{
public:
	virtual void leaving_file(const SourceFrame& frame) = 0;

protected:
	~SourceFrameObserver() {}
};

// The stack of files being read, as preprocessing tokens for the
// preprocessor. White space and new-lines become token flags; identifiers
// are interned; spellings the lexer built in scratch storage are copied into
// the arena, so every token stays valid for the translation unit.
//
// As an ExpansionInput it yields the text of the translation unit: an
// included file's tokens run on into its includer's, and the text stops at
// the `#` of each directive until begin_directive(). In line mode it yields
// the rest of a directive line instead.
class SourceStack final : public ExpansionInput
{
public:
	SourceStack(SourceFiles& files, IdentifierTable& identifiers, Arena& arena, const PreprocessorNames& names,
		SourceFrameObserver& observer);
	~SourceStack();

	// Reads `file` from its start. `included_at` is the location of the
	// #include directive that includes it in the current file (kNoLocation
	// for a file no directive includes).
	void enter(SourceFile* file, std::string presumed_name, std::size_t conditional_depth, int32_t search_index = -1,
		uint32_t included_at = kNoLocation);
	std::size_t depth() const { return frames_.size(); }
	SourceFrame& current() { return *frames_.back(); }
	const SourceFrame& current() const { return *frames_.back(); }
	SourceInclusion& current_inclusion() { return files_.inclusion(frames_.back()->inclusion); }

	// ExpansionInput.
	const PPToken& next() override;
	const PPToken& peek() override;

	// The text stopped at a directive's `#`, or at the end of the
	// translation unit.
	bool at_directive() const { return at_directive_; }
	bool at_end() const { return at_end_; }
	uint32_t directive_location() const { return directive_location_; }

	// Directive lines. begin_directive() starts the directive whose `#`
	// stopped the text; next_in_line() returns its tokens and then its
	// new-line. In line mode the
	// ExpansionInput reads the rest of the line, starting with `first` if
	// given, and ends at the new-line.
	void begin_directive() { at_directive_ = false; }
	const PPToken& next_in_line();
	void skip_line();
	void begin_line_input(const PPToken* first);
	void end_line_input();
	void request_header_name() { frames_.back()->lexer->request_header_name(); }
	void allow_va_args(bool allow) { allow_va_args_ = allow; }
	// Hosted lexical extensions (hexadecimal pp-numbers and opaque comment
	// bytes) in the files entered from now on.
	void set_hosted(bool enabled) { hosted_ = enabled; }

	// Skipped groups ([cpp.cond] 16.1p6): reads to the `#` of the next
	// directive in the current file; false at the file's end.
	bool skip_to_directive();

	telemetry::Counter tokens_read() const { return tokens_read_; }

private:
	void lex_text(PPToken& token);
	void lex_line(PPToken& token);
	void read_line_token(PPToken& token);
	void stabilize(PPToken& token, const SourceFrame& frame);
	void leave_file();

	SourceFiles& files_;
	IdentifierTable& identifiers_;
	Arena& arena_;
	const PreprocessorNames& names_;
	SourceFrameObserver& observer_;
	std::vector<std::unique_ptr<SourceFrame>> frames_;
	PPToken token_;
	PPToken peeked_;
	bool has_peeked_;
	bool line_mode_;
	bool line_ended_;
	bool at_directive_;
	bool at_end_;
	bool allow_va_args_;
	bool hosted_;
	uint32_t directive_location_;
	telemetry::Counter tokens_read_;
};
