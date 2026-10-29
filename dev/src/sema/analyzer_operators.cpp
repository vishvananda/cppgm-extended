#include "sema/analyzer.h"

#include <string>

#include "lex/identifier_table.h"
#include "lex/token_kind.h"

// Operators in expressions with an operand of class or enumeration type
// (13.3.1.2, 13.5; semantics mode): the candidate operator functions are
// the members of the left operand's class, the non-members unqualified
// lookup finds (member functions ignored) and those argument-dependent
// lookup finds; the best viable one makes the expression a call of it.
// When none is viable the built-in operator applies (13.3.1.2p7), which
// the callers type as before.
namespace sema {

namespace {

bool ClassOrEnumeration(const Type* type)
{
	type = type->unqualified;
	return type->kind == TypeKind::Class || type->kind == TypeKind::Enum;
}

bool IsIntType(const Type* type)
{
	type = type->unqualified;
	return type->kind == TypeKind::Fundamental && type->fundamental == FundamentalType::Int;
}

bool PossibleOperatorArity(const Type* type, uint32_t expected)
{
	if (type->variadic)
		return false;
	if (type->param_count == expected)
		return true;
	for (uint32_t i = 0; i < type->param_count; ++i)
		if (type->params[i]->kind == TypeKind::Expansion)
			return true;
	return false;
}

// 13.3.1.2p3: with no operand of class type, a non-member candidate has a
// first parameter of type T1 or reference to cv T1, T1 the left operand's
// enumeration type, or a second one of T2 (the right operand's).
bool EnumerationParameter(const Type* parameter, const Type* operand)
{
	operand = operand->unqualified;
	if (operand->kind != TypeKind::Enum)
		return false;
	if (parameter->is_reference())
		parameter = parameter->element;
	return parameter->unqualified == operand;
}

// An operand of the built-in arithmetic candidates (13.6p2: promoted
// arithmetic parameters take arithmetic and unscoped enumeration types).
bool ArithmeticOperand(const Type* type)
{
	return IsArithmetic(type) || IsIntegralOrUnscopedEnum(type);
}

// A pointer or a null pointer constant's type as a comparison operand.
bool IsPointerLikeOperand(const Type* type)
{
	return type->kind == TypeKind::Pointer ||
		(type->kind == TypeKind::Fundamental && type->fundamental == FundamentalType::NullptrT);
}

// Operators whose built-in candidates a class operand reaches through its
// conversion functions (13.6; the logical ones convert contextually, 5.14,
// 5.15, 5.3.1p9, and assignment to a class object is a member's).
bool ConvertedBuiltin(TokenKind op, uint32_t count)
{
	switch (op)
	{
	case TokenKind::OP_LAND:
	case TokenKind::OP_LOR:
	case TokenKind::OP_LNOT:
	case TokenKind::OP_COMMA:
	case TokenKind::OP_ASS:
	case TokenKind::OP_ARROW:
	case TokenKind::OP_LPAREN:
	case TokenKind::OP_AMP:
		return count == 2 && op == TokenKind::OP_AMP;
	default:
		return true;
	}
}

} // namespace

void Analyzer::class_builtin_candidates(TokenKind op, const syntax::Slice<syntax::Expr*>& operands, bool postfix)
{
	// Each class operand stands for the result of one of its non-explicit
	// conversion functions (its distinct decayed types; a reference result
	// kept as the reference type: an lvalue or xvalue of its referenced
	// type, 13.3.1.2p7, as 13.6p3 and p18 need), the others for themselves
	// (a null pointer constant too, 13.6p15); each combination's best
	// built-in candidate (13.6) is a candidate whose class operands convert
	// by user-defined conversions. The operands' options are this call's
	// part of operand_options_ (released at its end).
	const uint32_t count = operands.size;
	const size_t base = operand_options_.size();
	size_t begin[2] = { base, base };
	size_t size[2] = { 0, 0 };
	for (uint32_t i = 0; i < count; ++i)
	{
		begin[i] = operand_options_.size();
		const ExprFacts* facts = operands[i]->facts;
		// An unresolved overload set needs a function parameter's target
		// type; it cannot supply a built-in operator's operand type.
		if (facts->type == nullptr)
		{
			operand_options_.resize(base);
			return;
		}
		const ClassEntity* cls = ClassOf(facts->type);
		if (cls == nullptr)
		{
			operand_options_.push_back(
				facts->type->kind == TypeKind::Array ? types_.pointer(facts->type->element) : facts->type);
			size[i] = 1;
			continue;
		}
		const size_t functions = conversion_scratch_.size();
		if (require_complete(cls))
			conversion_functions(cls, conversion_scratch_);
		for (size_t k = functions; k < conversion_scratch_.size(); ++k)
		{
			const FunctionEntity* function = conversion_scratch_[k];
			if (function->is_explicit)
				continue;
			const Type* result = function->type->canonical->element;
			const Type* object = result->is_reference() ? result->element : result;
			if (object->kind == TypeKind::Array)
				result = types_.pointer(object->element);
			else if (!result->is_reference())
				result = result->unqualified;
			else if (object->kind == TypeKind::Function)
				result = types_.pointer(object);
			if (ClassOf(object) != nullptr)
				continue;
			bool seen = false;
			for (size_t j = begin[i]; j < operand_options_.size() && !seen; ++j)
				seen = operand_options_[j] == result;
			if (!seen)
				operand_options_.push_back(result);
		}
		conversion_scratch_.resize(functions);
		size[i] = operand_options_.size() - begin[i];
		if (size[i] == 0)
		{
			operand_options_.resize(base);
			return;
		}
	}
	syntax::Expr synthetic[2] = { syntax::Expr(), syntax::Expr() };
	ExprFacts views[2] = { ExprFacts(), ExprFacts() };
	syntax::Expr* pointers[2] = { &synthetic[0], &synthetic[1] };
	syntax::Slice<syntax::Expr*> converted;
	converted.data = pointers;
	converted.size = count;
	const size_t first_candidate = candidates_.size();
	const size_t second_count = count > 1 ? size[1] : 1;
	for (size_t a = 0; a < size[0]; ++a)
	{
		for (size_t b = 0; b < second_count; ++b)
		{
			for (uint32_t i = 0; i < count; ++i)
			{
				const ExprFacts* facts = operands[i]->facts;
				const Type* option = operand_options_[begin[i] + (i == 0 ? a : b)];
				views[i] = ExprFacts();
				views[i].type = option->is_reference() ? option->element : option;
				views[i].category = option->is_reference()
					? (option->kind == TypeKind::LValueReference ? ValueCategory::LValue : ValueCategory::XValue)
					: facts->type == option ? facts->category
											: ValueCategory::PRValue;
				if (ClassOf(facts->type) == nullptr)
					views[i].flags = facts->flags & kNullLiteral;
				synthetic[i].kind = syntax::ExprKind::Id;
				synthetic[i].facts = &views[i];
			}
			const Type* parameters[2] = { nullptr, nullptr };
			if (builtin_operator(op, converted, postfix, false, parameters) != count)
				continue;
			// One candidate per parameter-type-list, and none with that of
			// a (viable) non-member candidate that is not a function
			// template specialization (13.3.1.2p3 bullet 3).
			bool repeated = false;
			for (size_t k = first_candidate; k < candidates_.size() && !repeated; ++k)
			{
				repeated = true;
				for (uint32_t i = 0; i < count && repeated; ++i)
					repeated = sequences_[candidates_[k].first + i].target == parameters[i];
			}
			for (size_t k = 0; k < first_candidate && !repeated; ++k)
			{
				const FunctionEntity* function = candidates_[k].function;
				if (function == nullptr || candidates_[k].from_template || function->member)
					continue;
				const Type* type = function->type->canonical;
				repeated = type->param_count == count + (postfix ? 1 : 0);
				for (uint32_t i = 0; i < count && repeated; ++i)
					repeated = type->params[i] == parameters[i];
			}
			if (repeated)
				continue;
			const uint32_t first = static_cast<uint32_t>(sequences_.size());
			sequences_.resize(first + count);
			bool viable = true;
			for (uint32_t i = 0; viable && i < count; ++i)
			{
				viable = implicit_sequence(*operands[i], parameters[i], sequences_[first + i]);
				sequences_[first + i].target = parameters[i];
			}
			if (viable)
				candidates_.push_back(Candidate{ nullptr, first, false });
			else
				sequences_.resize(first);
		}
	}
	operand_options_.resize(base);
}

const Identifier* Analyzer::operator_key(TokenKind op)
{
	// Keyed as declared_name keys operator functions (`operator[` for
	// operator[], `operator(` for operator()).
	const size_t index = static_cast<size_t>(op);
	if (operator_keys_.size() <= index)
		operator_keys_.resize(index + 1, nullptr);
	if (operator_keys_[index] == nullptr)
		operator_keys_[index] = special_key((std::string("operator") + TokenKindSpelling(op)).c_str());
	return operator_keys_[index];
}

const Identifier* Analyzer::name_key(const syntax::NameComponent& component)
{
	// The lookup key of an identifier or an operator-function-id (13.5p4:
	// `operator+(a, b)` and `a.operator+(b)` name the operator function
	// like any function name).
	// A literal-operator-id: `operator ""` and its ud-suffix (13.5.8).
	if (component.kind == syntax::NameComponentKind::LiteralOperator)
		return special_key((std::string(kLiteralOperatorPrefix) + component.identifier->spelling).c_str());
	if (component.kind != syntax::NameComponentKind::Operator)
		return component.identifier;
	if (!component.array_form)
		return operator_key(component.op);
	return special_key((std::string("operator") + TokenKindSpelling(component.op) + "[]").c_str());
}

LookupResult Analyzer::operator_function_lookup(const syntax::Name& name, Scope* scope)
{
	// An operator-function-id looked up as lookup_name looks up an
	// identifier: unqualified, or in what its nested-name-specifier names
	// (the naming class, 11.2p5; a function's access waits for overload
	// resolution). Not found is the caller's to report.
	const Identifier* key = name_key(name.last());
	last_prefix_ = NamePrefix{ nullptr, nullptr };
	if (!name.qualified())
		return unqualified(scope, key, LookupFilter::Ordinary);
	last_prefix_ = name_prefix(name, scope);
	return lookup_in(last_prefix_, key, LookupFilter::Ordinary);
}

void Analyzer::operator_candidates(
	const Identifier* key, const syntax::Slice<syntax::Expr*>& operands, bool members_only, Scope* scope)
{
	// 13.3.1.2p3: the member candidates, then (for operators that may be
	// non-members, 13.5p6) the non-member candidates, each function once.
	std::vector<FunctionEntity*>& set = operator_set_;
	set.clear();
	// An overloaded function operand has no type until a candidate's
	// parameter resolves it, so it contributes no member candidates.
	const Type* left_type = operands[0]->facts->type;
	const ClassEntity* cls = left_type != nullptr ? ClassOf(left_type) : nullptr;
	if (key == assign_key_ && cls != nullptr && require_complete(cls) && cls->scope != nullptr)
	{
		// The class's own operator= members and its implicit copy and move
		// assignment operators (12.8p18-p20, p24).
		assignment_candidates(const_cast<ClassEntity*>(cls), set);
		return;
	}
	const uint32_t mark = ++candidate_epoch_;
	if (cls != nullptr && require_complete(cls) && cls->scope != nullptr)
	{
		const LookupResult members = names_.qualified(cls->scope, key, LookupFilter::Ordinary);
		// 10.2p7: members of different bases are an invalid set.
		if (members.ambiguous)
		{
			substitution_error(operands[0]->location, "ambiguous member operator name");
			return;
		}
		for (const FunctionList* node = members.found() ? members.functions : nullptr; node != nullptr;
			node = node->next)
		{
			FunctionEntity* function = static_cast<FunctionEntity*>(node->binding->entity);
			if (function->candidate_mark == mark || !function->member)
				continue;
			function->candidate_mark = mark;
			set.push_back(function);
		}
	}
	if (members_only)
		return;
	// Unqualified lookup in the expression's context ignoring member
	// functions: a class scope's members give way to the enclosing scopes.
	// The class scope that found a member is the innermost one on the way
	// out whose class is the member's or derives from it (the member may be
	// a base's, declared in another namespace); lookup resumes at that
	// scope's lexical parent.
	Scope* from = scope;
	while (from != nullptr)
	{
		const LookupResult found = names_.unqualified(from, key, LookupFilter::Ordinary);
		if (!found.found() || found.binding->kind != BindingKind::Function)
			break;
		const Entity* first = found.binding->entity;
		if (first->owner != nullptr && first->owner->kind == ScopeKind::Class)
		{
			const ClassEntity* home = static_cast<const ClassEntity*>(first->owner->entity);
			while (from != nullptr &&
				!(from->kind == ScopeKind::Class && from->entity != nullptr &&
					(from->entity == home ||
						conversions_.is_base_of(home, static_cast<const ClassEntity*>(from->entity)))))
				from = from->parent;
			if (from != nullptr)
				from = from->parent;
			continue;
		}
		for (const FunctionList* node = found.functions; node != nullptr; node = node->next)
		{
			FunctionEntity* function = static_cast<FunctionEntity*>(node->binding->entity);
			if (node->binding->invisible || function->member || function->candidate_mark == mark)
				continue;
			function->candidate_mark = mark;
			set.push_back(function);
		}
		break;
	}
	// Bound in the template's definition unless an operand may be
	// type-dependent there (14.6p10, 14.6.4.2; see kDependentUse).
	bool bound = true;
	for (const syntax::Expr* operand : operands)
		bound = bound && operand->facts != nullptr && (operand->facts->flags & kDependentUse) == 0;
	argument_dependent(operands, key, bound);
	for (FunctionEntity* function : adl_functions_)
	{
		if (function->candidate_mark == mark || function->member)
			continue;
		function->candidate_mark = mark;
		set.push_back(function);
	}
	adl_functions_.clear();
}

uint32_t Analyzer::builtin_operator(TokenKind op, const syntax::Slice<syntax::Expr*>& operands, bool postfix,
	bool same_enumeration_list, const Type** parameters)
{
	// 13.6 for operands of no class type: the parameter types of the best
	// built-in candidate, or 0 when none can be viable. The best member of
	// a family takes each operand to its own promoted type (any other
	// member converts some operand worse), so it stands for the family;
	// two operands of one enumeration type T compare best as (T, T),
	// unless a non-member candidate has that parameter-type-list
	// (13.3.1.2p3). `,` has none (13.3.1.2p3), ++ and -- none for an
	// enumeration (only arithmetic types, 13.6p3-p5).
	const Type* t1 = operands[0]->facts->type;
	const Type* boolean = types_.fundamental(FundamentalType::Bool);
	if (operands.size == 1)
	{
		// 13.6p3-p5: VQ T& operator++(VQ T&) and T operator++(VQ T&, int)
		// for an arithmetic type T (-- not for bool) or a pointer to an
		// object type: the operand binds a non-const lvalue reference.
		if (op == TokenKind::OP_INC || op == TokenKind::OP_DEC)
		{
			parameters[0] = types_.reference(t1, false);
			const bool counted = IsArithmetic(t1) ? op == TokenKind::OP_INC || t1->unqualified != boolean
												  : IsObjectPointer(t1) && !IsVoid(t1->unqualified->element);
			return counted && (t1->cv & kConst) == 0 && t1->kind != TypeKind::Array ? 1 : 0;
		}
		if (postfix)
			return 0;
		// 13.6p6: T& operator*(T*); 13.6p8: T* operator+(T*).
		if (op == TokenKind::OP_STAR || (op == TokenKind::OP_PLUS && t1->unqualified->kind == TypeKind::Pointer))
		{
			parameters[0] = t1->unqualified;
			return t1->unqualified->kind == TypeKind::Pointer ? 1 : 0;
		}
		parameters[0] = op == TokenKind::OP_LNOT ? boolean : promoted(t1);
		if (op == TokenKind::OP_LNOT ||
			((op == TokenKind::OP_PLUS || op == TokenKind::OP_MINUS) && ArithmeticOperand(t1)) ||
			(op == TokenKind::OP_COMPL && IsIntegralOrUnscopedEnum(t1)))
			return 1;
		return 0;
	}
	const Type* t2 = operands[1]->facts->type;
	const Type* p1 = t1->kind == TypeKind::Array ? types_.pointer(t1->element) : t1->unqualified;
	const Type* p2 = t2->kind == TypeKind::Array ? types_.pointer(t2->element) : t2->unqualified;
	const Type* difference = types_.fundamental(FundamentalType::LongInt);
	const bool arithmetic = ArithmeticOperand(t1) && ArithmeticOperand(t2);
	// 13.6p15: a null pointer constant converts to the other operand's
	// pointer type.
	const bool null1 = (operands[0]->facts->flags & kNullLiteral) != 0;
	const bool null2 = (operands[1]->facts->flags & kNullLiteral) != 0;
	const bool integral = IsIntegralOrUnscopedEnum(t1) && IsIntegralOrUnscopedEnum(t2);
	parameters[0] = promoted(t1);
	parameters[1] = promoted(t2);
	switch (op)
	{
	case TokenKind::OP_LT:
	case TokenKind::OP_GT:
	case TokenKind::OP_LE:
	case TokenKind::OP_GE:
	case TokenKind::OP_EQ:
	case TokenKind::OP_NE:
		// 13.6p16: bool operator<(T, T) for an enumeration type T.
		if (p1 == p2 && p1->kind == TypeKind::Enum && !same_enumeration_list)
		{
			parameters[0] = parameters[1] = p1;
			return 2;
		}
		// 13.6p16: member pointers have equality candidates. Use the
		// same composite type as a direct member-pointer comparison.
		if ((op == TokenKind::OP_EQ || op == TokenKind::OP_NE) &&
			(p1->kind == TypeKind::MemberPointer || p2->kind == TypeKind::MemberPointer))
		{
			const Type* common = composite_pointer(operands[0]->facts, operands[1]->facts, true);
			if (common == nullptr || common->kind != TypeKind::MemberPointer)
				return 0;
			parameters[0] = parameters[1] = common;
			return 2;
		}
		// 13.6p15: bool operator<(T*, T*) for the composite pointer type.
		if ((IsPointerLikeOperand(p1) || (null1 && p2->kind == TypeKind::Pointer)) &&
			(IsPointerLikeOperand(p2) || (null2 && p1->kind == TypeKind::Pointer)) &&
			(p1->kind == TypeKind::Pointer || p2->kind == TypeKind::Pointer))
		{
			const Type* common = p1->kind == TypeKind::Pointer ? p1 : p2;
			if (p1->kind == TypeKind::Pointer && p2->kind == TypeKind::Pointer && p1 != p2)
			{
				Conversion conversion = Conversion();
				if (conversions_.implicit(Operand{ p2, ValueCategory::PRValue, 0 }, p1, false, conversion))
					common = p1;
				else if (conversions_.implicit(Operand{ p1, ValueCategory::PRValue, 0 }, p2, false, conversion))
					common = p2;
				else
					return 0;
			}
			parameters[0] = parameters[1] = common;
			return 2;
		}
		return arithmetic ? 2 : 0;
	case TokenKind::OP_STAR:
	case TokenKind::OP_DIV:
		return arithmetic ? 2 : 0;
	case TokenKind::OP_PLUS:
	case TokenKind::OP_MINUS:
		// 13.6p13-p14: T* operator+(T*, std::ptrdiff_t) and its mirror.
		if (arithmetic)
			return 2;
		if (IsObjectPointer(p1) && IsIntegralOrUnscopedEnum(t2))
		{
			parameters[0] = p1;
			parameters[1] = difference;
			return 2;
		}
		if (op == TokenKind::OP_PLUS && IsIntegralOrUnscopedEnum(t1) && IsObjectPointer(p2))
		{
			parameters[0] = difference;
			parameters[1] = p2;
			return 2;
		}
		// 13.6p14: std::ptrdiff_t operator-(T*, T*), T the more qualified.
		if (op == TokenKind::OP_MINUS && IsObjectPointer(p1) && IsObjectPointer(p2) &&
			p1->element->unqualified == p2->element->unqualified)
		{
			parameters[0] = parameters[1] = (p1->element->cv & ~p2->element->cv) != 0 ? p1 : p2;
			return 2;
		}
		return 0;
	case TokenKind::OP_LSQUARE:
		// 13.6p13: T& operator[](T*, std::ptrdiff_t) and its mirror.
		if (IsObjectPointer(p1) && IsIntegralOrUnscopedEnum(t2))
		{
			parameters[0] = p1;
			parameters[1] = difference;
			return 2;
		}
		if (IsIntegralOrUnscopedEnum(t1) && IsObjectPointer(p2))
		{
			parameters[0] = difference;
			parameters[1] = p2;
			return 2;
		}
		return 0;
	case TokenKind::OP_MOD:
	case TokenKind::OP_AMP:
	case TokenKind::OP_XOR:
	case TokenKind::OP_BOR:
	case TokenKind::OP_LSHIFT:
	case TokenKind::OP_RSHIFT:
		return integral ? 2 : 0;
	case TokenKind::OP_LAND:
	case TokenKind::OP_LOR:
		parameters[0] = parameters[1] = boolean;
		return 2;
	case TokenKind::OP_STARASS:
	case TokenKind::OP_DIVASS:
	case TokenKind::OP_PLUSASS:
	case TokenKind::OP_MINUSASS:
	case TokenKind::OP_MODASS:
	case TokenKind::OP_BANDASS:
	case TokenKind::OP_XORASS:
	case TokenKind::OP_BORASS:
	case TokenKind::OP_LSHIFTASS:
	case TokenKind::OP_RSHIFTASS:
	{
		// 13.6p18-p22: VQ L& operator@=(VQ L&, R) for an arithmetic (for
		// %= and the bitwise ones integral) L, and T*VQ& operator+=(T*VQ&,
		// std::ptrdiff_t); an enumeration left operand has none.
		if ((t1->cv & kConst) != 0 || t1->kind == TypeKind::Array)
			return 0;
		const bool additive = op == TokenKind::OP_PLUSASS || op == TokenKind::OP_MINUSASS;
		const bool multiplicative = additive || op == TokenKind::OP_STARASS || op == TokenKind::OP_DIVASS;
		parameters[0] = types_.reference(t1, false);
		if (multiplicative && IsArithmetic(t1) && ArithmeticOperand(t2))
			return 2;
		if (!multiplicative && IsIntegralType(t1) && IsIntegralOrUnscopedEnum(t2))
			return 2;
		if (additive && IsObjectPointer(t1) && IsIntegralOrUnscopedEnum(t2))
		{
			parameters[1] = difference;
			return 2;
		}
		return 0;
	}
	default:
		return 0;
	}
}

ExprFacts* Analyzer::overloaded_operator(
	const syntax::Expr& expr, TokenKind op, const syntax::Slice<syntax::Expr*>& operands, bool postfix, Scope* scope)
{
	// 13.3.1.2p2: only an operand of class or enumeration type overloads.
	bool overloadable = false;
	bool class_operand = false;
	bool braced = false;
	for (const syntax::Expr* operand : operands)
	{
		// A braced-init-list operand (5.17p9: `x = {...}` is x.operator=(
		// {...})) converts as an argument (13.3.3.1.5).
		if (operand->kind == syntax::ExprKind::BracedInit)
		{
			braced = true;
			continue;
		}
		// An overloaded function name converts to a candidate's parameter
		// (13.4) beside a class or enumeration operand.
		const ExprFacts* facts = operand->facts;
		if ((facts->flags & kBoundMember) != 0)
			return nullptr;
		if (facts->type == nullptr || (facts->flags & kOverloadSet) != 0)
			continue;
		overloadable = overloadable || ClassOrEnumeration(facts->type);
		class_operand = class_operand || facts->type->unqualified->kind == TypeKind::Class;
	}
	if (!overloadable)
		return nullptr;
	// 13.5.3-13.5.6: assignment, subscripting and member access through ->
	// are member functions only.
	const bool members_only = op == TokenKind::OP_ASS || op == TokenKind::OP_LSQUARE || op == TokenKind::OP_ARROW;
	operator_candidates(operator_key(op), operands, members_only, scope);
	if (failed())
		return nullptr;
	std::vector<FunctionEntity*>& set = operator_set_;
	const bool converted_builtins = class_operand && !braced && ConvertedBuiltin(op, operands.size);
	if (set.empty() && !converted_builtins)
		return nullptr;
	// Each candidate's sequences, slot i for operand i: a member's implicit
	// object parameter takes the left operand (13.3.1p4); a postfix
	// operator's int parameter takes the implicit zero (13.5.7).
	const uint32_t count = operands.size;
	candidates_.clear();
	sequences_.clear();
	++graph_.resolutions;
	graph_.candidates += set.size();
	const Type* left = operands[0]->facts->type;
	const Type* right = count > 1 ? operands[1]->facts->type : nullptr;
	bool same_enumeration_list = false;
	for (FunctionEntity* candidate : set)
	{
		if (IgnoredByOverloadResolution(candidate))
			continue;
		// Filter a fixed parameter count before deduction can instantiate an
		// unusable candidate's return type (13.3.2p2). In particular, prefix
		// and postfix increment/decrement have different arities (13.5.7).
		const bool member = candidate->member && !candidate->is_static;
		const uint32_t expected = count - (member ? 1 : 0) + (postfix ? 1 : 0);
		if (!PossibleOperatorArity(candidate->type->canonical, expected))
			continue;
		// A function template's candidate is the specialization its operands
		// deduce (13.3.1p7, 14.8.2.1; a member's from those after the
		// object).
		FunctionEntity* function = candidate;
		if (candidate->templated)
		{
			syntax::Slice<syntax::Expr*> arguments = operands;
			if (candidate->member && !candidate->is_static)
			{
				++arguments.data;
				--arguments.size;
			}
			function = deduce_for_call(candidate, arguments);
			if (function == nullptr)
				continue;
		}
		const Type* type = function->type->canonical;
		const uint32_t parameters = type->param_count;
		if (parameters != expected || type->variadic)
			continue;
		if (postfix && !IsIntType(type->params[parameters - 1]))
			continue;
		// 13.3.1.2p3: with no operand of class type, a non-member candidate
		// takes an enumeration operand's own type (first parameter for the
		// left, second for the right); one with the parameter-type-list
		// (T, T) of the built-in comparison of enumeration T replaces it,
		// unless it is a function template specialization (13.3.1.2p3
		// bullet 3: the built-in candidate then stays and, its conversions
		// tying, wins as a non-template, 13.3.3p1).
		if (!class_operand)
		{
			if ((left == nullptr || !EnumerationParameter(type->params[0], left)) &&
				(right == nullptr || !EnumerationParameter(type->params[1], right)))
				continue;
			same_enumeration_list = same_enumeration_list ||
				(!candidate->templated && left != nullptr && right != nullptr && type->params[0] == left->unqualified &&
					type->params[1] == right->unqualified);
		}
		const uint32_t first = static_cast<uint32_t>(sequences_.size());
		sequences_.resize(first + count);
		bool viable = true;
		for (uint32_t i = 0; viable && i < count; ++i)
		{
			Conversion& slot = sequences_[first + i];
			// Member-only candidates share the left operand's member
			// lookup: a using-declaration makes that class the implicit
			// object's owner for ranking (13.3.1p4). The selected call still
			// converts to the declaring base. Other operators compete with
			// non-members using their own class.
			if (i == 0 && member)
				viable = object_sequence(operands[0]->facts, function, members_only, slot);
			else
				viable = implicit_sequence(*operands[i], type->params[i - (member ? 1 : 0)], slot);
		}
		if (!viable)
		{
			sequences_.resize(first);
			continue;
		}
		candidates_.push_back(Candidate{ function, first, candidate->templated });
	}
	// 13.3.1.2p3 bullet 3: class operands' built-in candidates (13.6)
	// through their conversion functions.
	if (converted_builtins)
		class_builtin_candidates(op, operands, postfix);
	if (candidates_.empty())
		return nullptr;
	// 13.3.1.2p3 bullet 3: operands of enumeration but no class type also
	// have the built-in candidates (13.6), the best of which competes as a
	// candidate of no function.
	if (!class_operand && left != nullptr && (count == 1 || right != nullptr))
	{
		const Type* parameters[2] = { nullptr, nullptr };
		if (builtin_operator(op, operands, postfix, same_enumeration_list, parameters) == count)
		{
			const uint32_t first = static_cast<uint32_t>(sequences_.size());
			sequences_.resize(first + count);
			bool viable = true;
			for (uint32_t i = 0; viable && i < count; ++i)
				viable = implicit_sequence(*operands[i], parameters[i], sequences_[first + i]);
			if (viable)
				candidates_.push_back(Candidate{ nullptr, first, false });
			else
				sequences_.resize(first);
		}
	}
	size_t best = 0;
	for (size_t i = 1; i < candidates_.size(); ++i)
	{
		if (compare_candidates(candidates_[i], candidates_[best], count, false) > 0)
			best = i;
	}
	for (size_t i = 0; i < candidates_.size(); ++i)
	{
		if (i != best && compare_candidates(candidates_[best], candidates_[i], count, false) <= 0)
		{
			substitution_error(expr.location, "ambiguous overloaded operator");
			return nullptr;
		}
	}
	const Candidate chosen = candidates_[best];
	FunctionEntity* selected = chosen.function;
	if (selected == nullptr)
	{
		// The built-in operator: a class operand becomes its conversion
		// function's result (13.3.1.2p7).
		std::vector<const FunctionEntity*>& converters = converter_scratch_;
		converters.clear();
		for (uint32_t i = 0; i < count; ++i)
			converters.push_back(sequences_[chosen.first + i].rank == ConversionRank::UserDefined
					? sequences_[chosen.first + i].constructor
					: nullptr);
		for (uint32_t i = 0; i < count; ++i)
		{
			if (converters[i] != nullptr && !converters[i]->constructor)
				convert_class_operand(*operands[i], converters[i]);
		}
		return nullptr;
	}
	check_not_deleted(selected, expr.location);
	const bool member = selected->member && !selected->is_static;
	// Clause 11: a member operator function is named in the left operand's
	// class (13.3.1.2p3, 11.2p5), its member of an unambiguous base
	// subobject there (10.2, 5.2.5p5).
	if (member)
	{
		const ClassEntity* cls = ClassOf(operands[0]->facts->type);
		const LookupResult members = names_.qualified(cls->scope, operator_key(op), LookupFilter::Ordinary);
		check_function_access(selected, members.found() ? members.functions : nullptr, cls, cls, scope, expr.location);
		if (!member_subobject(expr, selected, cls, cls, expr.location))
			return nullptr;
	}
	operator_conversions(operands, selected, chosen, scope);
	if (failed())
		return nullptr;
	if (ExprFacts* identity = type_identity(expr, op, operands, selected))
		return identity;
	odr_use(selected, true);
	if (!selected->nothrow)
		++potentially_throwing_;
	ValueCategory category;
	const Type* result = result_category(selected->type->canonical->element, category);
	ExprFacts* facts = new_facts(expr, result, category);
	facts->entity = selected;
	facts->flags |= kDirectCall | kOperatorCall;
	// An operator expression names no qualified-id: a virtual member
	// operator function's final overrider is called (5.2.2p1).
	if (member && selected->is_virtual)
		facts->flags |= kVirtualCall;
	return facts;
}

void Analyzer::operator_conversions(
	const syntax::Slice<syntax::Expr*>& operands, FunctionEntity* selected, const Candidate& chosen, Scope* scope)
{
	const uint32_t count = operands.size;
	const bool member = selected->member && !selected->is_static;
	// The operands' conversions: the object's to the member's class, each
	// argument's to its parameter (the sequences taken out of the scratch
	// first: a class parameter's copy resolves its constructor).
	const size_t scratch = argument_scratch_.size();
	for (uint32_t i = 0; i < count; ++i)
		argument_scratch_.push_back(sequences_[chosen.first + i]);
	for (uint32_t i = 0; i < count; ++i)
	{
		Conversion conversion = argument_scratch_[scratch + i];
		if (operands[i]->kind == syntax::ExprKind::BracedInit)
		{
			list_argument(static_cast<const syntax::BracedInitList&>(*operands[i]),
				selected->type->canonical->params[i - (member ? 1 : 0)], scope, operands[i]->location);
			continue;
		}
		if (i == 0 && member)
		{
			conversion = Conversion();
			object_sequence(operands[0]->facts, selected, false, conversion);
		}
		else if ((operands[i]->facts->flags & kOverloadSet) != 0)
		{
			// An overloaded function name: resolved to its parameter's target
			// (13.4), as a call argument is.
			expression_scope_ = scope;
			convert(*operands[i], selected->type->canonical->params[i - (member ? 1 : 0)], false);
			continue;
		}
		conversion.initialization = true;
		expression_scope_ = scope;
		// 4.10p3, 8.5.3p5: an argument's conversion to a base (a reference
		// parameter's binding) needs an unambiguous accessible base.
		if (i != 0 || !member)
			check_conversion_access(operands[i]->facts, conversion, operands[i]->location);
		if (conversion.constructor != nullptr)
			apply_user_conversion(operands[i]->facts, conversion, operands[i]->location);
		// A class prvalue operand bound to a reference parameter, or the
		// implicit object parameter (13.3.1p5), is a temporary.
		if (i != 0 || !member)
			check_binding_temporary(operands[i]->facts, conversion, operands[i]->location);
		else
			check_temporary_destruction(operands[i]->facts, operands[i]->location);
		odr_use_constants(*operands[i], &conversion);
		record_conversion(facts_of(*operands[i]), conversion);
		// 8.5p16: a class parameter from an operand of its class (or a
		// derived one) is initialized by the constructor it selects.
		if (i == 0 && member)
			continue;
		const Type* parameter = selected->type->canonical->params[i - (member ? 1 : 0)];
		ClassEntity* cls = parameter->is_reference() ? nullptr : const_cast<ClassEntity*>(ClassOf(parameter));
		// 5.2.2p4: a class parameter is destroyed in the caller's context.
		if (cls != nullptr)
			check_destruction(parameter, scope, operands[i]->location);
		if (cls != nullptr && same_class_source(cls, *operands[i]))
		{
			expression_scope_ = scope;
			record_copy(cls, parameter, *operands[i], operands[i]->location);
		}
	}
	argument_scratch_.resize(scratch);
}

ExprFacts* Analyzer::binary_operator(const syntax::BinaryExpr& expr, Scope* scope)
{
	syntax::Expr* pair[2] = { expr.left, expr.right };
	syntax::Slice<syntax::Expr*> operands;
	operands.data = pair;
	operands.size = 2;
	return overloaded_operator(expr, expr.op, operands, false, scope);
}

ExprFacts* Analyzer::unary_operator(const syntax::UnaryExpr& expr, bool postfix, Scope* scope)
{
	syntax::Slice<syntax::Expr*> operands;
	operands.data = const_cast<syntax::Expr**>(&expr.operand);
	operands.size = 1;
	return overloaded_operator(expr, expr.op, operands, postfix, scope);
}

void Analyzer::check_operator_declaration(const FunctionEntity* function, uint32_t location)
{
	// 13.5p6: a non-member operator function has a parameter of class or
	// enumeration type (or a reference to one); 13.5.3-13.5.6: `=`, `()`,
	// `[]` and `->` are non-static members.
	if (!function->is_operator || function->member || function->operator_token == TokenKind::KW_NEW ||
		function->operator_token == TokenKind::KW_DELETE)
		return;
	const TokenKind op = function->operator_token;
	if (op == TokenKind::OP_ASS || op == TokenKind::OP_LPAREN || op == TokenKind::OP_LSQUARE ||
		op == TokenKind::OP_ARROW)
		error(location, "this operator function must be a non-static member");
	// A template's parameter types may depend on its parameters.
	if (function->templated)
		return;
	const Type* type = function->type->canonical;
	for (uint32_t i = 0; i < type->param_count; ++i)
	{
		const Type* parameter = type->params[i];
		if (parameter->is_reference())
			parameter = parameter->element;
		if (ClassOrEnumeration(parameter))
			return;
	}
	error(location, "an operator function needs a parameter of class or enumeration type");
}

} // namespace sema
