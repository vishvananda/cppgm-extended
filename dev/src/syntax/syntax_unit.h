#pragma once

#include <string>

#include "lex/post_tokenizer.h"
#include "preprocess/preprocessor.h"
#include "support/internal_error.h"
#include "support/memory/arena.h"
#include "syntax/ast.h"
#include "syntax/parse_token_stream.h"
#include "syntax/parser.h"

namespace syntax {

// One translation unit through translation phases 1-7 and parsing. It owns
// what the tree refers to: the preprocessor (source images, interned
// identifiers, token spellings), the token stream's spelling arena and the
// syntax arena, all released together with this object.
class SyntaxUnit
{
public:
	// Throws PreprocessError if the source cannot be opened.
	SyntaxUnit(const PreprocessorOptions& options, const std::string& path);

	// Parses the unit; false after a preprocessing, lexical or syntax error,
	// described by error().
	bool parse();

	const TranslationUnit& tree() const { return *tree_; }
	const std::string& error() const { return error_; }
	// The unit's interned identifiers (semantic analysis interns the keys
	// of operator and special member names there).
	IdentifierTable& identifiers() { return preprocessor_.identifiers(); }
	// The presumed file and line of tree locations.
	const SourcePositions& positions() const { return preprocessor_; }
	// A diagnostic at a tree location, with its source position.
	std::string describe(uint32_t location, const char* message) const
	{
		return preprocessor_.describe(location, message);
	}
	// An internal error raised while the unit is processed, its location
	// (if it has one) rendered into the message as describe() renders it.
	InternalError locate(const InternalError& error) const;
	ParserCounters counters() const { return parser_.counters(); }
	PreprocessorCounters preprocessor_counters() const { return preprocessor_.counters(); }
	uint64_t spelling_bytes() const { return spellings_.bytes_reserved(); }

private:
	Preprocessor preprocessor_;
	PostTokenizer post_tokens_;
	Arena spellings_;
	Arena arena_;
	ParseTokenStream tokens_;
	Parser parser_;
	TranslationUnit* tree_;
	std::string error_;
};

} // namespace syntax
