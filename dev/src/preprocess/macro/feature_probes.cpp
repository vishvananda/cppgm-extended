#include "preprocess/macro/feature_probes.h"

#include <cstring>
#include <string>

#include "lex/gnu_builtins.h"

namespace {

// The C++11 language features (Clang's names), all of which this
// implementation provides, and the C11 _Atomic extension (`c_atomic`).
const char* const kLanguageFeatures[] = { "c_atomic", "cxx_access_control_sfinae", "cxx_alias_templates", "cxx_alignas",
	"cxx_alignof", "cxx_atomic", "cxx_attributes", "cxx_auto_type", "cxx_binary_literals", "cxx_constexpr",
	"cxx_decltype", "cxx_decltype_incomplete_return_types", "cxx_default_function_template_args",
	"cxx_defaulted_functions", "cxx_delegating_constructors", "cxx_deleted_functions", "cxx_explicit_conversions",
	"cxx_generalized_initializers", "cxx_implicit_moves", "cxx_inheriting_constructors", "cxx_inline_namespaces",
	"cxx_lambdas", "cxx_local_type_template_args", "cxx_noexcept", "cxx_nonstatic_member_init", "cxx_nullptr",
	"cxx_override_control", "cxx_range_for", "cxx_raw_string_literals", "cxx_reference_qualified_functions",
	"cxx_rvalue_references", "cxx_static_assert", "cxx_strong_enums", "cxx_thread_local", "cxx_trailing_return",
	"cxx_unicode_literals", "cxx_unrestricted_unions", "cxx_user_literals", "cxx_variable_templates",
	"cxx_variadic_templates" };

bool Equals(const char* entry, const char* spelling, uint32_t length)
{
	return std::strlen(entry) == length && std::memcmp(entry, spelling, length) == 0;
}

} // namespace

bool IsProvidedFeature(const char* spelling, uint32_t length, const FeatureModes& modes)
{
	if (length > 4 && std::memcmp(spelling, "__", 2) == 0 && std::memcmp(spelling + length - 2, "__", 2) == 0)
	{
		spelling += 2;
		length -= 4;
	}
	if (Equals("cxx_exceptions", spelling, length))
		return modes.exceptions;
	if (Equals("cxx_rtti", spelling, length))
		return modes.rtti;
	for (const char* feature : kLanguageFeatures)
	{
		if (Equals(feature, spelling, length))
			return true;
	}
	// A trait feature names the trait `__name`; of the transforms only
	// `underlying_type` is one.
	if ((length > 3 && std::memcmp(spelling, "is_", 3) == 0) || (length > 4 && std::memcmp(spelling, "has_", 4) == 0) ||
		Equals("underlying_type", spelling, length))
	{
		const std::string trait = "__" + std::string(spelling, length);
		GnuTrait found;
		return FindGnuTrait(trait.data(), static_cast<uint32_t>(trait.size()), found) &&
			(!IsGnuTypeTransform(found) || found == GnuTrait::UnderlyingType);
	}
	return false;
}
