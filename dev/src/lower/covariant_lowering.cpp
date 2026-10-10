#include "lower/program_lowering.h"

#include "sema/analyzer.h"
#include "sema/conversions.h"

// Covariant results (10.3p7) in virtual tables: a slot whose final
// overrider returns a pointer or reference to a class derived from the one
// the slot's introducing function returns, at a nonzero offset or through
// a virtual base, holds a covariant thunk adjusting the result (Itanium
// 2.5.3; the analysis gives such an overrider a slot of its own too, see
// Analyzer::result_needs_adjustment). The thunk's name spells a virtual
// base's offset by its position in the Itanium table of the class
// returned (lower/itanium_layout.cpp), the row the thunk's body reads.
namespace lower {

namespace {

// `base` is a base class of `derived` (or `derived` itself).
bool HasBase(const sema::ClassEntity* derived, const sema::ClassEntity* base)
{
	if (derived == base)
		return true;
	for (uint32_t i = 0; i < sema::VirtualBaseCount(derived); ++i)
	{
		if (sema::VirtualBases(derived)[i].base->base == base)
			return true;
	}
	for (const sema::BaseClass* edge = derived->bases; edge != nullptr; edge = edge->next)
	{
		if (!edge->is_virtual && HasBase(static_cast<const sema::ClassEntity*>(edge->base), base))
			return true;
	}
	return false;
}

} // namespace

bool ProgramLowering::result_adjustment(const sema::FunctionEntity* overrider, const sema::FunctionEntity* introducer,
	ResultAdjustment& out, const sema::ClassEntity* through)
{
	const sema::Type* mine = overrider->type->canonical->element;
	const sema::Type* theirs = introducer->type->canonical->element;
	if (mine == theirs || !(mine->kind == sema::TypeKind::Pointer || mine->is_reference()))
		return false;
	const sema::ClassEntity* from = sema::ClassOf(mine->element);
	const sema::ClassEntity* to = sema::ClassOf(theirs->element);
	if (from == nullptr || to == nullptr || from == to)
		return false;
	// Through the table's class when the result class has it as a base and
	// it reaches the target class by non-virtual bases (as GCC converts a
	// covariant result in a repeated base's table).
	if (through != nullptr && through != from && through != to && from->complete && HasBase(from, through) &&
		HasBase(through, to))
	{
		size_of(from->type, 0);
		const BaseRoute first = base_route(from, through, 0);
		const BaseRoute second = base_route(through, to, 0);
		if (second.via == nullptr)
		{
			const int64_t offset = static_cast<int64_t>(first.offset + second.offset);
			if (first.via == nullptr && offset == 0)
				return false;
			out = ResultAdjustment{ from, first.via, offset, mine->is_reference() };
			return true;
		}
	}
	// Once per overrider and introducer (the group's tables, construction
	// tables and the definition's thunks ask again).
	const FunctionPair key = FunctionPair{ overrider, introducer };
	if (const KnownAdjustment* known = result_adjustments_.find(key))
	{
		if (known->needed)
			out = known->adjustment;
		return known->needed;
	}
	size_of(from->type, 0);
	const BaseRoute route = base_route(from, to, 0);
	const KnownAdjustment found =
		KnownAdjustment{ ResultAdjustment{ from, route.via, static_cast<int64_t>(route.offset), mine->is_reference() },
			route.via != nullptr || route.offset != 0 };
	result_adjustments_.insert(key, found);
	if (found.needed)
		out = found.adjustment;
	return found.needed;
}

size_t ProgramLowering::FunctionPairHash::operator()(const FunctionPair& key) const
{
	return (reinterpret_cast<uintptr_t>(key.first) * 0x9E3779B97F4A7C15ull) ^ reinterpret_cast<uintptr_t>(key.second);
}

} // namespace lower
