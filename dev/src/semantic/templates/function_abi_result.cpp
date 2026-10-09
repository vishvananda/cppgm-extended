#include "semantic/analysis/analyzer.h"
#include "support/exceptions.h"

#include <functional>
#include <limits>
#include <unordered_set>
#include <vector>

namespace cppgm
{
namespace semantic
{
namespace
{

FunctionTemplateResultIdentityAtomKind ResultIdentityKind(std::uint64_t atom)
{
	return static_cast<FunctionTemplateResultIdentityAtomKind>(atom >> 56);
}

std::uint64_t ResultIdentityValue(std::uint64_t atom)
{
	return atom & 0x00ffffffffffffffULL;
}

FunctionTemplateAbiTypeId AppendAbiType(Program* program,
	const FunctionTemplateAbiType& type)
{
	if (program->function_template_abi_types.size() >=
		kNoFunctionTemplateAbiType)
		ThrowSemanticResourceLimit("too many function template ABI type nodes");
	const FunctionTemplateAbiTypeId result =
		static_cast<FunctionTemplateAbiTypeId>(
			program->function_template_abi_types.size());
	program->function_template_abi_types.push_back(type);
	return result;
}

FunctionTemplateAbiExpressionId AppendAbiExpression(Program* program,
	const FunctionTemplateAbiExpression& expression)
{
	if (program->function_template_abi_expressions.size() >=
		kNoFunctionTemplateAbiExpression)
		ThrowSemanticResourceLimit("too many function template ABI expressions");
	const FunctionTemplateAbiExpressionId result =
		static_cast<FunctionTemplateAbiExpressionId>(
			program->function_template_abi_expressions.size());
	program->function_template_abi_expressions.push_back(expression);
	return result;
}

class AbiPublication
{
public:
	explicit AbiPublication(Program* program)
		: program_(program), type_mark_(program->function_template_abi_types.size()),
		  argument_mark_(program->function_template_abi_arguments.size()),
		  expression_mark_(program->function_template_abi_expressions.size()),
		  committed_(false) {}

	~AbiPublication()
	{
		if (committed_) return;
		program_->function_template_abi_types.erase(
			program_->function_template_abi_types.begin() + type_mark_,
			program_->function_template_abi_types.end());
		program_->function_template_abi_arguments.erase(
			program_->function_template_abi_arguments.begin() + argument_mark_,
			program_->function_template_abi_arguments.end());
		program_->function_template_abi_expressions.erase(
			program_->function_template_abi_expressions.begin() + expression_mark_,
			program_->function_template_abi_expressions.end());
	}

	void Commit() { committed_ = true; }

private:
	AbiPublication(const AbiPublication&);
	AbiPublication& operator=(const AbiPublication&);

	Program* program_;
	std::size_t type_mark_, argument_mark_, expression_mark_;
	bool committed_;
};

NodeId FindDescendant(const SyntaxArena& arena, NodeId root, const char* tag)
{
	if (root == kNoNode) return kNoNode;
	std::vector<NodeId> pending(1, root);
	while (!pending.empty())
	{
		const NodeId node = pending.back();
		pending.pop_back();
		if (arena.IsTag(node, tag)) return node;
		for (std::uint32_t edge = arena.FirstEdge(node); edge != kNoEdge;
			edge = arena.NextEdge(edge))
			pending.push_back(arena.EdgeChild(edge));
	}
	return kNoNode;
}

struct ParsedComponent
{
	NameId name;
	EntityId entity;
	// A typedef of a type that is no class (`n::size_type`) has no entity;
	// its declaration still names the type.
	TypeId declared_type;
	ScopeId name_space;
	bool callable;
	std::size_t function_template;
	std::vector<FunctionTemplateAbiArgument> arguments;
	// The arguments written; the rest are the template's defaults, one of
	// which may not be expressible (a pack of an enclosing template).
	std::size_t written_arguments;
	bool unread_default;

	ParsedComponent() : name(0), entity(kNoEntity), declared_type(kNoType), name_space(kNoScope), callable(false), function_template(kNoDumpEdge), written_arguments(0), unread_default(false) {}
};

class AbiIdentityReader
{
public:
	AbiIdentityReader(Program* program,
		const std::deque<ClassTemplatePattern>& class_templates,
		const std::deque<FunctionTemplatePattern>& function_templates,
		const std::vector<std::uint32_t>& class_template_by_entity,
		const std::vector<TemplateParameter>& parameters,
		const std::vector<std::uint64_t>& atoms,
		bool group_template_packs = true)
		: program_(program), class_templates_(class_templates), function_templates_(function_templates),
		  class_template_by_entity_(class_template_by_entity),
		  parameters_(parameters), atoms_(atoms),
		  position_(0), group_template_packs_(group_template_packs) {}

	FunctionTemplateAbiTypeId ParseType(bool* pack_expansion = 0)
	{
		std::uint8_t cv = 0;
		if (!ParseCvQualifiers(&cv)) return kNoFunctionTemplateAbiType;
		FunctionTemplateAbiTypeId root = ParsePrimaryType();
		if (root == kNoFunctionTemplateAbiType) return root;
		// A cv-qualifier may also follow the type it qualifies
		// (`basic_regex<T> const`).
		if (!ParseCvQualifiers(&cv)) return kNoFunctionTemplateAbiType;
		if (cv) root = AppendAbiType(program_, FunctionTemplateAbiType(
			FUNCTION_TEMPLATE_ABI_TYPE_QUALIFIED, root, 0, 0,
			kNoTemplateParameter, cv));
		while (IsNode("abstract-declarator"))
			if (!ParseAbstractDeclarator(&root, pack_expansion))
				return kNoFunctionTemplateAbiType;
		return root;
	}

	FunctionTemplateAbiTypeId ParseParameterType()
	{
		NameId payload = 0;
		if (!BeginNode("parameter-declaration", &payload) || !BeginNode("decl-specifier-seq", &payload))
			return kNoFunctionTemplateAbiType;
		FunctionTemplateAbiTypeId type = ParseType();
		if (type == kNoFunctionTemplateAbiType || !EndNode()) return kNoFunctionTemplateAbiType;
		if (IsNode("declarator") && !ParseAbstractDeclarator(&type, 0, "declarator")) return kNoFunctionTemplateAbiType;
		return EndNode() ? type : kNoFunctionTemplateAbiType;
	}

	FunctionTemplateAbiExpressionId ParseExpression()
	{
		if (position_ >= atoms_.size())
			return kNoFunctionTemplateAbiExpression;
		const FunctionTemplateResultIdentityAtomKind kind =
			ResultIdentityKind(atoms_[position_]);
		if (kind == FUNCTION_TEMPLATE_RESULT_PARAMETER || kind == FUNCTION_TEMPLATE_RESULT_FUNCTION_PARAMETER)
		{
			const std::uint64_t parameter = ResultIdentityValue(atoms_[position_++]);
			if (parameter >= kNoTemplateParameter)
				return kNoFunctionTemplateAbiExpression;
			return AppendAbiExpression(program_, FunctionTemplateAbiExpression(
				kind == FUNCTION_TEMPLATE_RESULT_FUNCTION_PARAMETER ?
					FUNCTION_TEMPLATE_ABI_EXPRESSION_FUNCTION_PARAMETER : FUNCTION_TEMPLATE_ABI_EXPRESSION_TEMPLATE_PARAMETER,
				kNoFunctionTemplateAbiExpression,
				kNoFunctionTemplateAbiExpression, kNoFunctionTemplateAbiType,
				0, static_cast<std::uint32_t>(parameter)));
		}
		if (kind == FUNCTION_TEMPLATE_RESULT_LITERAL_ARGUMENT)
		{
			const std::uint64_t first = ResultIdentityValue(atoms_[position_++]);
			if (first >= program_->canonical_template_arguments.size() ||
				program_->canonical_template_arguments[first].kind != TEMPLATE_ARGUMENT_INTEGRAL)
				return kNoFunctionTemplateAbiExpression;
			return AppendAbiExpression(program_, FunctionTemplateAbiExpression(
				FUNCTION_TEMPLATE_ABI_EXPRESSION_INTEGRAL,
				kNoFunctionTemplateAbiExpression, kNoFunctionTemplateAbiExpression,
				kNoFunctionTemplateAbiType, 0, kNoTemplateParameter, OPERATOR_NONE,
				false, static_cast<std::uint32_t>(first), 1));
		}
		if (kind == FUNCTION_TEMPLATE_RESULT_QUALIFIED_BEGIN)
		{
			return ParseQualifiedExpression();
		}
		if (kind == FUNCTION_TEMPLATE_RESULT_TYPE || kind == FUNCTION_TEMPLATE_RESULT_BOUND_ARGUMENT)
			return kNoFunctionTemplateAbiExpression;
		if (kind != FUNCTION_TEMPLATE_RESULT_NODE_BEGIN)
			return kNoFunctionTemplateAbiExpression;
		const NameId tag = static_cast<NameId>(ResultIdentityValue(atoms_[position_]));
		const std::string node = program_->names.Get(tag);
		NameId payload = 0;
		if (!BeginNode(node.c_str(), &payload))
			return kNoFunctionTemplateAbiExpression;
		const std::string spelling = program_->names.Get(payload);
		FunctionTemplateAbiExpressionId expression =
			kNoFunctionTemplateAbiExpression;
		if (node == "id-expression")
			expression = position_ < atoms_.size() &&
				ResultIdentityKind(atoms_[position_]) == FUNCTION_TEMPLATE_RESULT_QUALIFIED_BEGIN ?
				ParseQualifiedExpression() : AtNodeEnd() ? UnqualifiedValue(payload) : ParseExpression();
		else if (node == "parenthesized-expression")
			expression = ParseExpression();
		else if (node == "keyword-literal")
			expression = KeywordValue(spelling);
		else if (node == "sizeof-pack-expression")
		{
			const FunctionTemplateAbiExpressionId operand = ParseExpression();
			if (operand != kNoFunctionTemplateAbiExpression)
				expression = AppendAbiExpression(program_, FunctionTemplateAbiExpression(
					FUNCTION_TEMPLATE_ABI_EXPRESSION_SIZEOF_PACK, operand));
		}
		else if (node == "sizeof-expression")
			expression = ParseSizeofOperand();
		else if (node == "unary-expression" || node == "binary-expression")
		{
			const bool unary = node == "unary-expression";
			const OperatorKind operation = OperatorForSpelling(spelling, unary);
			if (operation == OPERATOR_NONE) return kNoFunctionTemplateAbiExpression;
			const FunctionTemplateAbiExpressionId left = ParseExpression();
			const FunctionTemplateAbiExpressionId right = unary ?
				kNoFunctionTemplateAbiExpression : ParseExpression();
			if (left != kNoFunctionTemplateAbiExpression &&
				(unary || right != kNoFunctionTemplateAbiExpression))
				expression = AppendAbiExpression(program_, FunctionTemplateAbiExpression(
					unary ? FUNCTION_TEMPLATE_ABI_EXPRESSION_UNARY : FUNCTION_TEMPLATE_ABI_EXPRESSION_BINARY,
					left, right, kNoFunctionTemplateAbiType, 0, kNoTemplateParameter, operation));
		}
		else if (node == "conditional-expression")
		{
			std::vector<FunctionTemplateAbiArgument> operands;
			for (std::size_t i = 0; i < 3; ++i)
			{
				const FunctionTemplateAbiExpressionId operand = ParseExpression();
				if (operand == kNoFunctionTemplateAbiExpression) return operand;
				operands.push_back(FunctionTemplateAbiArgument(FUNCTION_TEMPLATE_ABI_ARGUMENT_EXPRESSION,
					kNoFunctionTemplateAbiType, operand));
			}
			expression = OperandList(FUNCTION_TEMPLATE_ABI_EXPRESSION_CONDITIONAL,
				kNoFunctionTemplateAbiType, operands);
		}
		else if (node == "call-expression")
			expression = ParseCall();
		else if (node == "member-expression")
		{
			const FunctionTemplateAbiExpressionId object = ParseExpression();
			NameId member = 0;
			if (object != kNoFunctionTemplateAbiExpression && BeginNode("identifier", &member) && EndNode())
				expression = AppendAbiExpression(program_, FunctionTemplateAbiExpression(
					FUNCTION_TEMPLATE_ABI_EXPRESSION_OBJECT_MEMBER, object,
					kNoFunctionTemplateAbiExpression, kNoFunctionTemplateAbiType, member,
					kNoTemplateParameter, OPERATOR_NONE, spelling == "->"));
		}
		else if (node == "cast-expression")
		{
			FunctionTemplateAbiTypeId type = ParseType();
			const FunctionTemplateAbiExpressionId operand = ParseExpression();
			if (type == kNoFunctionTemplateAbiType || operand == kNoFunctionTemplateAbiExpression)
				return kNoFunctionTemplateAbiExpression;
			if (spelling == "static_cast")
			{
				// Clang encodes the referred-to type for a reference cast.
				const FunctionTemplateAbiType& cast = program_->function_template_abi_types[type];
				if (cast.kind == FUNCTION_TEMPLATE_ABI_TYPE_LVALUE_REFERENCE || cast.kind == FUNCTION_TEMPLATE_ABI_TYPE_RVALUE_REFERENCE)
					type = cast.child;
				else if (cast.kind == FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE && program_->types.IsReference(cast.concrete_type))
					type = ConcreteType(program_->types.Get(cast.concrete_type).child);
				expression = AppendAbiExpression(program_, FunctionTemplateAbiExpression(
					FUNCTION_TEMPLATE_ABI_EXPRESSION_STATIC_CAST, operand, kNoFunctionTemplateAbiExpression, type));
			}
			// A C-style cast is a conversion of its one operand.
			else if (spelling.compare(0, 9, "OP_LPAREN") == 0)
				expression = OperandList(FUNCTION_TEMPLATE_ABI_EXPRESSION_CONVERSION, type,
					std::vector<FunctionTemplateAbiArgument>(1, FunctionTemplateAbiArgument(
						FUNCTION_TEMPLATE_ABI_ARGUMENT_EXPRESSION, kNoFunctionTemplateAbiType, operand)));
		}
		else if (node == "braced-init-list" || node == "initializer-list")
		{
			std::vector<FunctionTemplateAbiArgument> operands;
			if (!ParseOperands(&operands)) return kNoFunctionTemplateAbiExpression;
			expression = OperandList(FUNCTION_TEMPLATE_ABI_EXPRESSION_BRACED_LIST,
				kNoFunctionTemplateAbiType, operands);
		}
		if (expression == kNoFunctionTemplateAbiExpression || !EndNode())
			return kNoFunctionTemplateAbiExpression;
		return expression;
	}

	bool Complete() const { return position_ == atoms_.size(); }

	// The scope an unqualified name of a dependent expression is looked up
	// in: a type names a conversion, an enumerator a value, a function an
	// unresolved callee.
	void SetScope(ScopeId scope) { scope_ = scope; }

	// Forms the class a non-dependent template-id names, so a member it
	// qualifies resolves (`L_ZNSt14numeric_limitsImE6digitsE`).
	typedef std::function<TypeId(std::size_t, const std::vector<TemplateArgument>&)> ClassResolver;
	void SetClassResolver(const ClassResolver& resolver) { resolve_class_ = resolver; }

private:
	bool AtNodeEnd() const
	{
		return position_ < atoms_.size() &&
			ResultIdentityKind(atoms_[position_]) == FUNCTION_TEMPLATE_RESULT_NODE_END;
	}

	FunctionTemplateAbiTypeId ConcreteType(TypeId type)
	{
		return AppendAbiType(program_, FunctionTemplateAbiType(
			FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE, kNoFunctionTemplateAbiType,
			0, 0, kNoTemplateParameter, 0, type));
	}

	// A non-dependent operand is its value (`Lb1E`, `Lm8E`).
	FunctionTemplateAbiExpressionId Value(TypeId type, std::int64_t value)
	{
		FunctionTemplateAbiExpression expression(FUNCTION_TEMPLATE_ABI_EXPRESSION_VALUE,
			kNoFunctionTemplateAbiExpression, kNoFunctionTemplateAbiExpression, ConcreteType(type));
		expression.value = value;
		return AppendAbiExpression(program_, expression);
	}

	FunctionTemplateAbiExpressionId OperandList(FunctionTemplateAbiExpressionKind kind,
		FunctionTemplateAbiTypeId type, const std::vector<FunctionTemplateAbiArgument>& operands)
	{
		std::uint32_t begin = 0;
		if (!StoreArguments(operands, &begin)) return kNoFunctionTemplateAbiExpression;
		return AppendAbiExpression(program_, FunctionTemplateAbiExpression(kind,
			kNoFunctionTemplateAbiExpression, kNoFunctionTemplateAbiExpression, type, 0,
			kNoTemplateParameter, OPERATOR_NONE, false, begin,
			static_cast<std::uint32_t>(operands.size())));
	}

	static OperatorKind OperatorForSpelling(const std::string& spelling, bool unary)
	{
		if (unary)
			return spelling == "!" || spelling == "not" ? OPERATOR_LOGICAL_NOT :
				spelling == "-" ? OPERATOR_MINUS : spelling == "+" ? OPERATOR_PLUS :
				spelling == "*" ? OPERATOR_STAR : spelling == "&" ? OPERATOR_AMPERSAND :
				spelling == "~" ? OPERATOR_COMPLEMENT : OPERATOR_NONE;
		static const struct { const char* spelling; OperatorKind kind; } table[] = {
			{"+", OPERATOR_PLUS}, {"-", OPERATOR_MINUS}, {"*", OPERATOR_STAR},
			{"/", OPERATOR_DIVIDE}, {"%", OPERATOR_REMAINDER}, {"&", OPERATOR_AMPERSAND},
			{"|", OPERATOR_BIT_OR}, {"^", OPERATOR_BIT_XOR}, {"<<", OPERATOR_LEFT_SHIFT},
			{">>", OPERATOR_RIGHT_SHIFT}, {"==", OPERATOR_EQUAL}, {"!=", OPERATOR_NOT_EQUAL},
			{"<", OPERATOR_LESS}, {">", OPERATOR_GREATER}, {"<=", OPERATOR_LESS_EQUAL},
			{">=", OPERATOR_GREATER_EQUAL}, {"&&", OPERATOR_LOGICAL_AND}, {"and", OPERATOR_LOGICAL_AND},
			{"||", OPERATOR_LOGICAL_OR}, {"or", OPERATOR_LOGICAL_OR}, {",", OPERATOR_COMMA}};
		for (std::size_t i = 0; i < sizeof(table) / sizeof(table[0]); ++i)
			if (spelling == table[i].spelling) return table[i].kind;
		return OPERATOR_NONE;
	}

	// A fundamental type written with simple type specifiers (`int`,
	// `unsigned long`); false for anything else.
	bool FundamentalType(const std::vector<std::string>& words, TypeId* type) const
	{
		int is_unsigned = 0, is_signed = 0, longs = 0, shorts = 0;
		std::string base;
		for (std::size_t i = 0; i < words.size(); ++i)
		{
			const std::string& w = words[i];
			if (w == "unsigned") ++is_unsigned;
			else if (w == "signed") ++is_signed;
			else if (w == "long") ++longs;
			else if (w == "short") ++shorts;
			else if (base.empty()) base = w;
			else return false;
		}
		FundamentalKind kind;
		if (base == "void") kind = FUND_VOID;
		else if (base == "bool") kind = FUND_BOOL;
		else if (base == "char") kind = is_unsigned ? FUND_UNSIGNED_CHAR : is_signed ? FUND_SIGNED_CHAR : FUND_CHAR;
		else if (base == "wchar_t") kind = FUND_WCHAR_T;
		else if (base == "char16_t") kind = FUND_CHAR16_T;
		else if (base == "char32_t") kind = FUND_CHAR32_T;
		else if (base == "float") kind = FUND_FLOAT;
		else if (base == "double") kind = longs ? FUND_LONG_DOUBLE : FUND_DOUBLE;
		else if (base.empty() || base == "int")
		{
			if (base.empty() && !is_unsigned && !is_signed && !longs && !shorts) return false;
			kind = shorts ? (is_unsigned ? FUND_UNSIGNED_SHORT_INT : FUND_SHORT_INT) :
				longs >= 2 ? (is_unsigned ? FUND_UNSIGNED_LONG_LONG_INT : FUND_LONG_LONG_INT) :
				longs ? (is_unsigned ? FUND_UNSIGNED_LONG_INT : FUND_LONG_INT) :
				(is_unsigned ? FUND_UNSIGNED_INT : FUND_INT);
		}
		else return false;
		*type = program_->types.Fundamental(kind);
		return true;
	}

	// `this`, `true`, `false`.
	FunctionTemplateAbiExpressionId KeywordValue(const std::string& spelling)
	{
		if (spelling == "this")
			return AppendAbiExpression(program_, FunctionTemplateAbiExpression(
				FUNCTION_TEMPLATE_ABI_EXPRESSION_THIS));
		if (spelling == "true" || spelling == "false")
			return Value(program_->types.Fundamental(FUND_BOOL), spelling == "true");
		return kNoFunctionTemplateAbiExpression;
	}

	// An unqualified name in a value position: an enumerator is its value;
	// a function, or a name lookup does not find, is an unresolved callee
	// argument-dependent lookup completes at instantiation.
	FunctionTemplateAbiExpressionId UnqualifiedValue(NameId name)
	{
		if (scope_ != kNoScope)
		{
			const LookupResult found = program_->LookupName(scope_, name, LOOKUP_ORDINARY);
			if (found.ordinary != kNoBinding)
			{
				const BindingRecord& binding = program_->bindings[found.ordinary];
				if (binding.kind == BIND_ENUMERATOR)
					return Value(binding.type, binding.value);
				if (binding.kind != BIND_FUNCTION) return kNoFunctionTemplateAbiExpression;
				// A class's member function is no argument-dependent callee:
				// one declaration is resolved (`L_ZNS0_14accepts_lvalueES2_E`).
				if (binding.member_owner != kNoEntity && found.OrdinaryCount() == 1 &&
					!found.HasFunctionTemplateLookup())
				{
					FunctionTemplateAbiExpression resolved(FUNCTION_TEMPLATE_ABI_EXPRESSION_FUNCTION);
					resolved.value = binding.canonical;
					return AppendAbiExpression(program_, resolved);
				}
			}
		}
		return AppendAbiExpression(program_, FunctionTemplateAbiExpression(
			FUNCTION_TEMPLATE_ABI_EXPRESSION_SOURCE_NAME, kNoFunctionTemplateAbiExpression,
			kNoFunctionTemplateAbiExpression, kNoFunctionTemplateAbiType, name));
	}

	// The type an unqualified callee names, if it names one: a fundamental
	// type keyword or a type lookup finds and no function hides.
	TypeId UnqualifiedType(NameId name) const
	{
		TypeId type = kNoType;
		if (FundamentalType(std::vector<std::string>(1, program_->names.Get(name)), &type)) return type;
		if (scope_ == kNoScope) return kNoType;
		const LookupResult ordinary = program_->LookupName(scope_, name, LOOKUP_ORDINARY);
		if (ordinary.ordinary != kNoBinding || ordinary.HasFunctionTemplateLookup()) return kNoType;
		return program_->LookupName(scope_, name, LOOKUP_TYPE).type;
	}

	// Whether the qualified name at the cursor ends in a component a
	// declaration resolves to a type, so a call of it is a conversion.
	bool QualifiedNamesType() const
	{
		std::size_t depth = 0, terminal = atoms_.size();
		for (std::size_t i = position_; i < atoms_.size(); ++i)
		{
			const FunctionTemplateResultIdentityAtomKind kind = ResultIdentityKind(atoms_[i]);
			if (kind == FUNCTION_TEMPLATE_RESULT_QUALIFIED_BEGIN) ++depth;
			else if (kind == FUNCTION_TEMPLATE_RESULT_QUALIFIED_END && --depth == 0)
				break;
			else if (kind == FUNCTION_TEMPLATE_RESULT_COMPONENT && depth == 1) terminal = i;
		}
		if (terminal + 1 >= atoms_.size()) return false;
		const FunctionTemplateResultIdentityAtomKind marker = ResultIdentityKind(atoms_[terminal + 1]);
		const std::uint64_t value = ResultIdentityValue(atoms_[terminal + 1]);
		if (marker == FUNCTION_TEMPLATE_RESULT_ENTITY) return true;
		if (marker != FUNCTION_TEMPLATE_RESULT_DECLARATION || value >= program_->bindings.size())
			return false;
		const BindingKind kind = program_->bindings[program_->bindings[
			static_cast<BindingId>(value)].canonical].kind;
		return kind == BIND_TYPE || kind == BIND_TYPE_ALIAS;
	}

	// Call operands: an argument list's expressions, pack expansions and
	// braced lists, up to the enclosing node's end.
	bool ParseOperands(std::vector<FunctionTemplateAbiArgument>* operands)
	{
		NameId ignored = 0;
		while (position_ < atoms_.size() && !AtNodeEnd())
		{
			const bool expansion = IsNode("pack-expansion-expression");
			if (expansion && !BeginNode("pack-expansion-expression", &ignored))
				return false;
			const FunctionTemplateAbiExpressionId operand = ParseExpression();
			if (operand == kNoFunctionTemplateAbiExpression) return false;
			if (expansion && !EndNode()) return false;
			operands->push_back(FunctionTemplateAbiArgument(FUNCTION_TEMPLATE_ABI_ARGUMENT_EXPRESSION,
				kNoFunctionTemplateAbiType, operand, expansion));
		}
		return true;
	}

	// `f(args)` is a call, and `T(args)` of a type a conversion.
	FunctionTemplateAbiExpressionId ParseCall()
	{
		FunctionTemplateAbiTypeId type = kNoFunctionTemplateAbiType;
		FunctionTemplateAbiExpressionId callee = kNoFunctionTemplateAbiExpression;
		NameId payload = 0;
		if (IsNode("id-expression"))
		{
			if (!BeginNode("id-expression", &payload)) return kNoFunctionTemplateAbiExpression;
			if (position_ < atoms_.size() &&
				ResultIdentityKind(atoms_[position_]) == FUNCTION_TEMPLATE_RESULT_QUALIFIED_BEGIN)
			{
				if (QualifiedNamesType()) type = ParseQualifiedType(false, 0);
				else callee = ParseQualifiedExpression();
			}
			else if (AtNodeEnd())
			{
				const TypeId named = UnqualifiedType(payload);
				if (named != kNoType) type = ConcreteType(named);
				else callee = UnqualifiedValue(payload);
			}
			else callee = ParseExpression();
			if ((type == kNoFunctionTemplateAbiType && callee == kNoFunctionTemplateAbiExpression) ||
				!EndNode())
				return kNoFunctionTemplateAbiExpression;
		}
		else callee = ParseExpression();
		if (type == kNoFunctionTemplateAbiType && callee == kNoFunctionTemplateAbiExpression)
			return kNoFunctionTemplateAbiExpression;
		const char* list = IsNode("paren-argument-list") ? "paren-argument-list" : "argument-list";
		NameId ignored = 0;
		std::vector<FunctionTemplateAbiArgument> operands;
		if (!BeginNode(list, &ignored) || !ParseOperands(&operands) || !EndNode())
			return kNoFunctionTemplateAbiExpression;
		if (type != kNoFunctionTemplateAbiType)
			return OperandList(FUNCTION_TEMPLATE_ABI_EXPRESSION_CONVERSION, type, operands);
		std::uint32_t begin = 0;
		if (!StoreArguments(operands, &begin)) return kNoFunctionTemplateAbiExpression;
		return AppendAbiExpression(program_, FunctionTemplateAbiExpression(
			FUNCTION_TEMPLATE_ABI_EXPRESSION_CALL, callee, kNoFunctionTemplateAbiExpression,
			kNoFunctionTemplateAbiType, 0, kNoTemplateParameter, OPERATOR_NONE, false, begin,
			static_cast<std::uint32_t>(operands.size())));
	}

	bool StartsType() const
	{
		if (position_ >= atoms_.size()) return false;
		const FunctionTemplateResultIdentityAtomKind kind = ResultIdentityKind(atoms_[position_]);
		if (kind == FUNCTION_TEMPLATE_RESULT_TYPE || kind == FUNCTION_TEMPLATE_RESULT_BOUND_ARGUMENT)
			return true;
		if (kind == FUNCTION_TEMPLATE_RESULT_PARAMETER)
		{
			const std::uint64_t parameter = ResultIdentityValue(atoms_[position_]);
			return parameter < parameters_.size() && parameters_[parameter].kind == TEMPLATE_ARGUMENT_TYPE;
		}
		if (kind == FUNCTION_TEMPLATE_RESULT_QUALIFIED_BEGIN) return QualifiedNamesType();
		return IsNode("decltype-specifier") || IsNode("type-specifier") || IsNode("cv-qualifier") ||
			IsNode("builtin-transform-type") || IsDecltypeSpecifier();
	}

	bool IsDecltypeSpecifier() const
	{
		if (!IsNode("decl-specifier") || position_ + 1 >= atoms_.size() ||
			ResultIdentityKind(atoms_[position_ + 1]) != FUNCTION_TEMPLATE_RESULT_NODE_PAYLOAD)
			return false;
		return program_->names.Get(static_cast<NameId>(
			ResultIdentityValue(atoms_[position_ + 1]))).compare(0, 8, "decltype") == 0;
	}

	// `sizeof(T)` of a dependent type is `st <type>`, of a dependent
	// expression `sz <expr>`; a non-dependent one is its value.
	FunctionTemplateAbiExpressionId ParseSizeofOperand()
	{
		if (position_ < atoms_.size() && ResultIdentityKind(atoms_[position_]) == FUNCTION_TEMPLATE_RESULT_TYPE)
		{
			const TypeId type = static_cast<TypeId>(ResultIdentityValue(atoms_[position_++]));
			if (type == kNoType || type > program_->types.Size()) return kNoFunctionTemplateAbiExpression;
			return Value(program_->types.Fundamental(FUND_UNSIGNED_LONG_INT),
				static_cast<std::int64_t>(program_->SizeOf(type)));
		}
		if (StartsType())
		{
			const FunctionTemplateAbiTypeId type = ParseType();
			if (type == kNoFunctionTemplateAbiType) return kNoFunctionTemplateAbiExpression;
			return AppendAbiExpression(program_, FunctionTemplateAbiExpression(
				FUNCTION_TEMPLATE_ABI_EXPRESSION_SIZEOF_TYPE, kNoFunctionTemplateAbiExpression,
				kNoFunctionTemplateAbiExpression, type));
		}
		const FunctionTemplateAbiExpressionId operand = ParseExpression();
		if (operand == kNoFunctionTemplateAbiExpression) return operand;
		return AppendAbiExpression(program_, FunctionTemplateAbiExpression(
			FUNCTION_TEMPLATE_ABI_EXPRESSION_SIZEOF_EXPRESSION, operand));
	}

	bool ParseCvQualifiers(std::uint8_t* cv)
	{
		while (IsNode("cv-qualifier"))
		{
			NameId qualifier = 0;
			if (!BeginNode("cv-qualifier", &qualifier) || !EndNode()) return false;
			const std::string& spelling = program_->names.Get(qualifier);
			if (spelling == "const") *cv |= CV_CONST;
			else if (spelling == "volatile") *cv |= CV_VOLATILE;
			else return false;
		}
		return true;
	}

	bool IsNode(const char* tag) const
	{
		return position_ < atoms_.size() &&
			ResultIdentityKind(atoms_[position_]) ==
				FUNCTION_TEMPLATE_RESULT_NODE_BEGIN &&
			program_->names.Get(static_cast<NameId>(
				ResultIdentityValue(atoms_[position_]))) == tag;
	}

	bool BeginNode(const char* tag, NameId* payload)
	{
		if (!IsNode(tag) || ++position_ >= atoms_.size() ||
			ResultIdentityKind(atoms_[position_]) !=
				FUNCTION_TEMPLATE_RESULT_NODE_PAYLOAD)
			return false;
		*payload = static_cast<NameId>(ResultIdentityValue(atoms_[position_++]));
		return true;
	}

	bool EndNode()
	{
		if (position_ >= atoms_.size() ||
			ResultIdentityKind(atoms_[position_]) !=
				FUNCTION_TEMPLATE_RESULT_NODE_END) return false;
		++position_;
		return true;
	}

	bool ParseEmptyNode(const char* tag)
	{
		NameId payload = 0;
		return BeginNode(tag, &payload) && EndNode();
	}

	FunctionTemplateAbiExpressionId ParseQualifiedExpression()
	{
		FunctionTemplateAbiExpressionId expression = kNoFunctionTemplateAbiExpression;
		(void)ParseQualifiedType(true, &expression);
		if (expression == kNoFunctionTemplateAbiExpression ||
			!IsNode("abstract-declarator")) return expression;
		// Template arguments can retain an ambiguous type-id parse of a
		// qualified zero-argument call. In this expression slot the empty
		// function suffix denotes the call, not a formed function type.
		NameId payload = 0;
		if (!BeginNode("abstract-declarator", &payload) ||
			!ParseEmptyNode("parameter-clause") || !EndNode())
			return kNoFunctionTemplateAbiExpression;
		return AppendAbiExpression(program_, FunctionTemplateAbiExpression(
			FUNCTION_TEMPLATE_ABI_EXPRESSION_CALL, expression));
	}

	bool StoreArguments(const std::vector<FunctionTemplateAbiArgument>& arguments,
		std::uint32_t* begin)
	{
		if (arguments.size() > std::numeric_limits<std::uint32_t>::max() ||
			program_->function_template_abi_arguments.size() >
				std::numeric_limits<std::uint32_t>::max() - arguments.size()) return false;
		*begin = static_cast<std::uint32_t>(program_->function_template_abi_arguments.size());
		program_->function_template_abi_arguments.insert(
			program_->function_template_abi_arguments.end(), arguments.begin(), arguments.end());
		return true;
	}

	FunctionTemplateAbiTypeId ParsePrimaryType()
	{
		if (position_ >= atoms_.size()) return kNoFunctionTemplateAbiType;
		if (IsNode("builtin-transform-type"))
		{
			NameId name = 0;
			if (!BeginNode("builtin-transform-type", &name))
				return kNoFunctionTemplateAbiType;
			const FunctionTemplateAbiTypeId operand = ParseType();
			if (name == 0 || operand == kNoFunctionTemplateAbiType || !EndNode())
				return kNoFunctionTemplateAbiType;
			return AppendAbiType(program_, FunctionTemplateAbiType(
				FUNCTION_TEMPLATE_ABI_TYPE_BUILTIN_TRANSFORM, operand, name));
		}
		const FunctionTemplateResultIdentityAtomKind kind =
			ResultIdentityKind(atoms_[position_]);
		if (kind == FUNCTION_TEMPLATE_RESULT_PARAMETER || kind == FUNCTION_TEMPLATE_RESULT_FUNCTION_PARAMETER)
		{
			const std::uint64_t parameter = ResultIdentityValue(atoms_[position_++]);
			if (parameter >= kNoTemplateParameter)
				return kNoFunctionTemplateAbiType;
			return AppendAbiType(program_, FunctionTemplateAbiType(
				FUNCTION_TEMPLATE_ABI_TYPE_PARAMETER, kNoFunctionTemplateAbiType,
				0, 0, static_cast<std::uint32_t>(parameter)));
		}
		if (kind == FUNCTION_TEMPLATE_RESULT_BOUND_ARGUMENT)
		{
			const std::uint64_t list = ResultIdentityValue(atoms_[position_++]);
			if (list >= kNoTemplateArgumentList) return kNoFunctionTemplateAbiType;
			const TemplateArgument& argument = program_->GetTemplateArgument(
				static_cast<TemplateArgumentListId>(list), 0);
			if (argument.kind != TEMPLATE_ARGUMENT_TYPE || argument.type == kNoType)
				return kNoFunctionTemplateAbiType;
			return AppendAbiType(program_, FunctionTemplateAbiType(
				FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE, kNoFunctionTemplateAbiType,
				0, 0, kNoTemplateParameter, 0, argument.type));
		}
		if (kind == FUNCTION_TEMPLATE_RESULT_TYPE)
		{
			const std::uint64_t type = ResultIdentityValue(atoms_[position_++]);
			if (type == kNoType || type > program_->types.Size())
				return kNoFunctionTemplateAbiType;
			return AppendAbiType(program_, FunctionTemplateAbiType(
				FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE,
				kNoFunctionTemplateAbiType, 0, 0, kNoTemplateParameter, 0,
				static_cast<TypeId>(type)));
		}
		if (kind == FUNCTION_TEMPLATE_RESULT_QUALIFIED_BEGIN)
			return ParseQualifiedType(false, 0);
		if (IsNode("type-specifier"))
		{
			std::vector<std::string> words;
			while (IsNode("type-specifier"))
			{
				NameId word = 0;
				if (!BeginNode("type-specifier", &word) || !EndNode()) return kNoFunctionTemplateAbiType;
				words.push_back(program_->names.Get(word));
			}
			TypeId type = kNoType;
			return FundamentalType(words, &type) ? ConcreteType(type) : kNoFunctionTemplateAbiType;
		}
		const bool specifier = IsDecltypeSpecifier();
		if (!specifier && !IsNode("decltype-specifier"))
			return kNoFunctionTemplateAbiType;
		NameId payload = 0;
		if (!BeginNode(specifier ? "decl-specifier" : "decltype-specifier", &payload))
			return kNoFunctionTemplateAbiType;
		const bool id_expression = IsNode("id-expression") ||
			IsNode("member-expression") ||
			(position_ < atoms_.size() && (ResultIdentityKind(atoms_[position_]) == FUNCTION_TEMPLATE_RESULT_FUNCTION_PARAMETER ||
			 ResultIdentityKind(atoms_[position_]) == FUNCTION_TEMPLATE_RESULT_PARAMETER));
		const FunctionTemplateAbiExpressionId expression = ParseExpression();
		if (expression == kNoFunctionTemplateAbiExpression || !EndNode())
			return kNoFunctionTemplateAbiType;
		return AppendAbiType(program_, FunctionTemplateAbiType(
			id_expression ? FUNCTION_TEMPLATE_ABI_TYPE_DECLTYPE_ID :
				FUNCTION_TEMPLATE_ABI_TYPE_DECLTYPE, kNoFunctionTemplateAbiType,
			0, 0, kNoTemplateParameter, 0, kNoType, kNoEntity, 0, 0,
			expression));
	}

	FunctionTemplateAbiTypeId ApplyNestedDeclarator(FunctionTemplateAbiTypeId nested,
		FunctionTemplateAbiTypeId type)
	{
		if (nested == kNoFunctionTemplateAbiType) return type;
		FunctionTemplateAbiType modifier = program_->function_template_abi_types[nested];
		modifier.child = ApplyNestedDeclarator(modifier.child, type);
		return AppendAbiType(program_, modifier);
	}

	bool ParseAbstractDeclarator(FunctionTemplateAbiTypeId* root,
		bool* pack_expansion, const char* tag = "abstract-declarator")
	{
		NameId payload = 0;
		if (!BeginNode(tag, &payload)) return false;
		FunctionTemplateAbiTypeId nested = kNoFunctionTemplateAbiType;
		while (position_ < atoms_.size() &&
			ResultIdentityKind(atoms_[position_]) != FUNCTION_TEMPLATE_RESULT_NODE_END)
		{
			if (ResultIdentityKind(atoms_[position_]) == FUNCTION_TEMPLATE_RESULT_PACK_EXPANSION)
			{
				if (!pack_expansion) return false;
				*pack_expansion = true;
				++position_;
				continue;
			}
			if (IsNode("identifier"))
			{
				if (!ParseEmptyNode("identifier")) return false;
				continue;
			}
			if (IsNode("nested-declarator"))
			{
				if (!BeginNode("nested-declarator", &payload) ||
					!ParseAbstractDeclarator(&nested, pack_expansion, IsNode("declarator") ? "declarator" : "abstract-declarator") || !EndNode()) return false;
				continue;
			}
			if (IsNode("parameter-clause"))
			{
				if (!BeginNode("parameter-clause", &payload)) return false;
				std::vector<FunctionTemplateAbiArgument> arguments;
				while (IsNode("parameter-declaration"))
				{
					if (!BeginNode("parameter-declaration", &payload) ||
						!BeginNode("decl-specifier-seq", &payload)) return false;
					// A pack the enclosing class binds is its arguments, one
					// parameter each (`void(Ps...)` is `FvS2_S4_E`).
					if (position_ < atoms_.size() && ResultIdentityKind(atoms_[position_]) == FUNCTION_TEMPLATE_RESULT_SUBSTITUTION_BEGIN)
					{
						++position_;
						while (position_ < atoms_.size() && ResultIdentityKind(atoms_[position_]) != FUNCTION_TEMPLATE_RESULT_SUBSTITUTION_END)
						{
							const FunctionTemplateAbiTypeId bound = ParseType();
							if (bound == kNoFunctionTemplateAbiType) return false;
							arguments.push_back(FunctionTemplateAbiArgument(FUNCTION_TEMPLATE_ABI_ARGUMENT_TYPE, bound));
						}
						bool expansion = false;
						FunctionTemplateAbiTypeId pack = kNoFunctionTemplateAbiType;
						if (++position_ > atoms_.size() || !EndNode() ||
							(IsNode("declarator") && !ParseAbstractDeclarator(&pack, &expansion, "declarator")) ||
							pack != kNoFunctionTemplateAbiType || !EndNode())
							return false;
						continue;
					}
					FunctionTemplateAbiTypeId type = ParseType();
					if (type == kNoFunctionTemplateAbiType || !EndNode()) return false;
					bool expansion = false;
					if (IsNode("declarator") && !ParseAbstractDeclarator(&type, &expansion, "declarator")) return false;
					if (!EndNode()) return false;
					arguments.push_back(FunctionTemplateAbiArgument(FUNCTION_TEMPLATE_ABI_ARGUMENT_TYPE,
						type, kNoFunctionTemplateAbiExpression, expansion));
				}
				std::uint32_t begin = 0;
				if (!EndNode() || !StoreArguments(arguments, &begin)) return false;
				*root = AppendAbiType(program_, FunctionTemplateAbiType(FUNCTION_TEMPLATE_ABI_TYPE_FUNCTION,
					*root, 0, 0, kNoTemplateParameter, 0, kNoType, kNoEntity, begin,
					static_cast<std::uint32_t>(arguments.size())));
				continue;
			}
			if (!BeginNode("ptr-operator", &payload)) return false;
			FunctionTemplateAbiTypeKind kind = FUNCTION_TEMPLATE_ABI_TYPE_POINTER;
			std::uint32_t begin = 0, count = 0;
			const std::string& op = program_->names.Get(payload);
			if (op == "&") kind = FUNCTION_TEMPLATE_ABI_TYPE_LVALUE_REFERENCE;
			else if (op == "&&") kind = FUNCTION_TEMPLATE_ABI_TYPE_RVALUE_REFERENCE;
			else if (op.empty())
			{
				const FunctionTemplateAbiTypeId owner = ParseType();
				if (owner == kNoFunctionTemplateAbiType || !StoreArguments(
					std::vector<FunctionTemplateAbiArgument>(1, FunctionTemplateAbiArgument(
						FUNCTION_TEMPLATE_ABI_ARGUMENT_TYPE, owner)), &begin)) return false;
				kind = FUNCTION_TEMPLATE_ABI_TYPE_MEMBER_POINTER;
				count = 1;
			}
			else if (op != "*") return false;
			if (!EndNode()) return false;
			*root = AppendAbiType(program_, FunctionTemplateAbiType(kind, *root, 0, 0,
				kNoTemplateParameter, 0, kNoType, kNoEntity, begin, count));
		}
		*root = ApplyNestedDeclarator(nested, *root);
		return EndNode();
	}

	bool IsTypeParameterName(NameId name) const
	{
		for (std::size_t parameter = 0; parameter < parameters_.size(); ++parameter)
			if (parameters_[parameter].kind == TEMPLATE_ARGUMENT_TYPE &&
				parameters_[parameter].name == name) return true;
		return false;
	}

	TemplateArgumentKind ArgumentKind(EntityId entity, std::size_t ordinal) const
	{
		if (entity >= class_template_by_entity_.size())
			return TEMPLATE_ARGUMENT_TYPE;
		const std::uint32_t index = class_template_by_entity_[entity];
		if (index == kNoDumpEdge || index >= class_templates_.size() ||
			class_templates_[index].parameters.empty())
			return TEMPLATE_ARGUMENT_TYPE;
		const std::vector<TemplateParameter>& parameters =
			class_templates_[index].parameters;
		const std::size_t parameter = ordinal < parameters.size() ? ordinal :
			parameters.size() - 1;
		return parameters[parameter].kind;
	}

	EntityId EntityFromMarker(FunctionTemplateResultIdentityAtomKind kind,
		std::uint64_t value) const
	{
		if (kind == FUNCTION_TEMPLATE_RESULT_ENTITY)
			return value < program_->entities.size() ?
				static_cast<EntityId>(value) : kNoEntity;
		if (kind != FUNCTION_TEMPLATE_RESULT_DECLARATION ||
			value >= program_->bindings.size()) return kNoEntity;
		const BindingRecord& binding = program_->bindings[
			program_->bindings[static_cast<BindingId>(value)].canonical];
		if (binding.type == kNoType) return kNoEntity;
		const TypeRecord& type = program_->types.Get(
			program_->types.RemoveTopCv(binding.type));
		return type.kind == TYPE_NAMED ? type.entity : kNoEntity;
	}

	// Whether the argument at the cursor is written as an expression.
	bool StartsValueExpression() const
	{
		if (position_ < atoms_.size() && ResultIdentityKind(atoms_[position_]) == FUNCTION_TEMPLATE_RESULT_LITERAL_ARGUMENT)
			return true;
		return IsNode("cast-expression") || IsNode("unary-expression") || IsNode("binary-expression") ||
			IsNode("keyword-literal") || IsNode("sizeof-expression") || IsNode("conditional-expression") ||
			IsNode("parenthesized-expression");
	}

	bool FoldedValue(FunctionTemplateAbiExpressionId id, TypeId* type, std::int64_t* value) const
	{
		if (id == kNoFunctionTemplateAbiExpression) return false;
		const FunctionTemplateAbiExpression& node = program_->function_template_abi_expressions[id];
		if (node.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_VALUE)
		{
			*type = program_->function_template_abi_types[node.type].concrete_type;
			*value = node.value;
			return true;
		}
		if (node.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_INTEGRAL)
		{
			const TemplateArgument& argument = program_->canonical_template_arguments[node.argument_begin];
			*type = argument.type;
			*value = argument.value;
			return true;
		}
		TypeId operand_type = kNoType;
		std::int64_t operand = 0;
		if (node.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_UNARY && FoldedValue(node.left, &operand_type, &operand))
		{
			*type = operand_type;
			*value = node.operation == OPERATOR_MINUS ? -operand : node.operation == OPERATOR_COMPLEMENT ?
				~operand : node.operation == OPERATOR_LOGICAL_NOT ? !operand : operand;
			if (node.operation == OPERATOR_LOGICAL_NOT) *type = program_->types.Fundamental(FUND_BOOL);
			return node.operation == OPERATOR_MINUS || node.operation == OPERATOR_COMPLEMENT ||
				node.operation == OPERATOR_LOGICAL_NOT || node.operation == OPERATOR_PLUS;
		}
		if (node.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_STATIC_CAST && FoldedValue(node.left, &operand_type, &operand) &&
			program_->function_template_abi_types[node.type].kind == FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE)
		{
			*type = program_->function_template_abi_types[node.type].concrete_type;
			const std::size_t size = program_->SizeOf(*type);
			*value = size >= 8 ? operand : static_cast<std::int64_t>(
				static_cast<std::uint64_t>(operand) & ((UINT64_C(1) << (size * 8)) - 1));
			return program_->types.Get(*type).kind == TYPE_FUNDAMENTAL;
		}
		return false;
	}

	// A non-dependent template argument is its value (14.3.2): clang's
	// `Lm18446744073709551615E` for `static_cast<size_t>(-1)`.
	FunctionTemplateAbiExpressionId Fold(FunctionTemplateAbiExpressionId id)
	{
		TypeId type = kNoType;
		std::int64_t value = 0;
		if (!FoldedValue(id, &type, &value)) return id;
		const FunctionTemplateAbiExpression& node = program_->function_template_abi_expressions[id];
		if (node.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_VALUE || node.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_INTEGRAL)
			return id;
		return Value(type, value);
	}

	// Steps over a default argument the reader cannot express, to the end
	// of its argument; the component is then usable only as a written
	// qualifier.
	bool SkipArgument(std::size_t start, ParsedComponent* component, std::size_t mark)
	{
		component->unread_default = true;
		component->arguments.resize(mark, FunctionTemplateAbiArgument(FUNCTION_TEMPLATE_ABI_ARGUMENT_TYPE));
		std::size_t depth = 0;
		for (position_ = start; position_ < atoms_.size(); ++position_)
		{
			const FunctionTemplateResultIdentityAtomKind kind = ResultIdentityKind(atoms_[position_]);
			if (kind == FUNCTION_TEMPLATE_RESULT_ARGUMENT_BEGIN) ++depth;
			else if (kind == FUNCTION_TEMPLATE_RESULT_ARGUMENT_END && depth-- == 0)
			{
				++position_;
				return true;
			}
		}
		return false;
	}

	bool ParseComponent(ParsedComponent* component)
	{
		if (position_ >= atoms_.size() ||
			ResultIdentityKind(atoms_[position_]) !=
				FUNCTION_TEMPLATE_RESULT_COMPONENT)
			return false;
		component->name = static_cast<NameId>(
			ResultIdentityValue(atoms_[position_++]));
		if (position_ < atoms_.size())
		{
			const FunctionTemplateResultIdentityAtomKind marker =
				ResultIdentityKind(atoms_[position_]);
			if (marker == FUNCTION_TEMPLATE_RESULT_NAMESPACE)
				component->name_space = static_cast<ScopeId>(ResultIdentityValue(atoms_[position_++]));
			else if (marker == FUNCTION_TEMPLATE_RESULT_CALLABLE)
			{
				component->callable = true;
				++position_;
			}
			else if (marker == FUNCTION_TEMPLATE_RESULT_DECLARATION ||
				marker == FUNCTION_TEMPLATE_RESULT_ENTITY)
			{
				const std::uint64_t value = ResultIdentityValue(atoms_[position_++]);
				component->entity = EntityFromMarker(marker, value);
				if (marker == FUNCTION_TEMPLATE_RESULT_DECLARATION &&
					value < program_->bindings.size() &&
					(program_->bindings[program_->bindings[
						static_cast<BindingId>(value)].canonical].kind == BIND_TYPE ||
					 program_->bindings[program_->bindings[
						static_cast<BindingId>(value)].canonical].kind == BIND_TYPE_ALIAS))
					component->declared_type = program_->bindings[program_->bindings[
						static_cast<BindingId>(value)].canonical].type;
			}
		}
		if (position_ < atoms_.size() && ResultIdentityKind(atoms_[position_]) == FUNCTION_TEMPLATE_RESULT_FUNCTION_TEMPLATE)
			component->function_template = static_cast<std::size_t>(ResultIdentityValue(atoms_[position_++]));
		if (position_ >= atoms_.size() ||
			ResultIdentityKind(atoms_[position_]) !=
				FUNCTION_TEMPLATE_RESULT_ARGUMENTS_BEGIN) return true;
		++position_;
		for (std::size_t argument = 0; position_ < atoms_.size() &&
			ResultIdentityKind(atoms_[position_]) !=
				FUNCTION_TEMPLATE_RESULT_ARGUMENTS_END; ++argument)
		{
			if (ResultIdentityKind(atoms_[position_]) !=
				FUNCTION_TEMPLATE_RESULT_ARGUMENT_BEGIN) return false;
			++position_;
			const bool default_argument = position_ < atoms_.size() &&
				ResultIdentityKind(atoms_[position_]) == FUNCTION_TEMPLATE_RESULT_DEFAULT_ARGUMENT;
			if (default_argument) ++position_;
			else component->written_arguments = argument + 1;
			const std::size_t argument_start = position_;
			const std::size_t argument_mark = component->arguments.size();
			const bool value_argument = position_ < atoms_.size() &&
				ResultIdentityKind(atoms_[position_]) == FUNCTION_TEMPLATE_RESULT_VALUE_ARGUMENT;
			if (value_argument) ++position_;
			bool pack_expansion = position_ < atoms_.size() &&
				ResultIdentityKind(atoms_[position_]) ==
					FUNCTION_TEMPLATE_RESULT_PACK_EXPANSION;
			if (pack_expansion) ++position_;
			const bool value_parameter = position_ < atoms_.size() &&
				ResultIdentityKind(atoms_[position_]) == FUNCTION_TEMPLATE_RESULT_PARAMETER &&
				ResultIdentityValue(atoms_[position_]) < parameters_.size() &&
				parameters_[ResultIdentityValue(atoms_[position_])].kind == TEMPLATE_ARGUMENT_INTEGRAL;
			if (value_argument || value_parameter || StartsValueExpression() ||
				ArgumentKind(component->entity, argument) == TEMPLATE_ARGUMENT_INTEGRAL)
			{
				const FunctionTemplateAbiExpressionId expression = Fold(ParseExpression());
				if (expression == kNoFunctionTemplateAbiExpression && default_argument)
				{
					if (!SkipArgument(argument_start, component, argument_mark)) return false;
					continue;
				}
				if (expression == kNoFunctionTemplateAbiExpression) return false;
				component->arguments.push_back(FunctionTemplateAbiArgument(
					FUNCTION_TEMPLATE_ABI_ARGUMENT_EXPRESSION,
					kNoFunctionTemplateAbiType, expression,
					pack_expansion));
			}
			else
			{
				const FunctionTemplateAbiTypeId type = ParseType(&pack_expansion);
				if (type == kNoFunctionTemplateAbiType && default_argument)
				{
					if (!SkipArgument(argument_start, component, argument_mark)) return false;
					continue;
				}
				if (type == kNoFunctionTemplateAbiType) return false;
				component->arguments.push_back(FunctionTemplateAbiArgument(
					FUNCTION_TEMPLATE_ABI_ARGUMENT_TYPE, type,
					kNoFunctionTemplateAbiExpression, pack_expansion));
			}
			if (position_ < atoms_.size() && ResultIdentityKind(atoms_[position_]) ==
				FUNCTION_TEMPLATE_RESULT_PACK_EXPANSION)
			{
				component->arguments.back().pack_expansion = true;
				++position_;
			}
			if (position_ >= atoms_.size() ||
				ResultIdentityKind(atoms_[position_]) !=
					FUNCTION_TEMPLATE_RESULT_ARGUMENT_END) return false;
			++position_;
		}
		if (position_ >= atoms_.size() ||
			ResultIdentityKind(atoms_[position_]) !=
				FUNCTION_TEMPLATE_RESULT_ARGUMENTS_END) return false;
		++position_;
		return true;
	}

	bool ExpressionHasTemplateParameter(FunctionTemplateAbiExpressionId expression) const
	{
		if (expression == kNoFunctionTemplateAbiExpression) return false;
		const FunctionTemplateAbiExpression& node = program_->function_template_abi_expressions[expression];
		if (node.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_TEMPLATE_PARAMETER ||
			node.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_FUNCTION_PARAMETER) return true;
		if (HasTemplateParameter(node.type) || ExpressionHasTemplateParameter(node.left) ||
			ExpressionHasTemplateParameter(node.right)) return true;
		if (node.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_INTEGRAL) return false;
		for (std::size_t i = 0; i < node.argument_count; ++i)
			if (ArgumentHasTemplateParameter(program_->function_template_abi_arguments[node.argument_begin + i])) return true;
		return false;
	}

	bool ArgumentHasTemplateParameter(const FunctionTemplateAbiArgument& argument) const
	{
		return argument.kind == FUNCTION_TEMPLATE_ABI_ARGUMENT_TYPE ?
			HasTemplateParameter(argument.type) : ExpressionHasTemplateParameter(argument.expression);
	}

	bool HasTemplateParameter(FunctionTemplateAbiTypeId type) const
	{
		if (type == kNoFunctionTemplateAbiType) return false;
		const FunctionTemplateAbiType& node = program_->function_template_abi_types[type];
		if (node.kind == FUNCTION_TEMPLATE_ABI_TYPE_PARAMETER ||
			node.kind == FUNCTION_TEMPLATE_ABI_TYPE_TEMPLATE_PARAMETER_SPECIALIZATION) return true;
		if (HasTemplateParameter(node.child) || ExpressionHasTemplateParameter(node.expression)) return true;
		for (std::size_t i = 0; i < node.argument_count; ++i)
		{
			const FunctionTemplateAbiArgument& argument = program_->function_template_abi_arguments[node.argument_begin + i];
			if (ArgumentHasTemplateParameter(argument)) return true;
		}
		return false;
	}

	FunctionTemplateAbiTypeId ComponentType(
		const ParsedComponent& written, FunctionTemplateAbiTypeId owner,
		bool expression_owner = false, bool written_qualifier = false)
	{
		// A qualifier of a dependent name is spelled as written, without
		// the template's default arguments (clang's `sr11call_traitsI..E`).
		ParsedComponent component = written;
		// The root of a dependent name's qualifier, as written.
		if (written_root_) expression_owner = true;
		if (!expression_owner && written.unread_default) return kNoFunctionTemplateAbiType;
		if (expression_owner && written.written_arguments < component.arguments.size())
			component.arguments.erase(component.arguments.begin() + written.written_arguments,
				component.arguments.end());
		if (component.arguments.empty())
		{
			// A member type of a non-dependent class is that type
			// (`enable_if<true, void>::type` is `void`).
			const TypeId resolved_owner = owner == kNoFunctionTemplateAbiType || expression_owner ?
				kNoType : ResolvedClass(owner);
			if (resolved_owner != kNoType &&
				program_->types.Get(program_->types.RemoveTopCv(resolved_owner)).kind == TYPE_NAMED)
			{
				const LookupResult member = program_->LookupMember(
					program_->types.Get(program_->types.RemoveTopCv(resolved_owner)).entity,
					component.name, LOOKUP_TYPE);
				if (member.type != kNoType)
					return AppendAbiType(program_, FunctionTemplateAbiType(
						FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE, kNoFunctionTemplateAbiType,
						0, 0, kNoTemplateParameter, 0, member.type));
			}
			if (owner != kNoFunctionTemplateAbiType)
				return AppendAbiType(program_, FunctionTemplateAbiType(
					FUNCTION_TEMPLATE_ABI_TYPE_MEMBER, owner, component.name));
			for (std::size_t parameter = 0;
				parameter < parameters_.size(); ++parameter)
				if (parameters_[parameter].kind == TEMPLATE_ARGUMENT_TYPE &&
					parameters_[parameter].name == component.name)
					return AppendAbiType(program_, FunctionTemplateAbiType(
						FUNCTION_TEMPLATE_ABI_TYPE_PARAMETER,
						kNoFunctionTemplateAbiType, 0, 0,
						static_cast<std::uint32_t>(parameter)));
			if ((component.entity == kNoEntity ||
				component.entity >= program_->entities.size()) &&
				component.declared_type != kNoType && !written_qualifier)
				return AppendAbiType(program_, FunctionTemplateAbiType(
					FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE, kNoFunctionTemplateAbiType,
					0, 0, kNoTemplateParameter, 0, component.declared_type));
			if (component.entity == kNoEntity ||
				component.entity >= program_->entities.size())
				return kNoFunctionTemplateAbiType;
			if (written_qualifier)
				return AppendAbiType(program_, FunctionTemplateAbiType(
					FUNCTION_TEMPLATE_ABI_TYPE_WRITTEN_QUALIFIER,
					kNoFunctionTemplateAbiType, component.name, 0,
					kNoTemplateParameter, 0, kNoType, component.entity));
			return AppendAbiType(program_, FunctionTemplateAbiType(
				FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE, kNoFunctionTemplateAbiType,
				0, 0, kNoTemplateParameter, 0,
				program_->entities[component.entity].type));
		}
		// A template template parameter's specialization (`T_IT0_XT1_EE`).
		// Its arguments group into a pack only through the course's proxy
		// class templates, so a parameter whose own list ends in a pack is
		// left to them.
		if (owner == kNoFunctionTemplateAbiType && component.entity == kNoEntity)
			for (std::size_t parameter = 0; parameter < parameters_.size(); ++parameter)
				if (parameters_[parameter].kind == TEMPLATE_ARGUMENT_TEMPLATE &&
					parameters_[parameter].name == component.name &&
					!HasTrailingTemplateParameterPack(parameters_[parameter].template_parameters))
				{
					std::uint32_t begin = 0;
					if (!StoreArguments(component.arguments, &begin)) return kNoFunctionTemplateAbiType;
					return AppendAbiType(program_, FunctionTemplateAbiType(
						FUNCTION_TEMPLATE_ABI_TYPE_TEMPLATE_PARAMETER_SPECIALIZATION,
						kNoFunctionTemplateAbiType, component.name, 0,
						static_cast<std::uint32_t>(parameter), 0, kNoType, kNoEntity, begin,
						static_cast<std::uint32_t>(component.arguments.size())));
				}
		if (!expression_owner && owner == kNoFunctionTemplateAbiType && component.entity < program_->entities.size())
		{
			const EntityRecord& known = program_->entities[component.entity];
			bool concrete = known.template_argument_list != kNoTemplateArgumentList &&
				known.template_argument_count == component.arguments.size();
			for (std::size_t i = 0; concrete && i < component.arguments.size(); ++i)
			{
				const FunctionTemplateAbiArgument& argument = component.arguments[i];
				const TemplateArgument& formed = program_->GetTemplateArgument(known.template_argument_list, i);
				concrete = !argument.pack_expansion && argument.kind == FUNCTION_TEMPLATE_ABI_ARGUMENT_TYPE &&
					formed.kind == TEMPLATE_ARGUMENT_TYPE &&
					program_->function_template_abi_types[argument.type].kind == FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE &&
					program_->function_template_abi_types[argument.type].concrete_type == formed.type;
			}
			if (concrete)
				return AppendAbiType(program_, FunctionTemplateAbiType(FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE,
					kNoFunctionTemplateAbiType, 0, 0, kNoTemplateParameter, 0, known.type));
		}
		// A dependent member alias retains its owner, terminal and arguments.
		// Its source recipe does not require a fabricated class-template entity.
		const bool unresolved_member = !expression_owner && owner != kNoFunctionTemplateAbiType &&
			(component.entity == kNoEntity || (component.entity < program_->entities.size() &&
			program_->entities[component.entity].flavor == NAMED_TEMPLATE_PARAMETER &&
			program_->entities[component.entity].enclosing_class != kNoEntity &&
			(component.entity >= class_template_by_entity_.size() ||
			 class_template_by_entity_[component.entity] == kNoDumpEdge)));
		if (unresolved_member)
		{
			// A fully bound owner can expand its member alias through the
			// established semantic type; retain only an unresolved dependent owner.
			if (!HasTemplateParameter(owner)) return kNoFunctionTemplateAbiType;
			std::uint32_t begin = 0;
			if (!StoreArguments(component.arguments, &begin)) return kNoFunctionTemplateAbiType;
			return AppendAbiType(program_, FunctionTemplateAbiType(
				FUNCTION_TEMPLATE_ABI_TYPE_TEMPLATE_SPECIALIZATION, owner, component.name, 0,
				kNoTemplateParameter, 0, kNoType, kNoEntity, begin,
				static_cast<std::uint32_t>(component.arguments.size())));
		}
		if (component.entity == kNoEntity ||
			component.arguments.size() >
				std::numeric_limits<std::uint32_t>::max() ||
			program_->function_template_abi_arguments.size() >
				std::numeric_limits<std::uint32_t>::max() -
				component.arguments.size())
			return kNoFunctionTemplateAbiType;
		if (!written_qualifier && owner == kNoFunctionTemplateAbiType &&
			component.entity < program_->entities.size())
		{
			const EntityId enclosing = program_->entities[component.entity].enclosing_class;
			if (enclosing != kNoEntity && enclosing < program_->entities.size())
				owner = AppendAbiType(program_, FunctionTemplateAbiType(
					FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE, kNoFunctionTemplateAbiType,
					0, 0, kNoTemplateParameter, 0, program_->entities[enclosing].type));
		}
		const std::uint32_t begin = static_cast<std::uint32_t>(
			program_->function_template_abi_arguments.size());
		program_->function_template_abi_arguments.insert(
			program_->function_template_abi_arguments.end(),
			component.arguments.begin(), component.arguments.end());
		if (owner == kNoFunctionTemplateAbiType &&
			component.entity < class_template_by_entity_.size())
		{
			const std::uint32_t index =
				class_template_by_entity_[component.entity];
			if (index != kNoDumpEdge && index < class_templates_.size() &&
				class_templates_[index].template_parameter_proxy)
				return AppendAbiType(program_, FunctionTemplateAbiType(
					FUNCTION_TEMPLATE_ABI_TYPE_TEMPLATE_PARAMETER_SPECIALIZATION,
					kNoFunctionTemplateAbiType, component.name, 0,
					class_templates_[index].template_parameter_ordinal, 0,
					kNoType, component.entity, begin,
					static_cast<std::uint32_t>(component.arguments.size())));
		}
		std::uint32_t pack_parameter = kNoTemplateParameter;
		if (group_template_packs_ && !expression_owner &&
			component.entity < class_template_by_entity_.size())
		{
			const std::uint32_t index = class_template_by_entity_[component.entity];
			if (index < class_templates_.size() &&
				HasTrailingTemplateParameterPack(class_templates_[index].parameters))
				pack_parameter = static_cast<std::uint32_t>(
					FixedTemplateParameterCount(class_templates_[index].parameters));
		}
		return AppendAbiType(program_, FunctionTemplateAbiType(
			written_qualifier ? FUNCTION_TEMPLATE_ABI_TYPE_WRITTEN_QUALIFIER :
				FUNCTION_TEMPLATE_ABI_TYPE_TEMPLATE_SPECIALIZATION, owner,
			component.name, 0, pack_parameter, 0, kNoType,
			component.entity, begin,
			static_cast<std::uint32_t>(component.arguments.size())));
	}

	// The class a non-dependent qualifier type names: a concrete type, or a
	// class template-id whose arguments are all concrete types.
	TypeId ResolvedClass(FunctionTemplateAbiTypeId type)
	{
		const FunctionTemplateAbiType& node = program_->function_template_abi_types[type];
		if (node.kind == FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE) return node.concrete_type;
		if (!resolve_class_ || node.child != kNoFunctionTemplateAbiType || node.entity == kNoEntity ||
			(node.kind != FUNCTION_TEMPLATE_ABI_TYPE_TEMPLATE_SPECIALIZATION &&
			 node.kind != FUNCTION_TEMPLATE_ABI_TYPE_WRITTEN_QUALIFIER) ||
			node.entity >= class_template_by_entity_.size() ||
			class_template_by_entity_[node.entity] == kNoDumpEdge || node.argument_count == 0)
			return kNoType;
		std::vector<TemplateArgument> arguments;
		for (std::size_t i = 0; i < node.argument_count; ++i)
		{
			const FunctionTemplateAbiArgument& argument =
				program_->function_template_abi_arguments[node.argument_begin + i];
			TypeId type = kNoType;
			std::int64_t value = 0;
			if (argument.pack_expansion) return kNoType;
			if (argument.kind == FUNCTION_TEMPLATE_ABI_ARGUMENT_TYPE &&
				program_->function_template_abi_types[argument.type].kind == FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE)
				arguments.push_back(TemplateArgument(TEMPLATE_ARGUMENT_TYPE,
					program_->function_template_abi_types[argument.type].concrete_type));
			else if (argument.kind == FUNCTION_TEMPLATE_ABI_ARGUMENT_EXPRESSION &&
				FoldedValue(argument.expression, &type, &value))
				arguments.push_back(TemplateArgument(TEMPLATE_ARGUMENT_INTEGRAL, type, value));
			else return kNoType;
		}
		return resolve_class_(class_template_by_entity_[node.entity], arguments);
	}

	FunctionTemplateAbiTypeId ParseQualifiedType(bool value_terminal,
		FunctionTemplateAbiExpressionId* expression)
	{
		if (ResultIdentityKind(atoms_[position_++]) !=
			FUNCTION_TEMPLATE_RESULT_QUALIFIED_BEGIN)
			return kNoFunctionTemplateAbiType;
		FunctionTemplateAbiTypeId root = kNoFunctionTemplateAbiType;
		std::size_t components = 0;
		ScopeId name_space = kNoScope;
		// A qualifier an alias parameter binds is the bound type.
		if (position_ < atoms_.size() &&
			ResultIdentityKind(atoms_[position_]) != FUNCTION_TEMPLATE_RESULT_COMPONENT &&
			ResultIdentityKind(atoms_[position_]) != FUNCTION_TEMPLATE_RESULT_QUALIFIED_END)
		{
			const bool outer_root = written_root_;
			written_root_ = value_terminal;
			root = ParseType();
			written_root_ = outer_root;
			if (root == kNoFunctionTemplateAbiType) return root;
			++components;
			// The bound type qualifies as clang's <unresolved-type>: a
			// template-id is its unqualified name and arguments
			// (`sr9is_scalarIT_EE5value`).
			FunctionTemplateAbiType bound = program_->function_template_abi_types[root];
			if (value_terminal && bound.kind == FUNCTION_TEMPLATE_ABI_TYPE_TEMPLATE_SPECIALIZATION)
			{
				bound.kind = FUNCTION_TEMPLATE_ABI_TYPE_WRITTEN_QUALIFIER;
				bound.child = kNoFunctionTemplateAbiType;
				root = AppendAbiType(program_, bound);
			}
		}
		while (position_ < atoms_.size() &&
			ResultIdentityKind(atoms_[position_]) !=
				FUNCTION_TEMPLATE_RESULT_QUALIFIED_END)
		{
			ParsedComponent component;
			if (!ParseComponent(&component)) return kNoFunctionTemplateAbiType;
			++components;
			const bool terminal = position_ < atoms_.size() &&
				ResultIdentityKind(atoms_[position_]) ==
					FUNCTION_TEMPLATE_RESULT_QUALIFIED_END;
			if (value_terminal && terminal)
			{
				if (root == kNoFunctionTemplateAbiType &&
					name_space == kNoScope && (components != 1 || (component.arguments.empty() && !component.callable)))
					return kNoFunctionTemplateAbiType;
				std::uint32_t begin = 0;
				if (!StoreArguments(component.arguments, &begin)) return kNoFunctionTemplateAbiType;
				bool entity = component.function_template < function_templates_.size();
				if (entity)
				{
					const FunctionTemplatePattern& primary = function_templates_[component.function_template];
					entity = primary.abi_recipe != kNoFunctionTemplateAbiRecipe &&
						primary.parameters.size() == component.arguments.size();
					for (std::size_t p = 0; entity && p < component.arguments.size(); ++p)
						entity = !primary.parameters[p].pack && !component.arguments[p].pack_expansion &&
							component.arguments[p].kind == FUNCTION_TEMPLATE_ABI_ARGUMENT_TYPE &&
							program_->function_template_abi_types[component.arguments[p].type].kind == FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE;
				}
				if (entity)
				{
					const FunctionTemplatePattern& primary = function_templates_[component.function_template];
					const FunctionTemplateAbiExpressionId name = AppendAbiExpression(program_, FunctionTemplateAbiExpression(
						FUNCTION_TEMPLATE_ABI_EXPRESSION_NAMESPACE_MEMBER, kNoFunctionTemplateAbiExpression,
						kNoFunctionTemplateAbiExpression, kNoFunctionTemplateAbiType, primary.name, primary.owner));
					*expression = AppendAbiExpression(program_, FunctionTemplateAbiExpression(
						FUNCTION_TEMPLATE_ABI_EXPRESSION_TEMPLATE_ENTITY, name, kNoFunctionTemplateAbiExpression,
						kNoFunctionTemplateAbiType, 0, primary.abi_recipe, OPERATOR_NONE, false, begin,
						static_cast<std::uint32_t>(component.arguments.size())));
					continue;
				}
				// A member of a non-dependent class is resolved: a static data
				// member is its declaration (`L_ZNSt14numeric_limitsImE6digitsE`),
				// an enumerator its value.
				const TypeId resolved_owner = root == kNoFunctionTemplateAbiType ? kNoType : ResolvedClass(root);
				if (resolved_owner != kNoType && component.arguments.empty())
				{
					const TypeId owner = program_->types.RemoveTopCv(resolved_owner);
					const TypeRecord& owner_type = program_->types.Get(owner);
					const LookupResult member = owner_type.kind == TYPE_NAMED ?
						program_->LookupMember(owner_type.entity, component.name, LOOKUP_ORDINARY) : LookupResult();
					const BindingRecord* binding = member.ordinary == kNoBinding ? 0 :
						&program_->bindings[member.ordinary];
					if (binding && binding->kind == BIND_ENUMERATOR)
					{
						*expression = Value(binding->type, binding->value);
						continue;
					}
					if (binding && binding->kind == BIND_VARIABLE && !binding->non_static_data_member)
					{
						FunctionTemplateAbiExpression resolved(FUNCTION_TEMPLATE_ABI_EXPRESSION_FUNCTION);
						resolved.value = member.ordinary;
						*expression = AppendAbiExpression(program_, resolved);
						continue;
					}
				}
				const FunctionTemplateAbiExpressionKind kind = root == kNoFunctionTemplateAbiType && name_space != kNoScope ?
					FUNCTION_TEMPLATE_ABI_EXPRESSION_NAMESPACE_MEMBER : root != kNoFunctionTemplateAbiType ?
					FUNCTION_TEMPLATE_ABI_EXPRESSION_TYPE_MEMBER : component.arguments.empty() ?
					FUNCTION_TEMPLATE_ABI_EXPRESSION_SOURCE_NAME : FUNCTION_TEMPLATE_ABI_EXPRESSION_TEMPLATE_ID;
				*expression = AppendAbiExpression(program_, FunctionTemplateAbiExpression(kind,
					kNoFunctionTemplateAbiExpression, kNoFunctionTemplateAbiExpression, root,
					component.name, name_space, OPERATOR_NONE, false, begin,
					static_cast<std::uint32_t>(component.arguments.size())));
			}
			else
			{
				if (component.name_space != kNoScope) name_space = component.name_space;
				// Namespace qualifiers have no type entity.  Their full path is
				// carried by the first following type entity and reconstructed
				// from that entity's canonical owner during ABI lowering.
				if (root == kNoFunctionTemplateAbiType &&
					component.entity == kNoEntity &&
					component.arguments.empty() && !terminal &&
					!IsTypeParameterName(component.name))
					continue;
				root = ComponentType(component, root, value_terminal,
					value_terminal && components == 1 && root == kNoFunctionTemplateAbiType);
				if (root == kNoFunctionTemplateAbiType)
					return kNoFunctionTemplateAbiType;
			}
		}
		if (position_ >= atoms_.size() ||
			ResultIdentityKind(atoms_[position_++]) !=
				FUNCTION_TEMPLATE_RESULT_QUALIFIED_END)
			return kNoFunctionTemplateAbiType;
		return root;
	}

	Program* program_;
	const std::deque<ClassTemplatePattern>& class_templates_;
	const std::deque<FunctionTemplatePattern>& function_templates_;
	const std::vector<std::uint32_t>& class_template_by_entity_;
	const std::vector<TemplateParameter>& parameters_;
	const std::vector<std::uint64_t>& atoms_;
	std::size_t position_;
	bool group_template_packs_;
	ScopeId scope_ = kNoScope;
	ClassResolver resolve_class_;
	bool written_root_ = false;
};

FunctionTemplateAbiTypeId ApplyTypeModifiers(Program* program,
	TypeId shape, FunctionTemplateAbiTypeId root, bool shape_formed = true)
{
	if (root == kNoFunctionTemplateAbiType || shape == kNoType) return root;
	// A completely concrete alias already includes its underlying modifiers.
	// Consume the canonical formed type, including any outside declarator and
	// reference collapsing, rather than applying that shape to it a second time.
	// A shape that is a placeholder formed nothing; its modifiers wrap the
	// concrete type the retained name resolves to.
	FunctionTemplateAbiTypeId leaf = root;
	for (; shape_formed;)
	{
		const FunctionTemplateAbiType& type = program->function_template_abi_types[leaf];
		if (type.kind == FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE)
			return AppendAbiType(program, FunctionTemplateAbiType(
				FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE, kNoFunctionTemplateAbiType,
				0, 0, kNoTemplateParameter, 0, shape));
		if (type.kind != FUNCTION_TEMPLATE_ABI_TYPE_POINTER &&
			type.kind != FUNCTION_TEMPLATE_ABI_TYPE_LVALUE_REFERENCE &&
			type.kind != FUNCTION_TEMPLATE_ABI_TYPE_RVALUE_REFERENCE &&
			type.kind != FUNCTION_TEMPLATE_ABI_TYPE_QUALIFIED &&
			type.kind != FUNCTION_TEMPLATE_ABI_TYPE_ARRAY) break;
		leaf = type.child;
	}

	struct Modifier
	{
		FunctionTemplateAbiTypeKind kind;
		std::uint64_t bound;
		std::uint32_t parameter;
		std::uint8_t cv;
	};
	std::vector<Modifier> modifiers;
	TypeId source = shape;
	for (;;)
	{
		const TypeRecord& record = program->types.Get(source);
		Modifier modifier = { FUNCTION_TEMPLATE_ABI_TYPE_POINTER, 0,
			kNoTemplateParameter, 0 };
		if (record.kind == TYPE_QUALIFIED)
		{
			modifier.kind = FUNCTION_TEMPLATE_ABI_TYPE_QUALIFIED;
			modifier.cv = record.cv;
		}
		else if (record.kind == TYPE_POINTER)
			modifier.kind = FUNCTION_TEMPLATE_ABI_TYPE_POINTER;
		else if (record.kind == TYPE_LVALUE_REFERENCE)
			modifier.kind = FUNCTION_TEMPLATE_ABI_TYPE_LVALUE_REFERENCE;
		else if (record.kind == TYPE_RVALUE_REFERENCE)
			modifier.kind = FUNCTION_TEMPLATE_ABI_TYPE_RVALUE_REFERENCE;
		else if (record.kind == TYPE_ARRAY)
		{
			modifier.kind = FUNCTION_TEMPLATE_ABI_TYPE_ARRAY;
			modifier.bound = record.bound;
			modifier.parameter = record.dependent_bound_parameter;
		}
		else break;
		modifiers.push_back(modifier);
		source = record.child;
	}
	for (std::vector<Modifier>::const_reverse_iterator modifier =
		modifiers.rbegin(); modifier != modifiers.rend(); ++modifier)
		root = AppendAbiType(program, FunctionTemplateAbiType(modifier->kind,
			root, 0, modifier->bound, modifier->parameter, modifier->cv));
	return root;
}

FunctionTemplateAbiTypeId ApplyTemplateParameterModifiers(Program* program,
	const SyntaxArena& arena, const TemplateParameter& parameter,
	FunctionTemplateAbiTypeId root)
{
	if (parameter.value_type != kNoType)
		return ApplyTypeModifiers(program, parameter.value_type, root);
	if (parameter.declarator == kNoNode) return root;
	for (std::uint32_t edge = arena.FirstEdge(parameter.declarator);
		edge != kNoEdge; edge = arena.NextEdge(edge))
	{
		const NodeId child = arena.EdgeChild(edge);
		if (arena.IsTag(child, STAG_IDENTIFIER) || arena.IsTag(child, STAG_PARAMETER_PACK)) continue;
		if (!arena.IsTag(child, STAG_PTR_OPERATOR) || arena.FirstEdge(child) != kNoEdge)
			return kNoFunctionTemplateAbiType;
		const std::string& op = arena.SemanticPayload(child);
		FunctionTemplateAbiTypeKind kind = FUNCTION_TEMPLATE_ABI_TYPE_POINTER;
		if (op == "&") kind = FUNCTION_TEMPLATE_ABI_TYPE_LVALUE_REFERENCE;
		else if (op == "&&") kind = FUNCTION_TEMPLATE_ABI_TYPE_RVALUE_REFERENCE;
		else if (op != "*") return kNoFunctionTemplateAbiType;
		root = AppendAbiType(program, FunctionTemplateAbiType(kind, root));
	}
	return root;
}

FunctionTemplateAbiExpressionId PublishSyntaxExpression(Program* program,
	const SyntaxArena& arena, NodeId syntax,
	const std::vector<ParameterInfo>& parameters, ScopeId scope)
{
	if (syntax == kNoNode) return kNoFunctionTemplateAbiExpression;
	if (arena.IsTag(syntax, ::cppgm::syntax::STAG_PARENTHESIZED_EXPRESSION))
	{
		const std::uint32_t edge = arena.FirstEdge(syntax);
		return edge == kNoEdge ? kNoFunctionTemplateAbiExpression :
			PublishSyntaxExpression(
				program, arena, arena.EdgeChild(edge), parameters, scope);
	}
	if (arena.IsTag(syntax, ::cppgm::syntax::STAG_ID_EXPRESSION))
	{
		const NameId name = arena.SemanticPayloadId(syntax);
		for (std::size_t i = 0; i < parameters.size(); ++i)
			if (parameters[i].name == name)
				return AppendAbiExpression(program,
					FunctionTemplateAbiExpression(
						FUNCTION_TEMPLATE_ABI_EXPRESSION_FUNCTION_PARAMETER,
						kNoFunctionTemplateAbiExpression,
						kNoFunctionTemplateAbiExpression,
						kNoFunctionTemplateAbiType, 0,
						static_cast<std::uint32_t>(i)));
		const std::size_t first = arena.TokenFirst(syntax);
		if (arena.TokenLast(syntax) != first + 1 ||
			first >= arena.TokenCount() ||
			arena.TokenKind(first) != ::cppgm::syntax::kIdentifierToken)
			return kNoFunctionTemplateAbiExpression;
		const LookupResult found = program->LookupName(scope, name, LOOKUP_ORDINARY);
		if (found.ordinary != kNoBinding &&
			program->bindings[found.ordinary].kind != BIND_FUNCTION)
			return kNoFunctionTemplateAbiExpression;
		if (found.ordinary == kNoBinding && !found.HasFunctionTemplateLookup() &&
			program->LookupName(scope, name, LOOKUP_TYPE).type != kNoType)
			return kNoFunctionTemplateAbiExpression;
		return AppendAbiExpression(program, FunctionTemplateAbiExpression(
			FUNCTION_TEMPLATE_ABI_EXPRESSION_SOURCE_NAME,
			kNoFunctionTemplateAbiExpression, kNoFunctionTemplateAbiExpression,
			kNoFunctionTemplateAbiType, name));
	}
	const std::uint32_t first = arena.FirstEdge(syntax);
	if (first == kNoEdge) return kNoFunctionTemplateAbiExpression;
	if (arena.IsTag(syntax, ::cppgm::syntax::STAG_MEMBER_EXPRESSION))
	{
		const std::uint32_t second = arena.NextEdge(first);
		if (second == kNoEdge) return kNoFunctionTemplateAbiExpression;
		const FunctionTemplateAbiExpressionId object = PublishSyntaxExpression(
			program, arena, arena.EdgeChild(first), parameters, scope);
		if (object == kNoFunctionTemplateAbiExpression)
			return kNoFunctionTemplateAbiExpression;
		return AppendAbiExpression(program, FunctionTemplateAbiExpression(
			FUNCTION_TEMPLATE_ABI_EXPRESSION_OBJECT_MEMBER, object,
			kNoFunctionTemplateAbiExpression, kNoFunctionTemplateAbiType,
			arena.SemanticPayloadId(arena.EdgeChild(second)),
			kNoTemplateParameter, OPERATOR_NONE,
			ClassifyOperationSpelling(
				arena.SemanticPayload(syntax)) == OP_ARROW));
	}
	if (arena.IsTag(syntax, ::cppgm::syntax::STAG_CALL_EXPRESSION))
	{
		const FunctionTemplateAbiExpressionId callee = PublishSyntaxExpression(
			program, arena, arena.EdgeChild(first), parameters, scope);
		if (callee == kNoFunctionTemplateAbiExpression)
			return kNoFunctionTemplateAbiExpression;
		const std::uint32_t argument_edge = arena.NextEdge(first);
		std::vector<FunctionTemplateAbiArgument> arguments;
		for (std::uint32_t edge = argument_edge == kNoEdge ? kNoEdge :
			arena.FirstEdge(arena.EdgeChild(argument_edge)); edge != kNoEdge;
			edge = arena.NextEdge(edge))
		{
			const FunctionTemplateAbiExpressionId argument = PublishSyntaxExpression(
				program, arena, arena.EdgeChild(edge), parameters, scope);
			if (argument == kNoFunctionTemplateAbiExpression)
				return kNoFunctionTemplateAbiExpression;
			arguments.push_back(FunctionTemplateAbiArgument(
				FUNCTION_TEMPLATE_ABI_ARGUMENT_EXPRESSION,
				kNoFunctionTemplateAbiType, argument));
		}
		if (arguments.size() >= kNoFunctionTemplateAbiExpression ||
			program->function_template_abi_arguments.size() >=
				kNoFunctionTemplateAbiExpression - arguments.size())
			ThrowSemanticResourceLimit("too many dependent call arguments");
		const std::uint32_t begin = static_cast<std::uint32_t>(
			program->function_template_abi_arguments.size());
		program->function_template_abi_arguments.insert(
			program->function_template_abi_arguments.end(),
			arguments.begin(), arguments.end());
		return AppendAbiExpression(program, FunctionTemplateAbiExpression(
			FUNCTION_TEMPLATE_ABI_EXPRESSION_CALL, callee,
			kNoFunctionTemplateAbiExpression, kNoFunctionTemplateAbiType,
			0, kNoTemplateParameter, OPERATOR_NONE, false, begin,
			static_cast<std::uint32_t>(arguments.size())));
	}
	if (arena.IsTag(syntax, ::cppgm::syntax::STAG_BINARY_EXPRESSION) &&
		ClassifyOperationSpelling(arena.SemanticPayload(syntax)) == OP_MINUS)
	{
		const std::uint32_t second = arena.NextEdge(first);
		if (second == kNoEdge) return kNoFunctionTemplateAbiExpression;
		const FunctionTemplateAbiExpressionId left = PublishSyntaxExpression(
			program, arena, arena.EdgeChild(first), parameters, scope);
		const FunctionTemplateAbiExpressionId right = PublishSyntaxExpression(
			program, arena, arena.EdgeChild(second), parameters, scope);
		if (left == kNoFunctionTemplateAbiExpression ||
			right == kNoFunctionTemplateAbiExpression)
			return kNoFunctionTemplateAbiExpression;
		return AppendAbiExpression(program, FunctionTemplateAbiExpression(
			FUNCTION_TEMPLATE_ABI_EXPRESSION_BINARY, left, right,
			kNoFunctionTemplateAbiType, 0, kNoTemplateParameter,
			OPERATOR_MINUS));
	}
	return kNoFunctionTemplateAbiExpression;
}

// The decltype a type's specifiers name as its primary type (`decltype(e)`
// leading or trailing, or a parameter's), or kNoNode.
NodeId PrimaryDecltypeSpecifier(const SyntaxArena& arena, NodeId specifiers)
{
	for (std::size_t depth = 0; specifiers != kNoNode && depth < 4; ++depth)
	{
		NodeId nested = kNoNode;
		for (std::uint32_t edge = arena.FirstEdge(specifiers); edge != kNoEdge;
			edge = arena.NextEdge(edge))
		{
			const NodeId child = arena.EdgeChild(edge);
			if (arena.IsTag(child, STAG_DECLTYPE_SPECIFIER) ||
				(arena.IsTag(child, STAG_DECL_SPECIFIER) && arena.FirstEdge(child) != kNoEdge &&
				 arena.SemanticPayload(child).compare(0, 8, "decltype") == 0))
				return child;
			if (arena.IsTag(child, STAG_TYPE_ID) || arena.IsTag(child, STAG_TYPE_SPECIFIER_SEQ) ||
				arena.IsTag(child, STAG_DECL_SPECIFIER_SEQ))
				nested = child;
		}
		specifiers = nested;
	}
	return kNoNode;
}

// The parameter clause of a function declarator, through any nesting
// (`char (&f(T*))[2]`).
NodeId FunctionParameterClause(const SyntaxArena& arena, NodeId declarator)
{
	const auto child = [&arena](NodeId node, ::cppgm::syntax::SyntaxTagCode tag) -> NodeId {
		for (std::uint32_t edge = node == kNoNode ? kNoEdge : arena.FirstEdge(node);
			edge != kNoEdge; edge = arena.NextEdge(edge))
			if (arena.IsTag(arena.EdgeChild(edge), tag)) return arena.EdgeChild(edge);
		return kNoNode;
	};
	for (std::size_t depth = 0; declarator != kNoNode && depth < 16; ++depth)
	{
		const NodeId clause = child(declarator, STAG_PARAMETER_CLAUSE);
		if (clause != kNoNode) return clause;
		declarator = child(child(declarator, STAG_NESTED_DECLARATOR), STAG_DECLARATOR);
	}
	return kNoNode;
}

// The array bounds a parameter's declarator writes, outermost first.
void CollectArrayBounds(const SyntaxArena& arena, NodeId declarator, std::vector<NodeId>* bounds)
{
	if (declarator == kNoNode) return;
	for (std::uint32_t edge = arena.FirstEdge(declarator); edge != kNoEdge; edge = arena.NextEdge(edge))
	{
		const NodeId child = arena.EdgeChild(edge);
		if (arena.IsTag(child, STAG_ARRAY_SUFFIX))
			bounds->push_back(arena.FirstEdge(child) == kNoEdge ? kNoNode : arena.EdgeChild(arena.FirstEdge(child)));
	}
	for (std::uint32_t edge = arena.FirstEdge(declarator); edge != kNoEdge; edge = arena.NextEdge(edge))
	{
		const NodeId child = arena.EdgeChild(edge);
		if (arena.IsTag(child, STAG_NESTED_DECLARATOR))
			for (std::uint32_t inner = arena.FirstEdge(child); inner != kNoEdge; inner = arena.NextEdge(inner))
				CollectArrayBounds(arena, arena.EdgeChild(inner), bounds);
	}
}

// Whether a shape names a dependent qualified type (`typename C::value_type`
// in a default argument): it has no semantic type to mangle.
bool ContainsDependentQualifiedShape(const Program& program, TypeId type)
{
	std::vector<TypeId> pending(1, type);
	for (std::size_t visits = 0; !pending.empty() && visits < 4096; ++visits)
	{
		const TypeId current = pending.back();
		pending.pop_back();
		if (current == kNoType || current > program.types.Size()) continue;
		const TypeRecord& record = program.types.Get(current);
		if (record.kind == TYPE_NAMED && record.entity < program.entities.size())
		{
			const EntityRecord& entity = program.entities[record.entity];
			if (entity.flavor == NAMED_TYPENAME_PARAMETER && entity.deferred_template_completion &&
				program.names.Get(entity.identity_name).compare(0, 33, "__dependent_qualified_type_shape_") == 0)
				return true;
			// A value argument's type is no type the name spells.
			for (std::size_t i = 0; entity.template_argument_begin != kNoBinding &&
				i < entity.template_argument_count; ++i)
				if (entity.template_argument_begin + i >= program.canonical_template_arguments.size() ||
					program.canonical_template_arguments[entity.template_argument_begin + i].kind == TEMPLATE_ARGUMENT_TYPE)
					pending.push_back(program.template_arguments[entity.template_argument_begin + i]);
		}
		else if (record.kind == TYPE_FUNCTION)
		{
			const TypeId* parameters = program.types.Parameters(current);
			for (std::size_t i = 0; i < record.parameter_count; ++i) pending.push_back(parameters[i]);
			pending.push_back(record.child);
		}
		else if (record.child != kNoType) pending.push_back(record.child);
	}
	return false;
}

// Whether a type mentions one of the placeholders a shape uses where the
// semantic type is not known (a dependent decltype's).
bool MentionsShape(const Program& program, TypeId type, TypeId first, TypeId second)
{
	for (std::size_t depth = 0; type != kNoType && type <= program.types.Size() && depth < 64; ++depth)
	{
		if (type == first || type == second) return true;
		const TypeRecord& record = program.types.Get(type);
		if (record.kind == TYPE_FUNCTION)
		{
			const TypeId* parameters = program.types.Parameters(type);
			for (std::size_t i = 0; i < record.parameter_count; ++i)
				if (MentionsShape(program, parameters[i], first, second)) return true;
		}
		type = record.kind == TYPE_NAMED ? kNoType : record.child;
	}
	return false;
}

bool HasRetainedParameterRoot(const Program& program,
	const FunctionTemplatePattern& pattern,
	const std::vector<std::uint64_t>& atoms)
{
	std::size_t decltype_depth = 0;
	for (std::size_t atom = 0; atom < atoms.size(); ++atom)
	{
		const FunctionTemplateResultIdentityAtomKind kind = ResultIdentityKind(atoms[atom]);
		if (kind == FUNCTION_TEMPLATE_RESULT_NODE_BEGIN)
		{
			if (decltype_depth) ++decltype_depth;
			else if (program.names.Get(static_cast<NameId>(ResultIdentityValue(atoms[atom]))) == "decltype-specifier")
			{
				if (pattern.deferred_result_formation) return true;
				decltype_depth = 1;
			}
		}
		else if (kind == FUNCTION_TEMPLATE_RESULT_NODE_END && decltype_depth) --decltype_depth;
		else if (kind == FUNCTION_TEMPLATE_RESULT_PARAMETER) return true;
	}
	if (!atoms.empty() && ResultIdentityKind(atoms[0]) ==
		FUNCTION_TEMPLATE_RESULT_PARAMETER) return true;
	if (atoms.size() < 3 || ResultIdentityKind(atoms[0]) !=
		FUNCTION_TEMPLATE_RESULT_QUALIFIED_BEGIN ||
		ResultIdentityKind(atoms[1]) != FUNCTION_TEMPLATE_RESULT_COMPONENT)
		return false;
	std::size_t terminal = atoms.size();
	bool dependent = false;
	for (std::size_t atom = 1; atom < atoms.size(); ++atom)
	{
		const FunctionTemplateResultIdentityAtomKind kind =
			ResultIdentityKind(atoms[atom]);
		if (kind == FUNCTION_TEMPLATE_RESULT_COMPONENT)
		{
			terminal = atom;
			const NameId name = static_cast<NameId>(ResultIdentityValue(atoms[atom]));
			for (std::size_t parameter = 0; parameter < pattern.parameters.size(); ++parameter)
				if (pattern.parameters[parameter].name == name)
					dependent = true;
		}
		else if (kind == FUNCTION_TEMPLATE_RESULT_PARAMETER)
		{
			dependent = true;
			const std::uint64_t parameter = ResultIdentityValue(atoms[atom]);
			if (parameter < pattern.parameters.size() &&
				pattern.parameters[parameter].kind == TEMPLATE_ARGUMENT_INTEGRAL)
				return true;
		}
	}
	if (dependent && terminal < atoms.size())
	{
		bool terminal_resolved = false;
		for (std::size_t atom = terminal + 1; atom < atoms.size(); ++atom)
		{
			const FunctionTemplateResultIdentityAtomKind kind =
				ResultIdentityKind(atoms[atom]);
			if (kind == FUNCTION_TEMPLATE_RESULT_DECLARATION ||
				kind == FUNCTION_TEMPLATE_RESULT_ENTITY ||
				kind == FUNCTION_TEMPLATE_RESULT_ARGUMENTS_BEGIN)
				terminal_resolved = true;
			if (kind == FUNCTION_TEMPLATE_RESULT_QUALIFIED_END) break;
		}
		// A terminal dependent member has no canonical semantic type of its
		// own.  Retain its owner/member source DAG; an ordinary class-template
		// specialization remains on the established semantic TypeId path so
		// standard substitutions and cross-parameter slots stay shared.
		if (!terminal_resolved) return true;
	}
	const NameId source_root = static_cast<NameId>(
		ResultIdentityValue(atoms[1]));
	for (std::size_t parameter = 0;
		parameter < pattern.parameters.size(); ++parameter)
		if (pattern.parameters[parameter].kind == TEMPLATE_ARGUMENT_TYPE &&
			pattern.parameters[parameter].name == source_root) return true;
	EntityId entity = kNoEntity;
	const FunctionTemplateResultIdentityAtomKind marker =
		ResultIdentityKind(atoms[2]);
	const std::uint64_t value = ResultIdentityValue(atoms[2]);
	if (marker == FUNCTION_TEMPLATE_RESULT_ENTITY)
		entity = value < program.entities.size() ?
			static_cast<EntityId>(value) : kNoEntity;
	else if (marker == FUNCTION_TEMPLATE_RESULT_DECLARATION &&
		value < program.bindings.size())
	{
		const BindingRecord& binding = program.bindings[
			program.bindings[static_cast<BindingId>(value)].canonical];
		if (binding.type != kNoType)
		{
			const TypeRecord& type = program.types.Get(
				program.types.RemoveTopCv(binding.type));
			if (type.kind == TYPE_NAMED) entity = type.entity;
		}
	}
	if (entity == kNoEntity || entity >= program.entities.size()) return false;
	return program.entities[entity].identity_name != source_root;
}

}

// A parameter whose array bound is a dependent expression keeps that
// expression (`int (&)[sizeof(p)]` is `RAszfL0p__i`, `T (&)[N - 1]`
// `RAmiT0_Li1E_T_`): the semantic type has no bound to mangle.
bool Analyzer::PublishParameterArrayBounds(FunctionTemplatePattern* pattern,
	const DeclaratorInfo& declarator, std::size_t parameter, NodeId declaration,
	const std::unordered_set<NameId>& dependent_names)
{
	NodeId parameter_declarator = FindChild(declaration, STAG_DECLARATOR);
	if (parameter_declarator == kNoNode) parameter_declarator = FindChild(declaration, STAG_ABSTRACT_DECLARATOR);
	std::vector<NodeId> bounds;
	CollectArrayBounds(*arena_, parameter_declarator, &bounds);
	bool expression_bound = false;
	for (std::size_t i = 0; i < bounds.size(); ++i)
		expression_bound = expression_bound || (bounds[i] != kNoNode &&
			!arena_->IsTag(bounds[i], STAG_ID_EXPRESSION) &&
			SyntaxUsesAnyTemplateParameter(bounds[i], dependent_names));
	if (!expression_bound) return false;
	// The shape's modifiers, outermost first; its array levels are the
	// written bounds, after any the parameter's adjustment removed.
	std::vector<TypeId> levels;
	TypeId element = declarator.parameters[parameter].function_type;
	std::size_t arrays = 0;
	for (;;)
	{
		const TypeRecord& record = program_->types.Get(element);
		if (record.kind != TYPE_QUALIFIED && record.kind != TYPE_POINTER &&
			record.kind != TYPE_LVALUE_REFERENCE && record.kind != TYPE_RVALUE_REFERENCE &&
			record.kind != TYPE_ARRAY) break;
		arrays += record.kind == TYPE_ARRAY;
		levels.push_back(element);
		element = record.child;
	}
	if (arrays == 0 || arrays > bounds.size()) return false;
	std::size_t bound = bounds.size() - arrays;
	AbiPublication publication(program_);
	std::vector<FunctionTemplateAbiExpressionId> expressions;
	for (std::size_t i = 0; i < levels.size(); ++i)
	{
		if (program_->types.Get(levels[i]).kind != TYPE_ARRAY) continue;
		const NodeId source = bounds[bound++];
		FunctionTemplateAbiExpressionId expression = kNoFunctionTemplateAbiExpression;
		if (source != kNoNode && !arena_->IsTag(source, STAG_ID_EXPRESSION) &&
			SyntaxUsesAnyTemplateParameter(source, dependent_names))
		{
			FunctionTemplatePattern probe;
			probe.parameters = pattern->parameters;
			probe.lexical_scope = pattern->lexical_scope;
			probe.result_root_structure = source;
			for (std::size_t p = 0; p < declarator.parameters.size(); ++p)
				probe.function_parameter_names.push_back(declarator.parameters[p].name);
			InternExpandedFunctionTemplateResult(&probe, true);
			if (probe.expanded_result_identity == kNoFunctionTemplateResultIdentity) return false;
			std::vector<std::uint64_t> atoms;
			function_template_result_identities_.CopyAtoms(probe.expanded_result_identity, &atoms);
			AbiIdentityReader reader(program_, class_templates_, function_templates_,
				class_template_pattern_by_entity_, pattern->parameters, atoms);
			reader.SetScope(pattern->lexical_scope);
			expression = reader.ParseExpression();
			if (expression == kNoFunctionTemplateAbiExpression || !reader.Complete())
				return false;
		}
		expressions.push_back(expression);
	}
	// A template parameter's shape is the parameter (`T_`).
	std::size_t shape = 0;
	while (shape < function_template_shape_parameters_.size() &&
		function_template_shape_parameters_[shape] != element) ++shape;
	FunctionTemplateAbiTypeId root = shape < pattern->parameters.size() ?
		AppendAbiType(program_, FunctionTemplateAbiType(FUNCTION_TEMPLATE_ABI_TYPE_PARAMETER,
			kNoFunctionTemplateAbiType, 0, 0, static_cast<std::uint32_t>(shape))) :
		AppendAbiType(program_, FunctionTemplateAbiType(
			FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE, kNoFunctionTemplateAbiType,
			0, 0, kNoTemplateParameter, 0, element));
	for (std::size_t i = levels.size(); i != 0; --i)
	{
		const TypeRecord& record = program_->types.Get(levels[i - 1]);
		const FunctionTemplateAbiTypeKind kind = record.kind == TYPE_QUALIFIED ? FUNCTION_TEMPLATE_ABI_TYPE_QUALIFIED :
			record.kind == TYPE_POINTER ? FUNCTION_TEMPLATE_ABI_TYPE_POINTER :
			record.kind == TYPE_LVALUE_REFERENCE ? FUNCTION_TEMPLATE_ABI_TYPE_LVALUE_REFERENCE :
			record.kind == TYPE_RVALUE_REFERENCE ? FUNCTION_TEMPLATE_ABI_TYPE_RVALUE_REFERENCE :
			FUNCTION_TEMPLATE_ABI_TYPE_ARRAY;
		FunctionTemplateAbiType modifier(kind, root, 0, record.kind == TYPE_ARRAY ? record.bound : 0,
			record.kind == TYPE_ARRAY ? record.dependent_bound_parameter : kNoTemplateParameter,
			record.kind == TYPE_QUALIFIED ? record.cv : 0);
		if (record.kind == TYPE_ARRAY)
		{
			modifier.expression = expressions.back();
			expressions.pop_back();
		}
		root = AppendAbiType(program_, modifier);
	}
	pattern->abi_function_parameter_types[parameter] = root;
	publication.Commit();
	return true;
}

void Analyzer::PublishFunctionTemplateResultAbiType(
	FunctionTemplatePattern* pattern, const DeclaratorInfo& declarator)
{
	if (!pattern) return;
	const AbiIdentityReader::ClassResolver resolve_class =
		[this](std::size_t index, const std::vector<TemplateArgument>& arguments) -> TypeId {
			for (std::size_t i = 0; i < arguments.size(); ++i)
				if (arguments[i].kind == TEMPLATE_ARGUMENT_TYPE && FunctionTemplateTypeIsDependent(arguments[i].type))
					return kNoType;
			const BindingId specialization = InstantiateClassTemplate(index, arguments);
			return specialization == kNoBinding ? kNoType : program_->bindings[specialization].type;
		};
	pattern->abi_result_type = kNoFunctionTemplateAbiType;
	pattern->abi_template_parameter_types.assign(
		pattern->parameters.size(), kNoFunctionTemplateAbiType);
	pattern->abi_function_parameter_types.assign(
		declarator.parameters.size(), kNoFunctionTemplateAbiType);
	for (std::size_t p = 0; p < pattern->parameters.size(); ++p)
	{
		if (pattern->parameters[p].kind != TEMPLATE_ARGUMENT_INTEGRAL)
			continue;
		const NodeId root = FindDescendant(
			*arena_, pattern->parameters[p].specifiers, "structured-type-name");
		if (root == kNoNode) continue;
		FunctionTemplatePattern probe;
		probe.parameters = pattern->parameters;
		probe.lexical_scope = pattern->lexical_scope;
		probe.result_root_structure = root;
		probe.function_parameter_names.reserve(declarator.parameters.size());
		for (std::size_t parameter = 0;
			parameter < declarator.parameters.size(); ++parameter)
			probe.function_parameter_names.push_back(
				declarator.parameters[parameter].name);
		InternExpandedFunctionTemplateResult(&probe, true);
		if (probe.expanded_result_identity ==
			kNoFunctionTemplateResultIdentity) continue;
		std::vector<std::uint64_t> atoms;
		function_template_result_identities_.CopyAtoms(
			probe.expanded_result_identity, &atoms);
		AbiPublication publication(program_);
		// Clang's template-parameter declaration annotation retains the written
		// argument list, before grouping arguments into a primary's pack.
		AbiIdentityReader reader(program_, class_templates_, function_templates_,
			class_template_pattern_by_entity_, pattern->parameters, atoms, false);
		const FunctionTemplateAbiTypeId type = reader.ParseType();
		// A parameter of a concrete type (`std::size_t I`) takes no
		// declaration annotation; only a dependent one does (`Tn`).
		if (type != kNoFunctionTemplateAbiType && reader.Complete() &&
			program_->function_template_abi_types[type].kind !=
				FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE)
		{
			const FunctionTemplateAbiTypeId qualified = ApplyTemplateParameterModifiers(
				program_, *arena_, pattern->parameters[p], type);
			if (qualified != kNoFunctionTemplateAbiType)
			{
				pattern->abi_template_parameter_types[p] = qualified;
				publication.Commit();
			}
		}
	}
	bool has_template_pack = false;
	std::unordered_set<NameId> value_parameters;
	for (std::size_t p = 0; p < pattern->parameters.size(); ++p)
	{
		has_template_pack = has_template_pack || pattern->parameters[p].pack;
		if (pattern->parameters[p].kind == TEMPLATE_ARGUMENT_INTEGRAL &&
			pattern->parameters[p].name != 0) value_parameters.insert(pattern->parameters[p].name);
	}
	std::vector<NodeId> parameter_syntax, parameter_declarations;
	const NodeId parameter_clause = FunctionParameterClause(*arena_, pattern->declarator);
	for (std::uint32_t edge = parameter_clause == kNoNode ? kNoEdge : arena_->FirstEdge(parameter_clause);
		edge != kNoEdge; edge = arena_->NextEdge(edge))
		if (arena_->IsTag(arena_->EdgeChild(edge), STAG_PARAMETER_DECLARATION))
			parameter_declarations.push_back(arena_->EdgeChild(edge));
	std::unordered_set<NameId> dependent_names;
	for (std::size_t p = 0; p < pattern->parameters.size(); ++p)
		if (pattern->parameters[p].name != 0) dependent_names.insert(pattern->parameters[p].name);
	for (std::size_t p = 0; p < declarator.parameters.size(); ++p)
		if (declarator.parameters[p].name != 0) dependent_names.insert(declarator.parameters[p].name);
	for (std::size_t p = 0; p < declarator.parameters.size(); ++p)
	{
		if (p < parameter_declarations.size() && PublishParameterArrayBounds(
			pattern, declarator, p, parameter_declarations[p], dependent_names))
			continue;
		const TypeRecord& parameter_type = program_->types.Get(declarator.parameters[p].function_type);
		const bool member_pointer_pack = has_template_pack &&
			parameter_type.kind == TYPE_MEMBER_POINTER && program_->types.Get(parameter_type.child).kind == TYPE_FUNCTION &&
			program_->types.Get(parameter_type.child).variadic;
		if (member_pointer_pack && parameter_syntax.empty())
		{
			const NodeId clause = FindChild(pattern->declarator, STAG_PARAMETER_CLAUSE);
			for (std::uint32_t edge = arena_->FirstEdge(clause); edge != kNoEdge; edge = arena_->NextEdge(edge))
				if (arena_->IsTag(arena_->EdgeChild(edge), STAG_PARAMETER_DECLARATION))
					parameter_syntax.push_back(arena_->EdgeChild(edge));
		}
		if (member_pointer_pack && p >= parameter_syntax.size()) continue;
		// A decltype that is not instantiation-dependent is its type.
		const NodeId decltype_root = member_pointer_pack || !MentionsShape(*program_,
			declarator.parameters[p].function_type, function_template_dependent_result_shape_,
			function_template_nondeduced_type_shape_) ? kNoNode :
			PrimaryDecltypeSpecifier(*arena_, declarator.parameters[p].type_syntax);
		const NodeId root = member_pointer_pack ? parameter_syntax[p] :
			decltype_root != kNoNode ? decltype_root : FindDescendant(*arena_,
			declarator.parameters[p].type_syntax,
			"structured-type-name");
		if (root == kNoNode) continue;
		const bool written_expansion = has_template_pack &&
			(arena_->HasDescendantTag(root, STAG_PARAMETER_PACK) ||
			 arena_->HasDescendantTag(root, STAG_PACK_EXPANSION_EXPRESSION));
		const bool dependent_value = (!value_parameters.empty() &&
			SyntaxUsesAnyTemplateParameter(root, value_parameters)) ||
			ContainsDependentQualifiedShape(*program_, declarator.parameters[p].function_type);
		const NamePath path = decltype_root != kNoNode ? NamePath() : StructuredNamePath(root);
		if (!member_pointer_pack && decltype_root == kNoNode && path.Empty()) continue;
		const LookupResult marker = member_pointer_pack || decltype_root != kNoNode ? LookupResult() : LookupPath(
			pattern->lexical_scope, path, LOOKUP_TYPE);
		if (!member_pointer_pack && decltype_root == kNoNode && !declarator.parameters[p].nondeduced &&
			!written_expansion && !dependent_value &&
			FindAliasTemplateIndex(marker, path.Last()) >=
				alias_templates_.size()) continue;
		FunctionTemplatePattern probe;
		probe.parameters = pattern->parameters;
		probe.lexical_scope = pattern->lexical_scope;
		probe.result_root_structure = root;
		probe.function_parameter_names.reserve(declarator.parameters.size());
		for (std::size_t parameter = 0;
			parameter < declarator.parameters.size(); ++parameter)
			probe.function_parameter_names.push_back(
				declarator.parameters[parameter].name);
		InternExpandedFunctionTemplateResult(&probe, true);
		if (probe.expanded_result_identity ==
			kNoFunctionTemplateResultIdentity) continue;
		if (!member_pointer_pack && decltype_root == kNoNode && !declarator.parameters[p].nondeduced &&
			!written_expansion && !dependent_value &&
			!probe.expanded_result_has_alias) continue;
		std::vector<std::uint64_t> atoms;
		function_template_result_identities_.CopyAtoms(
			probe.expanded_result_identity, &atoms);
		AbiPublication publication(program_);
		AbiIdentityReader reader(program_, class_templates_, function_templates_,
			class_template_pattern_by_entity_, pattern->parameters, atoms);
		reader.SetScope(pattern->lexical_scope);
		reader.SetClassResolver(resolve_class);
		const FunctionTemplateAbiTypeId type = member_pointer_pack ? reader.ParseParameterType() : reader.ParseType();
		if (type != kNoFunctionTemplateAbiType && reader.Complete())
		{
			pattern->abi_function_parameter_types[p] = ApplyTypeModifiers(
				program_, declarator.parameters[p].function_type, type);
			publication.Commit();
		}
	}

	// A decltype result is its operand expression (Itanium 5.1.5.9), read
	// from the decltype itself rather than the first name inside it; one
	// that is not instantiation-dependent is its type.
	const NodeId result_decltype = PrimaryDecltypeSpecifier(*arena_,
		pattern->trailing_return_syntax != kNoNode ? pattern->trailing_return_syntax : pattern->specifiers);
	const bool dependent_decltype = result_decltype != kNoNode && MentionsShape(*program_,
		program_->types.Get(pattern->shape_type).child, function_template_dependent_result_shape_,
		function_template_nondeduced_type_shape_);
	if (result_decltype != kNoNode && !dependent_decltype) return;
	if (dependent_decltype)
	{
		FunctionTemplatePattern probe;
		probe.parameters = pattern->parameters;
		probe.lexical_scope = pattern->lexical_scope;
		probe.result_root_structure = result_decltype;
		for (std::size_t parameter = 0; parameter < declarator.parameters.size(); ++parameter)
			probe.function_parameter_names.push_back(declarator.parameters[parameter].name);
		InternExpandedFunctionTemplateResult(&probe, true);
		if (probe.expanded_result_identity != kNoFunctionTemplateResultIdentity)
		{
			std::vector<std::uint64_t> atoms;
			function_template_result_identities_.CopyAtoms(probe.expanded_result_identity, &atoms);
			AbiPublication publication(program_);
			AbiIdentityReader reader(program_, class_templates_, function_templates_,
				class_template_pattern_by_entity_, pattern->parameters, atoms);
			reader.SetScope(pattern->lexical_scope);
			reader.SetClassResolver(resolve_class);
			const FunctionTemplateAbiTypeId root = reader.ParseType();
			if (root != kNoFunctionTemplateAbiType && reader.Complete())
			{
				pattern->abi_result_type = ApplyTypeModifiers(program_,
					program_->types.Get(pattern->shape_type).child, root);
				publication.Commit();
				return;
			}
		}
	}
	if (pattern->expanded_result_identity !=
		kNoFunctionTemplateResultIdentity)
	{
		FunctionTemplatePattern spelling;
		spelling.parameters = pattern->parameters;
		spelling.lexical_scope = pattern->lexical_scope;
		spelling.result_root_structure = pattern->result_root_structure;
		spelling.function_parameter_names = pattern->function_parameter_names;
		InternExpandedFunctionTemplateResult(&spelling, true);
		std::vector<std::uint64_t> atoms;
		function_template_result_identities_.CopyAtoms(
			spelling.expanded_result_identity != kNoFunctionTemplateResultIdentity ?
				spelling.expanded_result_identity : pattern->expanded_result_identity, &atoms);
		// A result whose shape is the dependent placeholder has no semantic
		// TypeId to mangle; the written result is the encoding's (Itanium
		// 5.1.5.1), not the instantiated one.
		TypeId shape_result = program_->types.Get(pattern->shape_type).child;
		while (program_->types.Get(shape_result).kind == TYPE_QUALIFIED ||
			program_->types.Get(shape_result).kind == TYPE_POINTER ||
			program_->types.Get(shape_result).kind == TYPE_LVALUE_REFERENCE ||
			program_->types.Get(shape_result).kind == TYPE_RVALUE_REFERENCE)
			shape_result = program_->types.Get(shape_result).child;
		const bool placeholder_result = shape_result != kNoType &&
			shape_result == function_template_dependent_result_shape_;
		if (pattern->expanded_result_has_alias || placeholder_result ||
			HasRetainedParameterRoot(*program_, *pattern, atoms))
		{
			AbiPublication publication(program_);
			AbiIdentityReader reader(program_, class_templates_, function_templates_,
				class_template_pattern_by_entity_, pattern->parameters, atoms);
			reader.SetScope(pattern->lexical_scope);
			reader.SetClassResolver(resolve_class);
			const FunctionTemplateAbiTypeId root = reader.ParseType();
			if (root != kNoFunctionTemplateAbiType && reader.Complete())
			{
				pattern->abi_result_type = ApplyTypeModifiers(program_,
					program_->types.Get(pattern->shape_type).child, root, !placeholder_result);
				publication.Commit();
			}
		}
	}
	if (pattern->abi_result_type != kNoFunctionTemplateAbiType ||
		!pattern->deferred_result_formation) return;
	const NodeId decltype_specifier = FindDescendant(
		*arena_, pattern->trailing_return_syntax, "decltype-specifier");
	if (decltype_specifier == kNoNode) return;
	const std::uint32_t edge = arena_->FirstEdge(decltype_specifier);
	if (edge == kNoEdge) return;
	AbiPublication publication(program_);
	const FunctionTemplateAbiExpressionId expression = PublishSyntaxExpression(
		program_, *arena_, arena_->EdgeChild(edge), declarator.parameters,
		declarator.parameter_scope);
	if (expression == kNoFunctionTemplateAbiExpression) return;
	const NodeId operand = arena_->EdgeChild(edge);
	const bool id_expression = arena_->IsTag(operand, ::cppgm::syntax::STAG_ID_EXPRESSION) ||
		arena_->IsTag(operand, ::cppgm::syntax::STAG_MEMBER_EXPRESSION);
	const FunctionTemplateAbiTypeId root = AppendAbiType(program_,
		FunctionTemplateAbiType(id_expression ? FUNCTION_TEMPLATE_ABI_TYPE_DECLTYPE_ID :
			FUNCTION_TEMPLATE_ABI_TYPE_DECLTYPE,
			kNoFunctionTemplateAbiType, 0, 0, kNoTemplateParameter, 0,
			kNoType, kNoEntity, 0, 0, expression));
	pattern->abi_result_type = ApplyTypeModifiers(program_,
		program_->types.Get(pattern->shape_type).child, root);
	publication.Commit();
}

}
}
