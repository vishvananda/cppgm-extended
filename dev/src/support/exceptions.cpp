#include "abi/itanium/abi_mangle_errors.h"
#include "compiler_object/errors.h"
#include "cy86/errors.h"
#include "lowering/support/errors.h"
#include "lowir/optimize/errors.h"
#include "native/errors.h"
#include "support/exception_types.h"
#include "support/driver_errors.h"

#include <stdexcept>
#include <type_traits>

static_assert(!std::is_base_of<SemanticError, HardSemanticError>::value,
	"hard semantic failures must bypass ordinary semantic recovery");
static_assert(!std::is_base_of<std::runtime_error, CompilerError>::value,
	"project failures must not enter legacy runtime-error recovery");
static_assert(!std::is_base_of<std::logic_error, CompilerError>::value,
	"project failures must not enter legacy logic-error recovery");
static_assert(!std::is_base_of<SyntaxError, ResourceLimitError>::value,
	"resource limits must not be syntax recovery");
static_assert(!std::is_base_of<SyntaxError, InternalCompilerError>::value,
	"internal failures must not be syntax recovery");

namespace
{

SemanticErrorLocationHook semantic_error_location_hook = 0;

// A message that already names its own location keeps it; a few diagnostics
// point at a more useful node than the one being analysed.
std::string WithSemanticErrorLocation(const std::string& message)
{
	if (semantic_error_location_hook == 0) return message;
	if (message.find(" at ") != std::string::npos) return message;
	const std::string location = semantic_error_location_hook();
	return location.empty() ? message : message + location;
}

}

void SetSemanticErrorLocationHook(SemanticErrorLocationHook hook)
{
	semantic_error_location_hook = hook;
}

void ThrowSemanticError(const char* message)
{
	throw SemanticError(WithSemanticErrorLocation(message));
}

void ThrowSemanticError(const std::string& message)
{
	throw SemanticError(WithSemanticErrorLocation(message));
}

void ThrowSyntaxError(const char* message)
{
	throw SyntaxError(message);
}

void ThrowSyntaxError(const std::string& message)
{
	throw SyntaxError(message);
}

void ThrowSyntaxResourceLimit(const char* message)
{
	throw ResourceLimitError(message, CompilerErrorDomain::SYNTAX);
}

void ThrowSyntaxInternal(const char* message)
{
	throw InternalCompilerError(message, CompilerErrorDomain::SYNTAX);
}

void ThrowGeneralResourceLimit(const char* message)
{
	throw ResourceLimitError(message);
}

void ThrowInternalCompilerError(const char* message)
{
	throw InternalCompilerError(message);
}

void ThrowInternalCompilerError(const std::string& message)
{
	throw InternalCompilerError(message);
}

void ThrowSemanticResourceLimit(const char* message)
{
	throw ResourceLimitError(message, CompilerErrorDomain::SEMANTIC);
}

void ThrowSemanticResourceLimit(const std::string& message)
{
	throw ResourceLimitError(message, CompilerErrorDomain::SEMANTIC);
}

void ThrowSemanticInputOutput(const char* message)
{
	throw InputOutputError(message, CompilerErrorDomain::SEMANTIC);
}

void ThrowSemanticInputOutput(const std::string& message)
{
	throw InputOutputError(message, CompilerErrorDomain::SEMANTIC);
}

void ThrowSemanticInternal(const char* message)
{
	throw InternalCompilerError(message, CompilerErrorDomain::SEMANTIC);
}

void ThrowSemanticInternal(const std::string& message)
{
	throw InternalCompilerError(message, CompilerErrorDomain::SEMANTIC);
}

namespace cppgm {
namespace driver_errors {

void ThrowInvocation(const char* message)
{
	throw InvocationError(message);
}

void ThrowInvocation(const std::string& message)
{
	throw InvocationError(message);
}

void ThrowInputOutput(const char* message)
{
	throw InputOutputError(message, CompilerErrorDomain::DRIVER);
}

void ThrowInputOutput(const std::string& message)
{
	throw InputOutputError(message, CompilerErrorDomain::DRIVER);
}

void ThrowInternal(const char* message)
{
	throw InternalCompilerError(message, CompilerErrorDomain::DRIVER);
}

void ThrowLexicalSource(const std::string& message)
{
	throw SourceError(message, CompilerErrorDomain::LEXICAL);
}

}
}

namespace abi_mangle {

void ThrowAbiFactInput(const char* message)
{
	throw SerializedInputError(SerializedInputFormat::ABI_FACT, message);
}

void ThrowAbiFactInput(const std::string& message)
{
	throw SerializedInputError(SerializedInputFormat::ABI_FACT, message);
}

void ThrowAbiInputOutput(const char* message)
{
	throw InputOutputError(message, CompilerErrorDomain::ABI);
}

void ThrowAbiResourceLimit(const char* message)
{
	throw ResourceLimitError(message, CompilerErrorDomain::ABI);
}

void ThrowAbiInternal(const char* message)
{
	throw InternalCompilerError(message, CompilerErrorDomain::ABI);
}

void ThrowAbiInternal(const std::string& message)
{
	throw InternalCompilerError(message, CompilerErrorDomain::ABI);
}

}

namespace cppgm {
namespace lowering {

void ThrowLoweringInvocation(const char* message)
{
	throw InvocationError(message);
}

void ThrowLoweringInputOutput(const char* message)
{
	throw InputOutputError(message, CompilerErrorDomain::LOWERING);
}

void ThrowLoweringSource(const char* message)
{
	throw SourceError(message, CompilerErrorDomain::LOWERING);
}

void ThrowLoweringSource(const std::string& message)
{
	throw SourceError(message, CompilerErrorDomain::LOWERING);
}

void ThrowLoweringResourceLimit(const char* message)
{
	throw ResourceLimitError(message, CompilerErrorDomain::LOWERING);
}

void ThrowLoweringInternal(const char* message)
{
	throw InternalCompilerError(message, CompilerErrorDomain::LOWERING);
}

void ThrowLoweringInternal(const std::string& message)
{
	throw InternalCompilerError(message, CompilerErrorDomain::LOWERING);
}

}
}

namespace lowir_opt {

void ThrowOptimizerInvocationError(const std::string& message)
{
	throw InvocationError(message);
}

void ThrowOptimizerInternalError(const char* message)
{
	throw InternalCompilerError(message, CompilerErrorDomain::OPTIMIZER);
}

}

namespace cppgm {
namespace compiler_object {

void ThrowCompilerObjectInputError(const char* message)
{
	throw SerializedInputError(SerializedInputFormat::COMPILER_OBJECT, message);
}

void ThrowCompilerObjectInputError(const std::string& message)
{
	throw SerializedInputError(SerializedInputFormat::COMPILER_OBJECT, message);
}

void ThrowCompilerObjectInputOutputError(const std::string& message)
{
	throw InputOutputError(message, CompilerErrorDomain::COMPILER_OBJECT);
}

void ThrowCompilerObjectResourceLimit(const char* message)
{
	throw ResourceLimitError(message, CompilerErrorDomain::COMPILER_OBJECT);
}

void ThrowCompilerObjectInternalError(const char* message)
{
	throw InternalCompilerError(message, CompilerErrorDomain::COMPILER_OBJECT);
}

}
}

namespace cppgm {
namespace cy86_errors {

void ThrowSource(const char* message)
{
	throw SourceError(message, CompilerErrorDomain::CY86);
}

void ThrowSource(const std::string& message)
{
	throw SourceError(message, CompilerErrorDomain::CY86);
}

void ThrowInputOutput(const std::string& message)
{
	throw InputOutputError(message, CompilerErrorDomain::CY86);
}

void ThrowResourceLimit(const char* message)
{
	throw ResourceLimitError(message, CompilerErrorDomain::CY86);
}

void ThrowInternal(const char* message)
{
	throw InternalCompilerError(message, CompilerErrorDomain::CY86);
}

}
}
