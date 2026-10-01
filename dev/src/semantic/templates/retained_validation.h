#ifndef CPPGM_SEMANTIC_TEMPLATES_RETAINED_VALIDATION_H
#define CPPGM_SEMANTIC_TEMPLATES_RETAINED_VALIDATION_H

#include "semantic/analysis/analyzer.h"

namespace cppgm
{
namespace semantic
{

enum RetainedNameKind
{
	RETAINED_TYPE_NAME = 1,
	RETAINED_VALUE_NAME = 2,
	RETAINED_CLASS_NAME = 4,
	RETAINED_CLASS_DEFINITION = 8
};

enum RetainedCallLookupState
{
	RETAINED_CALL_LOOKUP_PUBLISHED = 1,
	RETAINED_CALL_ADL_ELIGIBLE = 2
};

enum RetainedSpecialMemberKind
{
	RETAINED_CONSTRUCTOR,
	RETAINED_DESTRUCTOR,
	RETAINED_CONVERSION_FUNCTION
};

enum RetainedExceptionState
{
	RETAINED_EXCEPTION_THROWING,
	RETAINED_EXCEPTION_NONTHROWING,
	RETAINED_EXCEPTION_DEFERRED
};

struct RetainedTemplateParameterKey
{
	NameId name;
	bool pack;

	RetainedTemplateParameterKey(NameId name_value, bool pack_value)
		: name(name_value), pack(pack_value) {}
};

struct RetainedTemplateParameterRange
{
	std::uint32_t first;
	std::uint32_t count;

	RetainedTemplateParameterRange() : first(0), count(0) {}
};

struct RetainedCurrentClass
{
	NameId name;
	NodeId source;
	RetainedTemplateParameterRange parameters;

	RetainedCurrentClass() : name(0), source(kNoNode) {}
};

struct RetainedExpressionType
{
	TypeId type;
	ValueCategory category;
	bool integer_literal_zero;
	explicit RetainedExpressionType(const ExpressionInfo& expression)
		: type(expression.type), category(expression.category),
		  integer_literal_zero(expression.integer_literal_zero) {}
};

struct RetainedScope
{
	ScopeId semantic_scope;
	std::size_t parent;
	std::unordered_map<NameId, std::uint8_t> names;
	std::unordered_map<NameId, std::vector<BindingId> > call_functions;
	std::unordered_map<NameId, std::vector<std::size_t> > call_templates;
	std::unordered_map<NameId, EntityId> call_naming_classes;
	std::unordered_set<NameId> dependent_values;
	std::unordered_set<NameId> class_type_names;
	std::unordered_set<NameId> class_type_definitions;
	RetainedTemplateParameterRange template_parameters;
	RetainedCurrentClass current_class;
	NodeId class_declaration;
	std::uint32_t switch_entry_barriers;
	bool defer_unknown_members;
	bool unmodeled_fixed_base;
	bool unmodeled_current_class;

	RetainedScope(ScopeId semantic, std::size_t owner, bool defer,
		bool fixed_base, bool current_class)
		: semantic_scope(semantic), parent(owner), class_declaration(kNoNode),
		  switch_entry_barriers(0),
		  defer_unknown_members(defer),
		  unmodeled_fixed_base(fixed_base),
		  unmodeled_current_class(current_class) {}
};

class RetainedTemplateValidator
{
public:
	RetainedTemplateValidator(Analyzer& analyzer, NodeId target,
		ScopeId lexical_scope, const std::vector<TemplateParameter>& parameters,
		NodeId class_declaration)
		: analyzer_(analyzer), target_(target), lexical_scope_(lexical_scope),
		  parameters_(parameters), class_declaration_(class_declaration) {}

	void Run();

private:
	typedef std::unordered_map<NameId, std::size_t> TemplateOrdinalMap;

	std::size_t AddScope(ScopeId semantic_scope, std::size_t parent,
		bool defer_unknown_members, bool unmodeled_fixed_base = false,
		bool unmodeled_current_class = false);
	std::size_t AddChildScope(std::size_t parent, ScopeKind kind,
		bool defer_unknown_members = false);
	void DeclareParameter(std::size_t scope,
		const TemplateParameter& parameter);
	void Declare(std::size_t scope, NameId name, RetainedNameKind kind,
		bool allow_existing = false);
	void DeclareClassType(std::size_t scope, NameId name, bool definition);
	std::uint8_t LookupLocal(std::size_t scope, NameId name) const;
	bool LookupLocalCallSets(std::size_t scope, NameId name,
		std::vector<BindingId>* functions,
		std::vector<std::size_t>* templates, EntityId* naming_class) const;
	bool IsDependentValue(std::size_t scope, NameId name) const;
	bool DefersUnknownMembers(std::size_t scope) const;
	bool HasUnmodeledFixedBase(std::size_t scope) const;
	bool HasUnmodeledCurrentClass(std::size_t scope) const;
	NodeId DeclarationDeclarator(NodeId node) const;
	bool IsQualifiedMemberDefinition(NodeId node) const;
	bool IsTypedef(NodeId specifiers) const;
	bool HasBaseClass(NodeId node) const;
	bool BaseSyntaxIsDependent(
		NodeId node, std::size_t scope, NameId injected);
	bool SyntaxUsesTemplateParameter(NodeId node) const;
	bool IsCurrentInstantiationQualifier(
		NodeId component, std::size_t scope) const;
	bool RequiresDependentTypename(NodeId node, std::size_t scope) const;
	void ValidateDependentTypenameSpecifiers(
		NodeId sequence, std::size_t scope) const;
	bool IsLoneQualifiedNameSpecifier(NodeId sequence) const;
	void SetTemplateParameterRange(std::size_t scope,
		const std::vector<TemplateParameter>& parameters);
	bool SyntaxUsesRetainedType(NodeId node, std::size_t scope) const;
	bool SyntaxUsesRetainedValue(NodeId node, std::size_t scope) const;
	void Visit(NodeId node, std::size_t scope, bool unknown_callee = false);
	void VisitChildren(NodeId node, std::size_t scope);
	bool VisitSwitchLabel(NodeId node, std::size_t scope);
	bool VisitControlStatement(NodeId node, std::size_t scope);
	void VisitClass(NodeId node, std::size_t scope);
	void PredeclareClassMembers(NodeId node, std::size_t scope);
	void PredeclareClassSimple(NodeId node, std::size_t scope);
	void DeclareEnumValues(NodeId node, std::size_t scope);
	void VisitFunction(NodeId node, std::size_t scope);
	NodeId FindParameterClause(NodeId declarator) const;
	void BindFunctionParameters(NodeId declarator, std::size_t scope);
	bool DeclareStructuredBindings(NodeId declarator, std::size_t scope);
	NodeId RetainedOperatorCallArgument(NodeId node) const;
	void VisitSimple(NodeId node, std::size_t scope, bool predeclared);
	void VisitUsing(NodeId node, std::size_t scope, bool predeclared = false);
	void VisitCall(NodeId node, std::size_t scope);
	void VisitSizeof(NodeId node, std::size_t scope);
	ExpressionInfo KnownExpressionFacts(NodeId node) const;
	void PublishExpressionFacts(NodeId node, const ExpressionInfo& expression);
	void ValidateFixedExpression(NodeId node);
	ExpressionInfo FixedUnaryExpression(NodeId node, ExpressionInfo operand);
	ExpressionInfo FixedBinaryExpression(NodeId node, ExpressionInfo left,
		ExpressionInfo right);
	void ValidateFixedCall(NodeId node);
	void VisitIdExpression(NodeId node, std::size_t scope,
		bool unknown_callee);
	void ValidateKnownTemplateArgumentKinds(NodeId node, ScopeId scope);
	const TemplateParameter* TemplateParameterUsedBy(NodeId node) const;
	void ValidateSpecialMemberExceptionSpecification();
	bool IsFunctionDeclarator(NodeId declarator) const;
	void PublishKnownAlias(std::size_t scope, NameId name, TypeId type);
	TypeId KnownDeclarationType(NodeId specifiers, NodeId declarator,
		ScopeId scope);
	std::uint64_t StaticMemberDeclarationKey() const;
	void ValidateStaticMemberDeclaration(ScopeId scope);
	RetainedSpecialMemberKind SpecialMemberKind(NodeId node) const;
	TemplateOrdinalMap TemplateOrdinals(
		const std::vector<TemplateParameter>& parameters) const;
	bool RetainedTypeSyntaxEquivalent(NodeId left, NodeId right,
		NodeId left_identifier, NodeId right_identifier,
		const TemplateOrdinalMap& left_parameters,
		const TemplateOrdinalMap& right_parameters) const;
	bool ParameterTypesEquivalent(NodeId left, NodeId right,
		const std::vector<TemplateParameter>& left_parameters) const;
	bool FunctionQualifiersEquivalent(NodeId left, NodeId right) const;
	RetainedExceptionState RetainedExceptionSpecificationState(
		NodeId declarator) const;

	Analyzer& analyzer_;
	NodeId target_;
	ScopeId lexical_scope_;
	const std::vector<TemplateParameter>& parameters_;
	NodeId class_declaration_;
	std::unordered_set<NameId> parameter_names_;
	std::unordered_set<NodeId> template_argument_validation_visited_;
	std::vector<RetainedTemplateParameterKey> template_parameter_keys_;
	std::vector<RetainedScope> scopes_;
	std::vector<std::size_t> switch_entry_scopes_;
	std::unordered_map<NodeId, RetainedExpressionType> expression_types_;
};

}
}

#endif
