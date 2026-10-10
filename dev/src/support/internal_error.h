#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

// The exit status of a tool stopped by an internal compiler error (BSD
// sysexits' EX_SOFTWARE). A rejected program exits with EXIT_FAILURE, so the
// harness can tell a correct rejection from a compiler bug.
static constexpr int CPPGM_EXIT_INTERNAL_ERROR = 70;

// A broken invariant of the compiler itself: a state, or an intermediate
// form the compiler produced (LowIR, machine IR, object code and its
// fixups), that a later step cannot accept and that no input can cause.
// What the input can cause is reported by the phase's own error instead:
// an ill-formed program, unreadable or malformed input, an unsupported
// option, or an implementation limit (std::runtime_error and the typed
// front-end errors).
//
// The location, when the failing step knows one, is a translation-unit
// location (preprocess/source/source_files.h); a tool that still holds the
// translation unit renders it into the message (syntax::SyntaxUnit).
class InternalError : public std::logic_error
{
public:
	static constexpr uint32_t kNoLocation = 0xffffffffu;

	explicit InternalError(const char* message, uint32_t location = kNoLocation)
		: std::logic_error(message), location_(location)
	{}
	explicit InternalError(const std::string& message, uint32_t location = kNoLocation)
		: std::logic_error(message), location_(location)
	{}

	bool has_location() const { return location_ != kNoLocation; }
	uint32_t location() const { return location_; }

private:
	uint32_t location_;
};
