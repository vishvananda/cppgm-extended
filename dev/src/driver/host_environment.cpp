#include "driver/host_environment.h"

#include <cstring>
#include <string>

#include "cppgm_builtin_host_config.h"

namespace driver {

namespace {

// The names the preprocessor predefines itself (preprocess/preprocessor.cpp,
// [cpp.predefined] 16.8), whose host definitions are not read.
bool OwnMacro(const char* name, size_t length)
{
	static const char* const kOwn[] = { "__cplusplus", "__STDC_HOSTED__", "__DATE__", "__TIME__", "__FILE__",
		"__LINE__", "__CPPGM__", "__CPPGM_AUTHOR__", "__STDC__" };
	for (const char* own : kOwn)
	{
		if (std::strlen(own) == length && std::memcmp(own, name, length) == 0)
			return true;
	}
	return false;
}

// The host macros announcing extension types whose values this
// implementation does not compute (__float80), whose limits are literals
// of a suffix it does not read (GCC's __BFLT16_* constants, BF16; Clang
// announces __bf16 by none) or whose constants its analysis does not
// evaluate (the binary16 and binary128 types: their limits are literals
// hosted headers use in constant expressions), or that it does not
// specialize hosted templates for (the other _FloatN types), which hosted
// headers would otherwise specialize templates for.
bool UnsupportedTypeMacro(const char* name, size_t length)
{
	static const char* const kPrefixes[] = { "__BFLT16_", "__FLT16_", "__FLT32_", "__FLT64_", "__FLT128_", "__FLT32X_",
		"__FLT64X_", "__FLT128X_" };
	for (const char* prefix : kPrefixes)
	{
		const size_t size = std::strlen(prefix);
		if (length >= size && std::memcmp(name, prefix, size) == 0)
			return true;
	}
	static const char* const kSizes[] = { "__SIZEOF_FLOAT128__", "__SIZEOF_FLOAT80__" };
	for (const char* size : kSizes)
	{
		if (std::strlen(size) == length && std::memcmp(size, name, length) == 0)
			return true;
	}
	return false;
}

bool Named(const char* name, size_t length, const char* spelling)
{
	return std::strlen(spelling) == length && std::memcmp(spelling, name, length) == 0;
}

// The host macros of a mode turned off: exceptions, RTTI.
bool ModeMacro(const char* name, size_t length, const FeatureModes& modes)
{
	if (!modes.exceptions && (Named(name, length, "__EXCEPTIONS") || Named(name, length, "__cpp_exceptions")))
		return true;
	return !modes.rtti && (Named(name, length, "__GXX_RTTI") || Named(name, length, "__cpp_rtti"));
}

// 16.8p4: an implementation's other predefined macro names begin with an
// underscore followed by an uppercase letter or a second underscore; the
// host dialect's others (gnu++11's `unix`, `linux`, `i386`) are the
// program's names.
bool ReservedMacro(const char* name, size_t length)
{
	return length >= 2 && name[0] == '_' && (name[1] == '_' || (name[1] >= 'A' && name[1] <= 'Z'));
}

// The host's `-dM -E` output for the dialect (GNU, or the strict ISO
// one's: __STRICT_ANSI__) without the definitions of unreserved,
// OwnMacro, UnsupportedTypeMacro and ModeMacro names, plus __STDC__, which
// GCC predefines without listing it. The host's are C++11's feature-test
// macros; under -std=c++14 (`cplusplus`) the C++14 features this
// implementation provides add theirs (SD-6): variable templates, which
// libstdc++'s C++14 <charconv> uses unconditionally.
std::string HostMacroText(const FeatureModes& modes, const std::string& cplusplus, bool strict)
{
	std::string text = "#define __STDC__ 1\n";
	if (cplusplus != "201103L")
		text += "#define __cpp_variable_templates 201304L\n";
	const char* line = strict ? cppgm_builtin_host_config::kHostStrictPredefinedMacros
							  : cppgm_builtin_host_config::kHostPredefinedMacros;
	while (*line != '\0')
	{
		const char* end = std::strchr(line, '\n');
		if (end == nullptr)
			end = line + std::strlen(line);
		static const char kDefine[] = "#define ";
		const size_t prefix = sizeof kDefine - 1;
		if (static_cast<size_t>(end - line) > prefix && std::memcmp(line, kDefine, prefix) == 0)
		{
			const char* name = line + prefix;
			const char* name_end = name;
			while (name_end < end && *name_end != ' ' && *name_end != '(')
				++name_end;
			const size_t length = static_cast<size_t>(name_end - name);
			if (ReservedMacro(name, length) && !OwnMacro(name, length) && !UnsupportedTypeMacro(name, length) &&
				!ModeMacro(name, length, modes))
			{
				text.append(line, end);
				text.push_back('\n');
			}
		}
		line = *end == '\n' ? end + 1 : end;
	}
	return text;
}

// GCC's builtin types: __builtin_va_list, as it declares it for the x86-64
// psABI (3.5.7), an array of one __va_list_tag (the library builtins whose
// types need it, __builtin_vsnprintf, are the registry's,
// lex/gnu_builtins.h; the ISO/IEC TS 18661-3 floating types glibc declares
// functions of are fundamental types, types/fundamental_type.h).
const char kBuiltinDeclarations[] =
	"struct __va_list_tag { unsigned int gp_offset; unsigned int fp_offset; void* overflow_arg_area; "
	"void* reg_save_area; };\n"
	"typedef __va_list_tag __builtin_va_list[1];\n"
	// Clang's builtin templates (GCC 14's too) as alias templates:
	// __make_integer_seq<S, T, N> is S<T, 0, ..., N-1> (by __integer_pack)
	// and __type_pack_element<I, T...> the I-th type of the pack.
	"template<template<class _Tp, _Tp...> class _Sp, class _Tp, _Tp _Np> using __make_integer_seq = "
	"_Sp<_Tp, __integer_pack(_Np)...>;\n"
	"template<unsigned long _Ip, class... _Tp> struct __cppgm_type_pack_element;\n"
	"template<class _Hp, class... _Tp> struct __cppgm_type_pack_element<0, _Hp, _Tp...> { typedef _Hp type; };\n"
	"template<unsigned long _Ip, class _Hp, class... _Tp> struct __cppgm_type_pack_element<_Ip, _Hp, _Tp...> "
	": __cppgm_type_pack_element<_Ip - 1, _Tp...> {};\n"
	"template<unsigned long _Ip, class... _Tp> using __type_pack_element = "
	"typename __cppgm_type_pack_element<_Ip, _Tp...>::type;\n";

} // namespace

void add_host_environment(PreprocessorOptions& options, bool nostdinc)
{
	options.hosted = true;
	options.host_predefines = HostMacroText(options.modes, options.cplusplus, options.strict);
	options.builtin_declarations = kBuiltinDeclarations;
	if (nostdinc)
		return;
	for (const char* const* path = cppgm_builtin_host_config::kStandardIncludePaths; *path != nullptr; ++path)
		options.system_include_paths.push_back(*path);
}

bool host_stdlib_selected(const std::string& name)
{
	// The build's flags, word by word: the last -stdlib= is its selection.
	const std::string flags = cppgm_builtin_host_config::kStdlibFlags;
	const std::string option = "-stdlib=";
	std::string selected;
	size_t begin = 0;
	while (begin < flags.size())
	{
		size_t end = flags.find(' ', begin);
		if (end == std::string::npos)
			end = flags.size();
		if (flags.compare(begin, option.size(), option) == 0 && end - begin >= option.size())
			selected = flags.substr(begin + option.size(), end - begin - option.size());
		begin = end + 1;
	}
	if (!selected.empty())
		return name == selected;
	// None: the host compiler's default library, the one whose headers the
	// build found (libc++'s directory is c++/v1, libstdc++'s c++/<version>).
	bool libcxx = false;
	bool libstdcxx = false;
	for (const char* const* path = cppgm_builtin_host_config::kStandardIncludePaths; *path != nullptr; ++path)
	{
		const std::string directory = *path;
		const size_t at = directory.find("/c++/");
		if (at == std::string::npos)
			continue;
		if (directory.compare(at, std::string::npos, "/c++/v1") == 0)
			libcxx = true;
		else
			libstdcxx = true;
	}
	return name == "libc++" ? libcxx : name == "libstdc++" && libstdcxx && !libcxx;
}

std::string host_query(const std::string& flag)
{
	if (flag == "--version" || flag == "-v")
		return std::string("cppgm++ (host configuration: ") + cppgm_builtin_host_config::kHostCxx + ")\n";
	if (flag == "-dumpmachine")
		return cppgm_builtin_host_config::kTarget;
	if (flag == "-dumpversion")
		return cppgm_builtin_host_config::kVersion;
	return cppgm_builtin_host_config::kSearchDirs;
}

} // namespace driver
