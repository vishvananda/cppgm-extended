#include "semantic/analysis/analyzer.h"
#include "support/exceptions.h"

#include "preprocess/hosted/builtin_registry.h"

#include <string>
#include <vector>

namespace cppgm
{
namespace semantic
{
namespace
{

TypeId RemoveCv(TypeTable* types, TypeId type, std::uint8_t removed)
{
	const TypeRecord record = types->Get(type);
	if (record.kind == TYPE_ARRAY)
	{
		const TypeId child = RemoveCv(types, record.child, removed);
		if (child == record.child) return type;
		return record.dependent_bound_parameter == kNoTemplateParameter ?
			types->Array(child, record.bound) :
			types->DependentArray(child, record.dependent_bound_type,
				record.dependent_bound_parameter);
	}
	if (record.kind != TYPE_QUALIFIED) return type;
	const std::uint8_t remaining =
		static_cast<std::uint8_t>(record.cv & ~removed);
	return remaining == CV_NONE ? record.child :
		types->Qualify(record.child, remaining);
}

TypeId RemoveReference(TypeTable* types, TypeId type)
{
	const TypeRecord record = types->Get(type);
	return record.kind == TYPE_LVALUE_REFERENCE ||
		record.kind == TYPE_RVALUE_REFERENCE ? record.child : type;
}

EntityId DirectNamedEntity(const TypeTable& types, TypeId type)
{
	const TypeRecord shape = types.Get(types.RemoveTopCv(type));
	return shape.kind == TYPE_NAMED ? shape.entity : kNoEntity;
}

bool IsFundamentalIntegral(const TypeRecord& record)
{
	if (record.kind == TYPE_BITINT) return true;
	return record.kind == TYPE_FUNDAMENTAL &&
		record.fundamental != FUND_VOID &&
		record.fundamental != FUND_NULLPTR_T &&
		record.fundamental != FUND_FLOAT &&
		record.fundamental != FUND_DOUBLE &&
		record.fundamental != FUND_LONG_DOUBLE &&
		record.fundamental != FUND_FLOAT16 &&
		record.fundamental != FUND_FLOAT32 &&
		record.fundamental != FUND_FLOAT32X &&
		record.fundamental != FUND_FLOAT64 &&
		record.fundamental != FUND_FLOAT64X &&
		record.fundamental != FUND_STDFLOAT128 &&
		record.fundamental != FUND_FLOAT128;
}

bool IsFundamentalFloating(const TypeRecord& record)
{
	return record.kind == TYPE_FUNDAMENTAL &&
		(record.fundamental == FUND_FLOAT ||
		 record.fundamental == FUND_DOUBLE ||
		 record.fundamental == FUND_LONG_DOUBLE ||
		 record.fundamental == FUND_FLOAT16 ||
		 record.fundamental == FUND_FLOAT32 ||
		 record.fundamental == FUND_FLOAT32X ||
		 record.fundamental == FUND_FLOAT64 ||
		 record.fundamental == FUND_FLOAT64X ||
		 record.fundamental == FUND_STDFLOAT128 ||
		 record.fundamental == FUND_FLOAT128);
}

bool IsSignedFundamental(FundamentalKind kind)
{
	return kind == FUND_SIGNED_CHAR || kind == FUND_SHORT_INT ||
		kind == FUND_INT || kind == FUND_LONG_INT ||
		kind == FUND_LONG_LONG_INT || kind == FUND_INT128 ||
		kind == FUND_FLOAT || kind == FUND_DOUBLE ||
		kind == FUND_LONG_DOUBLE || kind == FUND_FLOAT16 ||
		kind == FUND_FLOAT32 || kind == FUND_FLOAT32X ||
		kind == FUND_FLOAT64 || kind == FUND_FLOAT64X ||
		kind == FUND_STDFLOAT128 || kind == FUND_FLOAT128;
}

FundamentalKind SignednessKind(FundamentalKind kind, bool make_unsigned)
{
	switch (kind)
	{
	case FUND_CHAR:
	case FUND_SIGNED_CHAR:
	case FUND_UNSIGNED_CHAR:
		return make_unsigned ? FUND_UNSIGNED_CHAR : FUND_SIGNED_CHAR;
	case FUND_SHORT_INT:
	case FUND_UNSIGNED_SHORT_INT:
	case FUND_CHAR16_T:
		return make_unsigned ? FUND_UNSIGNED_SHORT_INT : FUND_SHORT_INT;
	case FUND_INT:
	case FUND_UNSIGNED_INT:
	case FUND_WCHAR_T:
	case FUND_CHAR32_T:
		return make_unsigned ? FUND_UNSIGNED_INT : FUND_INT;
	case FUND_LONG_INT:
	case FUND_UNSIGNED_LONG_INT:
		return make_unsigned ? FUND_UNSIGNED_LONG_INT : FUND_LONG_INT;
	case FUND_LONG_LONG_INT:
	case FUND_UNSIGNED_LONG_LONG_INT:
		return make_unsigned ? FUND_UNSIGNED_LONG_LONG_INT : FUND_LONG_LONG_INT;
	case FUND_INT128:
	case FUND_UINT128:
		return make_unsigned ? FUND_UINT128 : FUND_INT128;
	default: ThrowSemanticError("invalid signedness transform operand");
	}
}

bool IsEnumEntity(const EntityRecord& entity)
{
	return IsEnumNamedFlavor(entity.flavor);
}

bool IsClassEntity(const EntityRecord& entity)
{
	return IsClassNamedFlavor(entity.flavor);
}

// N3485 20.9.6: is_base_of relates two *class* types, and a reference is not
// one.  Resolving the operand to its entity strips a top-level reference, so
// answers for `X&` as though the reference were not there; this asks about the
// operand's own record, which is also one lookup less.
EntityId ClassEntityOf(const Program& program, TypeId type)
{
	const TypeRecord record = program.types.Get(program.types.RemoveTopCv(type));
	if (record.kind != TYPE_NAMED) return kNoEntity;
	return IsClassEntity(program.entities[record.entity]) ?
		record.entity : kNoEntity;
}

}

TypeId Analyzer::BuildBuiltinTransformType(NodeId node, ScopeId scope)
{
	using namespace hosted_builtin;
	const TypeTransformKind transform =
		FindTypeTransform(PayloadSource(node));
	if (transform == TYPE_TRANSFORM_NONE)
		ThrowSemanticError("unsupported builtin type transform");
	const NodeId operand = FindChild(node, ::cppgm::syntax::STAG_TYPE_ID);
	TypeId type = BuildTypeId(operand, scope);
	if (CandidateSubstitutionFailed() || type == kNoType) return kNoType;
	TypeTable* types = &program_->types;
	if (transform == TYPE_TRANSFORM_REMOVE_CONST)
		return RemoveCv(types, type, CV_CONST);
	if (transform == TYPE_TRANSFORM_REMOVE_VOLATILE)
		return RemoveCv(types, type, CV_VOLATILE);
	if (transform == TYPE_TRANSFORM_REMOVE_CV)
		return RemoveCv(types, type, CV_CONST | CV_VOLATILE);
	if (transform == TYPE_TRANSFORM_REMOVE_REFERENCE)
		return RemoveReference(types, type);
	if (transform == TYPE_TRANSFORM_REMOVE_CVREF)
		return RemoveCv(types, RemoveReference(types, type),
			CV_CONST | CV_VOLATILE);
	if (transform == TYPE_TRANSFORM_REMOVE_POINTER)
	{
		const TypeId unqualified = types->RemoveTopCv(type);
		const TypeRecord record = types->Get(unqualified);
		return record.kind == TYPE_POINTER ? record.child : type;
	}
	if (transform == TYPE_TRANSFORM_REMOVE_ALL_EXTENTS)
	{
		while (types->Get(type).kind == TYPE_ARRAY)
			type = types->Get(type).child;
		return type;
	}
	if (transform == TYPE_TRANSFORM_ADD_POINTER)
	{
		type = RemoveReference(types, type);
		const TypeId pointer = types->TryPointer(type);
		return pointer == kNoType ? type : pointer;
	}
	if (transform == TYPE_TRANSFORM_ADD_LVALUE_REFERENCE ||
		transform == TYPE_TRANSFORM_ADD_RVALUE_REFERENCE)
	{
		const TypeId reference = types->TryReference(
			transform == TYPE_TRANSFORM_ADD_LVALUE_REFERENCE ?
				TYPE_LVALUE_REFERENCE : TYPE_RVALUE_REFERENCE, type);
		return reference == kNoType ? type : reference;
	}
	if (transform == TYPE_TRANSFORM_DECAY)
	{
		type = RemoveCv(types, RemoveReference(types, type),
			CV_CONST | CV_VOLATILE);
		const TypeRecord shape = types->Get(type);
		if (shape.kind == TYPE_ARRAY) return types->Pointer(shape.child);
		if (shape.kind == TYPE_FUNCTION) return types->Pointer(type);
		return type;
	}
	if (transform == TYPE_TRANSFORM_UNDERLYING_TYPE)
	{
		const TypeId unqualified = types->RemoveTopCv(type);
		const TypeRecord shape = types->Get(unqualified);
		if (shape.kind == TYPE_NAMED &&
			IsEnumEntity(program_->entities[shape.entity]))
			return program_->entities[shape.entity].underlying;
		// A shape-only completion transforms stand-ins: keep them.
		if (FunctionTemplateTypeIsDependent(type) ||
			dependent_shape_completion_depth_ != 0) return type;
		ThrowSemanticError("underlying type operand is not an enum");
	}
	if (transform == TYPE_TRANSFORM_MAKE_SIGNED ||
		transform == TYPE_TRANSFORM_MAKE_UNSIGNED)
	{
		const TypeRecord top = types->Get(type);
		const std::uint8_t cv = top.kind == TYPE_QUALIFIED ? top.cv : CV_NONE;
		TypeId base = types->RemoveTopCv(type);
		TypeRecord shape = types->Get(base);
		if (shape.kind == TYPE_NAMED && IsEnumEntity(program_->entities[shape.entity]))
		{
			base = program_->entities[shape.entity].underlying;
			shape = types->Get(types->RemoveTopCv(base));
		}
		if (shape.kind != TYPE_FUNDAMENTAL)
		{
			if (FunctionTemplateTypeIsDependent(type) ||
				dependent_shape_completion_depth_ != 0) return type;
			ThrowSemanticError("invalid signedness transform operand");
		}
		base = types->Fundamental(SignednessKind(shape.fundamental,
			transform == TYPE_TRANSFORM_MAKE_UNSIGNED));
		return cv == CV_NONE ? base : types->Qualify(base, cv);
	}
	ThrowInternalCompilerError("unhandled builtin type transform");
}


namespace
{

// Traits whose answer depends only on the shape of the operand type, never
// on the members of a class it names.
bool TraitReadsOnlyTypeShape(hosted_builtin::TypeTraitKind trait)
{
	using namespace hosted_builtin;
	switch (trait)
	{
	case TYPE_TRAIT_IS_SAME:
	case TYPE_TRAIT_IS_CONST:
	case TYPE_TRAIT_IS_VOLATILE:
	case TYPE_TRAIT_IS_VOID:
	case TYPE_TRAIT_IS_POINTER:
	case TYPE_TRAIT_IS_REFERENCE:
	case TYPE_TRAIT_IS_LVALUE_REFERENCE:
	case TYPE_TRAIT_IS_RVALUE_REFERENCE:
	case TYPE_TRAIT_IS_ARRAY:
	case TYPE_TRAIT_IS_BOUNDED_ARRAY:
	case TYPE_TRAIT_IS_UNBOUNDED_ARRAY:
	case TYPE_TRAIT_ARRAY_RANK:
	case TYPE_TRAIT_IS_FUNCTION:
	case TYPE_TRAIT_IS_MEMBER_POINTER:
	case TYPE_TRAIT_IS_MEMBER_OBJECT_POINTER:
	case TYPE_TRAIT_IS_MEMBER_FUNCTION_POINTER:
	case TYPE_TRAIT_IS_INTEGRAL:
	case TYPE_TRAIT_IS_FLOATING_POINT:
	case TYPE_TRAIT_IS_ARITHMETIC:
	case TYPE_TRAIT_IS_FUNDAMENTAL:
	case TYPE_TRAIT_IS_COMPOUND:
	case TYPE_TRAIT_IS_OBJECT:
	case TYPE_TRAIT_IS_CLASS:
	case TYPE_TRAIT_IS_UNION:
	case TYPE_TRAIT_IS_ENUM:
	case TYPE_TRAIT_IS_COMPLETE_OR_UNBOUNDED:
		return true;
	default:
		return false;
	}
}

}
ExpressionInfo Analyzer::AnalyzeBuiltinTypeTrait(
	NodeId node, ScopeId scope)
{
	using namespace hosted_builtin;
	const TypeTraitKind trait = FindTypeTrait(PayloadSource(node));
	if (trait == TYPE_TRAIT_NONE)
		ThrowSemanticError("unsupported builtin type trait");
	std::vector<TypeId> operands;
	bool dependent = false;
	for (std::uint32_t edge = arena_->FirstEdge(node); edge != kNoEdge;
		edge = arena_->NextEdge(edge))
	{
		const NodeId holder = arena_->EdgeChild(edge);
		if (!arena_->IsTag(holder, ::cppgm::syntax::STAG_BUILTIN_TYPE_OPERAND)) continue;
		const NodeId type_id = FindChild(holder, ::cppgm::syntax::STAG_TYPE_ID);
		NodeId declarator = FindChild(type_id, ::cppgm::syntax::STAG_ABSTRACT_DECLARATOR);
		if (declarator == kNoNode)
			declarator = FindChild(type_id, ::cppgm::syntax::STAG_DECLARATOR);
		const bool expansion =
			FindChild(holder, ::cppgm::syntax::STAG_TYPE_PACK_EXPANSION) != kNoNode ||
			(declarator != kNoNode &&
			 FindChild(declarator, ::cppgm::syntax::STAG_PARAMETER_PACK) != kNoNode);
		if (expansion)
		{
			std::vector<ScopeId> element_scopes;
			if (ExpandPackElementScopes(type_id, scope, &element_scopes))
			{
				for (std::size_t i = 0; i < element_scopes.size(); ++i)
				{
					const TypeId element =
						BuildTypeId(type_id, element_scopes[i]);
					if (CandidateSubstitutionFailed() || element == kNoType)
						return ExpressionInfo();
					operands.push_back(element);
					dependent = dependent ||
						FunctionTemplateTypeIsDependent(element);
				}
				continue;
			}
			if (CandidateSubstitutionFailed()) return ExpressionInfo();
		}
		const TypeId type = BuildTypeId(type_id, scope);
		if (CandidateSubstitutionFailed() || type == kNoType)
			return ExpressionInfo();
		operands.push_back(type);
		dependent = dependent || FunctionTemplateTypeIsDependent(type) ||
			expansion;
	}
	if (operands.empty())
		ThrowSemanticError("builtin type trait has no operands");
	bool value = false;
	std::int64_t integral_value = 0;
	TypeId result_type = program_->types.Fundamental(FUND_BOOL);
	if (!dependent)
	{
		// A trait that reads only the shape of its operand -- its
		// qualification, its kind, its declared category -- has its answer
		// for an incomplete class too, and completing the class would be
		// wrong when the trait is asked from inside that class's own
		// definition, which libc++'s allocator does for every container
		// member type.  Every other trait looks inside a class and needs it
		// complete first.
		if (!TraitReadsOnlyTypeShape(trait))
			for (std::size_t i = 0; i < operands.size(); ++i)
			{
				const EntityId entity = DirectNamedEntity(program_->types, operands[i]);
				if (entity != kNoEntity &&
					IsClassEntity(program_->entities[entity]))
					EnsureClassDefinition(operands[i]);
			}
		const TypeId first = operands[0];
		const TypeId unqualified = program_->types.RemoveTopCv(first);
		const TypeRecord shape = program_->types.Get(unqualified);
		const EntityId entity = DirectNamedEntity(program_->types, first);
		const EntityRecord* named = entity == kNoEntity ? 0 :
			&program_->entities[entity];
		if (trait == TYPE_TRAIT_IS_POLYMORPHIC && named &&
			IsClassEntity(*named) && !named->complete)
			ThrowSemanticError(
				"polymorphic trait requires a complete class type");
		if (trait == TYPE_TRAIT_IS_BASE_OF && operands.size() == 2)
		{
			const EntityId base = ClassEntityOf(*program_, operands[0]);
			const EntityId derived = ClassEntityOf(*program_, operands[1]);
			// Only a class type has to be complete; for anything else the
			// answer is false without looking inside it.
			if (base != derived && derived != kNoEntity &&
				!program_->entities[derived].complete)
				ThrowSemanticError(
					"base-of trait requires a complete derived type");
		}
		if (trait == TYPE_TRAIT_ARRAY_RANK && operands.size() == 1)
		{
			TypeId ranked = program_->types.RemoveTopCv(first);
			while (program_->types.Get(ranked).kind == TYPE_ARRAY)
			{
				++integral_value;
				ranked = program_->types.RemoveTopCv(
					program_->types.Get(ranked).child);
			}
			result_type = program_->types.Fundamental(FUND_UNSIGNED_LONG_INT);
		}
		else if (trait == TYPE_TRAIT_IS_SAME && operands.size() == 2)
			value = operands[0] == operands[1];
		else if (trait == TYPE_TRAIT_IS_POINTER && operands.size() == 1)
			value = shape.kind == TYPE_POINTER;
		// N3485 3.9.3/1 puts cv-qualification on the type itself, so these two
		// read `first` rather than the stripped shape every other trait uses.
		// A reference is never cv-qualified, however it was written.
		else if (trait == TYPE_TRAIT_IS_CONST && operands.size() == 1)
			value = (program_->types.Get(first).cv & CV_CONST) != 0;
		else if (trait == TYPE_TRAIT_IS_VOLATILE && operands.size() == 1)
			value = (program_->types.Get(first).cv & CV_VOLATILE) != 0;
		else if (trait == TYPE_TRAIT_IS_VOID && operands.size() == 1)
			value = shape.kind == TYPE_FUNDAMENTAL &&
				shape.fundamental == FUND_VOID;
		else if (trait == TYPE_TRAIT_IS_ARRAY && operands.size() == 1)
			value = shape.kind == TYPE_ARRAY;
		// N3485 8.3.4/1: an array bound may be omitted, and the two forms are
		// distinct types.
		else if (trait == TYPE_TRAIT_IS_BOUNDED_ARRAY && operands.size() == 1)
			value = shape.kind == TYPE_ARRAY && !shape.IsIncompleteArray();
		else if (trait == TYPE_TRAIT_IS_UNBOUNDED_ARRAY && operands.size() == 1)
			value = shape.IsIncompleteArray();
		else if (trait == TYPE_TRAIT_IS_LVALUE_REFERENCE && operands.size() == 1)
			value = shape.kind == TYPE_LVALUE_REFERENCE;
		else if (trait == TYPE_TRAIT_IS_RVALUE_REFERENCE && operands.size() == 1)
			value = shape.kind == TYPE_RVALUE_REFERENCE;
		else if (trait == TYPE_TRAIT_IS_UNSIGNED && operands.size() == 1)
			value = shape.kind == TYPE_FUNDAMENTAL &&
				IsFundamentalIntegral(shape) &&
				!IsSignedFundamental(shape.fundamental);
		// N3485 3.9.1/9 and 3.9/9: the arithmetic types are the integral and
		// floating ones, and the fundamental types add void and nullptr_t.
		else if (trait == TYPE_TRAIT_IS_ARITHMETIC && operands.size() == 1)
			value = IsFundamentalIntegral(shape) || IsFundamentalFloating(shape);
		else if (trait == TYPE_TRAIT_IS_FUNDAMENTAL && operands.size() == 1)
			value = shape.kind == TYPE_FUNDAMENTAL;
		// N3485 3.9/9: an object type is any type that is not a function, a
		// reference, or void.  A compound type is everything that is not
		// fundamental.
		else if (trait == TYPE_TRAIT_IS_OBJECT && operands.size() == 1)
			value = shape.kind != TYPE_FUNCTION &&
				shape.kind != TYPE_LVALUE_REFERENCE &&
				shape.kind != TYPE_RVALUE_REFERENCE &&
				!(shape.kind == TYPE_FUNDAMENTAL &&
					shape.fundamental == FUND_VOID);
		else if (trait == TYPE_TRAIT_IS_COMPOUND && operands.size() == 1)
			value = shape.kind != TYPE_FUNDAMENTAL;
		// A referenceable type is one a reference may be formed to: everything
		// but void and a function type carrying cv-qualifiers or a
		// ref-qualifier, which this model does not spell separately.
		else if (trait == TYPE_TRAIT_IS_REFERENCEABLE && operands.size() == 1)
			value = !(shape.kind == TYPE_FUNDAMENTAL &&
				shape.fundamental == FUND_VOID);
		else if (trait == TYPE_TRAIT_IS_REFERENCE && operands.size() == 1)
			value = shape.kind == TYPE_LVALUE_REFERENCE ||
				shape.kind == TYPE_RVALUE_REFERENCE;
		else if (trait == TYPE_TRAIT_IS_FUNCTION && operands.size() == 1)
			value = shape.kind == TYPE_FUNCTION;
		else if (trait == TYPE_TRAIT_IS_MEMBER_POINTER && operands.size() == 1)
			value = shape.kind == TYPE_MEMBER_POINTER;
		else if (trait == TYPE_TRAIT_IS_MEMBER_FUNCTION_POINTER &&
			operands.size() == 1)
			value = shape.kind == TYPE_MEMBER_POINTER &&
				program_->types.Get(shape.child).kind == TYPE_FUNCTION;
		else if (trait == TYPE_TRAIT_IS_MEMBER_OBJECT_POINTER &&
			operands.size() == 1)
			value = shape.kind == TYPE_MEMBER_POINTER &&
				program_->types.Get(shape.child).kind != TYPE_FUNCTION;
		else if (trait == TYPE_TRAIT_IS_INTEGRAL && operands.size() == 1)
			value = IsFundamentalIntegral(shape);
		else if (trait == TYPE_TRAIT_IS_FLOATING_POINT && operands.size() == 1)
			value = IsFundamentalFloating(shape);
		else if (trait == TYPE_TRAIT_IS_SIGNED && operands.size() == 1)
			value = shape.kind == TYPE_FUNDAMENTAL &&
				IsSignedFundamental(shape.fundamental);
		else if (trait == TYPE_TRAIT_IS_ENUM && operands.size() == 1)
			value = named && IsEnumEntity(*named);
		else if (trait == TYPE_TRAIT_IS_UNION && operands.size() == 1)
			value = named && named->flavor == NAMED_UNION;
		else if (trait == TYPE_TRAIT_IS_CLASS && operands.size() == 1)
			value = named && IsClassEntity(*named) &&
				named->flavor != NAMED_UNION;
		else if (trait == TYPE_TRAIT_IS_SCALAR && operands.size() == 1)
			value = IsFundamentalIntegral(shape) || IsFundamentalFloating(shape) ||
				(shape.kind == TYPE_FUNDAMENTAL &&
				 shape.fundamental == FUND_NULLPTR_T) ||
				shape.kind == TYPE_POINTER || shape.kind == TYPE_MEMBER_POINTER ||
				shape.kind == TYPE_COMPLEX ||
				(named && IsEnumEntity(*named));
		else if (trait == TYPE_TRAIT_IS_EMPTY && operands.size() == 1)
			value = named && IsClassEntity(*named) && named->empty_class;
		else if (trait == TYPE_TRAIT_IS_AGGREGATE && operands.size() == 1)
			value = named && IsClassEntity(*named) && named->is_aggregate;
		else if (trait == TYPE_TRAIT_IS_ABSTRACT && operands.size() == 1)
			value = named && IsClassEntity(*named) && named->abstract_class;
		else if (trait == TYPE_TRAIT_IS_POLYMORPHIC && operands.size() == 1)
			value = named && IsClassEntity(*named) && named->polymorphic_class;
		else if ((trait == TYPE_TRAIT_IS_DESTRUCTIBLE ||
			trait == TYPE_TRAIT_IS_TRIVIALLY_DESTRUCTIBLE) && operands.size() == 1)
			value = shape.kind == TYPE_LVALUE_REFERENCE ||
				shape.kind == TYPE_RVALUE_REFERENCE || IsFundamentalIntegral(shape) ||
				IsFundamentalFloating(shape) || shape.kind == TYPE_COMPLEX ||
				shape.kind == TYPE_POINTER ||
				shape.kind == TYPE_MEMBER_POINTER ||
				(named && named->destructible &&
				 (trait == TYPE_TRAIT_IS_DESTRUCTIBLE || named->trivial_destructor));
		else if (trait == TYPE_TRAIT_IS_TRIVIALLY_COPYABLE &&
			operands.size() == 1)
			value = (named && IsClassEntity(*named)) ?
				EvaluateBuiltinTriviallyCopyable(first) :
				IsFundamentalIntegral(shape) || IsFundamentalFloating(shape) ||
				shape.kind == TYPE_COMPLEX || shape.kind == TYPE_POINTER ||
				shape.kind == TYPE_MEMBER_POINTER;
		else if ((trait == TYPE_TRAIT_IS_TRIVIAL || trait == TYPE_TRAIT_IS_POD ||
			trait == TYPE_TRAIT_IS_STANDARD_LAYOUT ||
			trait == TYPE_TRAIT_IS_LITERAL_TYPE) && operands.size() == 1)
			value = EvaluateBuiltinTrivialLayoutTrait(
				trait, first, shape, named);
		else if ((trait == TYPE_TRAIT_IS_CONSTRUCTIBLE ||
			trait == TYPE_TRAIT_IS_NOTHROW_CONSTRUCTIBLE ||
			trait == TYPE_TRAIT_IS_TRIVIALLY_CONSTRUCTIBLE))
		{
			BindingId selected = kNoBinding;
			std::vector<CallConversionFact> conversions;
			value = EvaluateBuiltinConstructibility(
				operands, &selected, &conversions);
			if (value && trait == TYPE_TRAIT_IS_NOTHROW_CONSTRUCTIBLE)
				value = BuiltinConstructionIsNonthrowing(
					operands[0], selected, conversions);
			else if (value && trait == TYPE_TRAIT_IS_TRIVIALLY_CONSTRUCTIBLE)
				value = BuiltinConstructionIsTrivial(
					operands[0], selected, conversions);
		}
		else if ((trait == TYPE_TRAIT_IS_ASSIGNABLE ||
			trait == TYPE_TRAIT_IS_NOTHROW_ASSIGNABLE ||
			trait == TYPE_TRAIT_IS_TRIVIALLY_ASSIGNABLE) && operands.size() == 2)
		{
			BindingId selected = kNoBinding;
			std::vector<CallConversionFact> conversions;
			value = EvaluateBuiltinAssignability(
				operands[0], operands[1], scope, &selected, &conversions);
			if (value && trait == TYPE_TRAIT_IS_NOTHROW_ASSIGNABLE)
				value = BuiltinAssignmentIsNonthrowing(
					selected, conversions);
			else if (value && trait == TYPE_TRAIT_IS_TRIVIALLY_ASSIGNABLE)
				value = BuiltinAssignmentIsTrivial(selected, conversions);
		}
		else if (trait == TYPE_TRAIT_IS_CONVERTIBLE && operands.size() == 2)
			value = EvaluateBuiltinConvertibility(operands[0], operands[1]);
		else if (trait == TYPE_TRAIT_IS_BASE_OF && operands.size() == 2)
		{
			const EntityId base = ClassEntityOf(*program_, operands[0]);
			const EntityId derived = ClassEntityOf(*program_, operands[1]);
			value = base != kNoEntity && derived != kNoEntity &&
				(base == derived || AccessIsBaseOf(base, derived));
		}
		else if (trait == TYPE_TRAIT_IS_COMPLETE_OR_UNBOUNDED &&
			operands.size() == 1)
			value = shape.kind == TYPE_ARRAY ? shape.bound == 0 :
				shape.kind != TYPE_FUNCTION && !IsVoid(first) &&
				(!named || named->complete);
		else if (trait == TYPE_TRAIT_HAS_TRIVIAL_CONSTRUCTOR &&
			operands.size() == 1)
			value = !named || named->trivial_default_constructor;
		else if (trait == TYPE_TRAIT_HAS_NOTHROW_COPY && operands.size() == 1)
			value = EvaluateBuiltinNothrowCopy(first);
		else if (trait == TYPE_TRAIT_HAS_VIRTUAL_DESTRUCTOR &&
			operands.size() == 1)
		{
			const BindingId destructor = named && IsClassEntity(*named) ?
				DestructorForType(first) : kNoBinding;
			value = destructor != kNoBinding &&
				program_->bindings[destructor].virtual_function;
		}
		else if (trait == TYPE_TRAIT_IS_FINAL && operands.size() == 1)
			value = named && IsClassEntity(*named) && named->final_class;
		else if (trait == TYPE_TRAIT_REFERENCE_BINDS_TO_TEMPORARY ||
			trait == TYPE_TRAIT_REFERENCE_CONSTRUCTS_FROM_TEMPORARY)
			value = false;
		else
			ThrowSemanticError("unsupported builtin type trait operands");
	}
	if (trait == TYPE_TRAIT_ARRAY_RANK)
	{
		if (operands.size() != 1)
			ThrowSemanticError("unsupported builtin type trait operands");
		result_type = program_->types.Fundamental(FUND_UNSIGNED_LONG_INT);
	}
	else integral_value = value ? 1 : 0;
	ExpressionInfo result = MakeLiteral(result_type,
		program_->names.Intern(std::to_string(integral_value)));
	result.constant = true;
	result.value = integral_value;
	dump_.nodes[result.node].template_parameter_constant = dependent;
	RecordExpressionFacts(result);
	return result;
}

}
}
