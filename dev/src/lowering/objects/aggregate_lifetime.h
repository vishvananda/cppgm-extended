#ifndef CPPGM_LOWERING_LIFETIME_AGGREGATE_HELPERS_H
#define CPPGM_LOWERING_LIFETIME_AGGREGATE_HELPERS_H

#include "lowering/abi/symbol_names.h"
#include "lowering/support/errors.h"
#include "lowering/support/sequences.h"
#include "lowering/ir/model.h"
#include "semantic/model/graph.h"

#include <string>

namespace cppgm
{
namespace lowering
{

using namespace semantic;
using namespace semantic;
using namespace lowering::ir;
using namespace lowering::support;

template <class Derived>
class AggregateHelperLowering
{
protected:
	void RegisterAggregateHelpers()
	{
		Derived& derived = static_cast<Derived&>(*this);
		for (std::size_t i = 0; i < derived.graph_.aggregate_helpers.size(); ++i)
		{
			const AggregateHelperInfo& helper = derived.graph_.aggregate_helpers[i];
			if (helper.entity >= derived.program_.entities.size())
				ThrowLoweringInternal("aggregate helper has invalid entity");
			const EntityRecord& entity = derived.program_.entities[helper.entity];
			const TypeRecord& object = derived.program_.types.Get(
				derived.program_.types.RemoveTopCv(helper.object_type));
			if (object.kind != TYPE_NAMED || object.entity != helper.entity)
				ThrowLoweringInternal("aggregate helper has invalid object type");
			const std::string proposed = abi::NormalizeSymbolName(
				derived.program_.RenderEntityEmissionName(helper.entity)) +
				"__" + abi::NormalizeSymbolName(
				derived.program_.names.Get(entity.identity_name)) + "__aggregate";
			derived.aggregate_helper_symbols_[i] = derived.AddSyntheticSymbol(
				Symbol::FUNCTION_SYMBOL, proposed, std::string(), true);
			derived.output_.symbols[
				derived.aggregate_helper_symbols_[i]].nonthrowing = true;
		}
	}

	void EmitAggregateHelpers()
	{
		Derived& derived = static_cast<Derived&>(*this);
		for (std::size_t i = 0; i < derived.graph_.aggregate_helpers.size(); ++i)
		{
			const AggregateHelperInfo& helper = derived.graph_.aggregate_helpers[i];
			const TypeRecord& function_type =
				derived.program_.types.Get(helper.function_type);
			const TypeId* source_parameters =
				derived.program_.types.Parameters(helper.function_type);
			if (function_type.kind != TYPE_FUNCTION ||
				helper.parameter_member_count > helper.members.size() ||
				function_type.parameter_count !=
					helper.parameter_member_count + 1)
				ThrowLoweringInternal("aggregate helper has invalid function type");
			Function result;
			result.symbol = derived.aggregate_helper_symbols_[i];
			result.result = LowVoid();
			for (std::size_t p = 0; p < function_type.parameter_count; ++p)
			{
				Parameter parameter;
				parameter.by_address = p != 0 &&
					derived.UsesIndirectClassParameter(source_parameters[p]);
				parameter.type = parameter.by_address ? LowPtr() :
					derived.LowerType(source_parameters[p]);
				parameter.reference =
					derived.IsReferenceType(source_parameters[p]);
				if (derived.output_.retain_local_names)
					parameter.name = derived.output_.strings.intern(p == 0 ?
						std::string("this") : derived.program_.names.Get(
							derived.program_.bindings[helper.members[p - 1]].name));
				result.parameters.push_back(parameter);
				Slot slot;
				slot.name = parameter.name;
				slot.type = parameter.by_address ?
					derived.LowerStorageType(source_parameters[p]) :
					parameter.type;
				slot.parameter_origin = ParameterId(
					static_cast<std::uint32_t>(p));
				result.slots.push_back(slot);
			}
			derived.BeginSyntheticFunction(&result);
			for (std::size_t p = 0; p < result.parameters.size(); ++p)
			{
				if (result.parameters[p].by_address) continue;
				Instruction store(Instruction::STORE);
				store.type = result.parameters[p].type;
				store.first = Operand(static_cast<ParameterId>(p), store.type);
				store.second = Operand(static_cast<SlotId>(p), store.type);
				derived.Emit(store);
			}
			for (std::size_t m = 0; m < helper.members.size(); ++m)
			{
				const BindingId member = helper.members[m];
				if (m >= helper.parameter_member_count)
				{
					const Operand object = derived.LoadStorage(
						Operand(static_cast<SlotId>(0), LowPtr()), LowPtr());
					const Operand destination =
						derived.ProjectAggregateMember(object, member);
					if (derived.program_.bindings[member].bit_field)
						derived.InitializeBitField(member,
							Operand(0, derived.LowerExpressionType(
								derived.program_.bindings[member].type)),
							destination, derived.LowerExpressionType(
								derived.program_.bindings[member].type));
					else derived.EmitZeroInitialization(
						derived.program_.bindings[member].type, destination, member);
					continue;
				}
				const LowType value_type = result.parameters[m + 1].type;
				const bool array_member = derived.IsArrayType(
					derived.program_.bindings[member].type);
				Operand value;
				if (!array_member)
					value = derived.LoadStorage(
						Operand(static_cast<SlotId>(m + 1), value_type),
						value_type);
				const Operand object = derived.LoadStorage(
					Operand(static_cast<SlotId>(0), LowPtr()), LowPtr());
				const Operand destination =
					derived.ProjectAggregateMember(object, member);
				if (array_member)
				{
					const Operand source = derived.LoadStorage(
						Operand(static_cast<SlotId>(m + 1), value_type),
						value_type);
					derived.EmitClassObjectCopy(
						derived.program_.bindings[member].type,
						source, destination);
					continue;
				}
				if (derived.program_.bindings[member].bit_field)
					derived.InitializeBitField(member, value, destination,
						derived.LowerExpressionType(
							derived.program_.bindings[member].type));
				else
				{
					Instruction store(Instruction::STORE);
					store.type = value_type;
					store.volatile_access = derived.TypeIsVolatile(
						derived.program_.bindings[member].type);
					store.first = value;
					store.second = destination;
					derived.Emit(store);
				}
			}
			derived.Emit(Instruction(Instruction::RETURN_VOID));
			derived.EndSyntheticFunction(result);
			derived.output_.functions.push_back(result);
			derived.output_.symbols[
				derived.aggregate_helper_symbols_[i]].definition_emitted = true;
		}
	}

	void LowerAggregateConstructionAction(std::uint32_t node,
		const Operand& destination)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const DumpNode& action = derived.arena_.nodes[node];
		if (action.kind != DUMP_AGGREGATE_CONSTRUCTION_ACTION ||
			action.aggregate_helper >= derived.graph_.aggregate_helpers.size() ||
			action.aggregate_helper >= derived.aggregate_helper_symbols_.size())
			ThrowLoweringInternal("invalid aggregate helper action identity");
		const AggregateHelperInfo& helper =
			derived.graph_.aggregate_helpers[action.aggregate_helper];
		const TypeRecord& function_type =
			derived.program_.types.Get(helper.function_type);
		const TypeId* parameters =
			derived.program_.types.Parameters(helper.function_type);
		const NodeChildren children = derived.Children(node);
		if (function_type.kind != TYPE_FUNCTION ||
			function_type.parameter_count != children.size() + 1 ||
			children.size() != helper.parameter_member_count ||
			helper.parameter_member_count > helper.members.size())
			ThrowLoweringInternal("aggregate helper boundary mismatch");
		CallArguments arguments;
		CallArgumentFlags references;
		arguments.Push(destination);
		references.Push(0);
		for (std::size_t i = 0; i < children.size(); ++i)
		{
			const LowType expected = derived.LowerType(parameters[i + 1]);
			const bool reference =
				derived.IsReferenceType(parameters[i + 1]);
			const TypeId member_type = derived.program_.bindings[
				helper.members[i]].type;
			if (derived.IsArrayType(member_type))
			{
				const LowType array_type = derived.LowerStorageType(member_type);
				const Operand slot(derived.EnsureGeneratedSlot(
					children[i], "argarr", array_type), array_type);
				const Operand destination = derived.AddressOfStorage(slot);
				if (derived.arena_.nodes[children[i]].kind !=
					DUMP_BRACED_INIT_LIST)
					ThrowLoweringSource(
						"aggregate array argument requires braces");
				derived.LowerRuntimeArrayValues(
					member_type, children[i], destination, true);
				arguments.Push(destination);
			}
			else if (reference)
				arguments.Push(derived.AddressOfStorage(
					derived.LowerStorage(children[i])));
			else arguments.Push(expected.kind == LOW_OBJECT ?
				derived.Convert(derived.LowerValue(children[i]), expected) :
				derived.LowerInitializerConvertedValue(children[i], expected));
			references.Push(reference ? 1 : 0);
		}
		Instruction call = derived.DirectCallInstruction(
			derived.aggregate_helper_symbols_[action.aggregate_helper], LowVoid());
		derived.AttachCallArguments(&call, arguments, references);
		derived.Emit(call);
	}
};

}
}

#endif
