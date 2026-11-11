#pragma once

#include "lex/token_kind.h"
#include "sema/constant_values.h"

namespace sema {

bool ConvertBinary128(ConstValue& value, const Type* target);
bool Binary128Fits(const ConstValue& value, const Type* target);
bool Binary128Operation(
	TokenKind operation, const Type* type, const ConstValue& left, const ConstValue& right, ConstValue& result);
bool Binary128Comparison(TokenKind operation, const ConstValue& left, const ConstValue& right);
bool Binary128Zero(const ConstValue& value);

} // namespace sema
