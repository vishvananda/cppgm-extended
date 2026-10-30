#include "sema/analyzer.h"

#include "lex/identifier_table.h"

// Copying and moving class objects (12.8; semantics mode): each class's
// copy and move constructors and assignment operators, the user-declared
// ones classified when the class completes and the implicit ones declared
// when first demanded (a class never copied, moved or assigned gets none),
// with the facts of the defaulted ones (deleted, trivial, non-throwing, and
// the function selected for each base and member) that overload resolution
// and lowering consume by identity.
namespace sema {

namespace {

// The class of an object of `type` or of an array (of arrays) of it.
ClassEntity* ObjectClass(const Type* type)
{
	while (type->kind == TypeKind::Array)
		type = type->element;
	type = type->unqualified;
	return type->kind == TypeKind::Class ? static_cast<ClassEntity*>(type->entity) : nullptr;
}

// The cv-qualifiers of an object of `type` (an array's are its elements').
uint8_t ObjectCv(const Type* type)
{
	while (type->kind == TypeKind::Array)
		type = type->element;
	return type->cv;
}

bool IsAssignment(CopyKind kind)
{
	return kind == CopyKind::CopyAssignment || kind == CopyKind::MoveAssignment;
}

bool IsMove(CopyKind kind)
{
	return kind == CopyKind::MoveConstructor || kind == CopyKind::MoveAssignment;
}

// A user-provided special member (8.4.2p4).
bool UserProvided(const FunctionEntity* function)
{
	return !function->implicit && !function->defaulted && !function->deleted;
}

// The call of a subobject's selected function does not throw: it is non-
// throwing (`nothrow`, its exception-specification's value) and takes no
// default arguments (a copy constructor's later parameters, 12.8p2; their
// evaluation, 8.3.6, is assumed to throw).
bool NothrowCopy(const FunctionEntity* function, bool nothrow)
{
	return nothrow && function->type->canonical->param_count == 1;
}

// The bit of a copy or move kind in ClassEntity::declared_copy_kinds.
uint8_t KindBit(CopyKind kind)
{
	return static_cast<uint8_t>(1u << static_cast<unsigned>(kind));
}

// A subobject's class whose implicit copy and move members are still to
// be declared, or null.
ClassEntity* PendingCopyMembers(ClassEntity* cls)
{
	return cls != nullptr && cls->complete && cls->copy_members == FactState::NotStarted ? cls : nullptr;
}

} // namespace

CopyKind Analyzer::copy_kind(const FunctionEntity* function, const ClassEntity* entity) const
{
	// 12.8p2-p3: a non-template constructor whose first parameter is a
	// (cv) X& or X&&, the others with default arguments; 12.8p17, p19: a
	// non-static non-template operator= of one parameter of type X, (cv)
	// X& or (cv) X&&. An inheriting constructor is neither (12.9p3).
	if (function->templated || function->inherited != nullptr || function->owner != entity->scope)
		return CopyKind::None;
	const Type* type = function->type->canonical;
	if (type->param_count == 0)
		return CopyKind::None;
	const Type* parameter = type->params[0];
	if (function->constructor)
	{
		if (!parameter->is_reference() || parameter->element->unqualified != entity->type)
			return CopyKind::None;
		for (uint32_t i = 1; i < type->param_count; ++i)
		{
			if (function->default_arguments == nullptr || function->default_arguments[i] == nullptr)
				return CopyKind::None;
		}
		return parameter->kind == TypeKind::RValueReference ? CopyKind::MoveConstructor : CopyKind::CopyConstructor;
	}
	if (function->name != assign_key_ || function->is_static || type->param_count != 1 || type->variadic)
		return CopyKind::None;
	if (parameter->kind == TypeKind::RValueReference)
		return parameter->element->unqualified == entity->type ? CopyKind::MoveAssignment : CopyKind::None;
	if (parameter->kind == TypeKind::LValueReference)
		parameter = parameter->element;
	return parameter->unqualified == entity->type ? CopyKind::CopyAssignment : CopyKind::None;
}

void Analyzer::classify_copy_members(ClassEntity* entity)
{
	// At completion: the user-declared copy and move members classified
	// (12.8p2-p3, p17, p19), which kinds there are recorded for the
	// implicit ones' declaration, and the facts of those defaulted on their
	// first declaration (p11-p12, p23, p25), which overload resolution
	// reads (a move defined as deleted is ignored).
	uint8_t declared = 0;
	const Identifier* keys[2] = { constructor_key_, assign_key_ };
	for (const Identifier* key : keys)
	{
		const NameEntry* entry = names_.find(entity->scope, key);
		for (const FunctionList* node = entry != nullptr ? entry->functions : nullptr; node != nullptr;
			node = node->next)
		{
			FunctionEntity* function = static_cast<FunctionEntity*>(node->binding->entity);
			const CopyKind kind = copy_kind(function, entity);
			if (kind == CopyKind::None || function->copy_kind != CopyKind::None)
				continue;
			function->copy_kind = kind;
			declared = static_cast<uint8_t>(declared | KindBit(kind));
			if (function->defaulted && !function->deleted)
				defaulted_copy_facts(function, entity, kind);
		}
	}
	entity->declared_copy_kinds = declared;
}

void Analyzer::implicit_copy_members(const ClassEntity* cls)
{
	// On first demand (each reader of a complete class's implicit copy and
	// move members, or lookup of its operator=, calls this first): the
	// classes of its subobjects first, deepest first from an explicit
	// stack, so that declaring a class's demands only declared ones (a
	// chain of member classes takes constant stack), then its own.
	ClassEntity* entity = const_cast<ClassEntity*>(cls);
	if (entity->copy_members != FactState::NotStarted || !entity->complete || !semantic())
		return;
	// The demand may come in the middle of another overload resolution (a
	// braced argument's constructors, 13.3.1.7): its scratch is kept aside.
	std::vector<FunctionEntity*> set;
	std::vector<Candidate> candidates;
	std::vector<Conversion> sequences;
	set.swap(overload_set_);
	candidates.swap(candidates_);
	sequences.swap(sequences_);
	const ClassEntity* saved_direct = direct_class_;
	direct_class_ = nullptr;
	const size_t base = copy_frames_.size();
	entity->copy_members = FactState::InProgress;
	copy_frames_.push_back(CopyFrame{ entity, entity->bases, entity->members });
	while (copy_frames_.size() > base)
	{
		CopyFrame& frame = copy_frames_.back();
		ClassEntity* next = nullptr;
		for (; next == nullptr && frame.base != nullptr; frame.base = frame.base->next)
			next = PendingCopyMembers(static_cast<ClassEntity*>(frame.base->base));
		for (; next == nullptr && frame.member != nullptr; frame.member = frame.member->next)
		{
			const Type* type = frame.member->variable->type;
			next = type->is_reference() ? nullptr : PendingCopyMembers(ObjectClass(type));
		}
		if (next != nullptr)
		{
			next->copy_members = FactState::InProgress;
			copy_frames_.push_back(CopyFrame{ next, next->bases, next->members });
			continue;
		}
		ClassEntity* done = frame.entity;
		copy_frames_.pop_back();
		declare_implicit_copy_members(done);
		done->copy_members = FactState::Done;
	}
	direct_class_ = saved_direct;
	overload_set_.swap(set);
	candidates_.swap(candidates);
	sequences_.swap(sequences);
}

void Analyzer::implicit_members_for(const Scope* scope, const Identifier* name)
{
	// Before a lookup of operator= in a class scope: a complete class's
	// includes the implicit ones (`B::operator=(x)`, 12.8p18, p20); of a
	// conversion function, a captureless closure's (5.1.2p6).
	if (scope == nullptr || scope->kind != ScopeKind::Class || scope->entity == nullptr)
		return;
	if (name == assign_key_)
		implicit_copy_members(static_cast<const ClassEntity*>(scope->entity));
	else if (name == conversion_key_)
		closure_conversion(static_cast<const ClassEntity*>(scope->entity));
}

void Analyzer::declare_implicit_copy_members(ClassEntity* entity)
{
	// 12.8p7 a copy constructor (deleted when a move constructor or move
	// assignment operator is user-declared), p9 a move constructor when no
	// copy or move member or destructor is user-declared, p18 a copy
	// assignment operator, p20 a move assignment operator under p9's
	// conditions; their facts follow p11-p12, p23, p25.
	const uint8_t declared = entity->declared_copy_kinds;
	const bool copy_constructor = (declared & KindBit(CopyKind::CopyConstructor)) != 0;
	const bool move_constructor = (declared & KindBit(CopyKind::MoveConstructor)) != 0;
	const bool copy_assignment = (declared & KindBit(CopyKind::CopyAssignment)) != 0;
	const bool move_assignment = (declared & KindBit(CopyKind::MoveAssignment)) != 0;
	const bool destructor = entity->destructor != nullptr && !entity->destructor->implicit;
	const bool moves = move_constructor || move_assignment;
	// A closure type has a deleted copy assignment operator (5.1.2p19),
	// declared: no move member is (the copy constructor copies).
	const bool closure = graph_.closure_call(entity) != nullptr;
	const bool none = !copy_constructor && !move_constructor && !copy_assignment && !move_assignment && !destructor &&
		!closure;
	if (!copy_constructor)
		entity->copy_constructor = implicit_copy_member(entity, CopyKind::CopyConstructor, moves);
	if (none)
		entity->move_constructor = implicit_copy_member(entity, CopyKind::MoveConstructor, false);
	if (!copy_assignment)
		entity->copy_assignment = implicit_copy_member(entity, CopyKind::CopyAssignment, moves || closure);
	if (none)
		entity->move_assignment = implicit_copy_member(entity, CopyKind::MoveAssignment, false);
	// The implicit assignment operators are members lookup finds
	// (`B::operator=(x)`, 12.8p18, p20), public and never printed.
	FunctionEntity* const assignments[] = { copy_assignment ? nullptr : entity->copy_assignment,
		none ? entity->move_assignment : nullptr };
	for (FunctionEntity* function : assignments)
	{
		if (function == nullptr)
			continue;
		Binding* binding = new_binding(BindingKind::Function, assign_key_, function, function->signature);
		binding->hidden = true;
		binding->declared_in = entity->scope;
		binding->access = Access::Public;
		names_.declare(entity->scope, binding);
	}
}

FunctionEntity* Analyzer::implicit_copy_member(ClassEntity* entity, CopyKind kind, bool deleted)
{
	// X(const X&) or X(X&) (12.8p8), X(X&&), X& operator=(const X&) or
	// X& operator=(X&) (p18), X& operator=(X&&); public, inline. A move
	// member defined as deleted is not declared (ignored by overload
	// resolution, p11, p23).
	const bool assignment = IsAssignment(kind);
	uint8_t cv = 0;
	if (!IsMove(kind) && const_copy_source(entity, assignment))
		cv = kConst;
	const Type* parameter = types_.reference(types_.qualified(entity->type, cv), IsMove(kind));
	const Type* result = assignment ? types_.reference(entity->type, false) : types_.fundamental(FundamentalType::Void);
	const Type* type = types_.function(result, &parameter, 1, false, 0, RefQualifier::None);
	FunctionEntity* function = new_entity<FunctionEntity>(
		EntityKind::Function, assignment ? assign_key_ : constructor_key_, entity->scope, type);
	function->label = assignment         ? "operator="
		: entity->name != nullptr        ? entity->name->spelling
		: entity->local_label != nullptr ? entity->local_label
										 : entity->label;
	function->signature = signature_of(type);
	function->member = true;
	function->constructor = !assignment;
	function->is_operator = assignment;
	if (assignment)
		function->operator_token = TokenKind::OP_ASS;
	function->implicit = true;
	function->defined = true;
	function->is_inline = true;
	function->inline_member = true;
	function->copy_kind = kind;
	function->deleted = deleted;
	if (!deleted)
		defaulted_copy_facts(function, entity, kind);
	if (function->deleted && IsMove(kind))
		return nullptr;
	return function;
}

bool Analyzer::const_copy_source(const ClassEntity* entity, bool assignment)
{
	// 12.8p8, p18: the implicit member takes a const X& when each direct
	// base and each member of class type (or array of it) has a copy
	// constructor (assignment operator) whose parameter is a const (or
	// const volatile) reference (or, for assignment, the class by value).
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		if (!accepts_const_source(static_cast<const ClassEntity*>(base->base), assignment))
			return false;
	}
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		const Type* type = member->variable->type;
		const ClassEntity* cls = type->is_reference() ? nullptr : ObjectClass(type);
		if (cls != nullptr && !accepts_const_source(cls, assignment))
			return false;
	}
	return true;
}

bool Analyzer::accepts_const_source(const ClassEntity* entity, bool assignment)
{
	implicit_copy_members(entity);
	const FunctionEntity* implicit = assignment ? entity->copy_assignment : entity->copy_constructor;
	if (implicit != nullptr)
		return (implicit->type->canonical->params[0]->element->cv & kConst) != 0;
	const CopyKind wanted = assignment ? CopyKind::CopyAssignment : CopyKind::CopyConstructor;
	const NameEntry* entry = entity->scope != nullptr
		? names_.find(entity->scope, assignment ? assign_key_ : constructor_key_)
		: nullptr;
	for (const FunctionList* node = entry != nullptr ? entry->functions : nullptr; node != nullptr; node = node->next)
	{
		const FunctionEntity* function = static_cast<const FunctionEntity*>(node->binding->entity);
		if (function->copy_kind != wanted)
			continue;
		const Type* parameter = function->type->canonical->params[0];
		if (!parameter->is_reference() || (parameter->element->cv & kConst) != 0)
			return true;
	}
	return false;
}

void Analyzer::defaulted_copy_facts(FunctionEntity* function, ClassEntity* entity, CopyKind kind)
{
	// 12.8p11, p23: deleted when a subobject cannot be copied or moved (its
	// selected function ambiguous, deleted or inaccessible), a constructor's
	// subobject has a deleted or inaccessible destructor, a copy
	// constructor's member is an rvalue reference, an assignment's member
	// is a reference or a const non-class object, or a variant member's
	// corresponding function is not trivial. 12.8p12, p25: trivial when
	// not user-provided, nothing is virtual and every selected function is
	// trivial. 15.4p14: non-throwing when every selected function is.
	const bool assignment = IsAssignment(kind);
	const uint8_t source_cv = static_cast<uint8_t>(function->type->canonical->params[0]->element->cv);
	// A constructor copies the indirect virtual bases too (12.8p15: the
	// most derived class's); an assignment assigns them through its bases
	// (12.8p28 leaves repeated assignment unspecified).
	const uint32_t count = entity->base_count + entity->member_count + (assignment ? 0 : IndirectVirtualBases(entity));
	SubobjectInit* inits = static_cast<SubobjectInit*>(
		arena_.allocate(sizeof(SubobjectInit) * (count == 0 ? 1 : count), alignof(SubobjectInit)));
	bool deleted = false, trivial = !entity->dynamic && !UserProvided(function), nothrow = true;
	const bool union_class = entity->key == TokenKind::KW_UNION;
	uint32_t index = 0;
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		SubobjectInit& init = inits[index++];
		init = SubobjectInit{ base, nullptr, nullptr, nullptr, nullptr, false, false, 0, nullptr, nullptr };
		ClassEntity* cls = static_cast<ClassEntity*>(base->base);
		trivial = trivial && !base->is_virtual;
		init.constructor = subobject_copy(cls, kind, source_cv, 0, entity, deleted);
		copy_parameter(init, cls, kind, source_cv, entity, deleted);
		if (!assignment && cls->destructor != nullptr &&
			!subobject_destructor_usable(cls->destructor, cls, entity, false))
			deleted = true;
	}
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		SubobjectInit& init = inits[index++];
		init = SubobjectInit{ nullptr, member, nullptr, nullptr, nullptr, false, false, 0, nullptr, nullptr };
		const Type* type = member->variable->type;
		if (type->is_reference())
		{
			deleted = deleted || assignment ||
				(kind == CopyKind::CopyConstructor && type->kind == TypeKind::RValueReference);
			continue;
		}
		ClassEntity* cls = ObjectClass(type);
		if (cls == nullptr)
		{
			if (assignment && (ObjectCv(type) & kConst) != 0)
				deleted = true;
			continue;
		}
		// A mutable member of a const source is not const (7.1.1p10).
		const bool is_mutable = member->variable->kind == EntityKind::Variable &&
			static_cast<const VariableEntity*>(member->variable)->is_mutable;
		const uint8_t cv = static_cast<uint8_t>(ObjectCv(type) | (is_mutable ? 0 : source_cv));
		FunctionEntity* selected = subobject_copy(
			cls, kind, cv, ObjectCv(type), entity, deleted, ConstructionKind::Direct, true);
		if (assignment && (ObjectCv(type) & kConst) != 0)
			deleted = true;
		// A union's variant members are copied as its storage (12.8p16):
		// one whose corresponding function is not trivial deletes it (an
		// anonymous union member's own implicit function says so).
		if (union_class && selected != nullptr && !selected->trivial)
			deleted = true;
		// An anonymous union's (or struct's) member object: its members
		// are the class's variant members (9.5p5), whose destruction is not
		// the object's. A named member of an unnamed class is no such
		// object.
		const bool anonymous = cls->storage != nullptr && cls->storage->entity == member->variable;
		if (!assignment && cls->destructor != nullptr && !anonymous &&
			!subobject_destructor_usable(cls->destructor, cls, entity, true))
			deleted = true;
		init.constructor = selected;
		copy_parameter(init, cls, kind, cv, entity, deleted);
		if (selected != nullptr)
		{
			trivial = trivial && selected->trivial;
			nothrow = nothrow && NothrowCopy(selected, function_nothrow(selected));
		}
	}
	for (uint32_t i = 0; !assignment && i < VirtualBaseCount(entity); ++i)
	{
		if (VirtualBases(entity)[i].direct)
			continue;
		const BaseClass* base = VirtualBases(entity)[i].base;
		SubobjectInit& init = inits[index++];
		init = SubobjectInit{ base, nullptr, nullptr, nullptr, nullptr, false, false, 0, nullptr, nullptr };
		ClassEntity* cls = static_cast<ClassEntity*>(base->base);
		trivial = false;
		init.constructor = subobject_copy(cls, kind, source_cv, 0, entity, deleted);
		if (cls->destructor != nullptr && !subobject_destructor_usable(cls->destructor, cls, entity, false))
			deleted = true;
	}
	for (uint32_t i = 0; i < count; ++i)
	{
		if ((i < entity->base_count || i >= entity->base_count + entity->member_count) &&
			inits[i].constructor != nullptr)
		{
			trivial = trivial && inits[i].constructor->trivial;
			nothrow = nothrow && NothrowCopy(inits[i].constructor, function_nothrow(inits[i].constructor));
		}
		if (inits[i].parameter_copy != nullptr)
			nothrow = nothrow && NothrowCopy(inits[i].parameter_copy, function_nothrow(inits[i].parameter_copy));
	}
	function->inits = inits;
	function->init_count = count;
	function->trivial = trivial;
	// 8.4.2p2: the implicit exception-specification, unless one is
	// written or the function is defaulted after its first declaration.
	if (!function->exception_spec && (function->implicit || function->defaulted))
		function->nothrow = nothrow;
	function->deleted = function->deleted || deleted;
}

void Analyzer::copy_parameter(
	SubobjectInit& init, ClassEntity* cls, CopyKind kind, uint8_t cv, const ClassEntity* from, bool& deleted)
{
	// 12.8p28: each subobject assigned as by `x.m = y.m`; a selected
	// operator= taking the class by value has its parameter copy-
	// initialized from the source's subobject (8.5p15, 5.2.2p4) by the
	// constructor an lvalue (copy) or xvalue (move) of it selects.
	const FunctionEntity* selected = init.constructor;
	if (!IsAssignment(kind) || selected == nullptr)
		return;
	const Type* parameter = selected->type->canonical->params[0];
	if (parameter->is_reference() || parameter->unqualified != cls->type)
		return;
	const CopyKind construction = IsMove(kind) ? CopyKind::MoveConstructor : CopyKind::CopyConstructor;
	init.parameter_copy = subobject_copy(cls, construction, cv, 0, from, deleted, ConstructionKind::Copy);
}

FunctionEntity* Analyzer::subobject_copy(ClassEntity* cls, CopyKind kind, uint8_t cv, uint8_t object_cv,
	const ClassEntity* from, bool& deleted, ConstructionKind construction, bool member)
{
	// Overload resolution (13.3.1.3 direct-initialization, or
	// `construction`; 13.3.1.2 for assignment of an object of `object_cv`)
	// of the subobject's constructors or operator= with an lvalue (copy) or
	// xvalue (move) of it of `cv`; null (and `deleted`) when none is
	// selected or the selection is deleted or inaccessible from `from`
	// (one protected, accessible as a member of a derived class P, acts on
	// an object of P, 11.4: a base subobject of `from`'s object, not a
	// `member` of the class's own type).
	syntax::Expr source = syntax::Expr();
	source.kind = syntax::ExprKind::Id;
	ExprFacts facts = ExprFacts();
	facts.type = types_.qualified(cls->type, cv);
	facts.category = IsMove(kind) ? ValueCategory::XValue : ValueCategory::LValue;
	source.facts = &facts;
	syntax::Expr* pointer = &source;
	syntax::Slice<syntax::Expr*> arguments;
	arguments.data = &pointer;
	arguments.size = 1;
	std::vector<FunctionEntity*>& set = overload_set_;
	set.clear();
	ExprFacts object = ExprFacts();
	const bool assignment = IsAssignment(kind);
	if (assignment)
	{
		object.type = types_.qualified(cls->type, object_cv);
		object.category = ValueCategory::LValue;
		assignment_candidates(cls, set);
	}
	else
		constructor_candidates(cls, arguments, construction, set);
	Candidate chosen = Candidate();
	if (set.empty() ||
		resolve(arguments, set, assignment ? &object : nullptr, chosen, assignment ? nullptr : cls) !=
			Resolution::Selected ||
		chosen.function->deleted)
	{
		deleted = true;
		return nullptr;
	}
	FunctionEntity* selected = chosen.function;
	if (!selected->implicit && cls->scope != nullptr)
	{
		const NameEntry* entry = names_.find(cls->scope, assignment ? assign_key_ : constructor_key_);
		for (const FunctionList* node = entry != nullptr ? entry->functions : nullptr; node != nullptr;
			node = node->next)
		{
			if (node->binding->entity != selected)
				continue;
			const ClassEntity* via = nullptr;
			const ClassEntity* acted = member ? cls : from;
			if (!accessible(cls, cls, node->binding->access, from->scope, &via, 0) ||
				(via != nullptr && via != acted && !conversions_.is_base_of(via, acted)))
				deleted = true;
		}
	}
	return selected;
}

void Analyzer::constructor_candidates(ClassEntity* entity, const syntax::Slice<syntax::Expr*>& arguments,
	ConstructionKind kind, std::vector<FunctionEntity*>& set)
{
	// 13.3.1.3, 13.3.1.4, 13.3.1.7: the class's constructors (copy-
	// initialization only its converting ones), each once, with the
	// implicitly declared default, copy and move constructors (12.1p5,
	// 12.8p7, p9) the arguments may call. 13.3.3.1p4: copy-
	// initialization's converting constructors (and a list's single
	// element that is itself a braced list, CWG 2076) take a copy or move
	// constructor's argument by standard conversions only: an expression of
	// the class or a class derived from it.
	const bool copies = arguments.size == 1 && kind != ConstructionKind::Converting &&
		(kind == ConstructionKind::Direct ||
			(kind == ConstructionKind::CopyList && arguments[0]->kind != syntax::ExprKind::BracedInit) ||
			same_class_source(entity, *arguments[0]));
	if (copies)
		implicit_copy_members(entity);
	const uint32_t mark = ++candidate_epoch_;
	const bool converting = kind == ConstructionKind::Copy || kind == ConstructionKind::Converting;
	converting_only_ = false;
	if (const NameEntry* entry = names_.find(entity->scope, constructor_key_))
	{
		for (const FunctionList* node = entry->functions; node != nullptr; node = node->next)
		{
			FunctionEntity* function = static_cast<FunctionEntity*>(node->binding->entity);
			if (function->candidate_mark == mark || (converting && function->is_explicit))
				continue;
			function->candidate_mark = mark;
			set.push_back(function);
			// A template's explicit(condition) is each specialization's
			// (12.3.1p2 with C++20's [dcl.fct.spec]p3): the resolution
			// that deduces it decides (a set with a template is resolved).
			converting_only_ = converting_only_ || (converting && function->templated);
		}
	}
	if (!entity->user_declared_constructor && arguments.empty())
	{
		if (FunctionEntity* implicit = implicit_constructor(entity))
			set.push_back(implicit);
	}
	if (copies)
	{
		if (entity->copy_constructor != nullptr)
			set.push_back(entity->copy_constructor);
		if (entity->move_constructor != nullptr)
			set.push_back(entity->move_constructor);
	}
}

void Analyzer::assignment_candidates(ClassEntity* entity, std::vector<FunctionEntity*>& set)
{
	// 13.5.3, 12.8p24: the class's own operator= members (an implicitly
	// declared one hides a base's), with its implicit copy and move
	// assignment operators.
	implicit_copy_members(entity);
	const uint32_t mark = ++candidate_epoch_;
	if (const NameEntry* entry = entity->scope != nullptr ? names_.find(entity->scope, assign_key_) : nullptr)
	{
		for (const FunctionList* node = entry->functions; node != nullptr; node = node->next)
		{
			FunctionEntity* function = static_cast<FunctionEntity*>(node->binding->entity);
			if (function->candidate_mark == mark || !function->member)
				continue;
			function->candidate_mark = mark;
			set.push_back(function);
		}
	}
	if (entity->copy_assignment != nullptr && entity->copy_assignment->candidate_mark != mark)
		set.push_back(entity->copy_assignment);
	if (entity->move_assignment != nullptr && entity->move_assignment->candidate_mark != mark)
		set.push_back(entity->move_assignment);
}

} // namespace sema
