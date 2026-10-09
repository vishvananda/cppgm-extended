// new-expressions and delete-expressions: the allocation function and
// deallocation function they select, array new with its cookie, and the
// construction and destruction actions each carries.  Split from
// analysis.cpp.
#include "semantic/analysis/analyzer.h"
#include "semantic/analysis/diagnostic_location.h"
#include "support/exceptions.h"
#include <algorithm>
#include <limits>

namespace cppgm
{
namespace semantic
{
namespace
{
bool IsClassEntity(const Program& program, EntityId entity)
{
	return entity != kNoEntity &&
		program.entities[entity].flavor != NAMED_ENUM;
}
}

BindingId Analyzer::SelectUsualDeallocation(ScopeId scope,
	EntityId entity, bool explicit_global, bool array, TypeId object_type)
{
	const bool class_object = IsClassEntity(*program_, entity);
	const char* spelling = array ? "operatordelete[]" : "operatordelete";
	const NameId name = program_->names.Intern(spelling);
	std::vector<BindingId> candidates;
	EntityId naming_class = kNoEntity;
	if (!explicit_global && class_object)
	{
		const LookupResult member = program_->LookupMember(entity,
			name, LOOKUP_ORDINARY);
		if (member.ordinary != kNoBinding &&
			program_->bindings[member.ordinary].kind == BIND_FUNCTION)
		{
			candidates = FunctionSet(member.ordinary);
			naming_class = member.naming_class;
		}
	}
	if (candidates.empty())
	{
		(void)EnsureBuiltinFunction(array ?
			BUILTIN_FUNCTION_OPERATOR_DELETE_ARRAY :
			BUILTIN_FUNCTION_OPERATOR_DELETE);
		candidates = FunctionCandidates(program_->GlobalScope(), name);
	}
	std::vector<BindingId> unsized;
	std::vector<BindingId> sized;
	const TypeId size_type =
		program_->types.Fundamental(FUND_UNSIGNED_LONG_INT);
	for (std::size_t i = 0; i < candidates.size(); ++i)
	{
		const TypeId function_type = GetFunction(candidates[i]).type;
		const TypeRecord& function = program_->types.Get(function_type);
		const TypeId* parameters = program_->types.Parameters(function_type);
		if (function.parameter_count == 1) unsized.push_back(candidates[i]);
		else if (function.parameter_count == 2 &&
			program_->types.RemoveTopCv(parameters[1]) == size_type)
			sized.push_back(candidates[i]);
	}
	std::vector<BindingId>& usual = unsized.empty() ? sized : unsized;
	if (usual.empty()) ThrowSemanticError("no usual deallocation function");
	ExpressionInfo pointer_argument;
	pointer_argument.type = program_->types.Pointer(
		program_->types.Fundamental(FUND_VOID));
	std::vector<NodeId> syntax(1, kNoNode);
	std::vector<ExpressionInfo> arguments(1, pointer_argument);
	if (unsized.empty())
	{
		ExpressionInfo size = MakeLiteral(size_type,
			InternNumber(static_cast<std::int64_t>(program_->SizeOf(object_type))));
		size.constant = true;
		size.value = static_cast<std::int64_t>(program_->SizeOf(object_type));
		dump_.nodes[size.node].constant = true;
		dump_.nodes[size.node].constant_value = size.value;
		syntax.push_back(kNoNode);
		arguments.push_back(size);
	}
	const BindingId selected = SelectOverload(scope, syntax, arguments,
		usual, 0, 0, 0);
	if (!CanAccessMember(selected, naming_class, entity))
		ThrowSemanticError("inaccessible deallocation function");
	DemandFunction(selected);
	return selected;
}

// Itanium C++ ABI 2.7: array new stores the element count in a cookie
// before the array unless the element type's destructor is trivial and its
// usual array deallocation function is not the two-argument (size) form;
// delete[] finds the count by the same rule. The global usual array
// deallocation function is always the one-argument form.
bool Analyzer::ArrayNeedsCookie(EntityId entity, bool explicit_global)
{
	if (!IsClassEntity(*program_, entity)) return false;
	if (!program_->entities[entity].trivial_destructor) return true;
	if (explicit_global) return false;
	const LookupResult member = program_->LookupMember(entity,
		program_->names.Intern("operatordelete[]"), LOOKUP_ORDINARY);
	if (member.ordinary == kNoBinding ||
		program_->bindings[member.ordinary].kind != BIND_FUNCTION)
		return false;
	const std::vector<BindingId> candidates = FunctionSet(member.ordinary);
	const TypeId size_type =
		program_->types.Fundamental(FUND_UNSIGNED_LONG_INT);
	bool sized = false;
	for (std::size_t i = 0; i < candidates.size(); ++i)
	{
		const TypeId function_type = GetFunction(candidates[i]).type;
		const TypeRecord& function = program_->types.Get(function_type);
		if (function.parameter_count == 1) return false;
		sized = sized || (function.parameter_count == 2 &&
			program_->types.RemoveTopCv(program_->types.Parameters(
				function_type)[1]) == size_type);
	}
	return sized;
}

// N3485 5.3.4p7: a runtime array new count that is negative, larger than
// `maximum` (the largest whose request fits the implementation limit) or
// smaller than `minimum` (the initializer-clauses of a braced-init-list)
// obtains no storage and throws std::bad_array_new_length. The count is
// converted to std::size_t through a checked conversion unless its type
// cannot hold such a value.
ExpressionInfo Analyzer::CheckArrayExtent(const ExpressionInfo& extent,
	std::uint64_t maximum, std::uint64_t minimum)
{
	const TypeId type = program_->types.RemoveTopCv(EffectiveType(extent.type));
	const TypeRecord& record = program_->types.Get(type);
	const bool boolean = record.kind == TYPE_FUNDAMENTAL &&
		record.fundamental == FUND_BOOL;
	const std::uint64_t bits = record.kind == TYPE_BITINT ? record.bound :
		static_cast<std::uint64_t>(program_->SizeOf(type)) * 8;
	if (minimum == 0 && (boolean || (IsUnsignedIntegral(type) && bits < 64 &&
		(std::uint64_t(1) << bits) - 1 <= maximum))) return extent;
	if (minimum > maximum || minimum > std::numeric_limits<std::uint32_t>::max())
		ThrowSemanticResourceLimit("array new initializer is too large");
	ExpressionInfo checked;
	checked.type = program_->types.Fundamental(FUND_UNSIGNED_LONG_INT);
	checked.category = VALUE_PRVALUE;
	checked.node = MakeDump(DUMP_CAST_EXPRESSION, checked.type, VALUE_PRVALUE);
	dump_.Add(checked.node, extent.node);
	dump_.nodes[checked.node].array_extent_check = true;
	dump_.nodes[checked.node].array_count = maximum;
	dump_.nodes[checked.node].array_extent_minimum =
		static_cast<std::uint32_t>(minimum);
	++expression_count_;
	return checked;
}

// The braced-init-list of an array new-expression (8.5.4p3, 8.5.1p7,
// 5.3.4p17): the first elements are copy-initialized from its clauses and
// the rest from an empty initializer list, which value-initializes them.
// An empty list is value-initialization, returned as a typed result
// without a node.
ExpressionInfo Analyzer::AnalyzeArrayNewList(NodeId node, ScopeId scope,
	TypeId element)
{
	ExpressionInfo list;
	list.node = kNoDumpEdge;
	const NodeId initializer = FindChild(node, ::cppgm::syntax::STAG_INITIALIZER);
	if (initializer == kNoNode) return list;
	const NodeId braced = FirstSemanticChild(initializer);
	if (braced == kNoNode || !arena_->IsTag(braced, ::cppgm::syntax::STAG_BRACED_INIT_LIST))
		ThrowSemanticError("array new initializer is not a braced-init-list");
	if (FirstSemanticChild(braced) == kNoNode)
	{
		list.type = element;
		return list;
	}
	list = AnalyzeBracedInit(braced, scope, program_->types.Array(element, 0));
	const TypeRecord& shape = program_->types.Get(list.type);
	if (dump_.nodes[list.node].kind != DUMP_BRACED_INIT_LIST ||
		shape.kind != TYPE_ARRAY || shape.IsIncompleteArray())
		ThrowInternalCompilerError("array new initializer has no element plan");
	return list;
}

ExpressionInfo Analyzer::AnalyzeArrayNewExpression(NodeId node,
	NodeId type_node, ScopeId scope, TypeId target)
{
	const NodeId specifiers = FindChild(type_node, ::cppgm::syntax::STAG_TYPE_SPECIFIER_SEQ);
	const SpecInfo spec = BuildSpecifiers(
		specifiers, scope, std::string(), false);
	const NodeId declarator = FindChild(type_node, ::cppgm::syntax::STAG_ABSTRACT_DECLARATOR);
	if (declarator == kNoNode)
		ThrowInternalCompilerError("array new has no abstract declarator");
	TypeId leaf_type = spec.type;
	std::vector<NodeId> suffixes;
	bool value_initialization = false;
	for (std::uint32_t edge = arena_->FirstEdge(declarator); edge != kNoEdge;
		edge = arena_->NextEdge(edge))
	{
		const NodeId child = arena_->EdgeChild(edge);
		if (arena_->IsTag(child, ::cppgm::syntax::STAG_ARRAY_SUFFIX)) suffixes.push_back(child);
		else if (arena_->IsTag(child, ::cppgm::syntax::STAG_PARAMETER_CLAUSE))
		{
			if (FirstSemanticChild(child) != kNoNode)
				ThrowSemanticError("array new initializer has parameters");
			value_initialization = true;
		}
		else if (arena_->IsTag(child, ::cppgm::syntax::STAG_PTR_OPERATOR))
		{
			if (PayloadSource(child) != "*")
				ThrowSemanticError("unsupported array new declarator");
			leaf_type = program_->types.Pointer(leaf_type);
		}
		else if (arena_->IsTag(child, ::cppgm::syntax::STAG_CV_QUALIFIER))
			leaf_type = program_->types.Qualify(leaf_type,
				PayloadSource(child) == "const" ? CV_CONST : CV_VOLATILE);
	}
	if (suffixes.empty())
		ThrowInternalCompilerError("array new has no array suffix");
	TypeId result_element_type = leaf_type;
	for (std::size_t i = suffixes.size(); i != 1; --i)
	{
		const NodeId bound_node = FirstSemanticChild(suffixes[i - 1]);
		const ExpressionInfo bound = AnalyzeExpression(bound_node, scope);
		if (!bound.constant || bound.value <= 0)
			ThrowSemanticError("invalid inner array bound");
		result_element_type = program_->types.Array(result_element_type,
			static_cast<std::uint64_t>(bound.value));
	}
	const NodeId extent_syntax = FirstSemanticChild(suffixes[0]);
	if (extent_syntax == kNoNode)
		ThrowSemanticError("array new has no extent");
	ExpressionInfo extent = AnalyzeExpression(extent_syntax, scope);
	if (!IsIntegral(extent.type) || (extent.constant && extent.value < 0))
		ThrowSemanticError("invalid array new extent");
	const ExpressionInfo list = AnalyzeArrayNewList(node, scope, result_element_type);
	value_initialization = value_initialization ||
		(list.node == kNoDumpEdge && list.type != kNoType);
	const std::uint64_t listed = list.node == kNoDumpEdge ? 0 :
		program_->types.Get(list.type).bound;
	const EntityId entity = EntityOf(leaf_type);
	const bool class_elements = IsClassEntity(*program_, entity);
	const bool explicit_global = FindChild(node, ::cppgm::syntax::STAG_GLOBAL_SCOPE) != kNoNode;
	if (class_elements) EnsureClassDefinition(leaf_type);
	const std::uint64_t cookie_size = ArrayNeedsCookie(entity, explicit_global) ?
		program_->ArrayCookieSize(leaf_type) : 0;
	const std::uint64_t row_size = program_->SizeOf(result_element_type);
	const std::uint64_t leaf_size = program_->SizeOf(leaf_type);
	if (leaf_size == 0 || row_size % leaf_size != 0)
		ThrowInternalCompilerError("invalid array element stride");
	// 5.3.4p10: the request is a std::size_t; a runtime count widens by its
	// promoted type's signedness before it is scaled and the cookie added.
	const TypeId size_type = program_->types.Fundamental(FUND_UNSIGNED_LONG_INT);
	ExpressionInfo allocation_size = extent;
	std::uint64_t flat_count = 0;
	if (extent.constant)
	{
		const std::uint64_t count = static_cast<std::uint64_t>(extent.value);
		if (count < listed)
			ThrowSemanticError("array new count is smaller than its initializer list");
		if (count > (std::numeric_limits<std::uint64_t>::max() - cookie_size) /
			row_size)
			ThrowSemanticResourceLimit("array allocation size overflow");
		const std::uint64_t bytes = count * row_size + cookie_size;
		const std::uint64_t inner_count = row_size / leaf_size;
		if (bytes > static_cast<std::uint64_t>(
			std::numeric_limits<std::int64_t>::max()) ||
			count > std::numeric_limits<std::uint64_t>::max() / inner_count)
			ThrowSemanticResourceLimit("array allocation exceeds PA12 limits");
		flat_count = count * inner_count;
		allocation_size = MakeLiteral(size_type,
			InternNumber(static_cast<std::int64_t>(bytes)));
		allocation_size.constant = true;
		allocation_size.value = static_cast<std::int64_t>(bytes);
		dump_.nodes[allocation_size.node].constant = true;
		dump_.nodes[allocation_size.node].constant_value = allocation_size.value;
	}
	else
	{
		const auto combine = [this, size_type](const ExpressionInfo& left,
			std::uint64_t right_value, const char* operation) -> ExpressionInfo
		{
			ExpressionInfo right = MakeLiteral(size_type,
				InternNumber(static_cast<std::int64_t>(right_value)));
			right.constant = true;
			right.value = static_cast<std::int64_t>(right_value);
			dump_.nodes[right.node].constant = true;
			dump_.nodes[right.node].constant_value = right.value;
			const TypeId arithmetic = size_type;
			const ExpressionInfo converted_left = ApplyTarget(left, arithmetic);
			const ExpressionInfo converted_right = ApplyTarget(right, arithmetic);
			const std::uint32_t expression = MakeDump(DUMP_BINARY_EXPRESSION,
				arithmetic, VALUE_PRVALUE, program_->names.Intern(operation));
			dump_.nodes[expression].operand_type = arithmetic;
			dump_.Add(expression, converted_left.node);
			dump_.Add(expression, converted_right.node);
			ExpressionInfo result;
			result.node = expression;
			result.type = arithmetic;
			result.category = VALUE_PRVALUE;
			++expression_count_;
			return result;
		};
		// The request is limited, as for a constant count, to PTRDIFF_MAX.
		allocation_size = CheckArrayExtent(extent,
			(static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) -
			 cookie_size) / row_size, listed);
		if (row_size != 1)
			allocation_size = combine(allocation_size, row_size, "*");
		if (cookie_size != 0)
			allocation_size = combine(allocation_size, cookie_size, "+");
		if (row_size == 1 && cookie_size == 0)
			allocation_size = ApplyTarget(allocation_size, size_type);
	}
	std::vector<NodeId> argument_syntax(1, kNoNode);
	std::vector<ExpressionInfo> arguments(1, allocation_size);
	std::vector<BindingId> candidates;
	EntityId naming_class = kNoEntity;
	if (!explicit_global && class_elements)
	{
		const LookupResult member = program_->LookupMember(entity,
			program_->names.Intern("operatornew[]"), LOOKUP_ORDINARY);
		if (member.ordinary != kNoBinding &&
			program_->bindings[member.ordinary].kind == BIND_FUNCTION)
		{
			candidates = FunctionSet(member.ordinary);
			naming_class = member.naming_class;
		}
	}
	if (candidates.empty())
	{
		(void)EnsureBuiltinFunction(BUILTIN_FUNCTION_OPERATOR_NEW_ARRAY);
		candidates = FunctionCandidates(
			program_->GlobalScope(), "operatornew[]");
	}
	std::vector<CallConversionFact> conversions;
	const BindingId selected = SelectOverload(scope, argument_syntax,
		arguments, candidates, 0, 0, &conversions);
	const ExpressionInfo allocation = BuildResolvedCall(selected, scope,
		argument_syntax, arguments, 0, kNoType, naming_class, 0, &conversions);
	std::uint32_t construction = kNoDumpEdge;
	BindingId destructor = kNoBinding;
	// Elements past a braced-init-list are value-initialized; a constant
	// count the list fills leaves none to default-construct.
	const bool listed_all = listed != 0 && extent.constant &&
		static_cast<std::uint64_t>(extent.value) == listed;
	if (class_elements && !listed_all)
	{
		const std::vector<NodeId> no_arguments;
		const std::uint32_t action = BuildConstructorAction(
			leaf_type, scope, no_arguments, false, false, false, false);
		const FunctionInfo& constructor = GetFunction(dump_.nodes[action].binding);
		std::vector<BindingId> empty_base_entries;
		const bool empty = EmptyDefaultConstructorChain(
			dump_.nodes[action].binding, &empty_base_entries) &&
			empty_base_entries.empty();
		if (!empty && !dump_.nodes[action].elide_empty_constructor &&
			!(constructor.implicit_constructor &&
			 program_->entities[entity].trivial_default_constructor))
		{
			construction = action;
			if (!dump_.nodes[action].trivial_special_member_action)
				DemandFunction(dump_.nodes[action].binding);
		}
	}
	if (class_elements)
	{
		const BindingId selected_destructor = DestructorForType(leaf_type);
		if (selected_destructor == kNoBinding ||
			GetFunction(selected_destructor).deleted_destructor)
			ThrowSemanticError("array element is not destructible");
		if (!CanAccessMember(selected_destructor, entity, entity))
			ThrowSemanticError("inaccessible array-new destructor");
		if (!program_->entities[entity].trivial_destructor)
		{
			destructor = selected_destructor;
			DemandFunction(destructor);
		}
	}
	const bool throwing_list = list.node != kNoDumpEdge &&
		!InitializationActionsAreNonthrowing(list.node);
	const BindingId cleanup = construction == kNoDumpEdge && !throwing_list ?
		kNoBinding : SelectUsualDeallocation(
			scope, entity, explicit_global, true, leaf_type);
	const TypeId result_type = program_->types.Pointer(result_element_type);
	const std::uint32_t result_node = MakeDump(DUMP_NEW_EXPRESSION,
		result_type, VALUE_PRVALUE, 0, selected);
	DumpNode& result_record = dump_.nodes[result_node];
	result_record.operand_type = leaf_type;
	result_record.array_action = true;
	result_record.array_cookie = cookie_size != 0;
	result_record.value_initialization = value_initialization;
	result_record.selected_binding = destructor;
	result_record.object_binding = cleanup;
	result_record.array_count_constant = extent.constant;
	result_record.array_count = flat_count;
	dump_.Add(result_node, allocation.node);
	if (construction != kNoDumpEdge) dump_.Add(result_node, construction);
	if (list.node != kNoDumpEdge) dump_.Add(result_node, list.node);
	ExpressionInfo result;
	result.node = result_node;
	result.type = result_type;
	result.category = VALUE_PRVALUE;
	++expression_count_;
	return ApplyTarget(result, target);
}

ExpressionInfo Analyzer::AnalyzeNewExpression(NodeId node,
	ScopeId scope, TypeId target)
{
	const NodeId type_node = FindChild(node, ::cppgm::syntax::STAG_TYPE_ID);
	if (type_node == kNoNode)
		ThrowSemanticError("new-expression has no allocated type");
	const NodeId new_declarator = FindChild(type_node, ::cppgm::syntax::STAG_ABSTRACT_DECLARATOR);
	if (new_declarator != kNoNode &&
		FindChild(new_declarator, ::cppgm::syntax::STAG_ARRAY_SUFFIX) != kNoNode)
		return AnalyzeArrayNewExpression(node, type_node, scope, target);
	TypeId object_type = BuildTypeId(type_node, scope);
	bool parsed_empty_initializer = false;
	const TypeRecord parsed_object = program_->types.Get(
		program_->types.RemoveTopCv(object_type));
	if (parsed_object.kind == TYPE_FUNCTION &&
		parsed_object.parameter_count == 0)
	{
		object_type = parsed_object.child;
		parsed_empty_initializer = true;
	}
	if (program_->types.Get(program_->types.RemoveTopCv(object_type)).kind ==
		TYPE_ARRAY)
		ThrowSemanticError("array new is outside PA11");
	// A synthetic template-parameter shape is not a complete object type, and
	// its allocation validity cannot be decided while materializing a partial
	// specialization.  Let the candidate become a non-deduced shape so PA18's
	// concrete replay can check the retained new-expression after deduction.
	if (CandidateSubstitutionActive() &&
		FunctionTemplateTypeIsDependent(object_type))
		return CandidateSubstitutionFailure();
	if (!IsMeasurableObjectType(object_type, false))
		return CandidateExpressionFailure(
			"new-expression requires a complete object type");
	const EntityId object_entity = EntityOf(object_type);
	if (object_entity != kNoEntity &&
		program_->entities[object_entity].abstract_class)
		return CandidateExpressionFailure(
			"cannot allocate an abstract class object");
	const NodeId placement = FindChild(node, ::cppgm::syntax::STAG_PLACEMENT);
	const NodeId placement_arguments = placement == kNoNode ? kNoNode :
		FindChild(placement, ::cppgm::syntax::STAG_PAREN_ARGUMENT_LIST);
	std::vector<NodeId> argument_syntax;
	std::vector<ExpressionInfo> arguments;
	ExpressionInfo size = MakeLiteral(program_->types.Fundamental(FUND_INT),
		InternNumber(static_cast<std::int64_t>(program_->SizeOf(object_type))));
	size.constant = true;
	size.value = static_cast<std::int64_t>(program_->SizeOf(object_type));
	dump_.nodes[size.node].constant = true;
	dump_.nodes[size.node].constant_value = size.value;
	argument_syntax.push_back(kNoNode);
	arguments.push_back(size);
	if (placement_arguments != kNoNode)
		for (std::uint32_t edge = arena_->FirstEdge(placement_arguments);
			edge != kNoEdge; edge = arena_->NextEdge(edge))
		{
			const NodeId argument = arena_->EdgeChild(edge);
			argument_syntax.push_back(argument);
			arguments.push_back(AnalyzeExpression(argument, scope));
		}
	const EntityId entity = EntityOf(object_type);
	const bool explicit_global = FindChild(node, ::cppgm::syntax::STAG_GLOBAL_SCOPE) != kNoNode;
	const NameId operator_new = program_->names.Intern("operatornew");
	std::vector<BindingId> candidates;
	EntityId naming_class = kNoEntity;
	if (!explicit_global && IsClassEntity(*program_, entity))
	{
		const LookupResult member = program_->LookupMember(entity,
			operator_new, LOOKUP_ORDINARY);
		if (member.ordinary != kNoBinding &&
			program_->bindings[member.ordinary].kind == BIND_FUNCTION)
		{
			candidates = FunctionSet(member.ordinary);
			naming_class = member.naming_class;
		}
	}
	if (candidates.empty())
	{
		(void)EnsureBuiltinFunction(BUILTIN_FUNCTION_OPERATOR_NEW);
		candidates = FunctionCandidates(program_->GlobalScope(), operator_new);
	}
	if (candidates.empty())
		ThrowSemanticError("operator new was not declared");
	std::vector<CallConversionFact> argument_conversions;
	const BindingId selected = SelectOverload(scope, argument_syntax,
		arguments, candidates, 0, 0, &argument_conversions);
	ExpressionInfo allocation = BuildResolvedCall(selected, scope,
		argument_syntax, arguments, 0, kNoType, naming_class, 0,
		&argument_conversions);
	const NodeId initializer_node = FindChild(node, ::cppgm::syntax::STAG_INITIALIZER);
	NodeId initializer = initializer_node == kNoNode ? kNoNode :
		FirstSemanticChild(initializer_node);
	std::uint32_t construction = kNoDumpEdge;
	if (IsClassEntity(*program_, entity))
	{
		if (initializer != kNoNode &&
			arena_->IsTag(initializer, ::cppgm::syntax::STAG_BRACED_INIT_LIST) &&
			program_->entities[entity].is_aggregate)
		{
			const ExpressionInfo aggregate = AnalyzeBracedInit(
				initializer, scope, object_type);
			construction = BuildAggregateConstructionAction(
				object_type, aggregate.node);
		}
		else
		{
			std::vector<NodeId> constructor_arguments;
			std::vector<ExpressionInfo> prepared_constructor_arguments;
			const bool list = initializer != kNoNode &&
				arena_->IsTag(initializer, ::cppgm::syntax::STAG_BRACED_INIT_LIST);
			if (initializer != kNoNode &&
				(arena_->IsTag(initializer, ::cppgm::syntax::STAG_PAREN_INITIALIZER) || list))
				for (std::uint32_t edge = arena_->FirstEdge(initializer);
					edge != kNoEdge; edge = arena_->NextEdge(edge))
					constructor_arguments.push_back(arena_->EdgeChild(edge));
			const std::vector<NodeId> original_constructor_arguments =
				constructor_arguments;
			const bool expanded_constructor_arguments = ExpandCallArgumentPacks(
				original_constructor_arguments, scope, &constructor_arguments,
				&prepared_constructor_arguments);
			construction = BuildConstructorAction(object_type, scope,
				constructor_arguments, false, list, false, true,
				list ? initializer : kNoNode,
				expanded_constructor_arguments ?
					&prepared_constructor_arguments : 0);
			DumpNode& action = dump_.nodes[construction];
			if (action.binding != kNoBinding)
			{
				const FunctionInfo& selected_constructor =
					GetFunction(action.binding);
				if ((initializer != kNoNode || parsed_empty_initializer) &&
					constructor_arguments.empty() &&
					(selected_constructor.implicit_constructor ||
					 selected_constructor.defaulted_constructor) &&
					!selected_constructor.user_provided_special_member)
					action.value_initialization = true;
				if (action.trivial_special_member_action &&
					(selected_constructor.special_member ==
						SPECIAL_MEMBER_COPY_CONSTRUCTOR ||
					 selected_constructor.special_member ==
						SPECIAL_MEMBER_MOVE_CONSTRUCTOR))
				{
					action.trivial_special_member_action = false;
					DemandFunction(action.binding);
				}
			}
			if (action.binding != kNoBinding &&
				GetFunction(action.binding).implicit_constructor &&
				program_->entities[entity].trivial_default_constructor)
				DemandFunction(action.binding);
		}
	}
	else if (initializer != kNoNode || parsed_empty_initializer)
	{
		if (initializer == kNoNode)
		{
			ExpressionInfo zero = MakeLiteral(object_type,
				program_->names.Intern("0"));
			zero.constant = true;
			zero.value = 0;
			RecordExpressionFacts(zero);
			construction = zero.node;
		}
		else if (arena_->IsTag(initializer, ::cppgm::syntax::STAG_PAREN_INITIALIZER))
		{
			std::vector<NodeId> scalar_syntax;
			for (std::uint32_t edge = arena_->FirstEdge(initializer);
				edge != kNoEdge; edge = arena_->NextEdge(edge))
				scalar_syntax.push_back(arena_->EdgeChild(edge));
			std::vector<NodeId> expanded_syntax;
			std::vector<ExpressionInfo> expanded_values;
			if (ExpandCallArgumentPacks(scalar_syntax, scope,
				&expanded_syntax, &expanded_values))
			{
				if (expanded_values.size() > 1)
					ThrowSemanticError(
						"scalar new has multiple initializers");
				if (expanded_values.empty())
				{
					ExpressionInfo zero = MakeLiteral(object_type,
						program_->names.Intern("0"));
					zero.constant = true;
					zero.value = 0;
					RecordExpressionFacts(zero);
					construction = zero.node;
				}
				else construction = ApplyTarget(
					expanded_values[0], object_type).node;
			}
			else
			{
			const std::uint32_t first = arena_->FirstEdge(initializer);
			if (first == kNoEdge)
			{
				ExpressionInfo zero = MakeLiteral(object_type,
					program_->names.Intern("0"));
				zero.constant = true;
				zero.value = 0;
				RecordExpressionFacts(zero);
				construction = zero.node;
			}
			else
			{
				if (arena_->NextEdge(first) != kNoEdge)
					ThrowSemanticError(
						"scalar new has multiple initializers");
				construction = AnalyzeExpression(arena_->EdgeChild(first),
					scope, object_type).node;
			}
			}
		}
		else construction = AnalyzeExpression(
			initializer, scope, object_type).node;
	}
	return BuildScalarNewResult(scope, object_type, selected, allocation.node,
		construction, explicit_global, target);
}

ExpressionInfo Analyzer::BuildScalarNewResult(ScopeId scope, TypeId object_type,
	BindingId selected, std::uint32_t allocation_node, std::uint32_t construction,
	bool explicit_global, TypeId target)
{
	const TypeId result_type = program_->types.Pointer(object_type);
	const std::uint32_t result_node = MakeDump(DUMP_NEW_EXPRESSION,
		result_type, VALUE_PRVALUE, 0, selected);
	dump_.nodes[result_node].operand_type = object_type;
	const FunctionInfo& allocation_function = GetFunction(selected);
	const TypeRecord& allocation_type =
		program_->types.Get(allocation_function.type);
	const bool ordinary_allocation = allocation_type.parameter_count == 1 &&
		!allocation_type.variadic;
	bool nonallocating_placement = false;
	if (allocation_type.parameter_count == 2)
	{
		const TypeId placement_type = program_->types.RemoveTopCv(EffectiveType(
			program_->types.Parameters(allocation_function.type)[1]));
		const TypeRecord& placement = program_->types.Get(placement_type);
		nonallocating_placement = placement.kind == TYPE_POINTER &&
			IsVoid(placement.child);
	}
	const bool allocation_nonthrowing = FunctionIsNonthrowing(selected);
	dump_.nodes[result_node].allocation_may_return_null =
		allocation_nonthrowing && !nonallocating_placement;
	if (construction != kNoDumpEdge && ordinary_allocation &&
		!InitializationActionsAreNonthrowing(construction))
		dump_.nodes[result_node].object_binding = SelectUsualDeallocation(
			scope, EntityOf(object_type), explicit_global, false, object_type);
	dump_.Add(result_node, allocation_node);
	if (construction != kNoDumpEdge) dump_.Add(result_node, construction);
	ExpressionInfo result;
	result.node = result_node;
	result.type = result_type;
	result.category = VALUE_PRVALUE;
	++expression_count_;
	return ApplyTarget(result, target);
}

ExpressionInfo Analyzer::AnalyzeDeleteExpression(NodeId node,
	ScopeId scope, TypeId target)
{
	const ScopedDiagnosticLocation located(arena_, node);
	const bool array = FindChild(node, ::cppgm::syntax::STAG_ARRAY_DELETE) != kNoNode;
	NodeId operand_syntax = kNoNode;
	for (std::uint32_t edge = arena_->FirstEdge(node); edge != kNoEdge;
		edge = arena_->NextEdge(edge))
	{
		const NodeId child = arena_->EdgeChild(edge);
		if (!arena_->IsTag(child, ::cppgm::syntax::STAG_GLOBAL_SCOPE) &&
			!arena_->IsTag(child, ::cppgm::syntax::STAG_ARRAY_DELETE)) operand_syntax = child;
	}
	if (operand_syntax == kNoNode)
		ThrowSemanticError("delete-expression has no operand");
	ExpressionInfo operand = AnalyzeExpression(operand_syntax, scope);
	// A body analysed with the template's parameters standing in for types
	// cannot say what is being deleted: the stand-in is not a pointer and has
	// no conversions, so every check below would be asking about a type nobody
	// has yet.  The delete is checked when the body is instantiated with real
	// arguments.
	if (IsDependentParameterShapeType(operand.type))
	{
		ExpressionInfo dependent;
		dependent.type = program_->types.Fundamental(FUND_VOID);
		dependent.category = VALUE_PRVALUE;
		return dependent;
	}
	if (!IsPointer(Decay(operand.type)))
	{
		std::vector<TypeId> results;
		AppendBuiltinConversionTargets(operand, &results);
		std::vector<TypeId> pointers;
		for (std::size_t i = 0; i < results.size(); ++i)
			if (IsPointer(results[i])) pointers.push_back(results[i]);
		if (pointers.size() != 1)
			ThrowSemanticError("delete operand is not a unique pointer: " +
				program_->RenderType(operand.type) +
				" has no unique conversion to a pointer");
		const CallConversionFact conversion =
			ConvertingFunction(operand, pointers[0], false);
		if (conversion.rank == CONVERSION_INVALID)
			ThrowSemanticError("invalid delete pointer conversion");
		operand = ApplyCallArgument(operand, pointers[0], &conversion);
	}
	const TypeRecord pointer = program_->types.Get(Decay(operand.type));
	if (pointer.kind != TYPE_POINTER)
		ThrowSemanticError("delete operand is not a pointer");
	const TypeId object_type = program_->types.RemoveTopCv(pointer.child);
	TypeId leaf_type = object_type;
	while (array && program_->types.Get(
		program_->types.RemoveTopCv(leaf_type)).kind == TYPE_ARRAY)
		leaf_type = program_->types.Get(
			program_->types.RemoveTopCv(leaf_type)).child;
	leaf_type = program_->types.RemoveTopCv(leaf_type);
	const EntityId entity = EntityOf(leaf_type);
	const bool class_object = IsClassEntity(*program_, entity);
	BindingId destructor = kNoBinding;
	if (class_object)
	{
		const BindingId selected_destructor = DestructorForType(leaf_type);
		if (selected_destructor == kNoBinding)
			ThrowInternalCompilerError("deleted class has no destructor identity");
		if (!CanAccessMember(selected_destructor, entity))
			ThrowSemanticError("inaccessible delete destructor");
		if (GetFunction(selected_destructor).deleted_destructor)
			ThrowSemanticError("deleted destructor is required");
		if (!program_->entities[entity].trivial_destructor)
		{
			destructor = selected_destructor;
			DemandFunction(destructor);
		}
	}
	const bool explicit_global = FindChild(node, ::cppgm::syntax::STAG_GLOBAL_SCOPE) != kNoNode;
	const BindingId deallocation = SelectUsualDeallocation(
		scope, entity, explicit_global, array, leaf_type);
	const TypeId void_type = program_->types.Fundamental(FUND_VOID);
	const std::uint32_t expression = MakeDump(DUMP_DELETE_EXPRESSION,
		void_type, VALUE_PRVALUE, 0, deallocation);
	dump_.nodes[expression].operand_type = leaf_type;
	dump_.nodes[expression].selected_binding = destructor;
	if (destructor != kNoBinding &&
		program_->bindings[destructor].virtual_function)
	{
		dump_.nodes[expression].virtual_call = true;
		const std::uint32_t complete_slot = VirtualSlotFor(destructor);
		if (complete_slot == kNoDumpEdge)
			ThrowInternalCompilerError("virtual destructor has no slot");
		dump_.nodes[expression].virtual_slot = complete_slot + 1;
	}
	dump_.nodes[expression].array_action = array;
	dump_.nodes[expression].array_cookie =
		array && ArrayNeedsCookie(entity, explicit_global);
	dump_.Add(expression, operand.node);
	ExpressionInfo result;
	result.node = expression;
	result.type = void_type;
	result.category = VALUE_PRVALUE;
	++expression_count_;
	return ApplyTarget(result, target);
}

}
}
