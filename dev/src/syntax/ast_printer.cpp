#include "syntax/ast_printer.h"

#include "lex/identifier_table.h"
#include "syntax/ast_render.h"

#include <vector>

namespace syntax {

namespace {

bool IsCvKeyword(TokenKind kind)
{
	return kind == TokenKind::KW_CONST || kind == TokenKind::KW_VOLATILE;
}

std::string Spelling(const Identifier* identifier)
{
	return identifier == nullptr ? std::string() : std::string(identifier->spelling, identifier->length);
}

std::string Spelling(const TokenSpelling& spelling)
{
	return std::string(spelling.text, spelling.length);
}

// `KW_INT:int`: a token's course term name and its spelling.
std::string TokenText(TokenKind kind, const std::string& spelling)
{
	return std::string(TokenKindName(kind)) + ":" + spelling;
}

std::string TokenText(TokenKind kind)
{
	return TokenText(kind, TokenKindSpelling(kind));
}

// Whether a declarator names something (8p4: its declarator-id, perhaps in
// a nested declarator).
bool HasDeclaratorId(const Declarator* declarator)
{
	for (; declarator != nullptr; declarator = declarator->nested)
	{
		if (declarator->name != nullptr)
			return true;
	}
	return false;
}

// Whether a clause's `...` written directly after its last parameter is
// dumped as that parameter's declarator's: a pack's ellipsis precedes the
// declarator-id (8p4), so one after a declarator-id or a default argument is
// the clause's (8.3.5p3), and one after a declarator without one (`int...`,
// `T&...`) dumps as the declarator's, as the grammar's abstract-pack-declarator
// reads it (which of the two it means is semantic, 8.3.5p14).
bool TrailingDeclaratorPack(const ParameterClause& clause)
{
	if (!clause.ellipsis || !clause.ellipsis_without_comma || clause.parameters.empty())
		return false;
	const ParameterDeclaration* last = clause.parameters[clause.parameters.size - 1];
	return last->default_argument == nullptr && !HasDeclaratorId(last->declarator);
}

const size_t kFlushThreshold = 1 << 16;

// One label per line; children are indented one level deeper.
class AstDumper
{
public:
	explicit AstDumper(std::ostream& stream) : stream_(stream), depth_(0) {}
	~AstDumper() { flush(); }

	void unit(const TranslationUnit& unit);

private:
	void line(const char* label, const std::string& text = std::string());
	void open(const char* label, const std::string& text = std::string())
	{
		line(label, text);
		++depth_;
	}
	void close() { --depth_; }

	void declarations(const Slice<Decl*>& declarations);
	void declaration(const Decl& decl);
	void template_declaration(const TemplateDecl& decl);
	void special_member(const SpecialMemberDecl& decl);
	void function_body(const FunctionBody& body);
	void ctor_initializer(const CtorInitializer& initializer);
	void decl_specifier_seq(const Slice<DeclSpecifier>& specifiers);
	void type_specifier_seq(const Slice<DeclSpecifier>& specifiers);
	bool decltype_qualified_name(const char* label, const Name& name);
	void owned_specifier(const DeclSpecifier& specifier);
	void class_specifier(const ClassSpecifier& specifier);
	void enum_specifier(const EnumSpecifier& specifier);
	void type_id(const TypeId& type);
	void declarator(const Declarator& declarator, bool trailing_pack = false);
	void declarator_suffix(const DeclaratorSuffix& suffix);
	void parameter_clause(const ParameterClause& clause);
	void initializer(const Initializer& initializer);
	void template_parameter_clause(const TemplateParameterList* parameters);
	void template_parameter(const TemplateParameter& parameter);
	void statement(const Stmt& stmt);
	void condition(const Condition& condition);
	void handler(const Handler& handler);
	void expression(const Expr& expr);
	void operand(const Expr& expr);
	void chain_label(const Expr& expr);
	void chain_rest(const Expr& expr);
	void expression_list(const char* label, const Slice<Expr*>& list);
	void lambda(const LambdaExpr& lambda);

	void flush()
	{
		stream_.write(out_.data(), static_cast<std::streamsize>(out_.size()));
		out_.clear();
	}

	std::ostream& stream_;
	std::string out_;
	unsigned depth_;
	// Chain nodes whose left operand is being printed (see expression).
	std::vector<const Expr*> spine_;
};

void AstDumper::line(const char* label, const std::string& text)
{
	out_.append(2 * depth_, ' ');
	out_ += label;
	if (!text.empty())
	{
		out_ += ' ';
		out_ += text;
	}
	out_ += '\n';
	if (out_.size() >= kFlushThreshold)
		flush();
}

void AstDumper::unit(const TranslationUnit& unit)
{
	open("translation-unit");
	declarations(unit.declarations);
	close();
}

void AstDumper::declarations(const Slice<Decl*>& declarations)
{
	for (const Decl* decl : declarations)
		declaration(*decl);
}

void AstDumper::declaration(const Decl& decl)
{
	switch (decl.kind)
	{
	case DeclKind::Empty:
		line("empty-declaration");
		break;
	case DeclKind::Asm:
		line("asm-definition");
		break;
	case DeclKind::Namespace:
	{
		const NamespaceDecl& ns = static_cast<const NamespaceDecl&>(decl);
		open("namespace-definition", ns.name == nullptr ? "<unnamed>" : Spelling(ns.name));
		if (ns.is_inline)
			line("inline");
		declarations(ns.declarations);
		close();
		break;
	}
	case DeclKind::NamespaceAlias:
	{
		const NamespaceAliasDecl& alias = static_cast<const NamespaceAliasDecl&>(decl);
		open("namespace-alias-definition", Spelling(alias.name));
		line("target", RenderName(*alias.target));
		close();
		break;
	}
	case DeclKind::UsingDirective:
	case DeclKind::UsingDeclaration:
		open(decl.kind == DeclKind::UsingDirective ? "using-directive" : "using-declaration");
		line("target", RenderName(*static_cast<const UsingDecl&>(decl).target));
		close();
		break;
	case DeclKind::Alias:
	{
		const AliasDecl& alias = static_cast<const AliasDecl&>(decl);
		open("alias-declaration", Spelling(alias.name));
		type_id(*alias.type);
		close();
		break;
	}
	case DeclKind::ExplicitInstantiation:
	{
		const ExplicitInstantiationDecl& instantiation = static_cast<const ExplicitInstantiationDecl&>(decl);
		open(instantiation.is_extern ? "explicit-instantiation-declaration" : "explicit-instantiation-definition");
		declaration(*instantiation.declaration);
		close();
		break;
	}
	case DeclKind::Template:
		template_declaration(static_cast<const TemplateDecl&>(decl));
		break;
	case DeclKind::Class:
		owned_specifier(static_cast<const ClassDecl&>(decl).specifier);
		break;
	case DeclKind::Enum:
		enum_specifier(*static_cast<const EnumDecl&>(decl).enum_specifier);
		break;
	case DeclKind::StaticAssert:
	{
		const StaticAssertDecl& assertion = static_cast<const StaticAssertDecl&>(decl);
		open("static-assert-declaration");
		expression(*assertion.condition);
		if (assertion.has_message)
			line("message", Spelling(assertion.message));
		close();
		break;
	}
	case DeclKind::SpecialMember:
		special_member(static_cast<const SpecialMemberDecl&>(decl));
		break;
	case DeclKind::Function:
	{
		const FunctionDecl& function = static_cast<const FunctionDecl&>(decl);
		open("function-definition");
		decl_specifier_seq(function.specifiers);
		declarator(*function.declarator);
		function_body(function.body);
		close();
		break;
	}
	case DeclKind::Simple:
	{
		const SimpleDecl& simple = static_cast<const SimpleDecl&>(decl);
		open("simple-declaration");
		decl_specifier_seq(simple.specifiers);
		if (!simple.declarators.empty())
		{
			open("init-declarator-list");
			for (const InitDeclarator& init : simple.declarators)
			{
				open("init-declarator");
				declarator(*init.declarator);
				if (init.initializer != nullptr)
					initializer(*init.initializer);
				close();
			}
			close();
		}
		close();
		break;
	}
	case DeclKind::Linkage:
	{
		const LinkageDecl& linkage = static_cast<const LinkageDecl&>(decl);
		open("linkage-specification", Spelling(linkage.language));
		declarations(linkage.declarations);
		close();
		break;
	}
	case DeclKind::BitField:
	{
		const BitFieldDecl& bit_field = static_cast<const BitFieldDecl&>(decl);
		open("bit-field-declaration");
		decl_specifier_seq(bit_field.specifiers);
		for (const BitFieldDeclarator& member : bit_field.declarators)
		{
			open("bit-field-declarator");
			if (member.declarator != nullptr)
				declarator(*member.declarator);
			expression(*member.width);
			close();
		}
		close();
		break;
	}
	case DeclKind::Access:
		line("access-specifier", TokenText(static_cast<const AccessDecl&>(decl).access));
		break;
	}
}

void AstDumper::template_declaration(const TemplateDecl& decl)
{
	open("template-declaration");
	template_parameter_clause(decl.parameters);
	declaration(*decl.declaration);
	close();
}

void AstDumper::special_member(const SpecialMemberDecl& decl)
{
	open(decl.definition ? "special-member-definition" : "special-member-declaration", RenderName(*decl.name));
	if (!decl.specifiers.empty())
	{
		open("member-specifiers");
		for (const DeclSpecifier& specifier : decl.specifiers)
		{
			// The course dump writes `explicit` without its token kind, and
			// its C++20 condition, if any, below it.
			if (specifier.keyword == TokenKind::KW_EXPLICIT)
			{
				open("specifier", "explicit");
				if (specifier.operand != nullptr)
					expression(*specifier.operand);
				close();
			}
			else
			{
				line("specifier", TokenText(specifier.keyword));
			}
		}
		close();
	}
	declarator(*decl.declarator);
	if (decl.initializer != nullptr)
		initializer(*decl.initializer);
	if (decl.definition)
		function_body(decl.body);
	close();
}

void AstDumper::function_body(const FunctionBody& body)
{
	if (body.try_block)
		open("function-try-block");
	if (body.ctor_initializer != nullptr)
		ctor_initializer(*body.ctor_initializer);
	statement(*body.body);
	if (body.try_block)
	{
		for (const Handler& h : body.try_block->handlers)
			handler(h);
		close();
	}
}

void AstDumper::ctor_initializer(const CtorInitializer& initializer)
{
	open("ctor-initializer");
	for (const MemInitializer& member : initializer.initializers)
	{
		open("mem-initializer");
		line("mem-initializer-id", RenderName(*member.name));
		if (member.braced)
			expression(*member.list);
		else
			expression_list("paren-argument-list", member.arguments);
		if (member.pack)
			line("pack-expansion", TokenText(TokenKind::OP_DOTS));
		close();
	}
	close();
}

void AstDumper::decl_specifier_seq(const Slice<DeclSpecifier>& specifiers)
{
	if (specifiers.empty())
		return;
	open("decl-specifier-seq");
	for (const DeclSpecifier& specifier : specifiers)
	{
		switch (specifier.kind)
		{
		case DeclSpecKind::Keyword:
			line("decl-specifier", TokenText(specifier.keyword));
			break;
		case DeclSpecKind::Name:
		{
			const Name& name = *specifier.name;
			const bool lone = !name.qualified() && !name.typename_keyword &&
				name.last().kind == NameComponentKind::Identifier && !name.last().has_arguments;
			if (lone)
				line("decl-specifier", "TT_IDENTIFIER:" + Spelling(name.last().identifier));
			else if (!decltype_qualified_name("decl-specifier", name))
				line("decl-specifier", RenderName(name));
			break;
		}
		case DeclSpecKind::Decltype:
			open("decl-specifier", "decltype(" + RenderExpression(*specifier.operand) + ")");
			expression(*specifier.operand);
			close();
			break;
		default:
			owned_specifier(specifier);
			break;
		}
	}
	close();
}

// A type named through a decltype-specifier, `decltype(e)::a::b`: the
// course dump shows the operand and the qualified rest below the label.
bool AstDumper::decltype_qualified_name(const char* label, const Name& name)
{
	if (name.global || name.components.size < 2 || name.components[0].kind != NameComponentKind::Decltype)
		return false;
	Name rest = name;
	rest.typename_keyword = false;
	rest.components = Slice<NameComponent>{ name.components.data + 1, name.components.size - 1 };
	open(label, RenderName(name));
	expression(*name.components[0].decltype_operand);
	line("qualified-type-name", RenderName(rest));
	close();
	return true;
}

void AstDumper::type_specifier_seq(const Slice<DeclSpecifier>& specifiers)
{
	open("type-specifier-seq");
	for (const DeclSpecifier& specifier : specifiers)
	{
		switch (specifier.kind)
		{
		case DeclSpecKind::Keyword:
			line(IsCvKeyword(specifier.keyword) ? "cv-qualifier" : "type-specifier", TokenText(specifier.keyword));
			break;
		case DeclSpecKind::Name:
			if (!decltype_qualified_name("decltype-specifier", *specifier.name))
				line("type-name", RenderName(*specifier.name));
			break;
		case DeclSpecKind::Decltype:
			open("decltype-specifier", "decltype(" + RenderExpression(*specifier.operand) + ")");
			expression(*specifier.operand);
			close();
			break;
		default:
			owned_specifier(specifier);
			break;
		}
	}
	close();
}

// Class and enumeration specifiers, printed where they appear.
void AstDumper::owned_specifier(const DeclSpecifier& specifier)
{
	switch (specifier.kind)
	{
	case DeclSpecKind::Class:
		class_specifier(*specifier.class_specifier);
		break;
	case DeclSpecKind::Elaborated:
		open("class-forward-declaration", RenderName(*specifier.name));
		line("class-key", TokenText(specifier.keyword));
		close();
		break;
	case DeclSpecKind::Enum:
		enum_specifier(*specifier.enum_specifier);
		break;
	case DeclSpecKind::Vector:
		open("vector-attribute", RenderExpression(*specifier.operand));
		expression(*specifier.operand);
		close();
		break;
	default:
		break;
	}
}

void AstDumper::class_specifier(const ClassSpecifier& specifier)
{
	open("class-specifier", specifier.name == nullptr ? std::string() : RenderName(*specifier.name));
	line("class-key", TokenText(specifier.key));
	if (specifier.final)
		line("class-virt-specifier", "TT_IDENTIFIER:final");
	if (specifier.has_base_clause)
	{
		open("base-clause");
		for (const BaseSpecifier& base : specifier.bases)
		{
			open("base-specifier");
			if (base.is_virtual && base.virtual_first)
				line("virtual", TokenText(TokenKind::KW_VIRTUAL));
			if (base.has_access)
				line("access-specifier", TokenText(base.access));
			if (base.is_virtual && !base.virtual_first)
				line("virtual", TokenText(TokenKind::KW_VIRTUAL));
			line("base-name", RenderName(*base.name));
			if (base.pack)
				line("pack-expansion", TokenText(TokenKind::OP_DOTS));
			close();
		}
		close();
	}
	declarations(specifier.members);
	close();
}

void AstDumper::enum_specifier(const EnumSpecifier& specifier)
{
	open("enum-specifier", specifier.name == nullptr ? std::string() : RenderName(*specifier.name));
	if (specifier.scoped)
		line("enum-key", TokenText(specifier.key));
	if (specifier.underlying != nullptr)
		type_id(*specifier.underlying);
	for (const Enumerator& enumerator : specifier.enumerators)
	{
		open("enumerator", Spelling(enumerator.name));
		if (enumerator.value != nullptr)
			expression(*enumerator.value);
		close();
	}
	close();
}

void AstDumper::type_id(const TypeId& type)
{
	open("type-id");
	type_specifier_seq(type.specifiers);
	if (type.declarator != nullptr)
		declarator(*type.declarator);
	close();
}

void AstDumper::declarator(const Declarator& declarator, bool trailing_pack)
{
	open(declarator.abstract ? "abstract-declarator" : "declarator");
	for (const PtrOperator& pointer : declarator.pointers)
	{
		if (pointer.kind == TokenKind::OP_COLON2)
			line("ptr-operator", RenderName(*pointer.class_name) + "::*");
		else
			line("ptr-operator", TokenText(pointer.kind, TokenKindSpelling(pointer.kind, pointer.alternative)));
		for (TokenKind cv : pointer.cv)
			line("cv-qualifier", TokenText(cv));
	}
	if (declarator.pack == PackPosition::BeforeName)
		line("parameter-pack", "...");
	if (declarator.name != nullptr)
		line("identifier", RenderName(*declarator.name));
	if (declarator.nested != nullptr)
	{
		open("nested-declarator");
		this->declarator(*declarator.nested);
		close();
	}
	for (const DeclaratorSuffix& suffix : declarator.suffixes)
		declarator_suffix(suffix);
	if (trailing_pack)
		line("parameter-pack", "...");
	close();
}

void AstDumper::declarator_suffix(const DeclaratorSuffix& suffix)
{
	if (suffix.kind == DeclaratorSuffixKind::Array)
	{
		open("array-suffix");
		if (suffix.bound != nullptr)
			expression(*suffix.bound);
		close();
		return;
	}
	parameter_clause(*suffix.parameters);
	for (const FunctionQualifier& qualifier : suffix.qualifiers)
	{
		std::string text;
		SyntaxRenderer renderer(text);
		switch (qualifier.kind)
		{
		case FunctionQualifierKind::Cv:
			line("cv-qualifier", TokenText(qualifier.token));
			break;
		case FunctionQualifierKind::Ref:
			line("ref-qualifier", TokenText(qualifier.token));
			break;
		case FunctionQualifierKind::Noexcept:
		case FunctionQualifierKind::Throw:
			renderer.function_qualifier(qualifier);
			open("function-qualifier", text);
			if (qualifier.kind == FunctionQualifierKind::Noexcept && qualifier.has_operand)
				expression(*qualifier.noexcept_operand);
			close();
			break;
		case FunctionQualifierKind::Virt:
			line("virt-specifier", "TT_IDENTIFIER:" + Spelling(qualifier.virt));
			break;
		case FunctionQualifierKind::TrailingReturn:
			open("trailing-return-type", RenderTypeId(*qualifier.trailing));
			type_id(*qualifier.trailing);
			close();
			break;
		}
	}
}

void AstDumper::parameter_clause(const ParameterClause& clause)
{
	open("parameter-clause");
	const bool trailing = TrailingDeclaratorPack(clause);
	for (const ParameterDeclaration* parameter : clause.parameters)
	{
		open("parameter-declaration");
		decl_specifier_seq(parameter->specifiers);
		const bool pack = trailing && parameter == clause.parameters[clause.parameters.size - 1];
		if (parameter->declarator != nullptr)
			declarator(*parameter->declarator, pack);
		else if (pack)
		{
			open("declarator");
			line("parameter-pack", "...");
			close();
		}
		if (parameter->default_argument != nullptr)
		{
			open("default-argument");
			initializer(*parameter->default_argument);
			close();
		}
		close();
	}
	if (clause.ellipsis && !trailing)
		line("parameter-pack", "...");
	close();
}

void AstDumper::initializer(const Initializer& initializer)
{
	open("initializer");
	switch (initializer.kind)
	{
	case InitializerKind::Assign:
	case InitializerKind::Braced:
		expression(*initializer.expression);
		break;
	case InitializerKind::Parenthesized:
		expression_list("paren-initializer", initializer.arguments);
		break;
	case InitializerKind::Default:
		line("special-initializer", "default");
		break;
	case InitializerKind::Delete:
		line("special-initializer", "delete");
		break;
	}
	close();
}

void AstDumper::template_parameter_clause(const TemplateParameterList* parameters)
{
	open("template-parameter-clause");
	if (parameters != nullptr && !parameters->parameters.empty())
	{
		open("template-parameter-list");
		for (const TemplateParameter* parameter : parameters->parameters)
			template_parameter(*parameter);
		close();
	}
	close();
}

void AstDumper::template_parameter(const TemplateParameter& parameter)
{
	if (parameter.kind == TemplateParameterKind::NonType)
	{
		open("non-type-template-parameter");
		decl_specifier_seq(parameter.specifiers);
		if (parameter.pack)
			line("parameter-pack", "...");
		if (parameter.declarator != nullptr)
			declarator(*parameter.declarator);
		if (parameter.default_value != nullptr)
		{
			open("default-template-argument");
			expression(*parameter.default_value);
			close();
		}
		close();
		return;
	}
	open("type-parameter");
	if (parameter.kind == TemplateParameterKind::Template)
	{
		line("template-template-parameter");
		template_parameter_clause(parameter.parameters);
	}
	line("parameter-key", TokenText(parameter.key));
	if (parameter.pack)
		line("parameter-pack", "...");
	if (parameter.name != nullptr)
		line("identifier", Spelling(parameter.name));
	if (parameter.default_type != nullptr)
	{
		open("default-template-argument");
		type_id(*parameter.default_type);
		close();
	}
	close();
}

void AstDumper::statement(const Stmt& stmt)
{
	switch (stmt.kind)
	{
	case StmtKind::Ambiguous:
		// The reading the parser found first (the syntax dump's).
		statement(*static_cast<const AmbiguousStmt&>(stmt).readings[0]);
		break;
	case StmtKind::Compound:
		open("compound-statement");
		for (const Stmt* item : static_cast<const CompoundStmt&>(stmt).items)
			statement(*item);
		close();
		break;
	case StmtKind::Expression:
		open("expression-statement");
		if (static_cast<const ExpressionStmt&>(stmt).expression != nullptr)
			expression(*static_cast<const ExpressionStmt&>(stmt).expression);
		close();
		break;
	case StmtKind::Declaration:
		declaration(*static_cast<const DeclarationStmt&>(stmt).declaration);
		break;
	case StmtKind::If:
	{
		const IfStmt& branch = static_cast<const IfStmt&>(stmt);
		open("if-statement");
		condition(branch.condition);
		open("then");
		statement(*branch.then_branch);
		close();
		if (branch.else_branch != nullptr)
		{
			open("else");
			statement(*branch.else_branch);
			close();
		}
		close();
		break;
	}
	case StmtKind::Switch:
	case StmtKind::While:
	{
		const ConditionalLoopStmt& loop = static_cast<const ConditionalLoopStmt&>(stmt);
		open(stmt.kind == StmtKind::Switch ? "switch-statement" : "while-statement");
		condition(loop.condition);
		statement(*loop.body);
		close();
		break;
	}
	case StmtKind::Do:
	{
		const DoStmt& loop = static_cast<const DoStmt&>(stmt);
		open("do-statement");
		statement(*loop.body);
		open("condition");
		expression(*loop.condition);
		close();
		close();
		break;
	}
	case StmtKind::For:
	{
		const ForStmt& loop = static_cast<const ForStmt&>(stmt);
		open("for-statement");
		open("for-init-statement");
		if (loop.init_declaration != nullptr)
			declaration(*loop.init_declaration);
		else if (loop.init_expression != nullptr)
			expression(*loop.init_expression);
		close();
		if (loop.condition != nullptr)
			condition(*loop.condition);
		if (loop.iteration != nullptr)
		{
			open("iteration");
			expression(*loop.iteration);
			close();
		}
		statement(*loop.body);
		close();
		break;
	}
	case StmtKind::RangeFor:
	{
		const RangeForStmt& loop = static_cast<const RangeForStmt&>(stmt);
		open("range-for-statement");
		open("range-declaration");
		decl_specifier_seq(loop.specifiers);
		declarator(*loop.declarator);
		close();
		open("range-initializer");
		expression(*loop.range);
		close();
		statement(*loop.body);
		close();
		break;
	}
	case StmtKind::Case:
	{
		const CaseStmt& label = static_cast<const CaseStmt&>(stmt);
		open("case-statement");
		expression(*label.value);
		statement(*label.body);
		close();
		break;
	}
	case StmtKind::Default:
		open("default-statement");
		statement(*static_cast<const LabeledStmt&>(stmt).body);
		close();
		break;
	case StmtKind::Labeled:
	{
		const LabeledStmt& label = static_cast<const LabeledStmt&>(stmt);
		open("labeled-statement", Spelling(label.label));
		statement(*label.body);
		close();
		break;
	}
	case StmtKind::Asm:
		// The PA5 grammar has no asm-definition: a line of its own.
		line("asm-statement");
		break;
	case StmtKind::Break:
		line("break-statement");
		break;
	case StmtKind::Continue:
		line("continue-statement");
		break;
	case StmtKind::Goto:
		line("goto-statement", Spelling(static_cast<const GotoStmt&>(stmt).label));
		break;
	case StmtKind::Return:
	case StmtKind::Throw:
		open(stmt.kind == StmtKind::Return ? "return-statement" : "throw-statement");
		if (static_cast<const ValueStmt&>(stmt).value != nullptr)
			expression(*static_cast<const ValueStmt&>(stmt).value);
		close();
		break;
	case StmtKind::Try:
	{
		const TryStmt& block = static_cast<const TryStmt&>(stmt);
		open("try-block");
		statement(*block.body);
		for (const Handler& h : block.handlers)
			handler(h);
		close();
		break;
	}
	}
}

void AstDumper::condition(const Condition& condition)
{
	open("condition");
	if (condition.declaration != nullptr)
	{
		open("condition-declaration");
		decl_specifier_seq(condition.declaration->specifiers);
		declarator(*condition.declaration->declarator);
		initializer(*condition.declaration->initializer);
		close();
	}
	else
	{
		expression(*condition.expression);
	}
	close();
}

void AstDumper::handler(const Handler& handler)
{
	open("handler");
	open("exception-declaration");
	if (handler.exception.ellipsis)
	{
		line("ellipsis", "...");
	}
	else
	{
		decl_specifier_seq(handler.exception.specifiers);
		if (handler.exception.declarator != nullptr)
			declarator(*handler.exception.declarator);
	}
	close();
	statement(*handler.body);
	close();
}

void AstDumper::expression_list(const char* label, const Slice<Expr*>& list)
{
	open(label);
	for (const Expr* expr : list)
		expression(*expr);
	close();
}

void AstDumper::lambda(const LambdaExpr& lambda)
{
	open("lambda-expression");
	std::string introducer;
	SyntaxRenderer(introducer).lambda_introducer(lambda);
	line("lambda-introducer", introducer);
	if (lambda.has_declarator)
	{
		open("lambda-declarator");
		parameter_clause(*lambda.parameters);
		if (lambda.is_mutable)
			line("lambda-specifier", TokenText(TokenKind::KW_MUTABLE));
		if (lambda.has_noexcept)
		{
			open("noexcept-specification");
			if (lambda.noexcept_operand != nullptr)
				expression(*lambda.noexcept_operand);
			close();
		}
		if (lambda.has_throw)
		{
			FunctionQualifier thrown = FunctionQualifier();
			thrown.kind = FunctionQualifierKind::Throw;
			thrown.throw_types = lambda.throw_types;
			std::string text;
			SyntaxRenderer renderer(text);
			renderer.function_qualifier(thrown);
			open("function-qualifier", text);
			close();
		}
		if (lambda.trailing != nullptr)
		{
			open("trailing-return-type");
			type_id(*lambda.trailing);
			close();
		}
		close();
	}
	statement(*lambda.body);
	close();
}

void AstDumper::expression(const Expr& expr)
{
	// Chains (see ChainOperand) are printed without recursing down their
	// left operands: each node opens on the way down the spine, and its other
	// operands follow on the way back up.
	const size_t base = spine_.size();
	const Expr* first = &expr;
	for (const Expr* left = ChainOperand(expr); left != nullptr; left = ChainOperand(*left))
	{
		chain_label(*first);
		spine_.push_back(first);
		first = left;
	}
	operand(*first);
	while (spine_.size() > base)
	{
		const Expr* node = spine_.back();
		spine_.pop_back();
		chain_rest(*node);
	}
}

void AstDumper::chain_label(const Expr& expr)
{
	switch (expr.kind)
	{
	case ExprKind::Binary:
	case ExprKind::Assign:
	{
		const BinaryExpr& binary = static_cast<const BinaryExpr&>(expr);
		open(expr.kind == ExprKind::Binary ? "binary-expression" : "assignment-expression",
			TokenText(binary.op, TokenKindSpelling(binary.op, binary.alternative)));
		break;
	}
	case ExprKind::Postfix:
	{
		const UnaryExpr& postfix = static_cast<const UnaryExpr&>(expr);
		open("postfix-expression", TokenText(postfix.op, TokenKindSpelling(postfix.op, postfix.alternative)));
		break;
	}
	case ExprKind::Call:
		open("call-expression");
		break;
	case ExprKind::Subscript:
		open("subscript-expression");
		break;
	case ExprKind::Member:
		open("member-expression", TokenText(static_cast<const MemberExpr&>(expr).op));
		break;
	default:
		break;
	}
}

void AstDumper::chain_rest(const Expr& expr)
{
	switch (expr.kind)
	{
	case ExprKind::Binary:
	case ExprKind::Assign:
		expression(*static_cast<const BinaryExpr&>(expr).right);
		break;
	case ExprKind::Call:
		expression_list("argument-list", static_cast<const CallExpr&>(expr).arguments);
		break;
	case ExprKind::Subscript:
		expression(*static_cast<const SubscriptExpr&>(expr).index);
		break;
	case ExprKind::Member:
	{
		std::string name;
		SyntaxRenderer(name).name(*static_cast<const MemberExpr&>(expr).member, false);
		line("identifier", name);
		break;
	}
	default:
		break;
	}
	close();
}

void AstDumper::operand(const Expr& expr)
{
	// Chain nodes are handled by expression().
	switch (expr.kind)
	{
	case ExprKind::Id:
		line("id-expression", RenderName(*static_cast<const IdExpr&>(expr).name));
		break;
	case ExprKind::Literal:
	{
		const LiteralExpr& literal = static_cast<const LiteralExpr&>(expr);
		line("literal", (literal.token_form ? "TT_LITERAL:" : "") + Spelling(literal.spelling));
		break;
	}
	case ExprKind::KeywordLiteral:
		line("keyword-literal", TokenText(static_cast<const KeywordLiteralExpr&>(expr).keyword));
		break;
	case ExprKind::Paren:
		open("parenthesized-expression");
		expression(*static_cast<const ParenExpr&>(expr).inner);
		close();
		break;
	case ExprKind::Conditional:
	{
		const ConditionalExpr& conditional = static_cast<const ConditionalExpr&>(expr);
		open("conditional-expression");
		expression(*conditional.condition);
		expression(*conditional.then_expr);
		expression(*conditional.else_expr);
		close();
		break;
	}
	case ExprKind::Unary:
	{
		const UnaryExpr& unary = static_cast<const UnaryExpr&>(expr);
		open("unary-expression", TokenText(unary.op, TokenKindSpelling(unary.op, unary.alternative)));
		expression(*unary.operand);
		close();
		break;
	}
	case ExprKind::FunctionalCast:
	{
		// A simple type keyword's arguments are a paren-argument-list; a named
		// type's (and a keyword sequence's) an argument-list, as the course
		// dump distinguishes.
		const FunctionalCastExpr& cast = static_cast<const FunctionalCastExpr&>(expr);
		const bool keywords = cast.type.size == 1 && cast.type[0].kind == DeclSpecKind::Keyword;
		open("call-expression");
		std::string type;
		for (const DeclSpecifier& specifier : cast.type)
		{
			if (!type.empty())
				type += ' ';
			if (specifier.kind == DeclSpecKind::Keyword)
				type += TokenKindSpelling(specifier.keyword);
			else if (specifier.kind == DeclSpecKind::Name)
				type += RenderName(*specifier.name);
			else
				type += "decltype(" + RenderExpression(*specifier.operand) + ")";
		}
		line("id-expression", type);
		if (cast.braced != nullptr)
			expression(*cast.braced);
		else
			expression_list(keywords ? "paren-argument-list" : "argument-list", cast.arguments);
		close();
		break;
	}
	case ExprKind::Cast:
	{
		const CastExpr& cast = static_cast<const CastExpr&>(expr);
		open("cast-expression", cast.cast == TokenKind::OP_LPAREN ? "OP_LPAREN:" : TokenText(cast.cast));
		type_id(*cast.type);
		expression(*cast.operand);
		close();
		break;
	}
	case ExprKind::Sizeof:
	{
		const SizeofExpr& size = static_cast<const SizeofExpr&>(expr);
		open("sizeof-expression");
		if (size.type != nullptr)
			type_id(*size.type);
		else
			expression(*size.operand);
		close();
		break;
	}
	case ExprKind::SizeofPack:
		line("sizeof-pack-expression", Spelling(static_cast<const SizeofPackExpr&>(expr).pack));
		break;
	case ExprKind::TypeTrait:
	{
		const TypeTraitExpr& trait = static_cast<const TypeTraitExpr&>(expr);
		open("type-trait-expression", TokenText(trait.keyword));
		if (trait.type != nullptr)
			type_id(*trait.type);
		else
			expression(*trait.operand);
		close();
		break;
	}
	case ExprKind::New:
	{
		const NewExpr& allocation = static_cast<const NewExpr&>(expr);
		open("new-expression");
		if (allocation.global)
			line("global-scope");
		if (allocation.has_placement)
		{
			std::string text = "(";
			SyntaxRenderer(text).expression_list(allocation.placement);
			text += ")";
			open("placement", text);
			expression_list("paren-argument-list", allocation.placement);
			close();
		}
		type_id(*allocation.type);
		if (allocation.initializer != nullptr)
			initializer(*allocation.initializer);
		close();
		break;
	}
	case ExprKind::Delete:
	{
		const DeleteExpr& deallocation = static_cast<const DeleteExpr&>(expr);
		open("delete-expression");
		if (deallocation.global)
			line("global-scope");
		if (deallocation.array)
			line("array-delete");
		expression(*deallocation.operand);
		close();
		break;
	}
	case ExprKind::Lambda:
		lambda(static_cast<const LambdaExpr&>(expr));
		break;
	case ExprKind::BracedInit:
		expression_list("braced-init-list", static_cast<const BracedInitList&>(expr).elements);
		break;
	case ExprKind::PackExpansion:
		open("pack-expansion-expression");
		expression(*static_cast<const PackExpansionExpr&>(expr).pattern);
		close();
		break;
	case ExprKind::Fold:
	{
		const FoldExpr& fold = static_cast<const FoldExpr&>(expr);
		open("fold-expression");
		expression(*fold.pattern);
		if (fold.init != nullptr)
			expression(*fold.init);
		close();
		break;
	}
	case ExprKind::Throw:
		open("throw-expression");
		if (static_cast<const ThrowExpr&>(expr).operand != nullptr)
			expression(*static_cast<const ThrowExpr&>(expr).operand);
		close();
		break;
	case ExprKind::Statement:
		open("statement-expression");
		statement(*static_cast<const StatementExpr&>(expr).body);
		close();
		break;
	case ExprKind::BuiltinTrait:
	{
		const BuiltinTraitExpr& trait = static_cast<const BuiltinTraitExpr&>(expr);
		open("builtin-trait-expression", GnuTraitName(trait.trait));
		for (const TypeId* type : trait.types)
			type_id(*type);
		if (trait.operand != nullptr)
			expression(*trait.operand);
		close();
		break;
	}
	case ExprKind::Binary:
	case ExprKind::Assign:
	case ExprKind::Postfix:
	case ExprKind::Call:
	case ExprKind::Subscript:
	case ExprKind::Member:
		break;
	}
}

} // namespace

void PrintTranslationUnit(const TranslationUnit& unit, std::ostream& out)
{
	AstDumper(out).unit(unit);
}

} // namespace syntax
