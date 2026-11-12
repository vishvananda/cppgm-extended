#pragma once

#include "lowir/program.h"

namespace machine {

enum class BuiltinContract : uint8_t
{
	Trap,
	StringLength,
	Copy,
	Move,
	Fill
};

// Semantic scalar boundary only. Identity and expansion costs belong to the
// caller; no symbol-name work is added to ordinary call paths.
inline bool valid_builtin_boundary(const lowir::Signature& signature, const lowir::Parameter* parameters,
	uint32_t arguments, uint32_t pointer_bytes, BuiltinContract kind)
{
	using namespace lowir;
	if (signature.boundary.arity != ir_model::CAM_FIXED || signature.parameter_count != arguments)
		return false;
	if (kind == BuiltinContract::Trap)
		return arguments == 0 && signature.result.is_void();
	const auto size_type = [&](Type type) { return type.is_integer() && storage_bytes(type) == pointer_bytes; };
	if (kind == BuiltinContract::StringLength)
		return arguments == 1 && parameters[0].type.is_pointer() && size_type(signature.result);
	return arguments == 3 && signature.result.is_pointer() && parameters[0].type.is_pointer() &&
		(kind == BuiltinContract::Fill ? parameters[1].type.is_integer() && storage_bytes(parameters[1].type) == 4
									   : parameters[1].type.is_pointer()) &&
		size_type(parameters[2].type);
}

} // namespace machine
