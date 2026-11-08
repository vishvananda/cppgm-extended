#include "lower/function_lowering.h"

#include <cstring>

#include "lex/number_literal.h"
#include "types/fundamental_type.h"

namespace lower {

RValue FunctionLowering::source_floating_literal(
	const sema::Type* type, const char* text, uint32_t length, uint32_t location)
{
	// The semantic facts already provide the literal's format and digit
	// extent. The source scanner accepts separators without extending LowIR's
	// independent literal grammar or rounding via another floating type.
	unsigned char bytes[16] = {};
	std::string scratch;
	if (!ScanFloatingValue(text, length, type->unqualified->fundamental, scratch, bytes))
		throw LoweringError(location, "a floating literal outside its type's range");
	lowir::Literal bits = {};
	std::memcpy(&bits.low, bytes, 8);
	std::memcpy(&bits.high, bytes + 8, 8);
	const SoftFloat soft = SoftFloatOf(type);
	if (soft != SoftFloat::None)
		return soft_constant(soft, bits.low, bits.high);
	return floating(lowering_.storage_type(type), bits);
}

} // namespace lower
