#include "preprocess/preprocessor_names.h"

#include <cstring>

namespace {

struct NamedRole
{
	const char* spelling;
	IdentifierRole role;
};

const NamedRole kNamedRoles[] = { { "if", IdentifierRole::If }, { "ifdef", IdentifierRole::Ifdef },
	{ "ifndef", IdentifierRole::Ifndef }, { "elif", IdentifierRole::Elif }, { "else", IdentifierRole::Else },
	{ "endif", IdentifierRole::Endif }, { "include", IdentifierRole::Include }, { "define", IdentifierRole::Define },
	{ "undef", IdentifierRole::Undef }, { "line", IdentifierRole::Line }, { "error", IdentifierRole::Error },
	{ "pragma", IdentifierRole::Pragma }, { "include_next", IdentifierRole::IncludeNext },
	{ "warning", IdentifierRole::Warning }, { "defined", IdentifierRole::Defined },
	{ "__VA_ARGS__", IdentifierRole::VaArgs }, { "_Pragma", IdentifierRole::PragmaOperator },
	{ "once", IdentifierRole::Once },
	// The names 16.8p1 predefines. This implementation defines none of the
	// conditional names of 16.8p2, and the host headers that complete it may:
	// glibc's <stdc-predef.h>, included by <features.h>, defines
	// __STDC_ISO_10646__.
	{ "__cplusplus", IdentifierRole::ReservedMacroName }, { "__DATE__", IdentifierRole::ReservedMacroName },
	{ "__FILE__", IdentifierRole::ReservedMacroName }, { "__LINE__", IdentifierRole::ReservedMacroName },
	{ "__STDC_HOSTED__", IdentifierRole::ReservedMacroName }, { "__TIME__", IdentifierRole::ReservedMacroName } };

Identifier* Intern(IdentifierTable& identifiers, const char* spelling)
{
	return identifiers.intern(spelling, static_cast<uint32_t>(std::strlen(spelling)));
}

} // namespace

PreprocessorNames::PreprocessorNames(IdentifierTable& identifiers)
{
	for (const NamedRole& named : kNamedRoles)
	{
		Identifier* identifier = identifiers.intern(named.spelling, static_cast<uint32_t>(std::strlen(named.spelling)));
		identifier->role = static_cast<uint8_t>(named.role);
	}
	defined = Intern(identifiers, "defined");
	va_args = Intern(identifiers, "__VA_ARGS__");
	pragma_operator = Intern(identifiers, "_Pragma");
	has_cpp_attribute = Intern(identifiers, "__has_cpp_attribute");
	has_attribute = Intern(identifiers, "__has_attribute");
	file = Intern(identifiers, "__FILE__");
	line = Intern(identifiers, "__LINE__");
	counter = Intern(identifiers, "__COUNTER__");
}
