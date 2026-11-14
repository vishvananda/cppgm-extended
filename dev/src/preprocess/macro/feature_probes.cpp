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

struct LanguageFeature
{
	const char* name;
	const char* value;
	unsigned language;
};

const LanguageFeature kLanguageMacros[] = { { "aggregate_nsdmi", "201304L", 14 }, { "binary_literals", "201304L", 14 },
	{ "constexpr", "201304L", 14 }, { "decltype_auto", "201304L", 14 }, { "digit_separators", "201309L", 14 },
	{ "generic_lambdas", "201304L", 14 }, { "init_captures", "201304L", 14 },
	{ "return_type_deduction", "201304L", 14 }, { "sized_deallocation", "201309L", 14 },
	{ "variable_templates", "201304L", 14 }, { "aggregate_bases", "201603L", 17 },
	{ "capture_star_this", "201603L", 17 }, { "constexpr", "201603L", 17 }, { "deduction_guides", "201703L", 17 },
	{ "enumerator_attributes", "201411L", 17 }, { "namespace_attributes", "201411L", 17 },
	{ "hex_float", "201603L", 17 }, { "fold_expressions", "201603L", 17 }, { "guaranteed_copy_elision", "201606L", 17 },
	{ "if_constexpr", "201606L", 17 }, { "inline_variables", "201606L", 17 },
	{ "nested_namespace_definitions", "201411L", 17 }, { "noexcept_function_type", "201510L", 17 },
	{ "nontype_template_args", "201411L", 17 }, { "nontype_template_parameter_auto", "201606L", 17 },
	{ "range_based_for", "201603L", 17 }, { "static_assert", "201411L", 17 }, { "structured_bindings", "201606L", 17 },
	{ "template_auto", "201606L", 17 }, { "template_template_args", "201611L", 17 },
	{ "variadic_using", "201611L", 17 } };

} // namespace

bool IsProvidedFeature(const char* spelling, uint32_t length, const FeatureModes& modes)
{
	if (length > 4 && std::memcmp(spelling, "__", 2) == 0 && std::memcmp(spelling + length - 2, "__", 2) == 0)
	{
		spelling += 2;
		length -= 4;
	}
	if (modes.language == 0 && length >= 4 && std::memcmp(spelling, "cxx_", 4) == 0)
		return false;
	if (Equals("cxx_exceptions", spelling, length))
		return modes.exceptions;
	if (Equals("cxx_rtti", spelling, length))
		return modes.rtti;
	if (Equals("gnu_asm", spelling, length))
		return true;
	static const char* const kCxx14[] = { "cxx_aggregate_nsdmi", "cxx_contextual_conversions", "cxx_decltype_auto",
		"cxx_generic_lambdas", "cxx_init_captures", "cxx_relaxed_constexpr", "cxx_return_type_deduction" };
	for (const char* feature : kCxx14)
		if (Equals(feature, spelling, length))
			return modes.language >= 14;
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

std::string LanguageFeatureMacros(unsigned language)
{
	std::string text = "#define __cpp_constexpr 200704L\n#define __cpp_range_based_for 200907L\n"
					   "#define __cpp_static_assert 200410L\n";
	for (const LanguageFeature& feature : kLanguageMacros)
	{
		if (feature.language > language)
			continue;
		text += std::string("#undef __cpp_") + feature.name + "\n#define __cpp_" + feature.name + " " + feature.value +
			"\n";
	}
	return text;
}

bool IsLanguageFeatureMacro(const char* spelling, uint32_t length)
{
	if (length < 7 || std::memcmp(spelling, "__cpp_", 6) != 0)
		return false;
	for (const LanguageFeature& feature : kLanguageMacros)
		if (Equals(feature.name, spelling + 6, length - 6))
			return true;
	return false;
}
