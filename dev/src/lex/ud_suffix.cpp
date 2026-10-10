#include "lex/ud_suffix.h"

#include "preprocess/lexer/char_classes.h"
#include "support/unicode/utf8.h"

bool IsUserDefinedSuffix(const char* text, uint32_t length, bool reserved)
{
	if (length == 0 || (text[0] != '_' && !(reserved && IsIdentifierStart(static_cast<unsigned char>(text[0])))))
		return false;
	const unsigned char* bytes = reinterpret_cast<const unsigned char*>(text);
	for (uint32_t i = 1; i < length;)
	{
		if (bytes[i] < 0x80)
		{
			if (!IsIdentifierContinue(bytes[i]))
				return false;
			++i;
			continue;
		}
		const Utf8Decoded c = DecodeUtf8(bytes + i, length - i);
		if (c.status != Utf8DecodeStatus::Ok || !IsIdentifierContinue(static_cast<int32_t>(c.code_point)))
			return false;
		i += c.length;
	}
	return true;
}
