#pragma once

#include "abi/itanium/abi_mangle.h"
#include "semantic/model/graph.h"
#include "semantic/model/program.h"

#include <cstdint>
#include <string>

namespace cppgm
{
namespace lowering
{
namespace abi
{

class LocalNames;

std::string MangleType(const semantic::Program& program, semantic::TypeId type,
	abi_mangle::AbiMangleStats* stats = 0,
	abi_mangle::AbiMangleContext* context = 0, LocalNames* names = 0);
std::string MangleFunction(const semantic::Program& program,
	const semantic::DumpNode& node,
	bool force_lifecycle_base_entry = false,
	abi_mangle::AbiMangleStats* stats = 0,
	abi_mangle::AbiMangleContext* context = 0, LocalNames* names = 0);
std::string MangleVariable(const semantic::Program& program,
	const semantic::DumpNode& node,
	abi_mangle::AbiMangleStats* stats = 0,
	abi_mangle::AbiMangleContext* context = 0, LocalNames* names = 0);
std::string MangleThreadLocalWrapper(const semantic::Program& program,
	semantic::BindingId binding, semantic::NameId fallback_name,
	abi_mangle::AbiMangleStats* stats = 0,
	abi_mangle::AbiMangleContext* context = 0, LocalNames* names = 0);
// A function's static variable, or its guard (`_ZZ3getvE5local`,
// `_ZGVZ3getvE5local`); `discriminator` is its ordinal among the function's
// entities of its name (0 for the first).
std::string MangleLocalStatic(const semantic::Program& program,
	semantic::BindingId object, semantic::BindingId function,
	std::uint32_t discriminator, bool guard,
	abi_mangle::AbiMangleStats* stats = 0,
	abi_mangle::AbiMangleContext* context = 0, LocalNames* names = 0);
bool HasWeakLinkage(const semantic::Program& program,
	semantic::BindingId binding, bool function);

}  // namespace abi
}  // namespace lowering
}  // namespace cppgm
