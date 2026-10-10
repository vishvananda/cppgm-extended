// C++17's structured binding declarations (11.5; an extension in C++11
// mode, as GCC and Clang accept them):
//   attribute-specifier-seq? decl-specifier-seq ref-qualifier?
//     [ identifier-list ] initializer ;
// The parser gives the declaration the object e's declarator first (its
// ref-qualifier, named by the list as Itanium 5.1.4 shows it, `[a, b]`),
// then each name alone (syntax/parser_declarations.cpp). The analysis
// declares e as the variable that declarator and the initializer make
// (unnamed once declared: no program names it), and each name as a
// BindingEntity recording what it designates (sema/entity_records.h): an
// element of e, a non-static data member of e, or, for a tuple-like E,
// the reference variable r_i itself, bound to get<i>(e). Later phases
// consume those records (the lowering's variable_place, the constant
// interpreter's entity_place); no expression is made for an element or a
// member.

#include "sema/analyzer.h"

#include <new>

#include "lex/identifier_table.h"
#include "sema/template_records.h"
#include "support/small_list.h"

namespace sema {

namespace {

const char* const kTupleSize = "tuple_size";
const char* const kTupleElement = "tuple_element";

template<typename T>
T* ImpliedNode(Arena& arena, uint32_t location)
{
	T* node = new (arena.allocate(sizeof(T), alignof(T))) T();
	node->location = location;
	return node;
}

} // namespace

// One structured binding declaration's analysis: e, then the names by the
// form E takes.
class Decomposer
{
public:
	Decomposer(Analyzer& analyzer, const syntax::SimpleDecl& decl, const Specifiers& specifiers, Scope* scope)
		: analyzer_(analyzer), decl_(decl), specifiers_(specifiers), scope_(scope), decomposition_(nullptr),
		  dependent_(false)
	{}

	void run();

private:
	VariableEntity* object();
	BindingEntity* declare_binding(uint32_t index, const Type* type);
	const ClassEntity* data_member_class(const ClassEntity* cls);
	void bind_members();
	ClassEntity* std_class_template(const char* spelling);
	bool tuple_size(uint64_t& size);
	const Type* tuple_element(uint64_t index);
	syntax::Expr* tuple_get(uint64_t index);
	void bind_tuple();

	Analyzer& analyzer_;
	const syntax::SimpleDecl& decl_;
	const Specifiers& specifiers_;
	Scope* scope_;
	Decomposition* decomposition_;
	// The declaration named something dependent (Binding::dependent).
	bool dependent_;
};

void Analyzer::structured_binding_declaration(
	const syntax::SimpleDecl& decl, const Specifiers& specifiers, Scope* scope)
{
	Decomposer decomposer(*this, decl, specifiers, scope);
	decomposer.run();
}

void Decomposer::run()
{
	// 11.5p1: the decl-specifier-seq holds auto and cv-qualifiers only
	// (and C++20's static and thread_local, which both host compilers
	// accept in C++17 mode).
	const uint32_t location = decl_.location;
	if (specifiers_.type != analyzer_.auto_placeholder_ || specifiers_.is_typedef || specifiers_.is_constexpr ||
		specifiers_.is_inline || specifiers_.is_extern || specifiers_.is_friend || specifiers_.is_virtual ||
		specifiers_.is_explicit || specifiers_.is_mutable)
		analyzer_.error(
			location, "a structured binding declaration's decl-specifiers are auto and cv-qualifiers (11.5p1)");
	if (analyzer_.template_scope_ != nullptr && scope_ == analyzer_.template_scope_)
		analyzer_.error(location, "a structured binding declaration as a template declaration");
	VariableEntity* e = object();
	if (e == nullptr || analyzer_.failed())
		return;
	Arena& arena = analyzer_.arena_;
	decomposition_ = new (arena.allocate(sizeof(Decomposition), alignof(Decomposition))) Decomposition();
	decomposition_->object = e;
	decomposition_->type = e->type->is_reference() ? e->type->element : e->type;
	decomposition_->count = decl_.declarators.size - 1;
	decomposition_->bindings = arena.allocate_array<BindingEntity*>(decomposition_->count);
	analyzer_.graph_.set_decomposition(e, decomposition_);
	const Type* type = decomposition_->type;
	if (analyzer_.template_depth_ != 0 && analyzer_.dependent_type(type))
	{
		// A template definition's E that depends on its parameters: the
		// names stand for something of a dependent type until its
		// instantiations bind them.
		decomposition_->form = DecompositionForm::Members;
		dependent_ = true;
		for (uint32_t i = 0; i < decomposition_->count; ++i)
			declare_binding(i, type);
		return;
	}
	if (type->kind == TypeKind::Array)
	{
		// 11.5p2: as many names as elements, each an lvalue designating
		// e[i] of the element type (cv-qualified as E is).
		decomposition_->form = DecompositionForm::Array;
		if (type->bound != decomposition_->count)
			analyzer_.error(location, "a structured binding of an array with as many names as elements (11.5p2)");
		for (uint32_t i = 0; i < decomposition_->count; ++i)
			declare_binding(i, type->element);
		return;
	}
	uint64_t size = 0;
	if (tuple_size(size))
	{
		decomposition_->form = DecompositionForm::Tuple;
		if (size != decomposition_->count)
			analyzer_.error(
				location, "a structured binding of a tuple-like type with std::tuple_size<E>::value names (11.5p3)");
		bind_tuple();
		return;
	}
	decomposition_->form = DecompositionForm::Members;
	bind_members();
}

VariableEntity* Decomposer::object()
{
	// e: `decl-specifier-seq ref-qualifier e initializer` as a variable
	// declaration, its auto deduced from the initializer (11.5p1); for the
	// names of a range-based for's declaration, the for-range-
	// declaration's variable of that name in the enclosing block.
	const syntax::InitDeclarator& declarator = decl_.declarators[0];
	const Identifier* key = declarator.declarator->name->last().identifier;
	VariableEntity* variable = nullptr;
	if (declarator.initializer == nullptr)
	{
		const LookupResult found = analyzer_.names_.unqualified(scope_, key, LookupFilter::Ordinary);
		if (!found.found() || found.binding->kind != BindingKind::Variable)
			analyzer_.error(decl_.location, "a range-based for's structured binding without its object");
		dependent_ = found.binding->dependent;
		variable = static_cast<VariableEntity*>(found.binding->entity);
		variable->name = nullptr;
		return variable;
	}
	analyzer_.auto_declarator_ = true;
	DeclaratorInfo info = analyzer_.declarator(
		declarator.declarator, specifiers_.type, scope_, decl_.location, &specifiers_);
	analyzer_.auto_declarator_ = false;
	const bool reference = info.type->is_reference();
	info.type = analyzer_.deduced_type(specifiers_, info.type, declarator.initializer, scope_, info.location);
	// An array initializer without a ref-qualifier makes e an array whose
	// elements it initializes (11.5p1): outside PA29, which binds a
	// reference to the array instead (`auto& [a, b] = array`).
	const syntax::Initializer& written = *declarator.initializer;
	const syntax::Expr* single = written.kind == syntax::InitializerKind::Parenthesized
		? (written.arguments.size == 1 ? written.arguments[0] : nullptr)
		: written.expression;
	if (!reference && single != nullptr && single->facts != nullptr && single->facts->type != nullptr &&
		single->facts->type->kind == TypeKind::Array)
		analyzer_.error(decl_.location, "a structured binding copy of an array is not supported (bind by reference)");
	Binding* binding = analyzer_.declare_variable(specifiers_, info, declarator.initializer, scope_, info.location);
	declarator.declared = binding;
	dependent_ = binding->dependent;
	variable = static_cast<VariableEntity*>(binding->entity);
	variable->name = nullptr;
	return variable;
}

BindingEntity* Decomposer::declare_binding(uint32_t index, const Type* type)
{
	// A name designating an element or member of e, of no storage of its
	// own, declared in e's scope as a variable would be (3.3.1p4: no other
	// declaration of the name there; 3.3.3p2: nor a parameter's of the
	// enclosing function's outermost block); its storage duration is e's
	// (a static one's name is no capture, 5.1.2p11).
	const syntax::Declarator& declarator = *decl_.declarators[index + 1].declarator;
	const Identifier* name = declarator.name->last().identifier;
	const uint32_t location = declarator.location;
	Scope* where = analyzer_.declaration_scope(scope_);
	analyzer_.check_template_parameter_name(scope_, name, location);
	analyzer_.check_parameter_redeclaration(name, where, location);
	const NameEntry* entry = analyzer_.names_.find(where, name);
	if (entry != nullptr && entry->value != nullptr && !entry->value->invisible)
		analyzer_.error(location, "a structured binding redeclares a name of its scope");
	const VariableEntity* e = decomposition_->object;
	BindingEntity* entity = analyzer_.new_entity<BindingEntity>(EntityKind::Variable, name, where, type);
	entity->structured_binding = true;
	entity->defined = true;
	entity->is_static = e->is_static;
	entity->is_thread_local = e->is_thread_local;
	entity->internal_linkage = e->internal_linkage;
	entity->referenced = type;
	entity->decomposition = decomposition_;
	entity->index = index;
	Binding* binding = analyzer_.new_binding(BindingKind::Variable, name, entity, type);
	analyzer_.print_in(scope_, binding);
	analyzer_.declare_in(where, binding);
	binding->dependent = dependent_;
	decomposition_->bindings[index] = entity;
	return entity;
}

const ClassEntity* Decomposer::data_member_class(const ClassEntity* cls)
{
	// The class among E and its base classes declaring non-static data
	// members (an unnamed bit-field is none, 9.6p2): E when none does. Two
	// of them declaring some is an error (11.5p4).
	// Each class once (a virtual base reached along many paths of a
	// lattice is one class to look at): a short list scanned, a map past
	// 16 classes.
	const ClassEntity* found = nullptr;
	SmallList<const ClassEntity*, 8> pending;
	SmallList<const ClassEntity*, 16> seen;
	PointerMap seen_map;
	const auto first_visit = [&](const ClassEntity* next) {
		if (seen.size() < 16)
		{
			for (const ClassEntity* earlier : seen)
			{
				if (earlier == next)
					return false;
			}
			seen.push_back(next);
			if (seen.size() == 16)
			{
				for (const ClassEntity* earlier : seen)
					seen_map.set(earlier, 0);
			}
			return true;
		}
		if (seen_map.find(next) != PointerMap::kMissing)
			return false;
		seen_map.set(next, 0);
		return true;
	};
	first_visit(cls);
	pending.push_back(cls);
	while (!pending.empty())
	{
		const ClassEntity* current = pending.back();
		pending.pop_back();
		for (const DataMember* member = current->members; member != nullptr; member = member->next)
		{
			if (member->variable->name == nullptr && member->bit_field)
				continue;
			if (found != nullptr && found != current)
				analyzer_.error(decl_.location,
					"a structured binding of a class whose non-static data members are of two classes (11.5p4)");
			found = current;
			break;
		}
		for (const BaseClass* base = current->bases; base != nullptr; base = base->next)
		{
			const ClassEntity* next = ClassOf(base->base->type);
			if (next != nullptr && first_visit(next))
				pending.push_back(next);
		}
	}
	return found != nullptr ? found : cls;
}

void Decomposer::bind_members()
{
	// 11.5p4: E is a class, no union, whose non-static data members are
	// all direct members of E or of one unambiguous base class of E, no
	// anonymous union among them, as many as the names; the i-th name
	// designates e.m_i, the members in declaration order, each accessible
	// where it is so named (CWG 2312's and P0969's reading, GCC's and
	// Clang's). Static members and member functions take no part.
	const uint32_t location = decl_.location;
	const Type* type = decomposition_->type;
	ClassEntity* cls = const_cast<ClassEntity*>(ClassOf(type));
	if (cls == nullptr || type->kind != TypeKind::Class || !analyzer_.require_complete(cls))
		analyzer_.error(
			location, "a structured binding of a type that is no array, tuple-like type or complete class (11.5)");
	if (cls->key == TokenKind::KW_UNION)
		analyzer_.error(location, "a structured binding of a union (11.5p4)");
	const ClassEntity* holder = data_member_class(cls);
	if (holder != cls)
	{
		if ((analyzer_.base_path_facts(holder, cls, location) & Analyzer::kBasePathCount) > 1)
			analyzer_.error(location, "a structured binding of the members of an ambiguous base class (11.5p4)");
		analyzer_.check_base_access(cls, holder, scope_, location);
	}
	uint32_t members = 0;
	for (const DataMember* member = holder->members; member != nullptr; member = member->next)
	{
		if (member->variable->name != nullptr)
			++members;
		else if (!member->bit_field)
			analyzer_.error(location, "a structured binding of a class with an anonymous union member (11.5p4)");
	}
	if (members != decomposition_->count)
		analyzer_.error(
			location, "a structured binding of a class with as many names as non-static data members (11.5p4)");
	uint32_t index = 0;
	for (const DataMember* member = holder->members; member != nullptr; member = member->next)
	{
		if (member->variable->name == nullptr)
			continue;
		const VariableEntity* variable = static_cast<const VariableEntity*>(member->variable);
		const LookupResult own = analyzer_.names_.own(holder->scope, variable->name, LookupFilter::Ordinary);
		if (own.found())
			analyzer_.check_access(own.binding, cls, cls, scope_, location);
		// The type of e.m_i (5.2.5p4: e's cv-qualifiers join the member's,
		// a mutable member's without const), which a reference member's
		// declared type is instead (decltype's referenced type, CWG 2312).
		const Type* declared = variable->type;
		const Type* referenced = declared->is_reference()
			? declared
			: analyzer_.types_.qualified(
				  declared, static_cast<uint8_t>(type->cv & (variable->is_mutable ? ~kConst : 0xff)));
		declare_binding(index, referenced)->member = variable;
		++index;
	}
}

ClassEntity* Decomposer::std_class_template(const char* spelling)
{
	// ::std::name, a class template the program declared (null otherwise).
	const LookupResult space = analyzer_.names_.qualified(
		analyzer_.graph_.global, analyzer_.std_key_, LookupFilter::Namespace);
	if (!space.found() || space.binding->entity == nullptr || space.binding->entity->kind != EntityKind::Namespace)
		return nullptr;
	const LookupResult found = analyzer_.names_.qualified(static_cast<NamespaceEntity*>(space.binding->entity)->scope,
		analyzer_.special_key(spelling), LookupFilter::Elaborated);
	return found.found() && !found.ambiguous ? Analyzer::class_template_of(found.binding->entity) : nullptr;
}

bool Decomposer::tuple_size(uint64_t& size)
{
	// 11.5p3 (as CWG 2386 amended it, which GCC and Clang follow): E is
	// tuple-like when std::tuple_size<E> names a complete class with a
	// member named value; std::tuple_size<E>::value, an integral constant
	// expression, is then the names' count.
	const uint32_t location = decl_.location;
	ClassEntity* templ = std_class_template(kTupleSize);
	if (templ == nullptr)
		return false;
	TypeList arguments;
	arguments.push_back(decomposition_->type);
	ClassEntity* specialization = analyzer_.class_specialization(templ, arguments, location, false);
	if (specialization == nullptr || !analyzer_.require_complete(specialization))
		return false;
	const LookupResult found = analyzer_.names_.qualified(
		specialization->scope, analyzer_.special_key("value"), LookupFilter::Ordinary);
	if (!found.found())
		return false;
	Constant value;
	if (found.binding->kind != BindingKind::Variable ||
		!analyzer_.variable_constant(static_cast<const VariableEntity*>(found.binding->entity), value) ||
		value.type == nullptr || !IsIntegralOrUnscopedEnum(value.type))
		analyzer_.error(location, "std::tuple_size<E>::value is no integral constant expression (11.5p3)");
	size = value.bits;
	return true;
}

const Type* Decomposer::tuple_element(uint64_t index)
{
	// T_i: std::tuple_element<i, E>::type (11.5p3).
	const uint32_t location = decl_.location;
	ClassEntity* templ = std_class_template(kTupleElement);
	if (templ == nullptr)
		analyzer_.error(location, "a tuple-like structured binding without std::tuple_element (11.5p3)");
	TypeList arguments;
	arguments.push_back(analyzer_.types_.value(analyzer_.types_.fundamental(FundamentalType::UnsignedLongInt), index));
	arguments.push_back(decomposition_->type);
	ClassEntity* specialization = analyzer_.class_specialization(templ, arguments, location);
	if (specialization == nullptr || !analyzer_.require_complete(specialization))
		analyzer_.error(
			location, "a tuple-like structured binding whose std::tuple_element<i, E> is incomplete (11.5p3)");
	const LookupResult found = analyzer_.names_.qualified(
		specialization->scope, analyzer_.special_key("type"), LookupFilter::Ordinary);
	if (!found.found() || (found.binding->kind != BindingKind::Type && found.binding->kind != BindingKind::TypeAlias))
		analyzer_.error(
			location, "a tuple-like structured binding whose std::tuple_element<i, E> has no type (11.5p3)");
	return analyzer_.binding_type(found.binding, location);
}

syntax::Expr* Decomposer::tuple_get(uint64_t index)
{
	// The initializer of r_i (11.5p3): e.get<i>() when class member access
	// lookup of get in E finds a function template whose first template
	// parameter is a non-type parameter (as P0961 amended it), else
	// get<i>(e) with get looked up by argument-dependent lookup alone; e
	// is an lvalue when its type is an lvalue reference, else an xvalue.
	// Its nodes are an implied construct's, typed once (kImplicit), as a
	// range-based for's begin-expr is (analyzer_range_for.cpp); the index
	// is a typed constant, no spelling.
	Arena& arena = analyzer_.arena_;
	TypeTable& types = analyzer_.types_;
	const uint32_t location = decl_.location;
	const VariableEntity* e = decomposition_->object;
	const Type* type = decomposition_->type;
	const Identifier* get = analyzer_.special_key("get");
	syntax::IdExpr* use = ImpliedNode<syntax::IdExpr>(arena, location);
	use->kind = syntax::ExprKind::Id;
	use->name = decl_.declarators[0].declarator->name;
	ExprFacts* used = analyzer_.new_facts(
		*use, type, e->type->kind == TypeKind::LValueReference ? ValueCategory::LValue : ValueCategory::XValue);
	used->entity = e;
	used->flags = kImplicit;
	// The template argument, parenthesized: its lookup takes no name for it.
	const Type* size_type = types.fundamental(FundamentalType::UnsignedLongInt);
	syntax::ParenExpr* constant = ImpliedNode<syntax::ParenExpr>(arena, location);
	constant->kind = syntax::ExprKind::Paren;
	constant->inner = analyzer_.implicit_constant(size_type, index, location);
	constant->facts = constant->inner->facts;
	syntax::TemplateArgument* argument = new (
		arena.allocate(sizeof(syntax::TemplateArgument), alignof(syntax::TemplateArgument))) syntax::TemplateArgument();
	argument->expression = constant;
	syntax::Name* name = analyzer_.implicit_name(get, location);
	name->components.data[0].has_arguments = true;
	name->components.data[0].arguments = syntax::Slice<syntax::TemplateArgument>{ argument, 1 };
	bool member = false;
	if (const ClassEntity* cls = ClassOf(type))
	{
		const LookupResult found = analyzer_.names_.qualified(cls->scope, get, LookupFilter::Ordinary);
		for (const FunctionList* node = found.found() && found.binding->kind == BindingKind::Function ? found.functions
																									  : nullptr;
			node != nullptr && !member; node = node->next)
		{
			const FunctionEntity* function = static_cast<const FunctionEntity*>(node->binding->entity);
			const TemplateHead* head = function->templated ? HeadOf(function) : nullptr;
			member = head != nullptr && head->count != 0 && head->parameters[0]->non_type;
		}
	}
	syntax::CallExpr* call = ImpliedNode<syntax::CallExpr>(arena, location);
	call->kind = syntax::ExprKind::Call;
	ExprFacts* facts = nullptr;
	if (member)
	{
		syntax::MemberExpr* access = ImpliedNode<syntax::MemberExpr>(arena, location);
		access->kind = syntax::ExprKind::Member;
		access->op = TokenKind::OP_DOT;
		access->base = use;
		access->member = name;
		call->callee = access;
		call->arguments = syntax::Slice<syntax::Expr*>{ nullptr, 0 };
		facts = analyzer_.typed(*call, scope_);
	}
	else
	{
		syntax::IdExpr* callee = ImpliedNode<syntax::IdExpr>(arena, location);
		callee->kind = syntax::ExprKind::Id;
		callee->name = name;
		syntax::Expr** operand = arena.allocate_array<syntax::Expr*>(1);
		operand[0] = use;
		call->callee = callee;
		call->arguments = syntax::Slice<syntax::Expr*>{ operand, 1 };
		analyzer_.expression_scope_ = scope_;
		analyzer_.adl_functions_.clear();
		analyzer_.argument_dependent(call->arguments, get, !dependent_);
		if (analyzer_.adl_functions_.empty())
			analyzer_.error(
				location, "no get function for a tuple-like structured binding (11.5p3: argument-dependent lookup)");
		TypeList explicit_arguments;
		explicit_arguments.push_back(types.value(size_type, index));
		analyzer_.explicit_arguments_ = &explicit_arguments;
		facts = analyzer_.resolved_call(*call, nullptr, nullptr, false, nullptr);
		analyzer_.explicit_arguments_ = nullptr;
	}
	if (facts == nullptr)
		analyzer_.error(location, "no viable get function for a tuple-like structured binding (11.5p3)");
	facts->flags |= kImplicit;
	return call;
}

void Decomposer::bind_tuple()
{
	// 11.5p3: each name is a variable r_i of type "reference to T_i" (an
	// lvalue reference when its initializer is an lvalue, else an rvalue
	// one) bound to get<i>(e), and names an lvalue of type T_i designating
	// the object bound to it; decltype gives T_i. The variable has the
	// name (as Itanium names it: _ZZ1fvE1x), e's storage duration, and is
	// initialized after e, in order. A template definition only types the
	// names (its instantiations make the calls).
	TypeTable& types = analyzer_.types_;
	for (uint32_t i = 0; i < decomposition_->count; ++i)
	{
		const Type* element = tuple_element(i);
		if (!analyzer_.semantic() || analyzer_.template_depth_ != 0)
		{
			declare_binding(i, element);
			continue;
		}
		syntax::Expr* call = tuple_get(i);
		const bool rvalue = call->facts->category != ValueCategory::LValue;
		const Type* type = element->is_reference()
			? types.reference(element->element, rvalue && element->kind == TypeKind::RValueReference)
			: types.reference(element, rvalue);
		syntax::Initializer* initializer = ImpliedNode<syntax::Initializer>(analyzer_.arena_, decl_.location);
		initializer->kind = syntax::InitializerKind::Assign;
		initializer->expression = call;
		const syntax::InitDeclarator& name = decl_.declarators[i + 1];
		DeclaratorInfo info = DeclaratorInfo();
		info.type = type;
		info.name = name.declarator->name;
		info.after = scope_;
		info.location = name.declarator->location;
		const Binding* declared = analyzer_.declare_variable(
			specifiers_, info, initializer, scope_, info.location, true);
		if (!static_cast<const VariableEntity*>(declared->entity)->structured_binding)
			analyzer_.error(info.location, "a structured binding redeclares a name of its scope");
		BindingEntity* binding = static_cast<BindingEntity*>(declared->entity);
		binding->decomposition = decomposition_;
		binding->index = i;
		binding->referenced = element;
		binding->initializer = initializer;
		name.declared = declared;
		decomposition_->bindings[i] = binding;
	}
}

} // namespace sema
