#include "sema/semantics_dump.h"

#include <cstring>
#include <deque>
#include <string>
#include <unordered_set>
#include <vector>

#include "lex/identifier_table.h"
#include "sema/integral_arithmetic.h"
#include "sema/template_records.h"
#include "sema/string_literals.h"
#include "syntax/ast_render.h"

namespace sema {

namespace {

const char* ClassKeySpelling(TokenKind key)
{
	switch (key)
	{
	case TokenKind::KW_CLASS:
		return "class";
	case TokenKind::KW_UNION:
		return "union";
	default:
		return "struct";
	}
}

const char* CategorySpelling(ValueCategory category)
{
	switch (category)
	{
	case ValueCategory::LValue:
		return "lvalue";
	case ValueCategory::XValue:
		return "xvalue";
	default:
		return "prvalue";
	}
}

void AppendCv(uint8_t cv, std::string& out)
{
	if (cv & kConst)
		out += "const ";
	if (cv & kVolatile)
		out += "volatile ";
}

// The presentation name of a class: an unnamed class's stable name, else
// its label or name.
const char* ClassLabel(const ClassEntity* entity)
{
	if (entity->local_label != nullptr)
		return entity->local_label;
	if (entity->label != nullptr)
		return entity->label;
	return entity->name != nullptr ? entity->name->spelling : "";
}

const char* EnumLabel(const EnumEntity* entity)
{
	if (entity->label != nullptr)
		return entity->label;
	return entity->name != nullptr ? entity->name->spelling : "";
}

// The enclosing named namespaces and classes of a scope, outermost first,
// each followed by `::` (function-local scopes add nothing: a local class
// is not qualified by its function). An unnamed namespace adds nothing to
// a function's name and GCC's `_GLOBAL__N_1` to a class's (`types`).
void AppendQualifier(const Scope* scope, std::string& out, bool types = false)
{
	std::vector<const char*> parts;
	for (; scope != nullptr; scope = scope->parent)
	{
		if (scope->kind == ScopeKind::Namespace)
		{
			const Entity* entity = scope->entity;
			if (entity == nullptr || entity->name == nullptr)
			{
				if (scope->parent == nullptr)
					break;
				if (types)
					parts.push_back("_GLOBAL__N_1");
				continue;
			}
			parts.push_back(entity->name->spelling);
			continue;
		}
		if (scope->kind == ScopeKind::Class && scope->entity != nullptr && scope->entity->kind == EntityKind::Class)
		{
			parts.push_back(ClassLabel(static_cast<const ClassEntity*>(scope->entity)));
			continue;
		}
		if (scope->kind == ScopeKind::Enum || scope->kind == ScopeKind::TemplateParameters)
			continue;
		break;
	}
	for (size_t i = parts.size(); i-- > 0;)
	{
		out += parts[i];
		out += "::";
	}
}

// A class's owner scope is its declaring scope; its qualifier includes it.
void AppendClassName(const ClassEntity* entity, std::string& out)
{
	AppendQualifier(entity->owner, out, true);
	out += ClassLabel(entity);
}

void AppendType(const Type* type, std::string& out);

// A non-static member function's implicit object: `pointer to cv C`.
struct ObjectPointer
{
	const Type* object;
	uint8_t cv;
};

void AppendObjectPointer(const ObjectPointer& pointer, std::string& out)
{
	out += "pointer to ";
	AppendCv(pointer.cv, out);
	AppendType(pointer.object, out);
}

void AppendFunctionSuffix(const Type* type, const ObjectPointer* object, std::string& out)
{
	out += "function of (";
	bool first = true;
	if (object != nullptr)
	{
		AppendObjectPointer(*object, out);
		first = false;
	}
	for (uint32_t i = 0; i < type->param_count; ++i)
	{
		if (!first)
			out += ", ";
		first = false;
		AppendType(type->params[i], out);
	}
	if (type->variadic)
		out += first ? "..." : ", ...";
	out += ')';
}

// The course spelling of a type, with class names qualified by their
// enclosing namespaces and classes.
void AppendType(const Type* type, std::string& out)
{
	// Declared types print in their canonical form (8.3.5p5 adjustments).
	type = type->canonical;
	for (;;)
	{
		if (type->kind != TypeKind::Array && type->kind != TypeKind::Function && !type->is_reference())
			AppendCv(type->cv, out);
		switch (type->kind)
		{
		case TypeKind::Fundamental:
			out += FundamentalTypeName(type->fundamental);
			return;
		case TypeKind::Class:
		{
			const ClassEntity* entity = static_cast<const ClassEntity*>(type->entity);
			out += ClassKeySpelling(entity->key);
			out += ' ';
			AppendClassName(entity, out);
			return;
		}
		case TypeKind::Enum:
		{
			const EnumEntity* entity = static_cast<const EnumEntity*>(type->entity);
			out += entity->scoped ? "enum class " : "enum ";
			out += EnumLabel(entity);
			return;
		}
		case TypeKind::TemplateParameter:
			out += "typename ";
			out += type->entity->name != nullptr ? type->entity->name->spelling : "";
			return;
		case TypeKind::Value:
			// A non-type template argument: its value; a pointer or
			// reference argument the entity it designates.
			if (AddressArgument(type))
			{
				out += type->entity == nullptr ? "nullptr" : type->element->is_reference() ? "" : "&";
				out += type->entity != nullptr && type->entity->name != nullptr ? type->entity->name->spelling : "";
				return;
			}
			out += IsSignedConstant(Constant{ type->bound, type->element })
				? std::to_string(static_cast<long long>(type->bound))
				: std::to_string(type->bound);
			return;
		case TypeKind::Pack:
			out += "pack of (";
			for (uint32_t i = 0; i < type->param_count; ++i)
			{
				if (i != 0)
					out += ", ";
				AppendType(type->params[i], out);
			}
			out += ')';
			return;
		case TypeKind::Expansion:
			out += "expansion of ";
			break;
		case TypeKind::BoundPack:
			out += "bound ";
			break;
		case TypeKind::TemplateTemplateParameter:
			out += "template-parameter ";
			out += type->entity->name != nullptr ? type->entity->name->spelling : "";
			return;
		case TypeKind::Template:
			out += "template ";
			out += type->entity->name != nullptr ? type->entity->name->spelling : "";
			return;
		case TypeKind::Pointer:
			out += "pointer to ";
			break;
		case TypeKind::LValueReference:
			out += "lvalue-reference to ";
			break;
		case TypeKind::RValueReference:
			out += "rvalue-reference to ";
			break;
		case TypeKind::MemberPointer:
			out += "member-pointer of ";
			AppendType(type->owner, out);
			out += " to ";
			break;
		case TypeKind::Array:
			out += "array of ";
			out += std::to_string(type->bound);
			out += ' ';
			break;
		case TypeKind::Complex:
			out += "complex ";
			break;
		case TypeKind::BlockPointer:
			out += "block-pointer to ";
			break;
		case TypeKind::Mode:
			out += "mode of ";
			break;
		case TypeKind::Vector:
			out += "vector of ";
			out += std::to_string(type->bound);
			out += ' ';
			break;
		case TypeKind::Function:
			AppendFunctionSuffix(type, nullptr, out);
			if (type->function_cv & kConst)
				out += " const";
			if (type->function_cv & kVolatile)
				out += " volatile";
			if (type->ref != RefQualifier::None)
				out += type->ref == RefQualifier::LValue ? " &" : " &&";
			out += " returning ";
			break;
		}
		type = type->element;
	}
}

void AppendValue(const Constant& value, std::string& out)
{
	if (IsSignedConstant(value) && static_cast<int64_t>(value.bits) < 0)
	{
		out += '-';
		out += std::to_string(0 - value.bits);
	}
	else
		out += std::to_string(value.bits);
}

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

// One pending line of an expression tree (see Printer::expression).
struct Item
{
	enum class Kind : uint8_t
	{
		Expr,
		// `id-expression prvalue <type> this` for an implicit object.
		This,
		// The unnamed object of an anonymous union.
		Storage,
		// `unary-expression prvalue <type> OP_AMP:&` over a member call's
		// object.
		Address,
		// `cast-expression prvalue pointer to cv <type>`: a member call's
		// object pointer converted to the member's base class.
		ObjectCast
	};
	Kind kind;
	// Print the node without the printed form of its conversion.
	bool bare;
	// Print the node with this category and type instead of its own.
	bool retyped;
	ValueCategory category;
	unsigned depth;
	// This: the object's cv-qualifiers.
	uint8_t cv;
	const syntax::Expr* expr;
	const Type* type;
	const Binding* binding;
};

class Printer
{
public:
	Printer(const SemanticGraph& graph, std::ostream& out) : graph_(graph), out_(out), function_(nullptr) {}
	~Printer() { flush(); }

	void unit(const syntax::TranslationUnit& unit);

private:
	void begin(unsigned depth) { line_.append(depth * 2, ' '); }
	void end()
	{
		line_ += '\n';
		if (line_.size() > (1u << 16))
			flush();
	}
	void flush()
	{
		out_.write(line_.data(), static_cast<std::streamsize>(line_.size()));
		line_.clear();
	}
	void simple_line(unsigned depth, const char* text)
	{
		begin(depth);
		line_ += text;
		end();
	}

	// Declarations.
	void declarations(const syntax::Slice<syntax::Decl*>& list, unsigned depth, bool block);
	void declaration(const syntax::Decl& decl, unsigned depth, bool block);
	void simple(const syntax::SimpleDecl& decl, unsigned depth, bool block);
	void declared(const Binding* binding, const syntax::Initializer* initializer, unsigned depth);
	void variable(const Binding* binding, const syntax::Initializer* initializer, unsigned depth);
	void storage(const Binding* binding, unsigned depth);
	bool string_initializer(const Type* type, const syntax::Initializer* initializer, unsigned depth);
	void constructor_action(
		const VariableEntity* variable, const char* name, const syntax::Initializer* initializer, unsigned depth);
	void function_definition(const FunctionEntity* function, const syntax::FunctionBody* body, unsigned depth);
	void function_declaration(const FunctionEntity* function, unsigned depth);
	void implicit_definition(const FunctionEntity* function, unsigned depth);
	void specialization_declaration(const FunctionEntity* function, unsigned depth);
	void function_name(const FunctionEntity* function, bool declaration = false);
	void function_type(const FunctionEntity* function);
	bool this_type(const FunctionEntity* function, ObjectPointer& out) const;
	void push_this(std::vector<Item>& stack, unsigned depth);

	// Statements.
	void statement(const syntax::Stmt& stmt, unsigned depth);
	void condition(const syntax::Condition& condition, unsigned depth);

	// Expressions.
	void expression(const syntax::Expr* expr, unsigned depth);
	void expression_retyped(const syntax::Expr* expr, unsigned depth, const Type* type);
	void run(std::vector<Item>& stack);
	void node(const Item& item, std::vector<Item>& stack);
	void call(const syntax::CallExpr& call, const ExprFacts* facts, const Item& item, std::vector<Item>& stack);
	void id_node(const syntax::IdExpr& id, const ExprFacts* facts, ValueCategory category, const Type* type,
		unsigned depth, std::vector<Item>& stack);
	bool conversion_node(const Item& item, const ExprFacts* facts, std::vector<Item>& stack);
	void push(std::vector<Item>& stack, const syntax::Expr* expr, unsigned depth);
	void header(const char* kind, ValueCategory category, const Type* type);
	void token(TokenKind kind, bool alternative);

	// The demanded implicitly defined and inline member functions.
	void trailing();
	void schedule(const DemandList* list);

	const SemanticGraph& graph_;
	std::ostream& out_;
	std::string line_;
	// The function whose body is printed (for `this`).
	const FunctionEntity* function_;
	std::vector<Item> stack_;
	std::deque<const FunctionEntity*> implicit_;
	std::deque<const FunctionEntity*> members_;
	std::unordered_set<const FunctionEntity*> scheduled_;
	std::vector<uint64_t> units_;
};

void Printer::unit(const syntax::TranslationUnit& unit)
{
	simple_line(0, "translation-unit");
	declarations(unit.declarations, 1, false);
	trailing();
}

// ------------------------------------------------------------ declarations

void Printer::declarations(const syntax::Slice<syntax::Decl*>& list, unsigned depth, bool block)
{
	for (const syntax::Decl* decl : list)
		declaration(*decl, depth, block);
}

void Printer::declaration(const syntax::Decl& decl, unsigned depth, bool block)
{
	switch (decl.kind)
	{
	case syntax::DeclKind::Namespace:
	{
		const syntax::NamespaceDecl& ns = static_cast<const syntax::NamespaceDecl&>(decl);
		begin(depth);
		line_ += "namespace-definition ";
		line_ += ns.name != nullptr ? ns.name->spelling : "<unnamed>";
		end();
		declarations(ns.declarations, depth + 1, false);
		return;
	}
	case syntax::DeclKind::Linkage:
		declarations(static_cast<const syntax::LinkageDecl&>(decl).declarations, depth, block);
		return;
	case syntax::DeclKind::Simple:
		simple(static_cast<const syntax::SimpleDecl&>(decl), depth, block);
		return;
	case syntax::DeclKind::Alias:
	{
		const syntax::AliasDecl& alias = static_cast<const syntax::AliasDecl&>(decl);
		if (alias.declared != nullptr)
			declared(alias.declared, nullptr, depth);
		return;
	}
	case syntax::DeclKind::Function:
	{
		const syntax::FunctionDecl& function = static_cast<const syntax::FunctionDecl&>(decl);
		if (function.declared == nullptr)
			return;
		const FunctionEntity* entity = static_cast<const FunctionEntity*>(function.declared->entity);
		// A definition emitted where odr-used (constexpr) waits for demand.
		if (!entity->inline_member)
			function_definition(entity, &function.body, depth);
		return;
	}
	case syntax::DeclKind::SpecialMember:
	{
		const syntax::SpecialMemberDecl& special = static_cast<const syntax::SpecialMemberDecl&>(decl);
		if (special.declared == nullptr)
			return;
		const FunctionEntity* entity = static_cast<const FunctionEntity*>(special.declared->entity);
		const bool defaulted = special.initializer != nullptr &&
			special.initializer->kind == syntax::InitializerKind::Default;
		if (special.definition || defaulted)
			function_definition(entity, special.definition ? &special.body : nullptr, depth);
		else if (block)
			function_declaration(entity, depth);
		return;
	}
	case syntax::DeclKind::Class:
	{
		const syntax::DeclSpecifier& specifier = static_cast<const syntax::ClassDecl&>(decl).specifier;
		if (specifier.kind == syntax::DeclSpecKind::Class && specifier.class_specifier->storage != nullptr)
		{
			if (block)
				simple_line(depth, "simple-declaration");
			storage(specifier.class_specifier->storage, block ? depth + 1 : depth);
			return;
		}
		// A block-scope class declaration without a body declares a name.
		if (block && specifier.kind != syntax::DeclSpecKind::Class)
			simple_line(depth, "simple-declaration");
		return;
	}
	case syntax::DeclKind::Enum:
		if (block)
			simple_line(depth, "simple-declaration");
		return;
	default:
		return;
	}
}

void Printer::simple(const syntax::SimpleDecl& decl, unsigned depth, bool block)
{
	const syntax::ClassSpecifier* anonymous = nullptr;
	for (const syntax::DeclSpecifier& specifier : decl.specifiers)
	{
		if (specifier.kind == syntax::DeclSpecKind::Class && specifier.class_specifier->storage != nullptr)
			anonymous = specifier.class_specifier;
	}
	if (block)
		simple_line(depth, "simple-declaration");
	const unsigned inner = block ? depth + 1 : depth;
	if (anonymous != nullptr)
		storage(anonymous->storage, inner);
	for (const syntax::InitDeclarator& init : decl.declarators)
	{
		if (init.declared != nullptr)
			declared(init.declared, init.initializer, inner);
	}
}

void Printer::declared(const Binding* binding, const syntax::Initializer* initializer, unsigned depth)
{
	switch (binding->kind)
	{
	case BindingKind::TypeAlias:
		begin(depth);
		line_ += "type-alias ";
		line_ += binding->name != nullptr ? binding->name->spelling : "";
		line_ += ' ';
		AppendType(binding->type, line_);
		end();
		return;
	case BindingKind::Function:
		// A deleted definition is not shown (as in the course view).
		if (initializer == nullptr || initializer->kind != syntax::InitializerKind::Delete)
			function_declaration(static_cast<const FunctionEntity*>(binding->entity), depth);
		return;
	case BindingKind::Variable:
		variable(binding, initializer, depth);
		return;
	default:
		return;
	}
}

void Printer::variable(const Binding* binding, const syntax::Initializer* initializer, unsigned depth)
{
	// Each declaration shows the type it gave the variable (an array bound
	// completed later is not yet known to an earlier declaration).
	const VariableEntity* entity = static_cast<const VariableEntity*>(binding->entity);
	begin(depth);
	line_ += "variable ";
	line_ += binding->label != nullptr ? binding->label : binding->name != nullptr ? binding->name->spelling : "";
	line_ += ' ';
	AppendType(binding->type, line_);
	end();
	if (initializer != nullptr && string_initializer(binding->type, initializer, depth + 1))
		return;
	if (entity->constructor != nullptr && (initializer != nullptr || !entity->is_extern))
	{
		constructor_action(
			entity, binding->label != nullptr ? binding->label : binding->name->spelling, initializer, depth + 1);
		return;
	}
	if (initializer == nullptr)
		return;
	switch (initializer->kind)
	{
	case syntax::InitializerKind::Assign:
	case syntax::InitializerKind::Braced:
	{
		const syntax::Expr* expr = initializer->expression;
		const ExprFacts* facts = WithoutParentheses(expr)->facts;
		// A constexpr object's initializer reads as its (const) type when
		// no value conversion applies.
		if (entity->is_constexpr && facts != nullptr && facts->conversion != nullptr &&
			facts->conversion->second == ValueConversion::None && facts->conversion->form == ConversionForm::Standard &&
			!entity->type->is_reference())
			expression_retyped(expr, depth + 1, entity->type);
		else
			expression(expr, depth + 1);
		return;
	}
	case syntax::InitializerKind::Parenthesized:
	{
		const ExprFacts* facts = initializer->arguments.size == 1 ? WithoutParentheses(initializer->arguments[0])->facts
																  : nullptr;
		if (entity->is_constexpr && facts != nullptr && facts->conversion != nullptr &&
			facts->conversion->second == ValueConversion::None && facts->conversion->form == ConversionForm::Standard &&
			!entity->type->is_reference())
			expression_retyped(initializer->arguments[0], depth + 1, entity->type);
		else
			for (const syntax::Expr* argument : initializer->arguments)
				expression(argument, depth + 1);
		return;
	}
	default:
		return;
	}
}

bool Printer::string_initializer(const Type* type, const syntax::Initializer* initializer, unsigned depth)
{
	// A character array initialized from a string literal (8.5.2) shows as
	// the list of its elements' values, the rest of the array zero.
	if (type->kind != TypeKind::Array || initializer->kind == syntax::InitializerKind::Parenthesized ||
		initializer->expression == nullptr)
		return false;
	const syntax::Expr* expr = WithoutParentheses(initializer->expression);
	if (expr->kind == syntax::ExprKind::BracedInit)
	{
		const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(*expr);
		if (list.elements.size != 1)
			return false;
		expr = WithoutParentheses(list.elements[0]);
	}
	if (expr->kind != syntax::ExprKind::Literal || expr->facts == nullptr || expr->facts->type->kind != TypeKind::Array)
		return false;
	StringLiteralFacts facts;
	if (!StringLiteralUnits(static_cast<const syntax::LiteralExpr&>(*expr).spelling, facts, units_))
		return false;
	begin(depth);
	header("braced-init-list", ValueCategory::LValue, type);
	end();
	const Type* element = type->element;
	IntegralKind kind = IntegralKind{ 8, true };
	IntegralKindOf(element, kind);
	for (uint64_t i = 0; i < type->bound; ++i)
	{
		const uint64_t unit = i < units_.size() ? units_[i] : 0;
		begin(depth + 1);
		header("literal", ValueCategory::PRValue, element);
		line_ += ' ';
		AppendValue(Constant{ ConvertIntegral(unit, kind), element->unqualified }, line_);
		end();
	}
	return true;
}

void Printer::storage(const Binding* binding, unsigned depth)
{
	variable(binding, nullptr, depth);
}

void Printer::constructor_action(
	const VariableEntity* variable, const char* name, const syntax::Initializer* initializer, unsigned depth)
{
	const FunctionEntity* constructor = variable->constructor;
	if (variable->type->kind == TypeKind::Array && initializer == nullptr)
	{
		// Each element's construction, shown as one action.
		begin(depth);
		header("constructor-array-action", ValueCategory::PRValue, nullptr);
		line_.resize(line_.size() - std::strlen("prvalue "));
		AppendType(variable->type, line_);
		end();
		begin(depth + 1);
		line_ += "constructor-action ";
		function_name(constructor);
		end();
		return;
	}
	begin(depth);
	line_ += "constructor-action ";
	function_name(constructor);
	end();
	if (initializer != nullptr)
	{
		if (initializer->kind == syntax::InitializerKind::Parenthesized)
		{
			for (const syntax::Expr* argument : initializer->arguments)
				expression(argument, depth + 1);
		}
		else
			expression(initializer->expression, depth + 1);
		return;
	}
	if (!constructor->implicit)
		return;
	// The implicitly defined default constructor runs on the object: a
	// call with its address as the implicit object argument.
	const Type* object = variable->type;
	while (object->kind == TypeKind::Array)
		object = object->element;
	begin(depth + 1);
	line_ += "call-expression prvalue void";
	end();
	begin(depth + 2);
	line_ += "callee ";
	function_name(constructor);
	line_ += ' ';
	function_type(constructor);
	end();
	begin(depth + 2);
	line_ += "unary-expression prvalue pointer to ";
	AppendType(object, line_);
	line_ += " OP_AMP:&";
	end();
	begin(depth + 3);
	line_ += "id-expression lvalue ";
	AppendType(object, line_);
	line_ += ' ';
	line_ += name;
	end();
}

bool Printer::this_type(const FunctionEntity* function, ObjectPointer& out) const
{
	if (function == nullptr || !function->member || function->is_static || function->owner->entity == nullptr)
		return false;
	out = ObjectPointer{ function->owner->entity->type, function->type->function_cv };
	return true;
}

void Printer::push_this(std::vector<Item>& stack, unsigned depth)
{
	ObjectPointer pointer = ObjectPointer{ nullptr, 0 };
	this_type(function_, pointer);
	Item object = Item();
	object.kind = Item::Kind::This;
	object.depth = depth;
	object.type = pointer.object;
	object.cv = pointer.cv;
	stack.push_back(object);
}

void Printer::function_name(const FunctionEntity* function, bool declaration)
{
	// A declaration's line qualifies a member by its class's spelling.
	const Scope* owner = function->owner;
	if (declaration && owner != nullptr && owner->kind == ScopeKind::Class && owner->entity != nullptr &&
		owner->entity->kind == EntityKind::Class)
	{
		AppendClassName(static_cast<const ClassEntity*>(owner->entity), line_);
		line_ += "::";
	}
	else
		AppendQualifier(owner, line_);
	line_ += function->label != nullptr ? function->label : function->name->spelling;
}

void Printer::function_type(const FunctionEntity* function)
{
	// A non-static member function reads with its implicit object
	// parameter: `function of (pointer to cv C, ...)`.
	const Type* type = function->type->canonical;
	ObjectPointer object;
	if (!this_type(function, object))
	{
		AppendType(type, line_);
		return;
	}
	AppendFunctionSuffix(type, &object, line_);
	line_ += " returning ";
	AppendType(type->element, line_);
}

void Printer::function_declaration(const FunctionEntity* function, unsigned depth)
{
	begin(depth);
	line_ += "function-declaration ";
	function_name(function, true);
	line_ += ' ';
	function_type(function);
	end();
}

void Printer::function_definition(const FunctionEntity* function, const syntax::FunctionBody* body, unsigned depth)
{
	begin(depth);
	line_ += "function-definition ";
	function_name(function, true);
	line_ += ' ';
	function_type(function);
	end();
	ObjectPointer object;
	if (this_type(function, object))
	{
		begin(depth + 1);
		line_ += "parameter this ";
		AppendObjectPointer(object, line_);
		end();
	}
	const Type* type = function->type->canonical;
	uint32_t index = 0;
	for (const Binding* binding = function->scope != nullptr ? function->scope->bindings : nullptr; binding != nullptr;
		binding = binding->next)
	{
		if (binding->kind != BindingKind::Parameter || index >= type->param_count)
			continue;
		begin(depth + 1);
		line_ += "parameter ";
		if (binding->name != nullptr)
			line_ += binding->name->spelling;
		line_ += ' ';
		AppendType(type->params[index++], line_);
		end();
	}
	const FunctionEntity* saved = function_;
	function_ = function;
	if (body != nullptr && body->body != nullptr)
	{
		// A constructor's or destructor's written body sits in the block of
		// its initialization or destruction actions.
		const bool special = function->constructor || (function->label != nullptr && function->label[0] == '~');
		if (special)
			simple_line(depth + 1, "compound-statement");
		statement(*body->body, special ? depth + 2 : depth + 1);
	}
	else
		simple_line(depth + 1, "compound-statement");
	function_ = saved;
}

void Printer::specialization_declaration(const FunctionEntity* function, unsigned depth)
{
	// A function template specialization's declaration, with the
	// parameters its template declared.
	function_declaration(function, depth);
	const Type* type = function->type->canonical;
	const syntax::ParameterClause* clause = function->parameters;
	for (uint32_t i = 0; i < type->param_count; ++i)
	{
		const syntax::Declarator* declarator = clause != nullptr && i < clause->parameters.size
			? clause->parameters[i]->declarator
			: nullptr;
		while (declarator != nullptr && declarator->name == nullptr)
			declarator = declarator->nested;
		begin(depth + 1);
		line_ += "parameter ";
		if (declarator != nullptr && declarator->name->last().kind == syntax::NameComponentKind::Identifier)
			line_ += declarator->name->last().identifier->spelling;
		line_ += ' ';
		AppendType(type->params[i], line_);
		end();
	}
}

void Printer::implicit_definition(const FunctionEntity* function, unsigned depth)
{
	function_definition(function, nullptr, depth);
}

// -------------------------------------------------------------- statements

void Printer::condition(const syntax::Condition& condition, unsigned depth)
{
	simple_line(depth, "condition");
	if (condition.declaration == nullptr)
	{
		expression(condition.expression, depth + 1);
		return;
	}
	simple_line(depth + 1, "condition-declaration");
	if (condition.declaration->declared != nullptr)
		declared(condition.declaration->declared, condition.declaration->initializer, depth + 2);
}

void Printer::statement(const syntax::Stmt& stmt, unsigned depth)
{
	switch (stmt.kind)
	{
	case syntax::StmtKind::Ambiguous:
		return statement(static_cast<const syntax::AmbiguousStmt&>(stmt).reading(), depth);
	case syntax::StmtKind::Compound:
		simple_line(depth, "compound-statement");
		for (const syntax::Stmt* item : static_cast<const syntax::CompoundStmt&>(stmt).items)
			statement(*item, depth + 1);
		return;
	case syntax::StmtKind::Expression:
	{
		simple_line(depth, "expression-statement");
		const syntax::Expr* expr = static_cast<const syntax::ExpressionStmt&>(stmt).expression;
		if (expr != nullptr)
			expression(expr, depth + 1);
		return;
	}
	case syntax::StmtKind::Declaration:
		declaration(*static_cast<const syntax::DeclarationStmt&>(stmt).declaration, depth, true);
		return;
	case syntax::StmtKind::If:
	{
		const syntax::IfStmt& if_stmt = static_cast<const syntax::IfStmt&>(stmt);
		simple_line(depth, "if-statement");
		condition(if_stmt.condition, depth + 1);
		simple_line(depth + 1, "then");
		statement(*if_stmt.then_branch, depth + 2);
		if (if_stmt.else_branch != nullptr)
		{
			simple_line(depth + 1, "else");
			statement(*if_stmt.else_branch, depth + 2);
		}
		return;
	}
	case syntax::StmtKind::Switch:
	case syntax::StmtKind::While:
	{
		const syntax::ConditionalLoopStmt& loop = static_cast<const syntax::ConditionalLoopStmt&>(stmt);
		simple_line(depth, stmt.kind == syntax::StmtKind::Switch ? "switch-statement" : "while-statement");
		condition(loop.condition, depth + 1);
		statement(*loop.body, depth + 1);
		return;
	}
	case syntax::StmtKind::Do:
	{
		const syntax::DoStmt& loop = static_cast<const syntax::DoStmt&>(stmt);
		simple_line(depth, "do-statement");
		statement(*loop.body, depth + 1);
		simple_line(depth + 1, "condition");
		expression(loop.condition, depth + 2);
		return;
	}
	case syntax::StmtKind::For:
	{
		const syntax::ForStmt& loop = static_cast<const syntax::ForStmt&>(stmt);
		simple_line(depth, "for-statement");
		simple_line(depth + 1, "for-init-statement");
		if (loop.init_declaration != nullptr)
			declaration(*loop.init_declaration, depth + 2, true);
		else if (loop.init_expression != nullptr)
			expression(loop.init_expression, depth + 2);
		if (loop.condition != nullptr)
			condition(*loop.condition, depth + 1);
		if (loop.iteration != nullptr)
		{
			simple_line(depth + 1, "iteration");
			expression(loop.iteration, depth + 2);
		}
		statement(*loop.body, depth + 1);
		return;
	}
	case syntax::StmtKind::Case:
	{
		const syntax::CaseStmt& case_stmt = static_cast<const syntax::CaseStmt&>(stmt);
		simple_line(depth, "case-statement");
		expression(case_stmt.value, depth + 1);
		statement(*case_stmt.body, depth + 1);
		return;
	}
	case syntax::StmtKind::Default:
		simple_line(depth, "default-statement");
		statement(*static_cast<const syntax::LabeledStmt&>(stmt).body, depth + 1);
		return;
	case syntax::StmtKind::Labeled:
		statement(*static_cast<const syntax::LabeledStmt&>(stmt).body, depth);
		return;
	case syntax::StmtKind::Break:
		simple_line(depth, "break-statement");
		return;
	case syntax::StmtKind::Continue:
		simple_line(depth, "continue-statement");
		return;
	case syntax::StmtKind::Return:
	{
		simple_line(depth, "return-statement");
		const syntax::Expr* value = static_cast<const syntax::ValueStmt&>(stmt).value;
		if (value != nullptr)
			expression(value, depth + 1);
		return;
	}
	default:
		return;
	}
}

// ------------------------------------------------------------- expressions

void Printer::header(const char* kind, ValueCategory category, const Type* type)
{
	line_ += kind;
	line_ += ' ';
	line_ += CategorySpelling(category);
	line_ += ' ';
	if (type != nullptr)
		AppendType(type, line_);
}

void Printer::token(TokenKind kind, bool alternative)
{
	line_ += ' ';
	line_ += TokenKindName(kind);
	line_ += ':';
	line_ += TokenKindSpelling(kind, alternative);
}

void Printer::push(std::vector<Item>& stack, const syntax::Expr* expr, unsigned depth)
{
	Item item = Item();
	item.kind = Item::Kind::Expr;
	item.depth = depth;
	item.expr = expr;
	stack.push_back(item);
}

void Printer::expression(const syntax::Expr* expr, unsigned depth)
{
	const size_t base = stack_.size();
	push(stack_, expr, depth);
	(void)base;
	run(stack_);
}

void Printer::expression_retyped(const syntax::Expr* expr, unsigned depth, const Type* type)
{
	Item item = Item();
	item.kind = Item::Kind::Expr;
	item.depth = depth;
	item.expr = expr;
	item.retyped = true;
	item.bare = true;
	item.type = type;
	item.category = WithoutParentheses(expr)->facts->category;
	stack_.push_back(item);
	run(stack_);
}

void Printer::run(std::vector<Item>& stack)
{
	// Each node prints its line and pushes its operands in reverse, so
	// chains nested as deeply as the source's take no call stack.
	while (!stack.empty())
	{
		const Item item = stack.back();
		stack.pop_back();
		node(item, stack);
	}
}

bool Printer::conversion_node(const Item& item, const ExprFacts* facts, std::vector<Item>& stack)
{
	const unsigned depth = item.depth;
	const Conversion* conversion = facts->conversion;
	// Printed forms of initializing conversions: a derived-to-base
	// conversion or a reference bound to a temporary whose value a
	// conversion made (not the operand's value itself) reads as a cast of
	// the operand, to the temporary's type (the pointer type itself for a
	// null pointer literal); a null pointer literal otherwise takes the
	// target type.
	if (!item.bare && conversion != nullptr && conversion->initialization)
	{
		const bool reference = conversion->form == ConversionForm::Reference;
		const bool base = conversion->second == ValueConversion::DerivedToBase;
		const bool temporary = reference && conversion->temporary &&
			(conversion->second != ValueConversion::None || conversion->first == LvalueTransform::ArrayToPointer ||
				conversion->first == LvalueTransform::FunctionToPointer);
		if (base || temporary)
		{
			const bool null_literal = conversion->second == ValueConversion::NullPointer &&
				(facts->flags & kNullLiteral) != 0;
			begin(depth);
			if (reference)
				header("cast-expression",
					temporary                          ? ValueCategory::PRValue
						: conversion->rvalue_reference ? ValueCategory::XValue
													   : ValueCategory::LValue,
					null_literal ? conversion->target->element->unqualified : conversion->target->element);
			else
				header("cast-expression", ValueCategory::PRValue, conversion->target->unqualified);
			end();
			Item inner = item;
			inner.bare = true;
			inner.depth = depth + 1;
			stack.push_back(inner);
			return true;
		}
	}
	return false;
}

void Printer::node(const Item& item, std::vector<Item>& stack)
{
	const unsigned depth = item.depth;
	switch (item.kind)
	{
	case Item::Kind::This:
		begin(depth);
		header("id-expression", ValueCategory::PRValue, nullptr);
		AppendObjectPointer(ObjectPointer{ item.type, item.cv }, line_);
		line_ += " this";
		end();
		return;
	case Item::Kind::Storage:
	{
		const VariableEntity* storage = static_cast<const VariableEntity*>(item.binding->entity);
		begin(depth);
		header("id-expression", ValueCategory::LValue, storage->type);
		line_ += ' ';
		line_ += item.binding->label;
		end();
		return;
	}
	case Item::Kind::ObjectCast:
		begin(depth);
		header("cast-expression", ValueCategory::PRValue, nullptr);
		AppendObjectPointer(ObjectPointer{ item.type, item.cv }, line_);
		end();
		return;
	case Item::Kind::Address:
		begin(depth);
		header("unary-expression", ValueCategory::PRValue, nullptr);
		line_ += "pointer to ";
		AppendType(item.type, line_);
		line_ += " OP_AMP:&";
		end();
		return;
	case Item::Kind::Expr:
		break;
	}
	const syntax::Expr* expr = WithoutParentheses(item.expr);
	const ExprFacts* facts = expr->facts;
	if (facts == nullptr)
		return;
	const Conversion* conversion = facts->conversion;
	if (conversion_node(item, facts, stack))
		return;
	ValueCategory category = item.retyped ? item.category : facts->category;
	const Type* type = item.retyped ? item.type : facts->type;
	if (!item.bare && conversion != nullptr && conversion->initialization &&
		conversion->second == ValueConversion::NullPointer && (facts->flags & kNullLiteral) != 0)
		type = conversion->result->unqualified;
	begin(depth);
	if ((facts->flags & kFolded) != 0 && expr->kind != syntax::ExprKind::Literal)
	{
		header("literal", ValueCategory::PRValue, item.retyped ? type : facts->type);
		line_ += ' ';
		AppendValue(facts->constant(), line_);
		end();
		return;
	}
	switch (expr->kind)
	{
	case syntax::ExprKind::Id:
		id_node(static_cast<const syntax::IdExpr&>(*expr), facts, category, type, depth, stack);
		return;
	case syntax::ExprKind::Literal:
	{
		const syntax::LiteralExpr& literal = static_cast<const syntax::LiteralExpr&>(*expr);
		header("literal", category, type);
		line_ += ' ';
		line_.append(literal.spelling.text, literal.spelling.length);
		end();
		return;
	}
	case syntax::ExprKind::KeywordLiteral:
	{
		const TokenKind keyword = static_cast<const syntax::KeywordLiteralExpr&>(*expr).keyword;
		if (keyword == TokenKind::KW_THIS)
		{
			header("id-expression", category, type);
			line_ += " this";
			end();
			return;
		}
		header("literal", category, type);
		token(keyword, false);
		end();
		return;
	}
	case syntax::ExprKind::Binary:
	case syntax::ExprKind::Assign:
	{
		const syntax::BinaryExpr& binary = static_cast<const syntax::BinaryExpr&>(*expr);
		header(expr->kind == syntax::ExprKind::Assign ? "assignment-expression" : "binary-expression", category, type);
		token(binary.op, binary.alternative);
		end();
		push(stack, binary.right, depth + 1);
		push(stack, binary.left, depth + 1);
		return;
	}
	case syntax::ExprKind::Conditional:
	{
		const syntax::ConditionalExpr& conditional = static_cast<const syntax::ConditionalExpr&>(*expr);
		header("conditional-expression", category, type);
		end();
		push(stack, conditional.else_expr, depth + 1);
		push(stack, conditional.then_expr, depth + 1);
		push(stack, conditional.condition, depth + 1);
		return;
	}
	case syntax::ExprKind::Unary:
	case syntax::ExprKind::Postfix:
	{
		const syntax::UnaryExpr& unary = static_cast<const syntax::UnaryExpr&>(*expr);
		header(expr->kind == syntax::ExprKind::Postfix ? "postfix-expression" : "unary-expression", category, type);
		token(unary.op, unary.alternative);
		end();
		push(stack, unary.operand, depth + 1);
		return;
	}
	case syntax::ExprKind::Call:
		call(static_cast<const syntax::CallExpr&>(*expr), facts, item, stack);
		return;
	case syntax::ExprKind::FunctionalCast:
	{
		// A functional cast to a reference type shows the reference type;
		// `void()` and `void{}` read as a literal, like T() of a scalar.
		const syntax::FunctionalCastExpr& cast = static_cast<const syntax::FunctionalCastExpr&>(*expr);
		if ((cast.braced == nullptr || cast.braced->elements.empty()) && cast.arguments.size == 0 &&
			type->kind == TypeKind::Fundamental && type->fundamental == FundamentalType::Void)
		{
			header("literal", category, type);
			line_ += " 0";
			end();
			return;
		}
		header("cast-expression", category, (facts->flags & kReferenceCast) != 0 ? facts->target : type);
		end();
		if (cast.braced != nullptr)
		{
			for (uint32_t i = cast.braced->elements.size; i-- > 0;)
				push(stack, cast.braced->elements[i], depth + 1);
		}
		else
		{
			for (uint32_t i = cast.arguments.size; i-- > 0;)
				push(stack, cast.arguments[i], depth + 1);
		}
		return;
	}
	case syntax::ExprKind::Cast:
	{
		const syntax::CastExpr& cast = static_cast<const syntax::CastExpr&>(*expr);
		if ((facts->flags & kReferenceCast) != 0 && facts->target != nullptr)
		{
			// A cast to a reference type reads as its operand, a glvalue of
			// the reference type.
			line_.resize(line_.size() - depth * 2);
			Item inner = Item();
			inner.kind = Item::Kind::Expr;
			inner.depth = depth;
			inner.expr = cast.operand;
			inner.retyped = true;
			inner.bare = true;
			inner.category = facts->category;
			inner.type = facts->target;
			stack.push_back(inner);
			return;
		}
		header("cast-expression", category, type);
		if (cast.cast == TokenKind::OP_LPAREN)
			line_ += " OP_LPAREN:";
		else
			token(cast.cast, false);
		end();
		push(stack, cast.operand, depth + 1);
		return;
	}
	case syntax::ExprKind::Subscript:
	{
		const syntax::SubscriptExpr& subscript = static_cast<const syntax::SubscriptExpr&>(*expr);
		header("subscript-expression", category, type);
		end();
		const bool commuted = (facts->flags & kCommuted) != 0;
		push(stack, commuted ? subscript.base : subscript.index, depth + 1);
		push(stack, commuted ? subscript.index : subscript.base, depth + 1);
		return;
	}
	case syntax::ExprKind::Member:
	{
		const syntax::MemberExpr& member = static_cast<const syntax::MemberExpr&>(*expr);
		header("member-expression", category, type);
		line_ += ' ';
		line_ += TokenKindName(member.op);
		line_ += ':';
		line_ += facts->entity != nullptr && facts->entity->name != nullptr ? facts->entity->name->spelling : "";
		end();
		push(stack, member.base, depth + 1);
		return;
	}
	case syntax::ExprKind::Sizeof:
	case syntax::ExprKind::TypeTrait:
	case syntax::ExprKind::BuiltinTrait:
		// alignof reads as a sizeof-expression in the course view.
		header("sizeof-expression", category, type);
		end();
		return;
	case syntax::ExprKind::BracedInit:
	{
		const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(*expr);
		header("braced-init-list", category, type);
		end();
		for (uint32_t i = list.elements.size; i-- > 0;)
			push(stack, list.elements[i], depth + 1);
		return;
	}
	default:
		header("expression", category, type);
		end();
		return;
	}
}

void Printer::id_node(const syntax::IdExpr& id, const ExprFacts* facts, ValueCategory category, const Type* type,
	unsigned depth, std::vector<Item>& stack)
{
	const Entity* entity = facts->entity;
	if ((facts->flags & (kAnonymousMember | kImplicitThis)) != 0)
	{
		header("member-expression", category, type);
		line_ += ' ';
		line_ += entity->name != nullptr ? entity->name->spelling : "";
		end();
		Item object = Item();
		object.depth = depth + 1;
		if ((facts->flags & kAnonymousMember) != 0)
		{
			const ClassEntity* home = static_cast<const ClassEntity*>(entity->owner->entity);
			object.kind = Item::Kind::Storage;
			object.type = home->type;
			object.binding = home->storage;
		}
		else
		{
			push_this(stack, depth + 1);
			return;
		}
		stack.push_back(object);
		return;
	}
	if (entity != nullptr && entity->kind == EntityKind::Variable && entity->owner != nullptr &&
		entity->owner->kind == ScopeKind::Function && entity->owner->entity != nullptr &&
		entity->owner->entity->kind == EntityKind::Function && entity->name != nullptr &&
		std::strcmp(entity->name->spelling, "__func__") == 0)
	{
		// The predefined __func__ (8.4.1p8) reads as the string it holds.
		header("literal", category, type);
		line_ += " \"";
		line_ += static_cast<const FunctionEntity*>(entity->owner->entity)->label;
		line_ += '"';
		end();
		return;
	}
	header("id-expression", category, nullptr);
	if (entity != nullptr && entity->kind == EntityKind::Function)
		function_type(static_cast<const FunctionEntity*>(entity));
	else if (type != nullptr)
		AppendType(type, line_);
	line_ += ' ';
	if (entity != nullptr && entity->kind == EntityKind::Variable && static_cast<const VariableEntity*>(entity)->member)
		line_ += entity->name->spelling;
	else
		line_ += syntax::RenderName(*id.name);
	end();
	return;
}

void Printer::call(const syntax::CallExpr& call, const ExprFacts* facts, const Item& item, std::vector<Item>& stack)
{
	const unsigned depth = item.depth;
	const syntax::Expr* callee = WithoutParentheses(call.callee);
	if (callee->facts == nullptr)
	{
		// `T(e)` whose type name the parser did not classify: a functional
		// cast (5.2.3).
		header("cast-expression", facts->category, (facts->flags & kReferenceCast) != 0 ? facts->target : facts->type);
		end();
		for (uint32_t i = call.arguments.size; i-- > 0;)
			push(stack, call.arguments[i], depth + 1);
		return;
	}
	if ((facts->flags & kDirectCall) == 0)
	{
		// Through a function lvalue or pointer: the call reads with the
		// callee's declared return type.
		const ExprFacts* target = callee->facts;
		const Type* function = target->type;
		if (function->kind != TypeKind::Function)
			function = function->unqualified->element;
		header("call-expression", facts->category, function->element);
		end();
		for (uint32_t i = call.arguments.size; i-- > 0;)
			push(stack, call.arguments[i], depth + 1);
		push(stack, call.callee, depth + 1);
		return;
	}
	const FunctionEntity* function = static_cast<const FunctionEntity*>(facts->entity);
	const Type* type = function->type->canonical;
	header("call-expression", facts->category, type->element);
	end();
	begin(depth + 1);
	line_ += "callee ";
	function_name(function);
	line_ += ' ';
	function_type(function);
	end();
	// Default arguments for the parameters without arguments.
	for (uint32_t i = type->param_count; i-- > call.arguments.size;)
	{
		if (function->default_arguments != nullptr && function->default_arguments[i] != nullptr)
			push(stack, function->default_arguments[i], depth + 1);
	}
	for (uint32_t i = call.arguments.size; i-- > 0;)
		push(stack, call.arguments[i], depth + 1);
	if (!function->member || function->is_static)
		return;
	// The implicit object argument of a non-static member function: the
	// object's address (or pointer, or `this`), converted to the member's
	// class when its recorded conversion binds a base of the object's.
	const syntax::MemberExpr* member = callee->kind == syntax::ExprKind::Member
		? static_cast<const syntax::MemberExpr*>(callee)
		: nullptr;
	if ((facts->flags & kImplicitThis) == 0 && member == nullptr)
		return;
	const Type* owner = function->owner->entity->type;
	const Conversion* object = callee->facts->conversion;
	const bool converted = object != nullptr && object->second == ValueConversion::DerivedToBase;
	const unsigned at = converted ? depth + 2 : depth + 1;
	if ((facts->flags & kImplicitThis) != 0)
		push_this(stack, at);
	else if (member->op == TokenKind::OP_ARROW)
		push(stack, member->base, at);
	else
	{
		push(stack, member->base, at + 1);
		Item address = Item();
		address.kind = Item::Kind::Address;
		address.depth = at;
		address.type = WithoutParentheses(member->base)->facts->type;
		stack.push_back(address);
	}
	if (converted)
	{
		Item cast = Item();
		cast.kind = Item::Kind::ObjectCast;
		cast.depth = depth + 1;
		cast.type = owner;
		cast.cv = function->type->function_cv;
		stack.push_back(cast);
	}
}

// ----------------------------------------------------- demanded functions

void Printer::schedule(const DemandList* list)
{
	// Implicitly defined special members wait in one queue, inline members
	// (and members never defined, printed as declarations) in another.
	for (; list != nullptr; list = list->next)
	{
		const FunctionEntity* function = list->function;
		if (scheduled_.count(function) != 0)
			continue;
		if (function->implicit)
		{
			if (function->type->param_count != 0)
				continue;
			implicit_.push_back(function);
		}
		else if (function->primary != nullptr || function->inline_member ||
			(function->member && !function->defined && list->call))
			members_.push_back(function);
		else
			continue;
		scheduled_.insert(function);
	}
}

void Printer::trailing()
{
	// Rounds: the pending implicit definitions, then the pending member
	// functions in demand order (those they demand join the queues), until
	// nothing is pending.
	schedule(graph_.root_demands);
	while (!implicit_.empty() || !members_.empty())
	{
		while (!implicit_.empty())
		{
			const FunctionEntity* function = implicit_.front();
			implicit_.pop_front();
			implicit_definition(function, 1);
			schedule(function->demands);
		}
		while (!members_.empty())
		{
			const FunctionEntity* function = members_.front();
			members_.pop_front();
			if (function->defined && function->body != nullptr)
				function_definition(function, function->body, 1);
			else if (function->primary != nullptr)
				specialization_declaration(function, 1);
			else
				function_declaration(function, 1);
			schedule(function->demands);
		}
	}
}

} // namespace

void PrintSemanticsDump(const syntax::TranslationUnit& unit, const SemanticGraph& graph, std::ostream& out)
{
	Printer printer(graph, out);
	printer.unit(unit);
}

} // namespace sema
