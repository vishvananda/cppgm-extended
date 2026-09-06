#pragma once

#include <iosfwd>
#include <string>

// A semantic error is much easier to act on with a source location, and there
// are some nine hundred throw sites -- threading a node through each one is not
// the shape of the problem.  Instead the analyzer publishes the syntax node it
// is currently working on and the throw helpers append its location.  The hook
// keeps `support` from depending on the syntax model.
typedef std::string (*SemanticErrorLocationHook)();
void SetSemanticErrorLocationHook(SemanticErrorLocationHook hook);

__attribute__((cold, noinline, noreturn))
void ThrowSemanticError(const char* message);
__attribute__((cold, noinline, noreturn))
void ThrowSemanticError(const std::string& message);
__attribute__((cold, noinline, noreturn))
void ThrowSyntaxError(const char* message);
__attribute__((cold, noinline, noreturn))
void ThrowSyntaxError(const std::string& message);
__attribute__((cold, noinline, noreturn))
void ThrowSyntaxResourceLimit(const char* message);
__attribute__((cold, noinline, noreturn))
void ThrowSyntaxInternal(const char* message);
__attribute__((cold, noinline, noreturn))
void ThrowGeneralResourceLimit(const char* message);
__attribute__((cold, noinline, noreturn))
void ThrowInternalCompilerError(const char* message);
__attribute__((cold, noinline, noreturn))
void ThrowInternalCompilerError(const std::string& message);
__attribute__((cold, noinline, noreturn))
void ThrowSemanticResourceLimit(const char* message);
__attribute__((cold, noinline, noreturn))
void ThrowSemanticResourceLimit(const std::string& message);
__attribute__((cold, noinline, noreturn))
void ThrowSemanticInputOutput(const char* message);
__attribute__((cold, noinline, noreturn))
void ThrowSemanticInputOutput(const std::string& message);
__attribute__((cold, noinline, noreturn))
void ThrowSemanticInternal(const char* message);
__attribute__((cold, noinline, noreturn))
void ThrowSemanticInternal(const std::string& message);
