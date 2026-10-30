// GCC's and Clang's builtin functions (lex/gnu_builtins.h). Each is declared
// in the global namespace from its registry entry when a call (or a
// redeclaration) first names it, so `::__builtin_x` names the same one. A
// function of its own signature (__builtin_memcpy, the C library forms) is
// called as one; a call of a type-generic, operand-checked or query builtin
// is typed here and expanded in place by the lowering (sema::Intrinsic),
// selected by the registry's operation, never by its spelling.

#include "sema/analyzer.h"

#include <cstring>

#include "lex/gnu_builtins.h"
#include "lex/identifier_table.h"
#include "preprocess/preprocessor.h"
#include "sema/constant_interpreter.h"
#include "sema/conversions.h"
#include "sema/integral_arithmetic.h"
#include "sema/string_literals.h"

namespace sema {

namespace {

BuiltinFunction FunctionKindOf(GnuBuiltin builtin)
{
	switch (builtin)
	{
	case GnuBuiltin::Abort:
		return BuiltinFunction::Abort;
	case GnuBuiltin::Memcpy:
		return BuiltinFunction::Memcpy;
	case GnuBuiltin::Memmove:
		return BuiltinFunction::Memmove;
	case GnuBuiltin::Strlen:
		return BuiltinFunction::Strlen;
	case GnuBuiltin::Unreachable:
		return BuiltinFunction::Unreachable;
	case GnuBuiltin::Expect:
		return BuiltinFunction::Expect;
	case GnuBuiltin::Trap:
		return BuiltinFunction::Trap;
	case GnuBuiltin::Library:
		return BuiltinFunction::Library;
	default:
		return BuiltinFunction::Intrinsic;
	}
}

// The floating fundamental type of a libm family's letter or a constant's
// variant (double, float, long double).
FundamentalType FloatingOf(uint32_t index)
{
	return index == 0 ? FundamentalType::Double : index == 1 ? FundamentalType::Float : FundamentalType::LongDouble;
}

} // namespace

FunctionEntity* Analyzer::builtin_function(const Identifier* name)
{
	// Reserved names (17.6.4.3.2p1) only, each looked up in the registry
	// once (a redeclared library function's name is asked again).
	if (name->length < 7 || name->spelling[0] != '_' || name->spelling[1] != '_')
		return nullptr;
	if (builtin_names_.size() <= name->id)
		builtin_names_.resize(name->id + 1, 0);
	if (builtin_names_[name->id] == 1)
		return nullptr;
	// Declared once: a later need (a template instance's lookup limited to
	// its definition's context among them) finds the same function.
	if (const NameEntry* entry = names_.find(graph_.global, name))
	{
		for (const FunctionList* node = entry->functions; node != nullptr; node = node->next)
		{
			FunctionEntity* function = static_cast<FunctionEntity*>(node->binding->entity);
			if (function->gnu_builtin != GnuBuiltin::None)
				return function;
		}
		return nullptr;
	}
	GnuBuiltinEntry entry;
	const bool found = FindGnuBuiltin(name->spelling, name->length, entry);
	builtin_names_[name->id] = found ? 2 : 1;
	if (!found)
		return nullptr;
	// The type-generic and query builtins have no type of their own; a
	// variadic int(...) stands for it.
	const Type* type = entry.signature != nullptr
		? builtin_signature(entry.signature, entry.floating)
		: types_.function(types_.fundamental(FundamentalType::Int), nullptr, 0, true, 0, RefQualifier::None);
	if (type == nullptr)
		return nullptr;
	FunctionEntity* function = new_entity<FunctionEntity>(EntityKind::Function, name, graph_.global, type);
	function->label = name->spelling;
	function->signature = signature_of(type);
	function->builtin = FunctionKindOf(entry.builtin);
	function->gnu_builtin = entry.builtin;
	function->builtin_variant = entry.variant;
	function->nothrow = true;
	function->c_linkage = true;
	Binding* binding = new_binding(BindingKind::Function, name, function, type);
	binding->hidden = true;
	declare_in(graph_.global, binding);
	return function;
}

LookupResult Analyzer::builtin_lookup(const Identifier* name, const LookupResult& none)
{
	const FunctionEntity* builtin = builtin_function(name);
	if (builtin == nullptr)
		return none;
	LookupResult result = LookupResult();
	result.functions = names_.find(graph_.global, name)->functions;
	for (FunctionList* node = result.functions; node != nullptr; node = node->next)
	{
		if (node->binding->entity == builtin)
			result.binding = node->binding;
	}
	return result;
}

const Type* Analyzer::builtin_signature(const char* signature, char floating)
{
	// The registry's letters (lex/gnu_builtins.h): the result, then the
	// parameters; null when a type it names is not declared (the hosted
	// __builtin_va_list).
	SmallList<const Type*, 6> types;
	for (const char* c = signature; *c != '\0';)
	{
		const uint8_t cv = *c == 'C' ? kConst : 0;
		if (cv != 0)
			++c;
		const bool complex = *c == 'Z';
		const char letter = *c == 'T' || complex ? floating : *c;
		++c;
		const Type* type = nullptr;
		switch (letter)
		{
		case 'v':
			type = types_.fundamental(FundamentalType::Void);
			break;
		case 'c':
			type = types_.fundamental(FundamentalType::Char);
			break;
		case 'i':
			type = types_.fundamental(FundamentalType::Int);
			break;
		case 'l':
			type = types_.fundamental(FundamentalType::LongInt);
			break;
		case 'x':
			type = types_.fundamental(FundamentalType::LongLongInt);
			break;
		case 'z':
			type = types_.fundamental(FundamentalType::UnsignedLongInt);
			break;
		case 'f':
			type = types_.fundamental(FundamentalType::Float);
			break;
		case 'd':
			type = types_.fundamental(FundamentalType::Double);
			break;
		case 'e':
			type = types_.fundamental(FundamentalType::LongDouble);
			break;
		case 'V':
		{
			const NameEntry* entry = names_.find(graph_.global, special_key("__builtin_va_list"));
			if (entry == nullptr || entry->type == nullptr || entry->type->kind != BindingKind::TypeAlias)
				return nullptr;
			type = entry->type->type;
			break;
		}
		default:
			return nullptr;
		}
		if (complex)
			type = types_.complex(type);
		if (cv != 0)
			type = types_.qualified(type, cv);
		for (; *c == '*'; ++c)
			type = types_.pointer(type);
		types.push_back(type);
	}
	return types_.function(
		types[0], types.data() + 1, static_cast<uint32_t>(types.size() - 1), false, 0, RefQualifier::None);
}

ExprFacts* Analyzer::builtin_operation(const syntax::CallExpr& expr, const FunctionEntity* builtin, Scope* scope)
{
	const uint8_t variant = builtin->builtin_variant;
	switch (builtin->gnu_builtin)
	{
	case GnuBuiltin::ConstantP:
	{
		ExprFacts* facts = constant_p_call(expr, scope);
		if (facts != nullptr)
			facts->entity = builtin;
		return facts;
	}
	case GnuBuiltin::ObjectSize:
		return object_size_call(expr, scope);
	case GnuBuiltin::Invoke:
		return builtin_invoke(expr, scope);
	case GnuBuiltin::FloatClassify:
	case GnuBuiltin::FloatCompare:
	case GnuBuiltin::FloatConstant:
		return floating_builtin(expr, builtin->gnu_builtin, variant, scope);
	case GnuBuiltin::Overflow:
		return overflow_builtin(expr, variant, scope);
	case GnuBuiltin::VectorLanes:
		return vector_lane_builtin(expr, variant, scope);
	case GnuBuiltin::VectorShiftBytes:
		return vector_shift_builtin(expr, variant, scope);
	case GnuBuiltin::VectorShuffle:
		return vector_shuffle_builtin(expr, variant, scope);
	case GnuBuiltin::OperatorNew:
	case GnuBuiltin::OperatorDelete:
		return allocation_builtin(expr, builtin->gnu_builtin == GnuBuiltin::OperatorNew, scope);
	case GnuBuiltin::SourceLocation:
		return source_location_builtin(expr, variant, scope);
	case GnuBuiltin::Atomic:
	{
		// The lock-free queries follow the operations (and share the
		// first's kind, unused for a constant).
		const GnuAtomicOperation& operation = kGnuAtomicOperations[variant];
		const Intrinsic kind = operation.result == 'C'
			? Intrinsic::AtomicLoadN
			: static_cast<Intrinsic>(static_cast<uint8_t>(Intrinsic::AtomicLoadN) + variant);
		return atomic_builtin(
			expr, AtomicForm{ kind, operation.roles, operation.result, 0, 0xff, false, false }, scope);
	}
	case GnuBuiltin::Sync:
	{
		const GnuSyncOperation& operation = kGnuSyncOperations[variant];
		const Intrinsic kind = operation.atomic == kGnuSyncCompareSwap
			? (operation.result == 'B' ? Intrinsic::SyncBoolCompareSwap : Intrinsic::SyncValCompareSwap)
			: static_cast<Intrinsic>(static_cast<uint8_t>(Intrinsic::AtomicLoadN) + operation.atomic);
		return atomic_builtin(expr,
			AtomicForm{
				kind, operation.roles, operation.result, static_cast<uint8_t>(operation.order + 1), 0xff, true, false },
			scope);
	}
	case GnuBuiltin::C11Atomic:
	{
		const GnuC11Operation& operation = kGnuC11Operations[variant];
		const Intrinsic kind = operation.result == 'C'
			? Intrinsic::AtomicLoadN
			: static_cast<Intrinsic>(static_cast<uint8_t>(Intrinsic::AtomicLoadN) + operation.atomic);
		return atomic_builtin(expr,
			AtomicForm{ kind, operation.roles, operation.result, operation.implied, operation.weak, false, true },
			scope);
	}
	default:
		return gnu_intrinsic(expr, builtin->gnu_builtin, variant, scope);
	}
}

ExprFacts* Analyzer::constant_p_call(const syntax::CallExpr& expr, Scope* scope)
{
	// GCC's __builtin_constant_p (the course-supported query): an int, 1
	// when its operand is an integral constant expression, or a floating,
	// null pointer or string literal address constant (both host
	// compilers; another object's address is not), else 0; the operand is
	// not evaluated.
	if (expr.arguments.size != 1)
		error(expr.location, "__builtin_constant_p takes one argument");
	const syntax::Expr& operand = *expr.arguments[0];
	++unevaluated_;
	const ExprFacts* typed_operand = typed_value(operand, scope);
	--unevaluated_;
	if (typed_operand == nullptr)
		return nullptr;
	Constant value;
	bool constant = constant_value(operand, scope, value, false);
	const Type* type = typed_operand->type->unqualified;
	if (!constant && semantic() && template_depth_ == 0 &&
		(IsFloating(type) || type->kind == TypeKind::Pointer || type->kind == TypeKind::Array ||
			(type->kind == TypeKind::Fundamental && type->fundamental == FundamentalType::NullptrT)))
	{
		ConstantInterpreter interpreter(*this);
		ConstValue result;
		constant = interpreter.evaluate(operand, false, result) &&
			(result.kind == ValueKind::Floating ||
				(result.kind == ValueKind::Pointer &&
					(result.root == RootKind::Null || result.root == RootKind::Literal)));
	}
	ExprFacts* facts = new_facts(expr, types_.fundamental(FundamentalType::Int), ValueCategory::PRValue);
	facts->flags |= kFolded;
	facts->value = constant ? 1u : 0u;
	return facts;
}

ExprFacts* Analyzer::object_size_call(const syntax::CallExpr& expr, Scope* scope)
{
	// GCC's __builtin_object_size(ptr, type) and
	// __builtin_dynamic_object_size: a size_t, the bytes from ptr to the
	// end of the object it points to (type 0 or 1: at most, 2 or 3: at
	// least), which GCC documents as (size_t)-1 for types 0 and 1 and 0 for
	// types 2 and 3 when the object is not determined. This compiler never
	// determines it; ptr is not evaluated, type is an integral constant
	// expression from 0 to 3, and the call is no constant expression
	// (Intrinsic::ObjectSize). Libstdc++'s operator>>(istream&, char*) asks
	// it under __OPTIMIZE__.
	if (expr.arguments.size != 2)
		error(expr.location, "__builtin_object_size takes two arguments");
	++unevaluated_;
	const ExprFacts* pointer = typed_value(*expr.arguments[0], scope);
	--unevaluated_;
	if (pointer == nullptr || typed_value(*expr.arguments[1], scope) == nullptr)
		return nullptr;
	const Type* type = pointer->type->unqualified;
	if (!dependent(type) && type->kind != TypeKind::Pointer && type->kind != TypeKind::Array &&
		!(type->kind == TypeKind::Fundamental && type->fundamental == FundamentalType::NullptrT))
		error(expr.arguments[0]->location, "the first argument of __builtin_object_size is no pointer");
	Constant kind = Constant();
	const bool constant = constant_value(*expr.arguments[1], scope, kind, false);
	if (constant && kind.bits > 3)
		error(expr.arguments[1]->location, "the second argument of __builtin_object_size is not from 0 to 3");
	if (!constant && template_depth_ == 0)
		error(expr.arguments[1]->location,
			"the second argument of __builtin_object_size is no integral constant expression");
	ExprFacts* facts = new_facts(expr, types_.fundamental(FundamentalType::UnsignedLongInt), ValueCategory::PRValue);
	facts->flags |= kIntrinsic;
	facts->value = static_cast<uint64_t>(Intrinsic::ObjectSize) | ((kind.bits & 2) != 0 ? 1u << 8 : 0u);
	return facts;
}

ExprFacts* Analyzer::floating_builtin(const syntax::CallExpr& expr, GnuBuiltin builtin, uint8_t variant, Scope* scope)
{
	const uint32_t count = expr.arguments.size;
	const Type* integer = types_.fundamental(FundamentalType::Int);
	if (builtin == GnuBuiltin::FloatConstant)
	{
		// GCC's __builtin_nan[f|l] and __builtin_nans[f|l] (const char*: a
		// quiet or signaling NaN with the payload a string literal spells,
		// sema/string_literals.h), __builtin_inf[f|l]() and
		// __builtin_huge_val[f|l](): constants of their floating type.
		const uint32_t kind = variant / 3;
		const uint32_t arguments = kind == 2 ? 0 : 1;
		if (count != arguments)
			error(expr.location, "wrong number of arguments to a floating constant builtin");
		if (arguments == 1)
		{
			const Type* text = types_.pointer(types_.qualified(types_.fundamental(FundamentalType::Char), kConst));
			if (typed_value(*expr.arguments[0], scope) == nullptr)
				return nullptr;
			// GCC folds a string literal spelling a payload (its real_nan);
			// another operand is read at run time by the C library's nan (a
			// call, no constant). The C library has no nans: GCC and Clang
			// call one no library defines, an error here.
			const syntax::Expr* operand = expr.arguments[0];
			while (operand->kind == syntax::ExprKind::Paren)
				operand = static_cast<const syntax::ParenExpr*>(operand)->inner;
			uint64_t payload = 0;
			const bool folded = operand->kind == syntax::ExprKind::Literal &&
				operand->facts->type->kind == TypeKind::Array && (operand->facts->flags & kUserLiteral) == 0 &&
				BuiltinNanText(*operand, payload);
			if (kind == 0 && !folded)
				return library_nan_call(expr, static_cast<uint8_t>(variant % 3), scope);
			if (kind == 1 && !folded)
				error(expr.arguments[0]->location, "__builtin_nans of no string literal spelling a payload");
			convert(*expr.arguments[0], text, false);
		}
		ExprFacts* facts = new_facts(expr, types_.fundamental(FloatingOf(variant % 3)), ValueCategory::PRValue);
		facts->flags |= kIntrinsic;
		facts->value = static_cast<uint64_t>(kind == 0 ? Intrinsic::Nan
				: kind == 1                            ? Intrinsic::NanSignaling
													   : Intrinsic::Inf);
		return facts;
	}
	if (builtin == GnuBuiltin::FloatCompare)
	{
		// GCC's quiet comparisons (C99 7.12.14): two arithmetic operands of
		// a floating common type (5p10), an int.
		if (count != 2)
			error(expr.location, "wrong number of arguments to a floating comparison builtin");
		ExprFacts* left = typed_value(*expr.arguments[0], scope);
		ExprFacts* right = left != nullptr ? typed_value(*expr.arguments[1], scope) : nullptr;
		if (right == nullptr)
			return nullptr;
		const Type* common = IsArithmetic(left->type) && IsArithmetic(right->type)
			? usual_arithmetic(left->type->unqualified, right->type->unqualified)
			: nullptr;
		if (common == nullptr || !IsFloating(common))
			error(expr.location, "a floating comparison of operands of no floating common type");
		convert_operand(*expr.arguments[0], common);
		convert_operand(*expr.arguments[1], common);
		ExprFacts* facts = new_facts(expr, integer, ValueCategory::PRValue);
		facts->flags |= kIntrinsic;
		facts->value = static_cast<uint64_t>(Intrinsic::IsGreater) + variant;
		return facts;
	}
	// GCC's type-generic floating classification: __builtin_isfinite,
	// __builtin_isinf, __builtin_isnan, __builtin_isnormal,
	// __builtin_signbit, __builtin_isinf_sign and __builtin_issignaling
	// (x), and
	// __builtin_fpclassify(nan, infinite, normal, subnormal, zero, x), each
	// an int; the classified operand of floating type, the others int.
	static const Intrinsic kClassifications[] = { Intrinsic::IsFinite, Intrinsic::IsInf, Intrinsic::IsNan,
		Intrinsic::IsNormal, Intrinsic::FpClassify, Intrinsic::SignBit, Intrinsic::IsInfSign, Intrinsic::IsSignaling };
	const Intrinsic kind = kClassifications[variant];
	if (count != (kind == Intrinsic::FpClassify ? 6u : 1u))
		error(expr.location, "wrong number of arguments to a floating classification builtin");
	for (uint32_t i = 0; i < count; ++i)
	{
		ExprFacts* facts = typed_value(*expr.arguments[i], scope);
		if (facts == nullptr)
			return nullptr;
		if (i + 1 < count)
			convert(*expr.arguments[i], integer, false);
		else if (!IsFloating(facts->type))
			error(expr.arguments[i]->location, "a floating classification of a non-floating operand");
		else
			convert_operand(*expr.arguments[i], facts->type->unqualified);
	}
	ExprFacts* facts = new_facts(expr, integer, ValueCategory::PRValue);
	facts->flags |= kIntrinsic;
	facts->value = static_cast<uint64_t>(kind);
	return facts;
}

ExprFacts* Analyzer::gnu_intrinsic(const syntax::CallExpr& expr, GnuBuiltin builtin, uint8_t variant, Scope* scope)
{
	// The GCC builtins hosted headers call that are no library functions
	// (see Intrinsic).
	const uint32_t count = expr.arguments.size;
	const Type* int_type = types_.fundamental(FundamentalType::Int);
	const Type* void_type = types_.fundamental(FundamentalType::Void);
	const Type* size_type = types_.fundamental(FundamentalType::UnsignedLongInt);
	ExprFacts* facts = nullptr;
	switch (builtin)
	{
	case GnuBuiltin::AddressOf:
	{
		if (count != 1)
			error(expr.location, "__builtin_addressof takes one argument");
		const ExprFacts* operand = typed_value(*expr.arguments[0], scope);
		if (operand == nullptr)
			return nullptr;
		if (operand->category != ValueCategory::LValue)
		{
			substitution_error(expr.location, "__builtin_addressof of something that is no lvalue");
			return nullptr;
		}
		facts = new_facts(expr, types_.pointer(operand->type), ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(Intrinsic::AddressOf);
		break;
	}
	case GnuBuiltin::BitCount:
	{
		// The operand's unsigned type by the name's suffix: bswap16/32/64,
		// clz (unsigned int), clzl (unsigned long), clzll (unsigned long
		// long), likewise ctz and popcount; ffs uses the signed types.
		static const Intrinsic kKinds[] = { Intrinsic::ByteSwap, Intrinsic::CountLeadingZeros,
			Intrinsic::CountTrailingZeros, Intrinsic::PopCount, Intrinsic::FindFirstSet };
		static const FundamentalType kSwapped[] = { FundamentalType::UnsignedShortInt, FundamentalType::UnsignedInt,
			FundamentalType::UnsignedLongInt };
		static const FundamentalType kCounted[] = { FundamentalType::UnsignedInt, FundamentalType::UnsignedLongInt,
			FundamentalType::UnsignedLongLongInt };
		static const FundamentalType kFirstSet[] = { FundamentalType::Int, FundamentalType::LongInt,
			FundamentalType::LongLongInt };
		const Intrinsic kind = kKinds[variant / 3];
		if (count != 1)
			error(expr.location, "wrong number of arguments to a bit builtin");
		if (typed_value(*expr.arguments[0], scope) == nullptr)
			return nullptr;
		const Type* type = types_.fundamental(kind == Intrinsic::ByteSwap ? kSwapped[variant % 3]
				: kind == Intrinsic::FindFirstSet                         ? kFirstSet[variant % 3]
																		  : kCounted[variant % 3]);
		convert(*expr.arguments[0], type, false);
		facts = new_facts(expr, kind == Intrinsic::ByteSwap ? type : int_type, ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(kind);
		break;
	}
	case GnuBuiltin::GenericBitCount:
	{
		// GCC 14's type-generic clzg, ctzg and popcountg: an operand of an
		// unsigned integer type (no bool or character type), the count an
		// int; clzg and ctzg take the int they give for 0 second (an int
		// after promotion, both host compilers).
		const bool fallback = variant != 2 && count == 2;
		if (count != 1 && !fallback)
			error(expr.location, "wrong number of arguments to a generic bit builtin");
		const ExprFacts* operand = typed_value(*expr.arguments[0], scope);
		if (operand == nullptr)
			return nullptr;
		const Type* type = operand->type->unqualified;
		IntegralKind kind;
		const FundamentalType fundamental = type->kind == TypeKind::Fundamental ? type->fundamental
																				: FundamentalType::Bool;
		if (!IsIntegralType(type) || !IntegralKindOf(type, kind) || kind.is_signed ||
			fundamental == FundamentalType::Bool || fundamental == FundamentalType::Char16T ||
			fundamental == FundamentalType::Char32T || fundamental == FundamentalType::WcharT)
			return substitution_error(
					   expr.location, "a generic bit builtin of a type that is no unsigned integer type"),
				   nullptr;
		convert_operand(*expr.arguments[0], type);
		if (fallback)
		{
			const ExprFacts* zero = typed_value(*expr.arguments[1], scope);
			if (zero == nullptr)
				return nullptr;
			const Type* zero_type = zero->type->unqualified;
			if (!IsIntegralOrUnscopedEnum(zero_type) || promoted(zero_type) != int_type)
				error(expr.arguments[1]->location, "a generic bit builtin's second operand is no int");
			convert(*expr.arguments[1], int_type, false);
		}
		facts = new_facts(expr, int_type, ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(Intrinsic::GenericCountLeadingZeros) + variant;
		break;
	}
	case GnuBuiltin::Alloca:
		if (count != 1)
			error(expr.location, "__builtin_alloca takes one argument");
		if (typed_value(*expr.arguments[0], scope) == nullptr)
			return nullptr;
		convert(*expr.arguments[0], size_type, false);
		facts = new_facts(expr, types_.pointer(void_type), ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(Intrinsic::Alloca);
		break;
	case GnuBuiltin::VaList:
	{
		// The argument lists are lvalues of __builtin_va_list (an array of
		// one __va_list_tag, read as its address); va_start's second
		// operand names the last parameter (ignored, as GCC's x86-64
		// expansion does). Only a variadic function's body starts one.
		const Intrinsic kind = variant == 0 ? Intrinsic::VaStart : variant == 1 ? Intrinsic::VaEnd : Intrinsic::VaCopy;
		if (count != (kind == Intrinsic::VaEnd ? 1u : 2u))
			error(expr.location, "wrong number of arguments to a variadic argument builtin");
		if (kind == Intrinsic::VaStart &&
			(body_ == nullptr || body_->function == nullptr || !body_->function->type->canonical->variadic))
			error(expr.location, "__builtin_va_start outside a variadic function");
		for (uint32_t i = 0; i < count; ++i)
		{
			const ExprFacts* operand = typed_value(*expr.arguments[i], scope);
			if (operand == nullptr)
				return nullptr;
			if (kind == Intrinsic::VaStart && i == 1)
				continue;
			const Type* type = operand->type;
			if (type->kind == TypeKind::Array)
				type = type->element;
			else if (type->kind == TypeKind::Pointer)
				type = type->element;
			if (ClassOf(type) == nullptr || type->unqualified->entity->name == nullptr ||
				std::strcmp(type->unqualified->entity->name->spelling, "__va_list_tag") != 0)
				error(expr.arguments[i]->location, "a variadic argument builtin's operand is no __builtin_va_list");
		}
		facts = new_facts(expr, void_type, ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(kind);
		break;
	}
	case GnuBuiltin::Prefetch:
	{
		// An address, then optionally rw (0 or 1) and locality (0 to 3),
		// integral constants; a hint only.
		if (count < 1 || count > 3)
			error(expr.location, "wrong number of arguments to __builtin_prefetch");
		const ExprFacts* address = typed_value(*expr.arguments[0], scope);
		if (address == nullptr)
			return nullptr;
		convert(*expr.arguments[0], types_.pointer(types_.qualified(void_type, kConst)), false);
		for (uint32_t i = 1; i < count; ++i)
		{
			Constant value;
			if (typed_value(*expr.arguments[i], scope) == nullptr)
				return nullptr;
			if (!integral_constant(*expr.arguments[i], scope, value) || value.bits > (i == 1 ? 1u : 3u))
				error(expr.arguments[i]->location, "__builtin_prefetch takes constant rw and locality operands");
			convert(*expr.arguments[i], int_type, false);
		}
		facts = new_facts(expr, void_type, ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(Intrinsic::Prefetch);
		break;
	}
	case GnuBuiltin::Launder:
	{
		if (count != 1)
			error(expr.location, "__builtin_launder takes one argument");
		ExprFacts* operand = typed_value(*expr.arguments[0], scope);
		if (operand == nullptr)
			return nullptr;
		const Type* type = decayed(operand);
		if (!dependent_type(type) &&
			(type->kind != TypeKind::Pointer || IsVoid(type->element) ||
				type->element->unqualified->kind == TypeKind::Function))
		{
			substitution_error(expr.location, "__builtin_launder requires a pointer to an object type");
			return nullptr;
		}
		convert(*expr.arguments[0], type, false);
		facts = new_facts(expr, type, ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(Intrinsic::Launder);
		break;
	}
	case GnuBuiltin::AssumeAligned:
	{
		// (const void* pointer, size_t alignment[, size_t offset]): the
		// pointer as void*, an alignment hint.
		if (count < 2 || count > 3)
			error(expr.location, "wrong number of arguments to __builtin_assume_aligned");
		for (uint32_t i = 0; i < count; ++i)
		{
			if (typed_value(*expr.arguments[i], scope) == nullptr)
				return nullptr;
			convert(
				*expr.arguments[i], i == 0 ? types_.pointer(types_.qualified(void_type, kConst)) : size_type, false);
		}
		facts = new_facts(expr, types_.pointer(void_type), ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(Intrinsic::AssumeAligned);
		break;
	}
	case GnuBuiltin::Complex:
	{
		// GCC: two operands of one floating type (no conversion between
		// them), the complex value of that type with those parts.
		if (count != 2)
			error(expr.location, "wrong number of arguments to __builtin_complex");
		const ExprFacts* real = typed_value(*expr.arguments[0], scope);
		const ExprFacts* imaginary = typed_value(*expr.arguments[1], scope);
		if (real == nullptr || imaginary == nullptr)
			return nullptr;
		const Type* type = real->type->unqualified;
		if (!IsFloating(type) || imaginary->type->unqualified != type)
			error(expr.location, "__builtin_complex of operands that are no two values of one floating type");
		for (uint32_t i = 0; i < count; ++i)
			convert(*expr.arguments[i], type, false);
		facts = new_facts(expr, types_.complex(type), ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(Intrinsic::MakeComplex);
		break;
	}
	case GnuBuiltin::Reduce:
	{
		// Clang: a vector operand, its lanes combined; max and min of
		// integral or floating lanes, the others of integral ones (and, or
		// and xor of bool ones too).
		if (count != 1)
			error(expr.location, "wrong number of arguments to a vector reduction");
		const ExprFacts* vector = typed_value(*expr.arguments[0], scope);
		if (vector == nullptr)
			return nullptr;
		const Type* type = vector->type->unqualified;
		const Type* lane = type->kind == TypeKind::Vector ? type->element->unqualified : nullptr;
		const bool bitwise = variant < 3;
		if (lane == nullptr || type->owner != nullptr || (variant < 5 && IsFloating(lane)) ||
			(!bitwise && lane->fundamental == FundamentalType::Bool))
			error(expr.location, "a vector reduction of no vector of suitable lanes");
		convert(*expr.arguments[0], type, false);
		facts = new_facts(expr, type->element->unqualified, ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(Intrinsic::ReduceAnd) + variant;
		break;
	}
	case GnuBuiltin::FltRounds:
		if (count != 0)
			error(expr.location, "__builtin_flt_rounds takes no arguments");
		facts = new_facts(expr, int_type, ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(Intrinsic::FltRounds);
		break;
	case GnuBuiltin::VectorTruncate:
	{
		if (count != 1)
			error(expr.location, "wrong number of arguments to a packed truncating conversion");
		if (typed_value(*expr.arguments[0], scope) == nullptr)
			return nullptr;
		convert(*expr.arguments[0], types_.vector(types_.fundamental(FundamentalType::Float), 4), false);
		facts = new_facts(expr, types_.vector(int_type, 4), ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(Intrinsic::VectorTruncate);
		break;
	}
	case GnuBuiltin::VectorByteMask:
	{
		if (count != 1)
			error(expr.location, "wrong number of arguments to a packed byte sign mask");
		if (typed_value(*expr.arguments[0], scope) == nullptr)
			return nullptr;
		convert(*expr.arguments[0], types_.vector(types_.fundamental(FundamentalType::Char), 16), false);
		facts = new_facts(expr, int_type, ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(Intrinsic::VectorByteMask);
		break;
	}
	case GnuBuiltin::VectorByteMinMax:
	{
		if (count != 2)
			error(expr.location, "wrong number of arguments to a packed byte minimum or maximum");
		const Type* type = types_.vector(types_.fundamental(FundamentalType::Char), 16);
		for (const syntax::Expr* argument : expr.arguments)
		{
			if (typed_value(*argument, scope) == nullptr)
				return nullptr;
			convert(*argument, type, false);
		}
		facts = new_facts(expr, type, ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(Intrinsic::VectorByteMinMax) | (static_cast<uint64_t>(variant) << 8);
		break;
	}
	case GnuBuiltin::VectorPermute:
		return vector_permute_builtin(expr, variant, scope);
	case GnuBuiltin::VectorPack:
	{
		if (count != 2)
			error(expr.location, "wrong number of arguments to a packed saturating conversion");
		const bool words = variant == 2;
		const Type* input = types_.vector(
			types_.fundamental(words ? FundamentalType::Int : FundamentalType::ShortInt), words ? 4 : 8);
		for (const syntax::Expr* argument : expr.arguments)
		{
			if (typed_value(*argument, scope) == nullptr)
				return nullptr;
			convert(*argument, input, false);
		}
		facts = new_facts(expr,
			types_.vector(
				types_.fundamental(words ? FundamentalType::ShortInt : FundamentalType::Char), words ? 8 : 16),
			ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(Intrinsic::VectorPack) | (static_cast<uint64_t>(variant) << 8);
		break;
	}
	case GnuBuiltin::VectorAndNot:
	{
		if (count != 2)
			error(expr.location, "wrong number of arguments to a packed bitwise and-not");
		const Type* type = types_.vector(types_.fundamental(FundamentalType::LongLongInt), 2);
		for (const syntax::Expr* argument : expr.arguments)
		{
			if (typed_value(*argument, scope) == nullptr)
				return nullptr;
			convert(*argument, type, false);
		}
		facts = new_facts(expr, type, ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(Intrinsic::VectorAndNot);
		break;
	}
	case GnuBuiltin::VectorUnpack:
	{
		if (count != 2)
			error(expr.location, "wrong number of arguments to a packed unpack operation");
		const FundamentalType lanes[] = { FundamentalType::Char, FundamentalType::ShortInt, FundamentalType::Int,
			FundamentalType::LongLongInt };
		const Type* type = types_.vector(types_.fundamental(lanes[variant / 2]), 16 >> (variant / 2));
		for (const syntax::Expr* argument : expr.arguments)
		{
			if (typed_value(*argument, scope) == nullptr)
				return nullptr;
			convert(*argument, type, false);
		}
		facts = new_facts(expr, type, ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(Intrinsic::VectorUnpack) | (static_cast<uint64_t>(variant) << 8);
		break;
	}
	case GnuBuiltin::VectorShiftLanes:
	{
		if (count != 2)
			error(expr.location, "wrong number of arguments to a packed lane shift");
		const FundamentalType lanes[] = { FundamentalType::ShortInt, FundamentalType::Int,
			FundamentalType::LongLongInt };
		const Type* type = types_.vector(types_.fundamental(lanes[variant / 3]), 8 >> (variant / 3));
		for (const syntax::Expr* argument : expr.arguments)
			if (typed_value(*argument, scope) == nullptr)
				return nullptr;
		convert(*expr.arguments[0], type, false);
		convert(*expr.arguments[1], int_type, false);
		facts = new_facts(expr, type, ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(Intrinsic::VectorShiftLanes) | (static_cast<uint64_t>(variant) << 8);
		break;
	}
	case GnuBuiltin::IsConstantEvaluated:
		if (count != 0)
			error(expr.location, "__builtin_is_constant_evaluated takes no arguments");
		facts = new_facts(expr, types_.fundamental(FundamentalType::Bool), ValueCategory::PRValue);
		facts->value = static_cast<uint64_t>(Intrinsic::IsConstantEvaluated);
		break;
	default:
		error(expr.location, "a builtin that is no function");
	}
	facts->flags |= kIntrinsic;
	return facts;
}

ExprFacts* Analyzer::vector_lane_builtin(const syntax::CallExpr& expr, uint8_t variant, Scope* scope)
{
	// GCC's types: init takes `lanes` scalars converted to the lane type;
	// ext and set a vector of the builtin's type (implicitly converted, as
	// any operand) and a selector that is an integral constant expression
	// below the lane count, set the new lane's value before it.
	const GnuVectorLaneOperation& operation = kGnuVectorLaneOperations[variant];
	FundamentalType fundamental = FundamentalType::Int;
	switch (operation.lane)
	{
	case 'a':
		fundamental = FundamentalType::SignedChar;
		break;
	case 'c':
		fundamental = FundamentalType::Char;
		break;
	case 's':
		fundamental = FundamentalType::ShortInt;
		break;
	case 'x':
		fundamental = FundamentalType::LongLongInt;
		break;
	case 'f':
		fundamental = FundamentalType::Float;
		break;
	case 'd':
		fundamental = FundamentalType::Double;
		break;
	default:
		break;
	}
	const Type* lane = types_.fundamental(fundamental);
	const Type* vector = types_.vector(lane, operation.lanes);
	const uint32_t count = expr.arguments.size;
	const uint32_t wanted = operation.kind == 'i' ? operation.lanes : operation.kind == 'e' ? 2 : 3;
	if (count != wanted)
		error(expr.location, "wrong number of arguments to a vector lane builtin");
	for (uint32_t i = 0; i < count; ++i)
	{
		if (typed_value(*expr.arguments[i], scope) == nullptr)
			return nullptr;
		const bool selector = operation.kind != 'i' && i == count - 1;
		convert(*expr.arguments[i],
			selector                              ? types_.fundamental(FundamentalType::Int)
				: operation.kind != 'i' && i == 0 ? vector
												  : lane,
			false);
	}
	uint64_t selected = 0;
	bool run_time = false;
	if (operation.kind != 'i')
	{
		Constant value;
		const syntax::Expr& selector = *expr.arguments[count - 1];
		if (constant_value(selector, scope, value, false))
		{
			if (static_cast<int64_t>(value.bits) < 0 || value.bits >= operation.lanes)
				error(selector.location, "a vector lane selector outside the vector");
			selected = value.bits;
		}
		else if (template_depth_ == 0)
		{
			// GCC's intrinsic wrappers at -O1 and above (<xmmintrin.h>'s
			// _mm_extract_pi16) pass their own parameter, which GCC's
			// mandatory inlining of an always_inline function makes a
			// constant: in such a body the selector is read at run time.
			if (body_ == nullptr || body_->function == nullptr || !body_->function->always_inline)
				error(selector.location, "a vector lane selector that is no integral constant expression");
			run_time = true;
		}
	}
	ExprFacts* facts = new_facts(expr, operation.kind == 'e' ? lane : vector, ValueCategory::PRValue);
	facts->flags |= kIntrinsic;
	const Intrinsic kind = operation.kind == 'i' ? Intrinsic::VectorInit
		: operation.kind == 'e'                  ? Intrinsic::VectorExtract
												 : Intrinsic::VectorInsert;
	facts->value = static_cast<uint64_t>(kind) | selected << 8 | (run_time ? uint64_t(1) << 16 : 0);
	return facts;
}

ExprFacts* Analyzer::vector_shift_builtin(const syntax::CallExpr& expr, uint8_t variant, Scope* scope)
{
	if (expr.arguments.size != 2)
		error(expr.location, "wrong number of arguments to a whole-register byte shift");
	const Type* vector = types_.vector(types_.fundamental(FundamentalType::LongLongInt), 2);
	for (uint32_t i = 0; i < 2; ++i)
	{
		if (typed_value(*expr.arguments[i], scope) == nullptr)
			return nullptr;
		convert(*expr.arguments[i], i == 0 ? vector : types_.fundamental(FundamentalType::Int), false);
	}
	Constant count;
	const bool constant = constant_value(*expr.arguments[1], scope, count, false);
	if (constant && (count.bits > 2040 || count.bits % 8 != 0))
		error(expr.arguments[1]->location, "a byte-shift bit count must be eight times an 8-bit immediate");
	if (!constant && template_depth_ == 0 &&
		(body_ == nullptr || body_->function == nullptr || !body_->function->always_inline))
		error(expr.arguments[1]->location, "a byte-shift count that is no integral constant expression");
	ExprFacts* facts = new_facts(expr, vector, ValueCategory::PRValue);
	facts->flags |= kIntrinsic;
	const Intrinsic kind = variant == 0 ? Intrinsic::VectorShiftLeftBytes : Intrinsic::VectorShiftRightBytes;
	facts->value = static_cast<uint64_t>(kind) | (constant ? count.bits / 8 << 8 : uint64_t(1) << 16);
	return facts;
}

ExprFacts* Analyzer::vector_permute_builtin(const syntax::CallExpr& expr, uint8_t variant, Scope* scope)
{
	if (variant != 0)
		return vector_index_shuffle_builtin(expr, scope);
	const uint32_t count = expr.arguments.size;
	if (count != 2 && count != 3)
		error(expr.location, "a vector permutation takes one or two vectors and a mask");
	const Type* arguments[3] = {};
	for (uint32_t i = 0; i < count; ++i)
	{
		const ExprFacts* value = typed_value(*expr.arguments[i], scope);
		if (value == nullptr)
			return nullptr;
		arguments[i] = value->type->unqualified;
	}
	const Type* vector = arguments[0];
	const Type* mask = arguments[count - 1];
	if (vector->kind != TypeKind::Vector || vector->element->unqualified->fundamental == FundamentalType::Bool ||
		(vector->bound & (vector->bound - 1)) != 0 || (count == 3 && vector->canonical != arguments[1]->canonical) ||
		mask->kind != TypeKind::Vector || mask->bound != vector->bound || !IsIntegralOrUnscopedEnum(mask->element))
		error(expr.location, "a vector permutation requires matching vectors and an integral lane mask");
	uint64_t bytes = 0, mask_bytes = 0, alignment = 0;
	if (!type_size(vector, bytes, alignment, expr.location) || !type_size(mask, mask_bytes, alignment, expr.location) ||
		bytes != mask_bytes)
		error(expr.location, "a vector permutation mask has another width than its input");
	for (uint32_t i = 0; i < count; ++i)
		convert(*expr.arguments[i], arguments[i], false);
	ExprFacts* facts = new_facts(expr, vector, ValueCategory::PRValue);
	facts->value = static_cast<uint64_t>(Intrinsic::VectorPermute);
	facts->flags |= kIntrinsic;
	return facts;
}

ExprFacts* Analyzer::vector_index_shuffle_builtin(const syntax::CallExpr& expr, Scope* scope)
{
	if (expr.arguments.size < 3)
		error(expr.location, "an indexed vector shuffle takes two vectors and constant indexes");
	const ExprFacts* first = typed_value(*expr.arguments[0], scope);
	const ExprFacts* second = typed_value(*expr.arguments[1], scope);
	if (first == nullptr || second == nullptr)
		return nullptr;
	const Type* input = first->type->unqualified;
	if (input->kind != TypeKind::Vector || input->canonical != second->type->unqualified->canonical ||
		input->element->unqualified->fundamental == FundamentalType::Bool)
		error(expr.location, "an indexed vector shuffle requires two matching vectors");
	for (uint32_t i = 2; i < expr.arguments.size; ++i)
	{
		if (typed_value(*expr.arguments[i], scope) == nullptr)
			return nullptr;
		Constant index;
		if (!integral_constant(*expr.arguments[i], scope, index) ||
			(index.bits >= 2 * input->bound && !(IsSignedConstant(index) && index.bits == ~uint64_t(0))))
			error(expr.arguments[i]->location, "an indexed vector shuffle selector is no constant index or -1");
		// Keep each proven constant on its existing typed expression. The
		// lowering consumes it directly, without re-evaluating the source.
		ExprFacts* selected = facts_of(*expr.arguments[i]);
		selected->flags |= kFolded;
		selected->value = index.bits;
	}
	convert(*expr.arguments[0], input, false);
	convert(*expr.arguments[1], input, false);
	const Type* output = checked_vector(input->element, expr.arguments.size - 2, false, expr.location);
	if (output == nullptr)
		return nullptr;
	ExprFacts* facts = new_facts(expr, output, ValueCategory::PRValue);
	facts->flags |= kIntrinsic;
	facts->value = static_cast<uint64_t>(Intrinsic::VectorPermute) | uint64_t(1) << 8;
	return facts;
}

ExprFacts* Analyzer::vector_shuffle_builtin(const syntax::CallExpr& expr, uint8_t variant, Scope* scope)
{
	const bool doubles = variant == 1;
	const bool words = variant == 2 || variant == 3;
	const bool floats = variant == 4;
	const uint32_t count = doubles || floats ? 3 : 2;
	if (expr.arguments.size != count)
		error(expr.location, "wrong number of arguments to a packed lane shuffle");
	const Type* vector = types_.vector(types_.fundamental(words ? FundamentalType::ShortInt
											   : doubles        ? FundamentalType::Double
											   : floats         ? FundamentalType::Float
																: FundamentalType::Int),
		words         ? 8
			: doubles ? 2
					  : 4);
	for (uint32_t i = 0; i < count; ++i)
	{
		if (typed_value(*expr.arguments[i], scope) == nullptr)
			return nullptr;
		convert(*expr.arguments[i], i == count - 1 ? types_.fundamental(FundamentalType::Int) : vector, false);
	}
	Constant selector;
	const bool constant = constant_value(*expr.arguments[count - 1], scope, selector, false);
	if (!constant && template_depth_ == 0 &&
		(body_ == nullptr || body_->function == nullptr || !body_->function->always_inline))
		error(expr.arguments[count - 1]->location,
			"a packed lane shuffle selector that is no integral constant expression");
	ExprFacts* facts = new_facts(expr, vector, ValueCategory::PRValue);
	facts->flags |= kIntrinsic;
	facts->value = static_cast<uint64_t>(Intrinsic::VectorShuffle) |
		(constant ? (selector.bits & (doubles ? 3U : 255U)) << 8 : uint64_t(1) << 16) |
		(variant == 3 ? uint64_t(1) << 17 : 0);
	return facts;
}

ExprFacts* Analyzer::overflow_builtin(const syntax::CallExpr& expr, uint8_t variant, Scope* scope)
{
	// GCC's __builtin_add/sub/mul_overflow(a, b, r): a and b of integral
	// or unscoped enumeration type (bool too; both host compilers), promoted,
	// and r a pointer to a non-const integral object (no bool or
	// enumeration); a bool.
	if (expr.arguments.size != 3)
		error(expr.location, "wrong number of arguments to an overflow builtin");
	for (uint32_t i = 0; i < 3; ++i)
	{
		const ExprFacts* operand = typed_value(*expr.arguments[i], scope);
		if (operand == nullptr)
			return nullptr;
		const Type* type = operand->type->unqualified;
		if (i == 2)
		{
			const Type* pointer = type->kind == TypeKind::Array ? types_.pointer(type->element) : type;
			const Type* result = pointer->kind == TypeKind::Pointer ? pointer->element : nullptr;
			if (result == nullptr || (result->cv & kConst) != 0 || !IsIntegralType(result->unqualified) ||
				result->unqualified->fundamental == FundamentalType::Bool)
				error(
					expr.arguments[i]->location, "an overflow builtin's result is no pointer to a modifiable integer");
			convert_operand(*expr.arguments[i], pointer);
			continue;
		}
		if (!IsIntegralOrUnscopedEnum(type))
			error(expr.arguments[i]->location, "an overflow builtin's operand is no integer");
		convert_operand(*expr.arguments[i], promoted(type));
	}
	ExprFacts* facts = new_facts(expr, types_.fundamental(FundamentalType::Bool), ValueCategory::PRValue);
	facts->flags |= kIntrinsic;
	facts->value = static_cast<uint64_t>(Intrinsic::AddOverflow) + variant;
	return facts;
}

ExprFacts* Analyzer::allocation_builtin(const syntax::CallExpr& expr, bool allocate, Scope* scope)
{
	// Clang's __builtin_operator_new and __builtin_operator_delete: a call
	// of the global allocation or deallocation function its arguments
	// select (3.7.4), as `::operator new(args)` is; the callee designates
	// the selected function.
	for (const syntax::Expr* argument : expr.arguments)
	{
		if (typed(*argument, scope) == nullptr || failed())
			return nullptr;
	}
	const NameEntry* entry = names_.find(
		graph_.global, operator_key(allocate ? TokenKind::KW_NEW : TokenKind::KW_DELETE));
	expression_scope_ = scope;
	adl_functions_.clear();
	return resolved_call(expr, entry != nullptr ? entry->functions : nullptr, nullptr, false, nullptr, false);
}

ExprFacts* Analyzer::library_nan_call(const syntax::CallExpr& expr, uint8_t variant, Scope* scope)
{
	// The C library function of the builtin's signature (C99 7.12.11.2),
	// declared nowhere: the call selects it alone. Its object name is the
	// builtin's without `__builtin_` (lower/symbol_lowering.cpp).
	static const char* const kNames[] = { "__builtin_nan", "__builtin_nanf", "__builtin_nanl" };
	static const char* const kSignatures[] = { "dCc*", "fCc*", "eCc*" };
	FunctionEntity*& function = library_nans_[variant];
	if (function == nullptr)
	{
		const Identifier* name = special_key(kNames[variant]);
		const Type* type = builtin_signature(kSignatures[variant], 0);
		function = new_entity<FunctionEntity>(EntityKind::Function, name, graph_.global, type);
		function->label = name->spelling;
		function->signature = signature_of(type);
		function->builtin = BuiltinFunction::Library;
		function->gnu_builtin = GnuBuiltin::Library;
		function->nothrow = true;
		function->c_linkage = true;
	}
	expression_scope_ = scope;
	adl_functions_.clear();
	adl_functions_.push_back(function);
	return resolved_call(expr, nullptr, nullptr, false, nullptr, false);
}

ExprFacts* Analyzer::source_location_builtin(const syntax::CallExpr& expr, uint8_t variant, Scope* scope)
{
	// GCC's __builtin_FILE() and __builtin_FUNCTION() (const char*),
	// __builtin_LINE() and __builtin_COLUMN() (int): typed here, their
	// values decided where they are evaluated (see Intrinsic::SourceFile),
	// which for a default argument is each call using it.
	(void)scope;
	if (expr.arguments.size != 0)
		error(expr.location, "a source location builtin takes no arguments");
	const bool text = variant == 0 || variant == 2;
	ExprFacts* facts = new_facts(expr,
		text ? types_.pointer(types_.qualified(types_.fundamental(FundamentalType::Char), kConst))
			 : types_.fundamental(FundamentalType::Int),
		ValueCategory::PRValue);
	facts->flags |= kIntrinsic;
	facts->value = static_cast<uint64_t>(Intrinsic::SourceFile) + variant;
	return facts;
}

uint64_t Analyzer::source_line(uint32_t location) const
{
	return positions_ != nullptr ? positions_->line_of(location) : 0;
}

std::string Analyzer::source_file(uint32_t location) const
{
	return positions_ != nullptr ? positions_->file_of(location) : std::string();
}

const syntax::LiteralExpr* Analyzer::text_literal(const std::string& text)
{
	// A constant evaluation's string object of these characters (a source
	// location builtin's): an ordinary string literal of the analysis's
	// own, one per distinct text, spelled with `"`, `\` and control
	// characters escaped.
	const Identifier* key = identifiers_.intern(text.data(), static_cast<uint32_t>(text.size()));
	const uint32_t found = text_literal_index_.find(key);
	if (found != PointerMap::kMissing)
		return text_literals_[found];
	std::string spelling(1, '"');
	for (char c : text)
	{
		const unsigned char byte = static_cast<unsigned char>(c);
		if (c == '"' || c == '\\')
		{
			spelling.push_back('\\');
			spelling.push_back(c);
		}
		else if (byte < 0x20 || byte == 0x7F)
		{
			const char octal[] = { '\\', static_cast<char>('0' + (byte >> 6)),
				static_cast<char>('0' + ((byte >> 3) & 7)), static_cast<char>('0' + (byte & 7)) };
			spelling.append(octal, sizeof octal);
		}
		else
			spelling.push_back(c);
	}
	spelling.push_back('"');
	syntax::LiteralExpr* literal = static_cast<syntax::LiteralExpr*>(
		arena_.allocate(sizeof(syntax::LiteralExpr), alignof(syntax::LiteralExpr)));
	*literal = syntax::LiteralExpr();
	literal->kind = syntax::ExprKind::Literal;
	literal->spelling = TokenSpelling{ copy_label(spelling.data(), spelling.size()),
		static_cast<uint32_t>(spelling.size()) };
	if (typed(*literal, graph_.global) == nullptr)
		return nullptr;
	text_literal_index_.set(key, static_cast<uint32_t>(text_literals_.size()));
	text_literals_.push_back(literal);
	return literal;
}

ExprFacts* Analyzer::atomic_builtin(const syntax::CallExpr& expr, const AtomicForm& form, Scope* scope)
{
	const Intrinsic kind = form.kind;
	const char* roles = form.roles;
	const char result_kind = form.result;
	// GCC's __atomic builtins ("Built-in Functions for Memory Model Aware
	// Atomic Operations") and its legacy __sync ones (which imply their
	// memory order). Operand roles (lex/gnu_builtins.h): p the object's
	// address (an integral or pointer type of 1, 2, 4 or 8 bytes,
	// arithmetic on a pointer unscaled; for the generic load, store,
	// exchange and compare_exchange any object type of those sizes, its
	// bytes as the integer of that size, as PA27's inline forms;
	// other sizes use the generic runtime ABI), v a value of its
	// type, r a pointer to one (the generic forms: of the object's size;
	// compare_exchange_n's expected), b bool (weak), o a memory order
	// (int), s a size; the result is the object's type (T), void (-), bool
	// (B) or a constant (C: __atomic_always_lock_free /
	// __atomic_is_lock_free). Only the loads leave the object unwritten,
	// and only the sources (a store's, an exchange's value, a
	// compare_exchange's desired) may point to const (GCC, Clang). A
	// __sync builtin's trailing operands, GCC's list of protected
	// variables, are ignored; a __c11_atomic builtin's object is _Atomic
	// (its atomic qualification read through).
	const uint32_t needed = static_cast<uint32_t>(std::strlen(roles));
	if (!form.trailing ? expr.arguments.size != needed : expr.arguments.size < needed)
	{
		substitution_error(expr.location, "wrong number of arguments to an atomic builtin");
		return nullptr;
	}
	const bool byte = kind == Intrinsic::AtomicTestAndSet || kind == Intrinsic::AtomicClear;
	const bool arithmetic_kind = kind >= Intrinsic::AtomicAddFetch && kind <= Intrinsic::AtomicFetchNand;
	const bool indirect_generic = kind == Intrinsic::AtomicLoad || kind == Intrinsic::AtomicStore ||
		kind == Intrinsic::AtomicExchange || kind == Intrinsic::AtomicCompareExchange;
	const bool generic = indirect_generic || (form.any_object && !arithmetic_kind);
	bool runtime_object = false;
	const bool loads = kind == Intrinsic::AtomicLoadN || kind == Intrinsic::AtomicLoad;
	const Type* object = nullptr;
	uint64_t size = 0, alignment = 0;
	for (uint32_t i = 0; i < expr.arguments.size; ++i)
	{
		const syntax::Expr& argument = *expr.arguments[i];
		const ExprFacts* operand = typed_value(argument, scope);
		if (operand == nullptr)
			return nullptr;
		if (i >= needed)
			continue;
		const char role = roles[i];
		if (role == 'p' || role == 'r')
		{
			const Type* pointer = operand->type->unqualified;
			if (pointer->kind == TypeKind::Array)
				pointer = types_.pointer(pointer->element);
			if (pointer->kind != TypeKind::Pointer && result_kind != 'C')
			{
				substitution_error(argument.location, "an atomic builtin's operand is no pointer");
				return nullptr;
			}
			const bool destination = role == 'p'
				? !loads
				: (kind == Intrinsic::AtomicLoad && i == 1) || (kind == Intrinsic::AtomicExchange && i == 2) ||
					((kind == Intrinsic::AtomicCompareExchange || kind == Intrinsic::AtomicCompareExchangeN) && i == 1);
			if (result_kind != 'C' && destination && (pointer->element->cv & kConst) != 0)
			{
				substitution_error(argument.location, "an atomic builtin writing through a pointer to const");
				return nullptr;
			}
			if (role == 'p' && !byte)
			{
				// A __c11_atomic builtin's object is _Atomic; its arithmetic
				// is an integer's, or a pointer to an object type's addition
				// and subtraction (Clang).
				if (form.any_object && result_kind != 'C' && (pointer->element->cv & kAtomic) == 0)
				{
					substitution_error(argument.location, "a __c11_atomic builtin on an object of no atomic type");
					return nullptr;
				}
				const Type* pointee = pointer->element->unqualified->kind == TypeKind::Pointer
					? pointer->element->unqualified->element->unqualified
					: nullptr;
				if (form.any_object && arithmetic_kind && pointee != nullptr &&
					(IsVoid(pointee) || pointee->kind == TypeKind::Function ||
						(kind != Intrinsic::AtomicFetchAdd && kind != Intrinsic::AtomicFetchSub)))
				{
					substitution_error(argument.location,
						"__c11_atomic arithmetic on a pointer other than an object pointer's addition or subtraction");
					return nullptr;
				}
				object = pointer->element->unqualified;
				const bool sized = type_size(object, size, alignment, expr.location);
				const bool scalar_size = size == 1 || size == 2 || size == 4 || size == 8;
				if (!sized || (!scalar_size && !indirect_generic) ||
					(!generic && !(IsIntegralOrUnscopedEnum(object) || object->kind == TypeKind::Pointer)))
				{
					substitution_error(expr.location,
						generic
							? "a generic atomic builtin on an incomplete or unsupported object type"
							: "an atomic builtin on an object of no integral or pointer type of 1, 2, 4 or 8 bytes");
					return nullptr;
				}
				runtime_object = !scalar_size;
			}
			// The generic forms' other objects are of the object's size.
			uint64_t other = 0, other_alignment = 0;
			if (role == 'r' && generic && object != nullptr &&
				(!type_size(pointer->element->unqualified, other, other_alignment, argument.location) || other != size))
			{
				substitution_error(
					argument.location, "a generic atomic builtin's operand of another size than the object");
				return nullptr;
			}
			convert_operand(argument, pointer->kind == TypeKind::Pointer ? pointer : operand->type->unqualified);
			continue;
		}
		FundamentalType type = role == 'b' ? FundamentalType::Bool
			: role == 's'                  ? FundamentalType::UnsignedLongInt
			: role == 'o'                  ? FundamentalType::Int
										   : FundamentalType::Void;
		// An update of a pointer adds or combines an integer (as uintptr_t).
		const bool arithmetic = kind >= Intrinsic::AtomicAddFetch && kind <= Intrinsic::AtomicFetchNand;
		if (type == FundamentalType::Void && object != nullptr && arithmetic && object->kind == TypeKind::Pointer)
			type = FundamentalType::LongInt;
		if (type != FundamentalType::Void)
			convert(argument, types_.fundamental(type), false);
		else if (object != nullptr)
			convert(argument, object, false);
	}
	if (result_kind == 'C')
	{
		// Lock-free for the sizes the x86-64 instructions cover.
		Constant constant;
		if (!integral_constant(*expr.arguments[0], scope, constant))
		{
			substitution_error(expr.location, "__atomic_always_lock_free of a size that is no constant");
			return nullptr;
		}
		// An _Atomic object's size is rounded up to a power of two (see
		// type_size), so __c11_atomic_is_lock_free answers for that.
		ExprFacts* facts = new_facts(expr, types_.fundamental(FundamentalType::Bool), ValueCategory::PRValue);
		facts->flags |= kFolded;
		facts->value = form.any_object
			? constant.bits != 0 && constant.bits <= 8
			: constant.bits == 1 || constant.bits == 2 || constant.bits == 4 || constant.bits == 8;
		return facts;
	}
	const Type* result = result_kind == 'T' ? object
		: result_kind == 'B'                ? types_.fundamental(FundamentalType::Bool)
											: types_.fundamental(FundamentalType::Void);
	if (form.weak != 0xff)
	{
		// The weakness the lowering reads as compare_exchange_n's fourth
		// operand, inserted as a literal.
		syntax::Expr** operands = static_cast<syntax::Expr**>(
			arena_.allocate(sizeof(syntax::Expr*) * 6, alignof(syntax::Expr*)));
		syntax::KeywordLiteralExpr* weak = static_cast<syntax::KeywordLiteralExpr*>(
			arena_.allocate(sizeof(syntax::KeywordLiteralExpr), alignof(syntax::KeywordLiteralExpr)));
		*weak = syntax::KeywordLiteralExpr();
		weak->kind = syntax::ExprKind::KeywordLiteral;
		weak->location = expr.location;
		weak->keyword = form.weak != 0 ? TokenKind::KW_TRUE : TokenKind::KW_FALSE;
		if (typed(*weak, scope) == nullptr)
			return nullptr;
		operands[0] = expr.arguments[0];
		operands[1] = expr.arguments[1];
		operands[2] = expr.arguments[2];
		operands[3] = weak;
		operands[4] = expr.arguments[3];
		operands[5] = expr.arguments[4];
		const_cast<syntax::CallExpr&>(expr).arguments = syntax::Slice<syntax::Expr*>{ operands, 6 };
	}
	ExprFacts* facts = new_facts(expr, result, ValueCategory::PRValue);
	facts->flags |= kIntrinsic;
	facts->value = static_cast<uint64_t>(kind) | static_cast<uint64_t>(form.implied) << 8 |
		(runtime_object ? kAtomicRuntimeCall : 0);
	return facts;
}

ExprFacts* Analyzer::builtin_invoke(const syntax::CallExpr& expr, Scope* scope)
{
	// Clang's __builtin_invoke(f, t1, ..., tN), INVOKE (20.8.2p1): with a
	// pointer to member function of a class T, (t1.*f)(t2, ..., tN) when t1
	// is an object of T or of a class derived from T, else ((*t1).*f)(t2,
	// ..., tN) (t1->*f for a pointer); otherwise f(t1, ..., tN), f
	// parenthesized (no argument-dependent lookup: f is an operand); with a
	// pointer to data member and one object t1.*f, or (*t1).*f. The call
	// is rewritten in place into that call (as member_access rewrites a
	// class object's ->) or member access and analyzed as it, so later
	// phases see an ordinary call or access; f and t1, typed to choose the
	// form, are not typed again.
	struct Built
	{
		syntax::ParenExpr callee;
		syntax::BinaryExpr access;
		syntax::UnaryExpr indirection;
	};
	if (expr.arguments.empty())
	{
		substitution_error(expr.location, "__builtin_invoke without a callable operand");
		return nullptr;
	}
	syntax::Expr* callable = expr.arguments[0];
	const ExprFacts* function = typed(*callable, scope);
	if (function == nullptr || failed())
		return nullptr;
	Built* built = static_cast<Built*>(arena_.allocate(sizeof(Built), alignof(Built)));
	built->callee = syntax::ParenExpr();
	built->callee.kind = syntax::ExprKind::Paren;
	built->callee.location = callable->location;
	built->callee.inner = callable;
	syntax::CallExpr& rewritten = const_cast<syntax::CallExpr&>(expr);
	const Type* type = function->type != nullptr ? function->type->unqualified : nullptr;
	if (type == nullptr || type->kind != TypeKind::MemberPointer)
	{
		rewritten.callee = &built->callee;
		rewritten.arguments = syntax::Slice<syntax::Expr*>{ expr.arguments.data + 1, expr.arguments.size - 1 };
		return invoke_call(rewritten, callable, nullptr, scope);
	}
	const bool data = type->element->kind != TypeKind::Function;
	if (expr.arguments.size < 2 || (data && expr.arguments.size != 2))
	{
		substitution_error(expr.location,
			data ? "__builtin_invoke of a pointer to data member needs one object"
				 : "__builtin_invoke of a pointer to member function without an object");
		return nullptr;
	}
	syntax::Expr* object = expr.arguments[1];
	const ExprFacts* target = typed(*object, scope);
	if (target == nullptr || failed())
		return nullptr;
	const ClassEntity* owner = ClassOf(type->owner);
	const ClassEntity* cls = ClassOf(target->type);
	built->access = syntax::BinaryExpr();
	built->access.kind = syntax::ExprKind::Binary;
	built->access.location = expr.location;
	built->access.op = TokenKind::OP_DOTSTAR;
	built->access.left = object;
	built->access.right = callable;
	if (cls == nullptr || owner == nullptr || (cls != owner && !conversions_.is_base_of(owner, cls)))
	{
		if (cls == nullptr)
			built->access.op = TokenKind::OP_ARROWSTAR;
		else
		{
			built->indirection = syntax::UnaryExpr();
			built->indirection.kind = syntax::ExprKind::Unary;
			built->indirection.location = object->location;
			built->indirection.op = TokenKind::OP_STAR;
			built->indirection.operand = object;
			built->access.left = &built->indirection;
		}
	}
	if (data)
	{
		// t1.*f (20.8.2p1, its third and fourth forms): the call is
		// rewritten into the form of a functional cast (a callee without
		// facts) of that access to its own type, which designates the
		// member as the access does.
		const syntax::Expr* saved[2] = { pretyped_[0], pretyped_[1] };
		pretyped_[0] = callable;
		pretyped_[1] = object;
		const ExprFacts* access = typed(built->access, scope);
		pretyped_[0] = saved[0];
		pretyped_[1] = saved[1];
		if (access == nullptr || failed())
			return nullptr;
		syntax::Expr** operand = static_cast<syntax::Expr**>(
			arena_.allocate(sizeof(syntax::Expr*), alignof(syntax::Expr*)));
		*operand = &built->access;
		rewritten.callee = &built->callee;
		rewritten.arguments = syntax::Slice<syntax::Expr*>{ operand, 1 };
		return new_facts(expr, access->type, access->category);
	}
	built->callee.inner = &built->access;
	rewritten.callee = &built->callee;
	rewritten.arguments = syntax::Slice<syntax::Expr*>{ expr.arguments.data + 2, expr.arguments.size - 2 };
	return invoke_call(rewritten, callable, object, scope);
}

ExprFacts* Analyzer::invoke_call(
	const syntax::CallExpr& rewritten, const syntax::Expr* callable, const syntax::Expr* object, Scope* scope)
{
	// The rewritten call analyzed with its typed operands reused (an
	// enclosing __builtin_invoke's, which its arguments may hold, restored
	// after).
	const syntax::Expr* saved[2] = { pretyped_[0], pretyped_[1] };
	pretyped_[0] = callable;
	pretyped_[1] = object;
	ExprFacts* facts = call(rewritten, scope);
	pretyped_[0] = saved[0];
	pretyped_[1] = saved[1];
	return facts;
}

} // namespace sema
