#pragma once

#include <cstddef>
#include <string>

namespace cppgm
{
namespace lowering
{
namespace abi
{

std::string NormalizeSymbolName(const std::string& name);
// The symbol of the index-th temporary a static-storage variable's
// initializer extends (Itanium 5.1.4.5), from the variable's symbol; empty
// when the variable has no name a temporary can be named after.
std::string ReferenceTemporarySymbol(const std::string& variable_symbol,
	std::size_t index);

}  // namespace abi
}  // namespace lowering
}  // namespace cppgm
