#include "semantic/analysis/analyzer.h"
#include "support/exceptions.h"

#include <limits>
#include <sstream>

namespace cppgm
{
namespace semantic
{

void Analyzer::MaterializeStaticReferenceTemporary(BindingId binding,
	TypeId type, ExpressionInfo* initializer)
{
	const ScopeKind scope = program_->KindOfScope(program_->bindings[binding].owner);
	if (initializer->category != VALUE_PRVALUE || !initializer->constant ||
		program_->bindings[binding].thread_local_storage ||
		(scope != SCOPE_NAMESPACE && scope != SCOPE_CLASS &&
		 program_->bindings[binding].storage_class != STORAGE_CLASS_STATIC))
		return;
	const TypeId object_type = EffectiveType(type);
	// Base-subobject temporary lifetimes retain their complete-object recipe.
	if (!SimilarUnqualified(initializer->type, object_type)) return;
	std::ostringstream generated;
	generated << "__constexpr_reference_temporary__" << binding;
	const std::string generated_name = generated.str();
	if (stats_)
		RecordGeneratedIdentityRender(
			SEMANTIC_GENERATED_STATIC_REFERENCE_TEMPORARY, generated_name, 1);
	const NameId name = program_->names.Intern(generated_name);
	const BindingId storage = program_->AddUnindexedBinding(
		program_->GlobalScope(), BIND_VARIABLE, name, object_type, kNoBinding);
	SpecInfo spec;
	spec.storage_class = STORAGE_CLASS_STATIC;
	spec.is_constexpr = true;
	PublishVariableDeclarationFacts(storage, program_->GlobalScope(), name,
		object_type, spec, false);
	program_->bindings[storage].compiler_generated = true;
	PublishConstantVariableInitializer(storage, object_type, spec, *initializer);
	bool declaration_only = false;
	const std::uint32_t variable = MakeVariableDeclarationDump(
		object_type, name, storage, false, true, &declaration_only);
	PublishVariableInitializerActions(variable, storage, object_type,
		*initializer, true, false, false);
	dump_.Add(root_, variable);
	AddNamespaceObjectAction(variable, storage, object_type, initializer->node);
	namespace_objects_.back().constant_object = ExpressionObject(*initializer);
	const ConstexprAddressValue* address = ConstexprAddressAt(
		ExpressionAddress(*initializer));
	if (address && address->kind != CONSTEXPR_ADDRESS_LOCAL)
	{
		StaticAddressInitializer& value = namespace_objects_.back().constant_address;
		value.kind = address->kind;
		value.identity = address->identity;
		value.offset = address->offset;
	}
	ExpressionInfo referent;
	referent.node = MakeDump(DUMP_ID_EXPRESSION, object_type,
		VALUE_LVALUE, name, storage);
	referent.type = object_type;
	referent.category = VALUE_LVALUE;
	referent.binding = storage;
	SetExpressionBindingConstant(&referent, storage);
	LvalueAddress(&referent);
	*initializer = referent;
}

std::uint32_t Analyzer::InternConstexprAddress(
	const ConstexprAddressValue& address)
{
	std::unordered_map<ConstexprAddressValue, std::uint32_t,
		ConstexprAddressValueHash>::const_iterator found =
		constexpr_address_index_.find(address);
	if (found != constexpr_address_index_.end()) return found->second;
	if (constexpr_addresses_.size() >= kNoConstexprAddress)
		ThrowSemanticResourceLimit("too many constexpr address facts");
	const std::uint32_t result =
		static_cast<std::uint32_t>(constexpr_addresses_.size());
	constexpr_addresses_.push_back(address);
	constexpr_address_index_.insert(std::make_pair(address, result));
	return result;
}

const ConstexprAddressValue* Analyzer::ConstexprAddressAt(
	std::uint32_t address) const
{
	return address == kNoConstexprAddress ||
		address >= constexpr_addresses_.size() ? 0 :
		&constexpr_addresses_[address];
}

std::uint32_t Analyzer::NullConstexprAddress()
{
	return InternConstexprAddress(ConstexprAddressValue());
}

void Analyzer::SetExpressionAddress(ExpressionInfo* expression,
	std::uint32_t address) const
{
	if (!ConstexprAddressAt(address))
		ThrowInternalCompilerError("invalid constexpr address identity");
	const ConstexprAddressValue* value = ConstexprAddressAt(address);
	expression->constant = value->kind == CONSTEXPR_ADDRESS_NULL ||
		constant_expression_required_depth_ != 0 ||
		constexpr_evaluation_depth_ != 0;
	expression->floating_constant = false;
	expression->constexpr_object = kNoConstexprObject;
	expression->constexpr_complete_object = kNoConstexprObject;
	expression->constexpr_address = address;
}

void Analyzer::SetExpressionLvalueAddress(ExpressionInfo* expression,
	std::uint32_t address) const
{
	if (!ConstexprAddressAt(address))
		ThrowInternalCompilerError("invalid constexpr lvalue address identity");
	expression->constexpr_lvalue_address = address;
}

std::uint32_t Analyzer::ExpressionAddress(
	const ExpressionInfo& expression) const
{
	return ConstexprAddressAt(expression.constexpr_address) ?
		expression.constexpr_address : kNoConstexprAddress;
}

std::uint32_t Analyzer::BindingAddress(BindingId binding) const
{
	if (binding == kNoBinding || binding >= program_->bindings.size())
		return kNoConstexprAddress;
	BindingId owner = binding;
	if ((owner >= constexpr_address_by_binding_.size() ||
		constexpr_address_by_binding_[owner] == kNoConstexprAddress) &&
		program_->bindings[owner].canonical != owner)
		owner = program_->bindings[owner].canonical;
	if (owner >= constexpr_address_by_binding_.size())
		return kNoConstexprAddress;
	const std::uint32_t address = constexpr_address_by_binding_[owner];
	return ConstexprAddressAt(address) ? address : kNoConstexprAddress;
}

void Analyzer::PublishBindingAddress(BindingId binding,
	std::uint32_t address, bool constant)
{
	if (binding == kNoBinding || binding >= program_->bindings.size() ||
		!ConstexprAddressAt(address))
		ThrowInternalCompilerError("invalid constexpr address publication");
	program_->bindings[binding].constant =
		program_->bindings[binding].constant || constant;
	if (constexpr_address_by_binding_.size() <= binding)
		constexpr_address_by_binding_.resize(
			static_cast<std::size_t>(binding) + 1, kNoConstexprAddress);
	constexpr_address_by_binding_[binding] = address;
}

std::uint32_t Analyzer::LvalueAddress(ExpressionInfo* expression)
{
	if (constant_expression_required_depth_ == 0 &&
		constexpr_evaluation_depth_ == 0 && unevaluated_depth_ == 0 &&
		expression->binding != kNoBinding &&
		expression->binding < program_->bindings.size() &&
		program_->IsStaticDataMember(expression->binding))
		EnsureStaticMemberStorage(expression->binding, true);
	if (ConstexprAddressAt(expression->constexpr_lvalue_address))
		return expression->constexpr_lvalue_address;
	if (expression->constexpr_local < constexpr_locals_.size())
	{
		ConstexprLocalValue& local =
			constexpr_locals_[expression->constexpr_local];
		const TypeRecord top = program_->types.Get(local.type);
		if ((top.kind == TYPE_LVALUE_REFERENCE ||
			top.kind == TYPE_RVALUE_REFERENCE) &&
			ConstexprAddressAt(local.address))
		{
			SetExpressionLvalueAddress(expression, local.address);
			return local.address;
		}
		if (local.storage_identity == 0)
			local.storage_identity = next_constexpr_storage_identity_++;
		const std::int64_t extent = static_cast<std::int64_t>(
			program_->SizeOf(EffectiveType(local.type)));
		const std::uint32_t address = InternConstexprAddress(
			ConstexprAddressValue(CONSTEXPR_ADDRESS_LOCAL,
				local.storage_identity, 0, 0, extent));
		SetExpressionLvalueAddress(expression, address);
		return address;
	}
	const std::uint32_t object = ExpressionObject(*expression);
	const bool temporary_object = object != kNoConstexprObject &&
		(expression->binding == kNoBinding ||
		 (expression->binding < program_->bindings.size() &&
		  program_->bindings[expression->binding].kind == BIND_FUNCTION));
	if (temporary_object)
	{
		const std::int64_t extent = static_cast<std::int64_t>(
			program_->SizeOf(EffectiveType(expression->type)));
		const std::uint32_t address = InternConstexprAddress(
			ConstexprAddressValue(CONSTEXPR_ADDRESS_LOCAL,
				next_constexpr_storage_identity_++, 0, 0, extent));
		SetExpressionLvalueAddress(expression, address);
		return address;
	}
	if (expression->binding != kNoBinding &&
		expression->binding < program_->bindings.size())
	{
		const BindingRecord& binding =
			program_->bindings[expression->binding];
		if (program_->types.IsReference(binding.type))
		{
			const std::uint32_t referred = BindingAddress(expression->binding);
			if (referred != kNoConstexprAddress)
			{
				SetExpressionLvalueAddress(expression, referred);
				return referred;
			}
		}
		const BindingId canonical = binding.canonical;
		const TypeRecord storage = program_->types.Get(
			program_->types.RemoveTopCv(EffectiveType(binding.type)));
		const bool function_binding = binding.kind == BIND_FUNCTION;
		const bool function_storage = storage.kind == TYPE_FUNCTION;
		// A call keeps its callee binding for lowering, but the returned
		// object is not storage belonging to that function declaration.
		if (function_binding && program_->types.Get(program_->types.RemoveTopCv(
			EffectiveType(expression->type))).kind != TYPE_FUNCTION)
			return kNoConstexprAddress;
		if (!function_storage && storage.kind == TYPE_ARRAY &&
			storage.bound == 0)
			return kNoConstexprAddress;
		// A parameter can be a reference to a function. Its binding remains a
		// parameter identity, but the referred function has no object extent.
		const TypeId object_type = EffectiveType(binding.type);
		const std::int64_t extent = function_storage ||
			!IsMeasurableObjectType(object_type, false) ? 0 :
			static_cast<std::int64_t>(program_->SizeOf(object_type));
		const std::uint32_t address = InternConstexprAddress(
			ConstexprAddressValue(function_binding ? CONSTEXPR_ADDRESS_FUNCTION :
				CONSTEXPR_ADDRESS_BINDING, canonical, 0, 0, extent));
		SetExpressionLvalueAddress(expression, address);
		return address;
	}
	if (expression->string_unit_begin != kNoDumpEdge &&
		expression->string_unit_count != 0)
	{
		const TypeRecord array = program_->types.Get(
			program_->types.RemoveTopCv(expression->type));
		if (array.kind != TYPE_ARRAY) return kNoConstexprAddress;
		const std::int64_t extent = static_cast<std::int64_t>(
			expression->string_unit_count * program_->SizeOf(array.child));
		const std::uint32_t address = InternConstexprAddress(
			ConstexprAddressValue(CONSTEXPR_ADDRESS_STRING,
				dump_.nodes[expression->node].text, 0, 0, extent));
		SetExpressionLvalueAddress(expression, address);
		return address;
	}
	return kNoConstexprAddress;
}

std::uint32_t Analyzer::OffsetConstexprAddress(
	std::uint32_t address, std::int64_t byte_offset, bool narrow,
	std::int64_t extent)
{
	const ConstexprAddressValue* source = ConstexprAddressAt(address);
	if (!source) return kNoConstexprAddress;
	if (source->kind == CONSTEXPR_ADDRESS_NULL)
		return byte_offset == 0 ? address : kNoConstexprAddress;
	if ((byte_offset > 0 && source->offset >
		std::numeric_limits<std::int64_t>::max() - byte_offset) ||
		(byte_offset < 0 && source->offset <
		std::numeric_limits<std::int64_t>::min() - byte_offset))
		return kNoConstexprAddress;
	const std::int64_t offset = source->offset + byte_offset;
	if (offset < source->lower_bound || offset > source->upper_bound)
		return kNoConstexprAddress;
	std::int64_t lower = source->lower_bound;
	std::int64_t upper = source->upper_bound;
	if (narrow)
	{
		if (extent < 0 || offset >
			std::numeric_limits<std::int64_t>::max() - extent ||
			offset + extent > source->upper_bound)
			return kNoConstexprAddress;
		lower = offset;
		upper = offset + extent;
	}
	return InternConstexprAddress(ConstexprAddressValue(source->kind,
		source->identity, offset, lower, upper));
}

std::uint32_t Analyzer::ProjectConstexprBaseAddress(std::uint32_t address,
	const ExpressionInfo& source, const DumpNode& conversion)
{
	const ConstexprAddressValue* stored = ConstexprAddressAt(address);
	if (!stored) return kNoConstexprAddress;
	if (stored->kind == CONSTEXPR_ADDRESS_NULL) return address;
	const ConstexprAddressValue value = *stored;
	if (conversion.base_projection_offset > static_cast<std::uint64_t>(
		std::numeric_limits<std::int64_t>::max())) return kNoConstexprAddress;
	std::int64_t adjustment = static_cast<std::int64_t>(conversion.base_projection_offset);
	if (conversion.inverse_base_projection) adjustment = -adjustment;
	TypeId from = program_->types.RemoveTopCv(EffectiveType(source.type));
	const TypeRecord from_record = program_->types.Get(from);
	if (from_record.kind == TYPE_POINTER) from = from_record.child;
	const EntityId derived = EntityOf(from);
	if (!conversion.inverse_base_projection && derived != kNoEntity &&
		program_->entities[derived].virtual_base_count != 0)
	{
		// The conversion's layout is already complete for the binding's root object.
		if (value.kind == CONSTEXPR_ADDRESS_BINDING && value.offset == 0 &&
			value.identity < program_->bindings.size() &&
			program_->types.RemoveTopCv(program_->bindings[value.identity].type) ==
				program_->types.RemoveTopCv(from))
			return adjustment == 0 ? address : OffsetConstexprAddress(address, adjustment, false);
		TypeId to = program_->types.RemoveTopCv(EffectiveType(conversion.type));
		const TypeRecord to_record = program_->types.Get(to);
		if (to_record.kind == TYPE_POINTER) to = to_record.child;
		const EntityId base = EntityOf(to);
		if (!program_->HasVirtualBasePath(derived, base))
			return adjustment == 0 ? address : OffsetConstexprAddress(address, adjustment, false);
		// A virtual base's offset belongs to the complete object's layout.
		// Address identity can prove that layout without a constant object value.
		TypeId complete = kNoType;
		std::int64_t origin = 0;
		if (value.kind == CONSTEXPR_ADDRESS_BINDING && value.identity < program_->bindings.size())
		{
			complete = EffectiveType(program_->bindings[value.identity].type);
			while (program_->types.Get(program_->types.RemoveTopCv(complete)).kind == TYPE_ARRAY)
				complete = program_->types.Get(program_->types.RemoveTopCv(complete)).child;
			if (IsClassObjectType(complete) && program_->types.Get(program_->types.RemoveTopCv(
				EffectiveType(program_->bindings[value.identity].type))).kind == TYPE_ARRAY)
			{
				const std::uint64_t stride = program_->SizeOf(complete);
				if (stride == 0 || stride > static_cast<std::uint64_t>(
					std::numeric_limits<std::int64_t>::max()) || value.offset < 0) return kNoConstexprAddress;
				origin = value.offset - value.offset % static_cast<std::int64_t>(stride);
			}
		}
		const std::uint32_t object = ExpressionCompleteObject(source);
		if (object != kNoConstexprObject && object < constexpr_objects_.size())
		{
			complete = constexpr_objects_[object].type;
			origin = value.lower_bound;
		}
		if (source.binding != kNoBinding && source.binding < program_->bindings.size())
		{
			const BindingRecord& member = program_->bindings[source.binding];
			if (member.non_static_data_member && !program_->types.IsReference(member.type) &&
				EntityOf(member.type) == derived)
			{
				complete = member.type;
				origin = value.offset;
			}
		}
		const EntityId owner = complete == kNoType ? kNoEntity : EntityOf(complete);
		std::uint64_t offset = 0;
		bool ambiguous = false;
		if (owner == kNoEntity || !program_->IsBaseOf(derived, owner) ||
			!program_->QueryBasePath(owner, base, 0, 0, &offset, &ambiguous) || ambiguous ||
			offset > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) ||
			origin > std::numeric_limits<std::int64_t>::max() - static_cast<std::int64_t>(offset))
			return kNoConstexprAddress;
		adjustment = origin + static_cast<std::int64_t>(offset) - value.offset;
	}
	return adjustment == 0 ? address : OffsetConstexprAddress(address, adjustment, false);
}

bool Analyzer::ExpressionTruth(
	const ExpressionInfo& expression) const
{
	const TypeRecord type = program_->types.Get(program_->types.RemoveTopCv(
		EffectiveType(expression.type)));
	if (type.kind == TYPE_MEMBER_POINTER)
		return expression.binding != kNoBinding || expression.value != 0;
	const ConstexprAddressValue* address =
		ConstexprAddressAt(ExpressionAddress(expression));
	return address ? address->kind != CONSTEXPR_ADDRESS_NULL :
		ScalarTruth(ExpressionScalar(expression));
}

bool Analyzer::TryAnalyzeConstexprIndirectCall(ExpressionInfo* callee,
	ScopeId scope, const std::vector<NodeId>& argument_syntax,
	const std::vector<ExpressionInfo>& arguments, TypeId target,
	ExpressionInfo* result,
	const std::vector<CallConversionFact>* argument_conversions)
{
	if (unevaluated_depth_ != 0) return false;
	if (callee->indirect_constant_designator) return false;
	if (callee->binding != kNoBinding &&
		callee->binding < program_->bindings.size() &&
		program_->bindings[callee->binding].kind == BIND_FUNCTION &&
		!program_->bindings[callee->binding].static_member_function &&
		callee->node < dump_.nodes.size() &&
		dump_.nodes[callee->node].kind == DUMP_BINARY_EXPRESSION)
	{
		const DumpNode& application = dump_.nodes[callee->node];
		const std::string operation =
			program_->names.Get(application.text);
		const bool dot_star = operation.find(".*") != std::string::npos;
		const bool arrow_star = operation.find("->*") != std::string::npos;
		if ((dot_star || arrow_star) &&
			application.first_edge != kNoDumpEdge)
		{
			const std::uint32_t object_node =
				dump_.edges[application.first_edge].child;
			ExpressionInfo object;
			object.node = object_node;
			object.type = dump_.nodes[object_node].type;
			object.category = dump_.nodes[object_node].category;
			const std::uint32_t receiver_object = ExpressionObject(*callee);
			const std::uint32_t receiver_complete =
				ExpressionCompleteObject(*callee);
			const std::uint32_t receiver_address = ExpressionAddress(*callee);
			if (receiver_address != kNoConstexprAddress)
			{
				if (arrow_star) SetExpressionAddress(&object, receiver_address);
				else SetExpressionLvalueAddress(&object, receiver_address);
			}
			if (receiver_object != kNoConstexprObject &&
				receiver_complete != kNoConstexprObject)
				SetExpressionSubobject(
					&object, receiver_object, receiver_complete);
			if (dot_star) object = MakeImplicitObjectPointer(object);
			const EntityId owner =
				program_->bindings[callee->binding].member_owner;
			*result = BuildResolvedCall(callee->binding, scope,
				argument_syntax, arguments, &object, target, owner, 0,
				argument_conversions, true);
			return true;
		}
	}
	std::uint32_t callable_address = ExpressionAddress(*callee);
	if (callable_address == kNoConstexprAddress)
		callable_address = callee->constexpr_lvalue_address;
	if (callable_address == kNoConstexprAddress &&
		callee->binding != kNoBinding &&
		callee->binding < program_->bindings.size() &&
		program_->bindings[callee->binding].kind == BIND_FUNCTION &&
		program_->types.Get(program_->types.RemoveTopCv(
			EffectiveType(callee->type))).kind == TYPE_FUNCTION)
		callable_address = LvalueAddress(callee);
	const ConstexprAddressValue* address =
		ConstexprAddressAt(callable_address);
	if (!address || address->kind != CONSTEXPR_ADDRESS_FUNCTION ||
		address->offset != 0 || address->identity >= program_->bindings.size())
		return false;
	const BindingId function = static_cast<BindingId>(address->identity);
	if (program_->bindings[function].kind != BIND_FUNCTION) return false;
	*result = BuildResolvedCall(function, scope, argument_syntax,
		arguments, 0, target, kNoEntity, 0, argument_conversions);
	return true;
}

ConstexprFlow Analyzer::EvaluateConstexprReturn(NodeId expression,
	ScopeId scope, TypeId result_type, ConstexprScalarValue* result,
	bool* result_has_scalar, std::uint32_t* result_address,
	std::uint32_t* result_object,
	std::uint32_t* result_complete_object)
{
	ExpressionInfo value;
	if (!AnalyzeConstexprExpression(expression, scope, result_type, &value))
		return CONSTEXPR_FLOW_INVALID;
	const TypeRecord returned = program_->types.Get(result_type);
	const TypeId object_type = program_->types.RemoveTopCv(
		EffectiveType(result_type));
	const EntityId object_entity = EntityOf(object_type);
	const bool class_result = object_entity != kNoEntity &&
		(program_->entities[object_entity].flavor == NAMED_STRUCT ||
		 program_->entities[object_entity].flavor == NAMED_CLASS ||
		 program_->entities[object_entity].flavor == NAMED_UNION);
	if (returned.kind == TYPE_LVALUE_REFERENCE ||
		returned.kind == TYPE_RVALUE_REFERENCE)
	{
		const std::uint32_t address = LvalueAddress(&value);
		if (address == kNoConstexprAddress) return CONSTEXPR_FLOW_INVALID;
		const ConstexprAddressValue* returned_address =
			ConstexprAddressAt(address);
		if (returned_address->kind == CONSTEXPR_ADDRESS_LOCAL &&
			returned_address->identity >=
				constexpr_frames_.back().first_storage_identity)
			return CONSTEXPR_FLOW_INVALID;
		*result_address = address;
		*result_object = ExpressionObject(value);
		*result_complete_object = ExpressionCompleteObject(value);
		if (value.constant &&
			(IsIntegral(EffectiveType(value.type), true) ||
			 IsFloating(EffectiveType(value.type)) ||
			 IsMemberPointer(EffectiveType(value.type))))
		{
			*result = ExpressionScalar(value);
			*result_has_scalar = true;
		}
	}
	else if (IsPointer(EffectiveType(result_type)))
	{
		const std::uint32_t address = ExpressionAddress(value);
		if (address == kNoConstexprAddress) return CONSTEXPR_FLOW_INVALID;
		const ConstexprAddressValue* returned_address =
			ConstexprAddressAt(address);
		if (returned_address->kind == CONSTEXPR_ADDRESS_LOCAL &&
			returned_address->identity >=
				constexpr_frames_.back().first_storage_identity)
			return CONSTEXPR_FLOW_INVALID;
		*result_address = address;
		*result_object = ExpressionObject(value);
		*result_complete_object = ExpressionCompleteObject(value);
	}
	else if (class_result)
	{
		const std::uint32_t object = ExpressionObject(value);
		if (object == kNoConstexprObject) return CONSTEXPR_FLOW_INVALID;
		*result_object = object;
		*result_complete_object = ExpressionCompleteObject(value);
	}
	else
	{
		if (!value.constant ||
			(!IsIntegral(value.type, true) && !IsFloating(value.type) &&
			 !IsMemberPointer(value.type)))
			return CONSTEXPR_FLOW_INVALID;
		*result = ConvertScalarConstant(
			value.type, result_type, ExpressionScalar(value));
		*result_has_scalar = true;
	}
	return CONSTEXPR_FLOW_RETURN;
}

ExpressionInfo Analyzer::MaterializeConstexprAddress(
	std::uint32_t address_id, TypeId type)
{
	const ConstexprAddressValue* address = ConstexprAddressAt(address_id);
	if (!address) ThrowInternalCompilerError("invalid constexpr address materialization");
	if (address->offset != 0)
		ThrowSemanticError(
			"offset constexpr address materialization is unsupported");
	if (address->kind == CONSTEXPR_ADDRESS_NULL)
	{
		ExpressionInfo result = MakeLiteral(EffectiveType(type),
			program_->names.Intern("0"));
		result.integer_literal_zero = true;
		SetExpressionAddress(&result, address_id);
		RecordExpressionFacts(result);
		return result;
	}
	if (address->kind == CONSTEXPR_ADDRESS_STRING)
	{
		if (address->identity > std::numeric_limits<NameId>::max())
			ThrowInternalCompilerError("invalid constexpr string spelling identity");
		ExpressionInfo result = MakeStringLiteral(program_->names.Get(
			static_cast<NameId>(address->identity)));
		return ApplyTarget(result, type);
	}
	if ((address->kind != CONSTEXPR_ADDRESS_BINDING &&
		 address->kind != CONSTEXPR_ADDRESS_FUNCTION) ||
		address->identity >= program_->bindings.size())
		ThrowSemanticError(
			"transient constexpr address cannot escape evaluation");
	const BindingId binding = static_cast<BindingId>(address->identity);
	const BindingRecord& record = program_->bindings[binding];
	ExpressionInfo base;
	base.node = MakeDump(DUMP_ID_EXPRESSION, EffectiveType(record.type),
		VALUE_LVALUE, record.name, binding);
	base.type = EffectiveType(record.type);
	base.category = VALUE_LVALUE;
	base.binding = binding;
	SetExpressionLvalueAddress(&base, address_id);
	++expression_count_;
	const TypeRecord shape = program_->types.Get(
		program_->types.RemoveTopCv(base.type));
	if (program_->types.IsReference(type)) return ApplyTarget(base, type);
	if (shape.kind == TYPE_FUNCTION)
		return ApplyTarget(base, type);
	ExpressionInfo result;
	result.node = MakeDump(DUMP_UNARY_EXPRESSION, EffectiveType(type),
		VALUE_PRVALUE, program_->names.Intern("OP_AMP:&"));
	dump_.Add(result.node, base.node);
	result.type = EffectiveType(type);
	result.category = VALUE_PRVALUE;
	SetExpressionAddress(&result, address_id);
	++expression_count_;
	return result;
}

}
}
