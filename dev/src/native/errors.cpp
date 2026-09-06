// The native backend's throw helpers.  They ship with the scaffold model,
// whose name helpers report an invalid identity through them.
#include "native/errors.h"
#include "support/exception_types.h"

#include <string>

namespace native_errors {

void ThrowInvocation(const std::string& message)
{
	throw InvocationError(message);
}

void ThrowInputOutput(const std::string& message)
{
	throw InputOutputError(message, CompilerErrorDomain::NATIVE);
}

void ThrowLowirInput(const char* message)
{
	throw SerializedInputError(SerializedInputFormat::LOWIR, message);
}

void ThrowLowirInput(const std::string& message)
{
	throw SerializedInputError(SerializedInputFormat::LOWIR, message);
}

void ThrowSource(const char* message)
{
	throw SourceError(message, CompilerErrorDomain::NATIVE);
}

void ThrowSource(const std::string& message)
{
	throw SourceError(message, CompilerErrorDomain::NATIVE);
}

void ThrowResourceLimit(const char* message)
{
	throw ResourceLimitError(message, CompilerErrorDomain::NATIVE);
}

void ThrowResourceLimit(const std::string& message)
{
	throw ResourceLimitError(message, CompilerErrorDomain::NATIVE);
}

void ThrowInternal(const char* message)
{
	throw InternalCompilerError(message, CompilerErrorDomain::NATIVE);
}

void ThrowInternal(const std::string& message)
{
	throw InternalCompilerError(message, CompilerErrorDomain::NATIVE);
}

}
