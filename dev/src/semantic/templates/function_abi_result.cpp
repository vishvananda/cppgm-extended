#include "semantic/analysis/analyzer.h"
#include "support/exceptions.h"

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
	ScopeId name_space;
	bool callable;
	std::size_t function_template;
	std::vector<FunctionTemplateAbiArgument> arguments;

	ParsedComponent() : name(0), entity(kNoEntity), name_space(kNoScope), callable(false), function_template(kNoDumpEdge) {}
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
		while (IsNode("cv-qualifier"))
		{
			NameId qualifier = 0;
			if (!BeginNode("cv-qualifier", &qualifier) || !EndNode())
				return kNoFunctionTemplateAbiType;
			const std::string& spelling = program_->names.Get(qualifier);
			if (spelling == "const") cv |= CV_CONST;
			else if (spelling == "volatile") cv |= CV_VOLATILE;
			else return kNoFunctionTemplateAbiType;
		}
		FunctionTemplateAbiTypeId root = ParsePrimaryType();
		if (root == kNoFunctionTemplateAbiType) return root;
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
		if (!IsNode("parenthesized-expression") &&
			!IsNode("id-expression") && !IsNode("unary-expression") &&
			!IsNode("call-expression") && !IsNode("binary-expression") &&
			!IsNode("sizeof-pack-expression") && !IsNode("cast-expression"))
			return kNoFunctionTemplateAbiExpression;
		NameId payload = 0;
		const NameId tag = static_cast<NameId>(
			ResultIdentityValue(atoms_[position_]));
		if (!BeginNode(program_->names.Get(tag).c_str(), &payload))
			return kNoFunctionTemplateAbiExpression;
		const std::string& node = program_->names.Get(tag);
		FunctionTemplateAbiExpressionId expression =
			kNoFunctionTemplateAbiExpression;
		if (node == "id-expression")
			expression = position_ < atoms_.size() &&
				ResultIdentityKind(atoms_[position_]) ==
					FUNCTION_TEMPLATE_RESULT_QUALIFIED_BEGIN ?
				ParseQualifiedExpression() : ParseExpression();
		else if (node == "parenthesized-expression")
			expression = ParseExpression();
		else if (node == "sizeof-pack-expression")
		{
			const FunctionTemplateAbiExpressionId operand = ParseExpression();
			if (operand != kNoFunctionTemplateAbiExpression)
				expression = AppendAbiExpression(program_, FunctionTemplateAbiExpression(
					FUNCTION_TEMPLATE_ABI_EXPRESSION_SIZEOF_PACK, operand));
		}
		else if (node == "binary-expression")
		{
			const std::string& spelling = program_->names.Get(payload);
			const OperatorKind operation = spelling == "+" ? OPERATOR_PLUS : spelling == "-" ? OPERATOR_MINUS :
				spelling == "==" ? OPERATOR_EQUAL : spelling == "<" ? OPERATOR_LESS : OPERATOR_NONE;
			if (operation == OPERATOR_NONE) return kNoFunctionTemplateAbiExpression;
			const FunctionTemplateAbiExpressionId left = ParseExpression(), right = ParseExpression();
			if (left != kNoFunctionTemplateAbiExpression && right != kNoFunctionTemplateAbiExpression)
				expression = AppendAbiExpression(program_, FunctionTemplateAbiExpression(
					FUNCTION_TEMPLATE_ABI_EXPRESSION_BINARY, left, right,
					kNoFunctionTemplateAbiType, 0, kNoTemplateParameter, operation));
		}
		else if (node == "call-expression")
		{
			const FunctionTemplateAbiExpressionId callee = ParseExpression();
			NameId ignored = 0;
			if (callee == kNoFunctionTemplateAbiExpression || !BeginNode("argument-list", &ignored))
				return kNoFunctionTemplateAbiExpression;
			std::vector<FunctionTemplateAbiArgument> arguments;
			while (position_ < atoms_.size() && ResultIdentityKind(atoms_[position_]) != FUNCTION_TEMPLATE_RESULT_NODE_END)
			{
				const bool expansion = IsNode("pack-expansion-expression");
				if (expansion && !BeginNode("pack-expansion-expression", &ignored))
					return kNoFunctionTemplateAbiExpression;
				const FunctionTemplateAbiExpressionId argument = ParseExpression();
				if (argument == kNoFunctionTemplateAbiExpression) return argument;
				if (expansion && !EndNode()) return kNoFunctionTemplateAbiExpression;
				arguments.push_back(FunctionTemplateAbiArgument(FUNCTION_TEMPLATE_ABI_ARGUMENT_EXPRESSION,
					kNoFunctionTemplateAbiType, argument, expansion));
			}
			std::uint32_t begin = 0;
			if (!EndNode() || !StoreArguments(arguments, &begin)) return kNoFunctionTemplateAbiExpression;
			expression = AppendAbiExpression(program_, FunctionTemplateAbiExpression(
				FUNCTION_TEMPLATE_ABI_EXPRESSION_CALL, callee, kNoFunctionTemplateAbiExpression,
				kNoFunctionTemplateAbiType, 0, kNoTemplateParameter, OPERATOR_NONE, false, begin,
				static_cast<std::uint32_t>(arguments.size())));
		}
		else if (node == "cast-expression" && program_->names.Get(payload) == "static_cast")
		{
			FunctionTemplateAbiTypeId type = ParseType();
			const FunctionTemplateAbiExpressionId operand = ParseExpression();
			if (type != kNoFunctionTemplateAbiType && operand != kNoFunctionTemplateAbiExpression)
			{
				// Clang encodes the referred-to type for a reference cast.
				const FunctionTemplateAbiTypeKind cast_kind = program_->function_template_abi_types[type].kind;
				if (cast_kind == FUNCTION_TEMPLATE_ABI_TYPE_LVALUE_REFERENCE || cast_kind == FUNCTION_TEMPLATE_ABI_TYPE_RVALUE_REFERENCE)
					type = program_->function_template_abi_types[type].child;
				expression = AppendAbiExpression(program_, FunctionTemplateAbiExpression(
					FUNCTION_TEMPLATE_ABI_EXPRESSION_STATIC_CAST, operand, kNoFunctionTemplateAbiExpression, type));
			}
		}
		else if (program_->names.Get(payload) == "*")
		{
			const FunctionTemplateAbiExpressionId operand = ParseExpression();
			if (operand != kNoFunctionTemplateAbiExpression)
				expression = AppendAbiExpression(program_,
					FunctionTemplateAbiExpression(
						FUNCTION_TEMPLATE_ABI_EXPRESSION_UNARY, operand,
						kNoFunctionTemplateAbiExpression,
						kNoFunctionTemplateAbiType, 0,
						kNoTemplateParameter, OPERATOR_STAR));
		}
		if (expression == kNoFunctionTemplateAbiExpression || !EndNode())
			return kNoFunctionTemplateAbiExpression;
		return expression;
	}

	bool Complete() const { return position_ == atoms_.size(); }

private:
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
		if (!IsNode("decltype-specifier"))
			return kNoFunctionTemplateAbiType;
		NameId payload = 0;
		if (!BeginNode("decltype-specifier", &payload))
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
				component->entity = EntityFromMarker(
					marker, ResultIdentityValue(atoms_[position_++]));
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
			if (value_argument || value_parameter || ArgumentKind(component->entity, argument) ==
				TEMPLATE_ARGUMENT_INTEGRAL)
			{
				const FunctionTemplateAbiExpressionId expression = ParseExpression();
				if (expression == kNoFunctionTemplateAbiExpression) return false;
				component->arguments.push_back(FunctionTemplateAbiArgument(
					FUNCTION_TEMPLATE_ABI_ARGUMENT_EXPRESSION,
					kNoFunctionTemplateAbiType, expression,
					pack_expansion));
			}
			else
			{
				const FunctionTemplateAbiTypeId type = ParseType(&pack_expansion);
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
		const ParsedComponent& component, FunctionTemplateAbiTypeId owner,
		bool expression_owner = false, bool written_qualifier = false)
	{
		if (component.arguments.empty())
		{
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

	FunctionTemplateAbiTypeId ParseQualifiedType(bool value_terminal,
		FunctionTemplateAbiExpressionId* expression)
	{
		if (ResultIdentityKind(atoms_[position_++]) !=
			FUNCTION_TEMPLATE_RESULT_QUALIFIED_BEGIN)
			return kNoFunctionTemplateAbiType;
		FunctionTemplateAbiTypeId root = kNoFunctionTemplateAbiType;
		std::size_t components = 0;
		ScopeId name_space = kNoScope;
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
};

FunctionTemplateAbiTypeId ApplyTypeModifiers(Program* program,
	TypeId shape, FunctionTemplateAbiTypeId root)
{
	if (root == kNoFunctionTemplateAbiType || shape == kNoType) return root;
	// A completely concrete alias already includes its underlying modifiers.
	// Consume the canonical formed type, including any outside declarator and
	// reference collapsing, rather than applying that shape to it a second time.
	FunctionTemplateAbiTypeId leaf = root;
	for (;;)
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

void Analyzer::PublishFunctionTemplateResultAbiType(
	FunctionTemplatePattern* pattern, const DeclaratorInfo& declarator)
{
	if (!pattern) return;
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
		InternExpandedFunctionTemplateResult(&probe);
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
		if (type != kNoFunctionTemplateAbiType && reader.Complete())
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
	std::vector<NodeId> parameter_syntax;
	for (std::size_t p = 0; p < declarator.parameters.size(); ++p)
	{
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
		const NodeId root = member_pointer_pack ? parameter_syntax[p] : FindDescendant(*arena_,
			declarator.parameters[p].type_syntax,
			"structured-type-name");
		if (root == kNoNode) continue;
		const bool written_expansion = has_template_pack &&
			(arena_->HasDescendantTag(root, STAG_PARAMETER_PACK) ||
			 arena_->HasDescendantTag(root, STAG_PACK_EXPANSION_EXPRESSION));
		const bool dependent_value = !value_parameters.empty() &&
			SyntaxUsesAnyTemplateParameter(root, value_parameters);
		const NamePath path = StructuredNamePath(root);
		if (!member_pointer_pack && path.Empty()) continue;
		const LookupResult marker = member_pointer_pack ? LookupResult() : LookupPath(
			pattern->lexical_scope, path, LOOKUP_TYPE);
		if (!member_pointer_pack && !declarator.parameters[p].nondeduced && !written_expansion && !dependent_value &&
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
		InternExpandedFunctionTemplateResult(&probe);
		if (probe.expanded_result_identity ==
			kNoFunctionTemplateResultIdentity) continue;
		if (!member_pointer_pack && !declarator.parameters[p].nondeduced && !written_expansion && !dependent_value &&
			!probe.expanded_result_has_alias) continue;
		std::vector<std::uint64_t> atoms;
		function_template_result_identities_.CopyAtoms(
			probe.expanded_result_identity, &atoms);
		AbiPublication publication(program_);
		AbiIdentityReader reader(program_, class_templates_, function_templates_,
			class_template_pattern_by_entity_, pattern->parameters, atoms);
		const FunctionTemplateAbiTypeId type = member_pointer_pack ? reader.ParseParameterType() : reader.ParseType();
		if (type != kNoFunctionTemplateAbiType && reader.Complete())
		{
			pattern->abi_function_parameter_types[p] = ApplyTypeModifiers(
				program_, declarator.parameters[p].function_type, type);
			publication.Commit();
		}
	}

	if (pattern->expanded_result_identity !=
		kNoFunctionTemplateResultIdentity)
	{
		std::vector<std::uint64_t> atoms;
		function_template_result_identities_.CopyAtoms(
			pattern->expanded_result_identity, &atoms);
		if (pattern->expanded_result_has_alias ||
			HasRetainedParameterRoot(*program_, *pattern, atoms))
		{
			AbiPublication publication(program_);
			AbiIdentityReader reader(program_, class_templates_, function_templates_,
				class_template_pattern_by_entity_, pattern->parameters, atoms);
			const FunctionTemplateAbiTypeId root = reader.ParseType();
			if (root != kNoFunctionTemplateAbiType && reader.Complete())
			{
				pattern->abi_result_type = ApplyTypeModifiers(program_,
					program_->types.Get(pattern->shape_type).child, root);
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
