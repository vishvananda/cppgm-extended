// The exception classes themselves: constructors, the message accessor and
// the serialized-input domain map, kept apart from the stage-specific throw
// helpers so a scaffold that includes exception_types.h links them alone.
#include "support/exception_types.h"

#include <cstdint>
#include <string>

CompilerError::CompilerError(CompilerErrorDisposition disposition,
	CompilerErrorDomain domain, const std::string& message, std::uint16_t code)
	: message_(message), disposition_(disposition), domain_(domain), code_(code)
{}

CompilerError::~CompilerError() noexcept = default;

const char* CompilerError::what() const noexcept
{
	return message_.c_str();
}

InvocationError::InvocationError(const std::string& message,
	std::uint16_t code)
	: CompilerError(CompilerErrorDisposition::INVOCATION,
		CompilerErrorDomain::DRIVER, message, code)
{}

InputOutputError::InputOutputError(const std::string& message,
	CompilerErrorDomain domain, std::uint16_t code)
	: CompilerError(CompilerErrorDisposition::INPUT_OUTPUT,
		domain, message, code)
{}

SourceError::SourceError(const std::string& message,
	CompilerErrorDomain domain, std::uint16_t code)
	: CompilerError(CompilerErrorDisposition::SOURCE, domain, message, code)
{}

SyntaxError::SyntaxError(const std::string& message, std::uint16_t code)
	: CompilerError(CompilerErrorDisposition::SYNTAX,
		CompilerErrorDomain::SYNTAX, message, code)
{}

SemanticError::SemanticError(const std::string& message, std::uint16_t code)
	: CompilerError(CompilerErrorDisposition::SEMANTIC,
		CompilerErrorDomain::SEMANTIC, message, code)
{}

HardSemanticError::HardSemanticError(const std::string& message,
	std::uint16_t code)
	: CompilerError(CompilerErrorDisposition::HARD_SEMANTIC,
		CompilerErrorDomain::SEMANTIC, message, code)
{}

CompilerErrorDomain SerializedInputError::DomainFor(
	SerializedInputFormat format)
{
	switch (format)
	{
	case SerializedInputFormat::ABI_FACT:
		return CompilerErrorDomain::ABI;
	case SerializedInputFormat::LOWIR:
		return CompilerErrorDomain::LOWIR;
	case SerializedInputFormat::COMPILER_OBJECT:
		return CompilerErrorDomain::COMPILER_OBJECT;
	case SerializedInputFormat::CY86:
		return CompilerErrorDomain::CY86;
	case SerializedInputFormat::MIR:
		return CompilerErrorDomain::NATIVE;
	}
	return CompilerErrorDomain::GENERAL;
}

SerializedInputError::SerializedInputError(SerializedInputFormat format,
	const std::string& message, std::uint16_t code)
	: CompilerError(CompilerErrorDisposition::SERIALIZED_INPUT,
		DomainFor(format), message, code), format_(format)
{}

ResourceLimitError::ResourceLimitError(const std::string& message,
	CompilerErrorDomain domain, std::uint16_t code)
	: CompilerError(CompilerErrorDisposition::RESOURCE_LIMIT,
		domain, message, code)
{}

InternalCompilerError::InternalCompilerError(const std::string& message,
	CompilerErrorDomain domain, std::uint16_t code)
	: CompilerError(CompilerErrorDisposition::INTERNAL,
		domain, message, code)
{}

InternalCompilerError::InternalCompilerError(const char* message,
	CompilerErrorDomain domain, std::uint16_t code)
	: CompilerError(CompilerErrorDisposition::INTERNAL,
		domain, message, code)
{}
