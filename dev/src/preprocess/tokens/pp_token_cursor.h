#pragma once

#include "preprocess/lexer/pp_token.h"

// A pull source of preprocessing tokens: the lexer through PA3, the
// preprocessor from PA4. The returned token and its spelling are valid until
// the next call; after EndOfFile every call returns EndOfFile.
class PPTokenCursor
{
public:
	virtual const PPToken& next() = 0;

protected:
	~PPTokenCursor() {}
};
