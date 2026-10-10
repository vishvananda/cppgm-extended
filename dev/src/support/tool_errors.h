#pragma once

#include <stdexcept>
#include <string>

// The errors a tool reports that no phase owns: what its command line, its
// files or the size of its input cause. A phase reports an ill-formed
// program or input by its own error (PreprocessError, ParseError,
// sema::SemanticError, lower::LoweringError, native::BackendError, ...),
// and a broken invariant of the compiler is an InternalError
// (support/internal_error.h). Each tool reports these as `ERROR: <message>`
// and exits with EXIT_FAILURE.

// The command line asks for what the tool does not do: an unknown,
// malformed or repeated option, a missing operand, options that conflict,
// or an input of a kind the mode does not take.
class InvocationError : public std::runtime_error
{
public:
	explicit InvocationError(const char* message) : std::runtime_error(message) {}
	explicit InvocationError(const std::string& message) : std::runtime_error(message) {}
};

// A file the command names cannot be opened, read or written.
class InputOutputError : public std::runtime_error
{
public:
	explicit InputOutputError(const char* message) : std::runtime_error(message) {}
	explicit InputOutputError(const std::string& message) : std::runtime_error(message) {}
};

// The input exceeds an implementation limit (C++11 Annex B): a source file
// or token text beyond 32-bit offsets, or more identifiers than a
// translation unit's table can number.
class LimitError : public std::runtime_error
{
public:
	explicit LimitError(const char* message) : std::runtime_error(message) {}
	explicit LimitError(const std::string& message) : std::runtime_error(message) {}
};
