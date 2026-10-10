#pragma once

#include <cstdint>

// The language modes the hosted feature probes answer by (-fno-exceptions,
// -fno-rtti).
struct FeatureModes
{
	bool exceptions = true;
	bool rtti = true;
};

// Clang's `__has_feature ( identifier )` and `__has_extension`: whether
// this implementation provides the named language feature (`cxx_lambdas`,
// `cxx_exceptions` ...) or, by its trait-feature name (`is_pod`,
// `has_trivial_copy`, `underlying_type`), the type trait `__name` the parser
// recognizes (lex/gnu_builtins.h). `__name__` spells the same feature. The
// C++14 extensions this implementation accepts in C++11 mode (binary
// literals, variable templates) are both features and extensions, as the
// course reference reports them; features it lacks (modules, blocks) are
// neither.
bool IsProvidedFeature(const char* spelling, uint32_t length, const FeatureModes& modes);
