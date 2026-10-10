#pragma once

#include <stdexcept>
#include <string>

namespace native {

// LowIR the native back end cannot compile: an instruction, operand or
// exception-handling structure outside its supported subset, or a frame or
// object beyond the 32-bit displacements it encodes. Input can cause it
// (hand-written LowIR given to lowir2native or to `cppgm++ -c x.lowir`), so
// it is the program's error; a broken invariant of the back end is an
// InternalError instead.
class BackendError : public std::runtime_error
{
public:
	explicit BackendError(const char* message) : std::runtime_error(message) {}
	explicit BackendError(const std::string& message) : std::runtime_error(message) {}
};

} // namespace native
