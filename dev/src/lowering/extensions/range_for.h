#ifndef CPPGM_LOWERING_EXTENSIONS_RANGE_FOR_H
#define CPPGM_LOWERING_EXTENSIONS_RANGE_FOR_H

#include "lowering/support/sequences.h"
#include "lowering/ir/model.h"
#include "semantic/model/graph.h"

namespace cppgm
{
namespace lowering
{

using namespace semantic;
using namespace semantic;
using namespace lowering::ir;
using namespace lowering::support;

template <class Derived>
class RangeForLowering
{
protected:
	bool LowerScalarCallReferenceInitialization(const DumpNode& record,
		const NodeChildren& children, const Operand& retained_destination)
	{
		Derived& derived = static_cast<Derived&>(*this);
		if (!derived.IsReferenceType(record.type) || children.size() != 1)
			return false;
		// A reference bound to a scalar prvalue needs storage of its own: a
		// call marked for materialization, or any other non-class prvalue (a
		// literal, an arithmetic result, a conversion).
		const DumpNode& source = derived.arena_.nodes[children[0]];
		if (source.category != VALUE_PRVALUE ||
			derived.IsClassObjectType(source.type) ||
			derived.IsArrayType(source.type) ||
			derived.IsReferenceType(source.type))
			return false;
		if (source.kind == DUMP_CALL_EXPRESSION &&
			!source.reference_call_materialization &&
			derived.UsesIndirectClassResult(source.type, source.binding))
			return false;
		const LowType value_type = derived.LowerStorageType(source.type);
		const Operand temporary(derived.EnsureGeneratedSlot(
			children[0], "tmpref", value_type), value_type);
		Instruction retain(Instruction::STORE);
		retain.type = value_type;
		retain.first = derived.LowerInitializerConvertedValue(
			children[0], value_type);
		retain.second = temporary;
		derived.Emit(retain);
		Instruction bind(Instruction::STORE);
		bind.type = LowPtr();
		bind.first = derived.AddressOfStorage(temporary);
		bind.second = retained_destination.kind == Operand::NONE ?
			derived.StorageFor(record.binding, LowPtr()) : retained_destination;
		derived.Emit(bind);
		return true;
	}
};

}
}

#endif
