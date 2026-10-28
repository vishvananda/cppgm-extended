#include "sema/analyzer.h"

#include "lex/identifier_table.h"

// Member access control (clause 11; semantics mode): each class member
// declaration's access and each base-specifier's are recorded as the class
// is defined; a name, constructor, destructor or derived-to-base conversion
// used at a point is accessible there by 11.2p4-p5 (the base path from the
// naming class, and whether the point is in a member or friend of a class
// on it) and 11.4 (a protected member through an object).
namespace sema {

namespace {

Access Stricter(Access a, Access b)
{
	return static_cast<uint8_t>(a) > static_cast<uint8_t>(b) ? a : b;
}

bool NonStaticMember(const Entity* entity)
{
	if (entity->kind == EntityKind::Variable)
		return static_cast<const VariableEntity*>(entity)->member;
	if (entity->kind == EntityKind::Function)
	{
		const FunctionEntity* function = static_cast<const FunctionEntity*>(entity);
		return function->member && !function->is_static;
	}
	return false;
}

// The classes whose friend declarations name the class or function a
// scope is of (null lists for another scope): those naming it, and those
// naming its class or function template, a friend template declaration
// befriending every specialization (14.5.4p1). The class, for a class
// scope.
const ClassEntity* Befriending(const Scope* s, const ClassList** lists)
{
	lists[0] = nullptr;
	lists[1] = nullptr;
	if (s->kind == ScopeKind::Class && s->entity != nullptr)
	{
		const ClassEntity* cls = static_cast<const ClassEntity*>(s->entity);
		lists[0] = cls->befriending;
		if (cls->templ != nullptr && cls->templ->role == TemplateRole::Specialization)
			lists[1] = cls->templ->pattern->befriending;
		return cls;
	}
	if (s->kind == ScopeKind::Function && s->entity != nullptr && s->entity->kind == EntityKind::Function)
	{
		const FunctionEntity* function = static_cast<const FunctionEntity*>(s->entity);
		lists[0] = function->befriending;
		if (function->primary != nullptr)
			lists[1] = function->primary->befriending;
	}
	return nullptr;
}

// access_walk's and leads_to_base's value for a class whose frame is on
// the stack (beside an access or kNoAccess).
const uint32_t kPendingAccess = 0xfe;

// Whether a member of `end` with access `declared` there is a member of
// `top`, whose chain of single bases (of access `path`) reaches `end`: a
// private member is a member of no derived class (11.2p1); its access as
// one.
bool ChainMember(const ClassEntity* top, const ClassEntity* end, Access path, Access declared, Access& out)
{
	if (end != top && declared == Access::Private)
		return false;
	out = Stricter(path, declared);
	return true;
}

} // namespace

Access Analyzer::member_access() const
{
	return current_access_ == TokenKind::KW_PRIVATE  ? Access::Private
		: current_access_ == TokenKind::KW_PROTECTED ? Access::Protected
													 : Access::Public;
}

void Analyzer::declare_in(Scope* scope, Binding* binding)
{
	// A member declaration in its class's member-specification takes the
	// access in effect; a redeclaration outside it keeps the member's.
	binding->declared_in = scope;
	if (scope->kind == ScopeKind::Class)
		binding->access = scope == member_scope_ ? member_access() : redeclared_access(scope, binding);
	// A block-scope declaration is taken as dependent unless its
	// declaration decides otherwise (see Binding::dependent).
	if (scope->kind == ScopeKind::Block)
		binding->dependent = true;
	names_.declare(scope, binding);
}

Access Analyzer::redeclared_access(const Scope* scope, const Binding* binding) const
{
	if (binding->entity == nullptr)
		return Access::Public;
	if (binding->kind == BindingKind::Function)
	{
		const FunctionEntity* function = static_cast<const FunctionEntity*>(binding->entity);
		const FunctionList* same = function->templated ? nullptr
													   : names_.overload(scope, binding->name, function->signature);
		return same != nullptr && same->binding->entity == function ? same->binding->access : Access::Public;
	}
	const NameEntry* entry = names_.find(scope, binding->name);
	if (entry != nullptr && entry->value != nullptr && entry->value->entity == binding->entity)
		return entry->value->access;
	return Access::Public;
}

Scope* Analyzer::member_declaration_class(const syntax::Declarator* declarator, Scope* scope)
{
	// The class a qualified declarator-id names a member of (11p6: the
	// whole declaration has the member's access), else null.
	const syntax::Declarator* named = declarator;
	while (named != nullptr && named->name == nullptr)
		named = named->nested;
	if (named == nullptr || !named->name->qualified() || !semantic() || template_depth_ != 0)
		return nullptr;
	const NamePrefix saved = last_prefix_;
	const NamePrefix prefix = name_prefix(*named->name, scope, true);
	last_prefix_ = saved;
	return prefix.scope != nullptr && prefix.scope->kind == ScopeKind::Class ? prefix.scope : nullptr;
}

size_t EntityPairHash::operator()(const EntityPair& key) const
{
	return (reinterpret_cast<uintptr_t>(key.first) * 0x9E3779B97F4A7C15ull) ^ reinterpret_cast<uintptr_t>(key.second);
}

void Analyzer::befriend_class(ClassEntity* friend_class, Scope* scope)
{
	// 11.3p2: `friend class X;` in the class being defined.
	Scope* where = declaration_scope(scope);
	if (where->kind != ScopeKind::Class || where->entity == nullptr || friend_class == nullptr)
		return;
	ClassEntity* befriending = static_cast<ClassEntity*>(where->entity);
	record_friend(befriending, friend_class);
	ClassList* reverse = static_cast<ClassList*>(arena_.allocate(sizeof(ClassList), alignof(ClassList)));
	*reverse = ClassList{ befriending, friend_class->befriending };
	friend_class->befriending = reverse;
}

void Analyzer::befriend_function(FunctionEntity* function, ClassEntity* befriending)
{
	// A friend declaration naming a function, a function template's
	// specialization, or the template itself (14.5.4p1): a friend of the
	// class for access, and the function's befriending class, through
	// which argument-dependent lookup finds a friend no ordinary lookup
	// does (3.4.2p4).
	record_friend(befriending, function);
	ClassList* edge = static_cast<ClassList*>(arena_.allocate(sizeof(ClassList), alignof(ClassList)));
	*edge = ClassList{ befriending, function->befriending };
	function->befriending = edge;
}

void Analyzer::record_friend(const ClassEntity* befriending, const Entity* befriended)
{
	const EntityPair key = EntityPair{ befriending, befriended };
	if (friendships_.find(key) == nullptr)
		friendships_.insert(key, true);
}

bool Analyzer::befriends(const ClassEntity* befriending, const Entity* befriended)
{
	return friendships_.find(EntityPair{ befriending, befriended }) != nullptr;
}

const Scope* Analyzer::access_point(const Scope* scope) const
{
	// The point a placeholder substitution checks its copy's own names at
	// (the scope of the copy or one in it; see substituted_value), not
	// the scope of the placeholder's first declaration: facts a class
	// caches (whether its implicit copy constructor is deleted, 12.8p11)
	// are checked from the class itself.
	for (const Scope* s = access_point_ != nullptr ? scope : nullptr; s != nullptr; s = s->parent)
	{
		if (s == access_scope_)
			return access_point_;
	}
	return scope;
}

bool Analyzer::member_or_friend_of(const ClassEntity* entity, const Scope* scope)
{
	// 11.2p5: the point is in a member of the class (a member function, a
	// nested class's member, 11.7) or a friend of it (a function it
	// befriends, a member of a class it befriends). A member's declaration
	// outside its class is in the member (11p6), its leading type included.
	if (declaration_class_ != nullptr && scope != nullptr && scope->kind == ScopeKind::Namespace &&
		member_or_friend_of(entity, declaration_class_))
		return true;
	for (const Scope* s = scope; s != nullptr; s = s->parent)
	{
		if (s->kind == ScopeKind::Class && s->entity != nullptr)
		{
			// A friend class template declaration befriends every
			// specialization (14.5.4p1).
			const ClassEntity* cls = static_cast<const ClassEntity*>(s->entity);
			if (cls == entity || befriends(entity, cls) ||
				(cls->templ != nullptr && cls->templ->role == TemplateRole::Specialization &&
					befriends(entity, cls->templ->pattern)))
				return true;
		}
		else if (s->kind == ScopeKind::Function && s->entity != nullptr && s->entity->kind == EntityKind::Function)
		{
			// A friend function template declaration befriends every
			// specialization of the template (14.5.4p1), one naming a member
			// of a class template the member of every specialization (p5).
			const FunctionEntity* function = static_cast<const FunctionEntity*>(s->entity);
			if (befriends(entity, function) || (function->primary != nullptr && befriends(entity, function->primary)) ||
				(PatternOf(function) != nullptr && befriends(entity, PatternOf(function))))
				return true;
		}
	}
	return false;
}

bool Analyzer::access_as_member(const ClassEntity* naming, const ClassEntity* declaring, Access declared, Access& out)
{
	// 11.2p1: a member of a base is a member of the derived class with the
	// stricter of its access and the base-specifier's; a private member of
	// a base is not accessible as a member of the derived class. Down a
	// chain of single bases that is one walk: the stricter of the declared
	// access and every base-specifier, none when the member or a
	// base-specifier below the naming class's own is private. At a class
	// with several bases the path is the one giving most access (11.7p1),
	// each base below decided once per query (access_paths_: a lattice of
	// repeated bases costs its edges, not its paths).
	access_paths_.clear();
	return access_walk(naming, declaring, declared, out, access_paths_);
}

bool Analyzer::access_walk(
	const ClassEntity* naming, const ClassEntity* declaring, Access declared, Access& out, PointerMap& paths)
{
	// The member's access as a member of `naming` (false: none). A class
	// whose chain of single bases ends at a class of several is a frame of
	// base_frames_ (a stack: a hierarchy of several bases per class may be
	// deeper than the native one) until each base below that is not the
	// declaring class and has bases is decided (`paths`, kNoAccess for
	// none); the most access through them is the class's.
	if (declared == Access::Private && naming != declaring)
		return false;
	const size_t first = base_frames_.size();
	uint32_t access = access_chain(naming, declaring, declared);
	// The access through the top frame's next base once decided (by the
	// frame of that base, just done).
	uint32_t known = kPendingAccess;
	while (base_frames_.size() > first)
	{
		BaseWalkFrame& frame = base_frames_.back();
		const BaseClass* base = frame.next;
		for (; base != nullptr; base = base->next)
		{
			if (known == kPendingAccess)
			{
				known = static_cast<uint32_t>(declared);
				const ClassEntity* inner = static_cast<const ClassEntity*>(base->base);
				if (inner != declaring)
					known = inner->bases == nullptr || access_blocked(inner, declaring) ? kNoAccess : paths.find(inner);
				if (known == PointerMap::kMissing)
				{
					// Decided now, or first by the frame pushed for it.
					frame.next = base;
					known = access_chain(inner, declaring, declared);
					if (known == kPendingAccess)
						break;
					paths.set(inner, known);
				}
			}
			if (known != kNoAccess && known != static_cast<uint32_t>(Access::Private))
			{
				const Access candidate = Stricter(static_cast<Access>(known), base->access);
				if (!frame.found || static_cast<uint8_t>(candidate) < static_cast<uint8_t>(frame.best))
					frame.best = candidate;
				frame.found = true;
			}
			known = kPendingAccess;
		}
		if (base != nullptr)
			continue;
		// The class's access: the result, or kept for the frame below,
		// whose next base it is.
		const BaseWalkFrame done = frame;
		base_frames_.pop_back();
		Access through = Access::Public;
		access = done.found && ChainMember(done.top, done.bottom, done.path, done.best, through)
			? static_cast<uint32_t>(through)
			: kNoAccess;
		if (base_frames_.size() > first)
		{
			paths.set(done.top, access);
			known = access;
		}
	}
	if (access == kNoAccess)
		return false;
	out = static_cast<Access>(access);
	return true;
}

uint32_t Analyzer::access_chain(const ClassEntity* top, const ClassEntity* declaring, Access declared)
{
	// access_walk down the chain of single bases from `top`: the access
	// (kNoAccess for none), or kPendingAccess with a frame pushed for the
	// class of several bases it ends at. There a direct base reached once
	// (a simple hierarchy) is the only path, and a public base-specifier
	// naming the declaring class gives the most access any path can
	// (11.6p1); a base without bases, not the declaring class, is none.
	Access path = Access::Public;
	const ClassEntity* cls = top;
	Access access = Access::Public;
	while (cls != declaring)
	{
		++graph_.base_steps;
		const BaseClass* base = cls->bases;
		if (base == nullptr)
			return kNoAccess;
		if (base->next != nullptr)
		{
			const BaseClass* direct = base_specifier(cls, declaring);
			if (direct == nullptr ||
				(direct->access != Access::Public && (direct->is_virtual || !simple_hierarchy(cls))))
			{
				base_frames_.push_back(BaseWalkFrame{ top, cls, base, path, Access::Private, false });
				return kPendingAccess;
			}
			if (declared == Access::Private)
				return kNoAccess;
			return ChainMember(top, cls, path, Stricter(declared, direct->access), access)
				? static_cast<uint32_t>(access)
				: kNoAccess;
		}
		if (cls != top && base->access == Access::Private)
			return kNoAccess;
		path = Stricter(path, base->access);
		cls = static_cast<const ClassEntity*>(base->base);
		if (access_blocked(cls, declaring))
			return kNoAccess;
	}
	return ChainMember(top, cls, path, declared, access) ? static_cast<uint32_t>(access) : kNoAccess;
}

const ClassEntity* Analyzer::derived_context(const ClassEntity* naming, const ClassEntity* declaring, Access declared,
	const Scope* scope, const ClassEntity* object, const ClassEntity** via, bool public_path, bool& object_context)
{
	// 11.2p5: a class P derived from the naming class whose member or
	// friend the point is, the member being a member of P (the query's
	// member: its bases' accesses in member_paths_).
	object_context = object == nullptr ||
		(declaration_class_ != nullptr && scope != nullptr && scope->kind == ScopeKind::Namespace);
	// A public chain allows a privilege in any class below to be used at
	// once. A restricted chain still needs its individual base accesses.
	public_path = public_path && object != nullptr;
	auto permits = [&](const ClassEntity* candidate) {
		if (candidate == nullptr)
			return false;
		const bool derived = public_path || conversions_.is_base_of(naming, candidate);
		if (!derived && object == nullptr)
			return false;
		Access access = Access::Public;
		if (!access_walk(candidate, declaring, declared, access, member_paths_) ||
			!access_object(candidate, object, derived ? via : nullptr))
			return false;
		object_context = true;
		return derived;
	};
	for (const Scope* s = scope; s != nullptr; s = s->parent)
	{
		const ClassList* candidates[2];
		const ClassEntity* direct = Befriending(s, candidates);
		if (permits(direct))
			return direct;
		for (const ClassList* list : candidates)
		{
			for (const ClassList* node = list; node != nullptr; node = node->next)
			{
				if (permits(node->entity))
					return node->entity;
			}
		}
		// A member instance's pattern has a separate friendship path in
		// member_or_friend_of; retain the full walk when it could apply.
		if (object != nullptr && s->kind == ScopeKind::Function && s->entity != nullptr &&
			s->entity->kind == EntityKind::Function)
		{
			const FunctionEntity* pattern = PatternOf(static_cast<const FunctionEntity*>(s->entity));
			if (pattern != nullptr && pattern->befriending != nullptr)
				object_context = true;
		}
	}
	return nullptr;
}

bool Analyzer::access_object(const ClassEntity* privilege, const ClassEntity* object, const ClassEntity** via)
{
	// 11.4: try another privilege or base path when this derived-class
	// privilege cannot access a non-static member through the object.
	if (object == nullptr || object == privilege || conversions_.is_base_of(privilege, object))
		return true;
	if (via != nullptr)
		*via = privilege;
	return false;
}

bool Analyzer::base_accessible(const ClassEntity* derived, const BaseClass* base, const Scope* scope,
	const ClassEntity* object, const ClassEntity** via)
{
	// 11.2p4: an invented public member of the base would be a public
	// member of the derived class N, or the point is in a member or friend
	// of N; or, the base being protected, in a member or friend of a class
	// P derived from N (the invented member a member of P).
	if (base->access == Access::Public || member_or_friend_of(derived, scope))
		return true;
	if (base->access != Access::Protected)
		return false;
	Access access = Access::Public;
	for (const Scope* s = scope; s != nullptr; s = s->parent)
	{
		const ClassList* befriending[2];
		const ClassEntity* cls = Befriending(s, befriending);
		if (cls != nullptr && access_as_member(cls, derived, Access::Protected, access) &&
			access_object(cls, object, via))
			return true;
		for (const ClassList* list : befriending)
		{
			for (const ClassList* node = list; node != nullptr; node = node->next)
			{
				if (access_as_member(node->entity, derived, Access::Protected, access) &&
					access_object(node->entity, object, via))
					return true;
			}
		}
	}
	return false;
}

bool Analyzer::accessible(const ClassEntity* naming, const ClassEntity* declaring, Access declared, const Scope* scope,
	const ClassEntity** via, uint32_t location, const ClassEntity* object)
{
	// One query: its member's accesses as a member of the classes below
	// and the classes bullet 4 found no access through are kept until the
	// next (a lattice of repeated bases costs its edges, not its paths).
	access_location_ = location;
	member_paths_.clear();
	access_unreached_.clear();
	base_leads_.clear();
	base_leads_target_ = declaring;
	return accessible_below(naming, declaring, declared, scope, via, object);
}

bool Analyzer::accessible_below(const ClassEntity* naming, const ClassEntity* declaring, Access declared,
	const Scope* scope, const ClassEntity** via, const ClassEntity* object)
{
	// 11.2p5 down the base path from the naming class: the member's access
	// as a member of each class on it (p1) decides bullets 1-3 there;
	// bullet 4 continues through the base toward the declaring class when
	// that base is accessible (p4). A chain of single bases is summarized
	// in one walk (its deepest private and non-public base-specifiers), so
	// each class's access follows without a walk below it; a class with
	// several bases tries each base that leads to the member.
	uint32_t edges = 0, deepest_private = 0, deepest_restricted = 0;
	const ClassEntity* bottom = naming;
	while (bottom != declaring && bottom->bases != nullptr && bottom->bases->next == nullptr)
	{
		++edges;
		if (bottom->bases->access == Access::Private)
			deepest_private = edges;
		if (bottom->bases->access != Access::Public)
			deepest_restricted = edges;
		bottom = static_cast<const ClassEntity*>(bottom->bases->base);
		if (access_blocked(bottom, declaring))
			return false;
	}
	Access bottom_access = declared;
	const bool bottom_member = bottom == declaring ||
		access_walk(bottom, declaring, declared, bottom_access, member_paths_);
	const ClassEntity* cls = naming;
	for (uint32_t level = 0;; ++level)
	{
		// The base-specifiers below this class are edges level + 1 on.
		bool member = bottom_member;
		Access access = bottom_access;
		if (level < edges)
		{
			member = member && bottom_access != Access::Private && deepest_private <= level + 1;
			access = Stricter(access,
				deepest_private > level          ? Access::Private
					: deepest_restricted > level ? Access::Protected
												 : Access::Public);
		}
		if (member && (access == Access::Public || member_or_friend_of(cls, scope)))
			return true;
		if (member && access == Access::Protected)
		{
			bool object_context = false;
			if (const ClassEntity* derived = derived_context(cls, declaring, declared, scope, object, via,
					bottom == declaring && deepest_restricted == 0, object_context))
			{
				if (via != nullptr)
					*via = derived;
				return true;
			}
			// Without any privilege compatible with this object, searching
			// more bases would only repeat the same unsuccessful scope walk.
			if (!object_context)
				return false;
		}
		if (level == edges)
			break;
		if (!base_accessible(cls, cls->bases, scope, object, via))
			return false;
		cls = static_cast<const ClassEntity*>(cls->bases->base);
	}
	if (bottom == declaring)
		return false;
	// A class below that gave no access (whatever the path to it) is not
	// tried again, nor one not derived from the declaring class
	// (leads_to_base, whose memo serves the query).
	for (const BaseClass* base = bottom->bases; base != nullptr; base = base->next)
	{
		const ClassEntity* inner = static_cast<const ClassEntity*>(base->base);
		if (inner != declaring &&
			(access_blocked(inner, declaring) || access_unreached_.find(inner) != PointerMap::kMissing ||
				!leads_to_base(inner, declaring)))
			continue;
		if (!base_accessible(bottom, base, scope, object, via))
			continue;
		check_stack(access_location_);
		if (accessible_below(inner, declaring, declared, scope, via, object))
			return true;
		if (inner != declaring)
			access_unreached_.set(inner, 1);
	}
	return false;
}

bool Analyzer::accessible_declarations(const Binding* binding, const ClassEntity* naming, const Scope* scope,
	const ClassEntity** via, uint32_t location, const ClassEntity* object)
{
	// A member of a name some using-declaration introduces a member of:
	// the lookup in the naming class finds, on each path down from it, the
	// first class declaring the entity (its own declaration or a
	// using-declaration, with its access; 10.2p3-p6, 7.3.3p1). Unless that
	// lookup (cached for a complete class) merged several, the binding is
	// the one. Found through several, the name has the access of the path
	// giving most (11.7p1): each declaration's paths are tried, those
	// through another's class ending there (access_blocked).
	const ClassEntity* home = static_cast<const ClassEntity*>(binding->declared_in->entity);
	if (naming->scope == nullptr)
		return accessible(naming, home, binding->access, scope, via, location, object);
	const LookupResult found = names_.qualified(naming->scope, binding->name, LookupFilter::Ordinary);
	if (found.found() && !found.redeclared &&
		(found.binding->kind == BindingKind::Function || found.binding->entity == binding->entity))
		return accessible(naming, home, binding->access, scope, via, location, object);
	// The classes declaring the entity: those using-declarations introduce
	// members into, and its own class (a member of an anonymous class may
	// be declared in the enclosing one too, 9.5p5: then every class is
	// searched). Each chain of single bases is walked once from its top,
	// which is remembered as each class of several bases is.
	const Scope* owner = binding->entity->owner;
	const bool every = owner == nullptr || owner->kind != ScopeKind::Class || owner->entity == nullptr ||
		static_cast<const ClassEntity*>(owner->entity)->anonymous;
	access_frontier_.clear();
	access_declarations_.clear();
	access_classes_.assign(1, naming);
	while (!access_classes_.empty())
	{
		const ClassEntity* top = access_classes_.back();
		access_classes_.pop_back();
		if (top->bases != nullptr)
		{
			if (access_frontier_.find(top) != PointerMap::kMissing)
				continue;
			access_frontier_.set(top, 0);
		}
		for (const ClassEntity* cls = top;;)
		{
			const Scope* own = cls->scope;
			const Binding* declaration = own != nullptr && (every || own == owner || own->using_members)
				? member_declaration(cls, binding)
				: nullptr;
			if (declaration != nullptr)
			{
				if (access_frontier_.find(cls) != 1)
				{
					access_frontier_.set(cls, 1);
					access_declarations_.push_back(std::make_pair(cls, declaration));
				}
				break;
			}
			const BaseClass* base = cls->bases;
			if (base == nullptr)
				break;
			if (base->next == nullptr)
			{
				cls = static_cast<const ClassEntity*>(base->base);
				continue;
			}
			if (cls != top)
			{
				if (access_frontier_.find(cls) != PointerMap::kMissing)
					break;
				access_frontier_.set(cls, 0);
			}
			for (; base != nullptr; base = base->next)
				access_classes_.push_back(static_cast<const ClassEntity*>(base->base));
			break;
		}
	}
	if (access_declarations_.size() < 2)
	{
		const bool one = !access_declarations_.empty();
		const ClassEntity* declaring = one ? access_declarations_[0].first : home;
		const Access declared = one ? access_declarations_[0].second->access : binding->access;
		return accessible(naming, declaring, declared, scope, via, location, object);
	}
	bool reached = false;
	for (size_t i = 0; i < access_declarations_.size() && !reached; ++i)
	{
		access_target_ = access_declarations_[i].first;
		reached = accessible(
			naming, access_target_, access_declarations_[i].second->access, scope, via, location, object);
	}
	access_target_ = nullptr;
	return reached;
}

const Binding* Analyzer::member_declaration(const ClassEntity* cls, const Binding* binding) const
{
	// The declaration of the binding's entity among the class's own
	// declarations of its name, or null.
	const NameEntry* entry = cls->scope != nullptr ? names_.find(cls->scope, binding->name) : nullptr;
	if (entry == nullptr)
		return nullptr;
	const Entity* entity = binding->entity;
	if (entry->value != nullptr && entry->value->entity == entity)
		return entry->value;
	if (entry->type != nullptr && entry->type->entity == entity)
		return entry->type;
	if (entry->tag != nullptr && entry->tag->entity == entity)
		return entry->tag;
	for (const FunctionList* node = entry->functions; node != nullptr; node = node->next)
	{
		if (node->binding->entity == entity)
			return node->binding;
	}
	return nullptr;
}

bool Analyzer::access_blocked(const ClassEntity* cls, const ClassEntity* declaring) const
{
	// While accessible_declarations tries the paths to one declaration, a
	// class of another ends the paths through it (the lookup stops there).
	return access_target_ == declaring && cls != declaring && access_frontier_.find(cls) == 1;
}

void Analyzer::check_access(
	const Binding* binding, const ClassEntity* naming, const ClassEntity* object, const Scope* scope, uint32_t location)
{
	// The member a binding declares, named in class `naming` at the point
	// `scope`; a non-static member through an object of class `object`
	// accessible only as a protected member of a derived class P needs an
	// object of P (11.4).
	if (binding == nullptr || naming == nullptr || scope == nullptr || binding->declared_in == nullptr ||
		binding->declared_in->kind != ScopeKind::Class || binding->declared_in->entity == nullptr)
		return;
	const ClassEntity* declaring = static_cast<const ClassEntity*>(binding->declared_in->entity);
	if (binding->access == Access::Public && naming == declaring)
		return;
	scope = access_point(scope);
	// Access checking is part of a substitution (14.8.2p8): there a
	// violation is its failure, recorded while the analysis goes on to the
	// boundary (never deferred to a declaration's end).
	const ClassEntity* via = nullptr;
	const ClassEntity* receiver = binding->entity != nullptr && NonStaticMember(binding->entity) ? object : nullptr;
	const bool reached = binding->entity != nullptr && binding->name != nullptr &&
			binding->name->id < using_names_.size() && using_names_[binding->name->id]
		? accessible_declarations(binding, naming, scope, &via, location, receiver)
		: accessible(naming, declaring, binding->access, scope, &via, location, receiver);
	if (!reached)
	{
		if (via == nullptr && soft_substitution_ == 0 && defer_access(binding, naming, object, scope, location))
			return;
		substitution_error(location,
			via != nullptr ? "protected member accessed through an object of another class" : "inaccessible member");
		return;
	}
}

void Analyzer::check_unqualified_access(const Binding* binding, const Scope* scope, uint32_t location)
{
	// A class member an unqualified name found (a type, the injected-class-
	// name of a base, 11.1p5, an enumerator or a variable) is named in the
	// innermost class enclosing the point that has it as a member (11.2p5).
	if (binding->declared_in == nullptr || binding->declared_in->kind != ScopeKind::Class ||
		binding->declared_in->entity == nullptr || scope == nullptr)
		return;
	const ClassEntity* home = static_cast<const ClassEntity*>(binding->declared_in->entity);
	check_access(binding, naming_class(home, scope), nullptr, scope, location);
}

void Analyzer::check_function_access(const FunctionEntity* function, const FunctionList* functions,
	const ClassEntity* naming, const ClassEntity* object, const Scope* scope, uint32_t location)
{
	// Access is checked for the function overload resolution selected
	// (13.3p? access applies after resolution), through the declaration
	// that brought it into the set: a member template's specialization's
	// through its template's (14.5.2p2, 11p4: access applies to member
	// templates as to other members).
	if (function == nullptr || !function->member)
		return;
	const FunctionEntity* declared = function->primary != nullptr ? function->primary : function;
	for (const FunctionList* node = functions; node != nullptr; node = node->next)
	{
		if (node->binding->entity == function || node->binding->entity == declared)
		{
			check_access(node->binding, naming, object, scope, location);
			return;
		}
	}
}

void Analyzer::check_special_access(const FunctionEntity* function, const ClassEntity* entity, const Scope* scope,
	uint32_t location, const ClassEntity* object)
{
	// A constructor or destructor used for an object of its class: its
	// declaration's access (an implicit one is public). A protected one
	// accessible only as a member of a derived class P acts on an object
	// that must be of P (11.4): the object itself, or for a base subobject
	// the object of the class whose constructor is planned (base_object_).
	if (function == nullptr || (function->implicit && function->inherited == nullptr) || entity == nullptr ||
		entity->scope == nullptr)
		return;
	const ClassEntity* acted = object != nullptr ? object : base_object_ != nullptr ? base_object_ : entity;
	const Identifier* key = function->destructor ? destructor_key_ : constructor_key_;
	// Its declaration through the overload index (a template's by the set).
	const FunctionList* same = function->templated ? nullptr : names_.overload(entity->scope, key, function->signature);
	if (same != nullptr && same->binding->entity == function)
	{
		check_access(same->binding, entity, acted, scope, location);
		return;
	}
	// A constructor template's specialization has its template's access,
	// and so has the inheriting constructor made for one (12.9p4: the
	// template is declared in the class with the base constructor's),
	// through a base's inheriting constructor made for it (the recorded
	// root).
	const FunctionEntity* inherited = graph_.inherited_root(function);
	const FunctionEntity* declared = function->primary != nullptr ? function->primary
		: inherited != nullptr && inherited->primary != nullptr   ? inherited->primary
																  : function;
	const NameEntry* entry = names_.find(entity->scope, key);
	for (const FunctionList* node = entry != nullptr ? entry->functions : nullptr; node != nullptr; node = node->next)
	{
		if (node->binding->entity == declared)
		{
			check_access(node->binding, entity, acted, scope, location);
			return;
		}
	}
}

const BaseClass* Analyzer::direct_base(const ClassEntity* cls, const ClassEntity* base)
{
	const BaseClass* found = base_specifier(cls, base);
	return found != nullptr && !found->is_virtual ? found : nullptr;
}

const BaseClass* Analyzer::base_specifier(const ClassEntity* cls, const ClassEntity* base)
{
	// A few bases are scanned; a complete class of many is indexed at its
	// first query, so each later one costs a probe.
	static const uint32_t kIndexed = 8;
	const BaseClass* found = nullptr;
	if (cls->base_count < kIndexed || !cls->complete)
	{
		for (const BaseClass* step = cls->bases; step != nullptr && found == nullptr; step = step->next)
			found = step->base == base ? step : nullptr;
	}
	else
	{
		if (direct_base_classes_.find(cls) == PointerMap::kMissing)
		{
			direct_base_classes_.set(cls, 1);
			for (const BaseClass* step = cls->bases; step != nullptr; step = step->next)
			{
				if (direct_bases_.find(EntityPair{ cls, step->base }) == nullptr)
					direct_bases_.insert(EntityPair{ cls, step->base }, step);
			}
		}
		const BaseClass* const* entry = direct_bases_.find(EntityPair{ cls, base });
		found = entry != nullptr ? *entry : nullptr;
	}
	return found;
}

void Analyzer::check_base_access(
	const ClassEntity* derived, const ClassEntity* base, const Scope* scope, uint32_t location)
{
	// 4.10p3, 5.2.9p11: a derived-to-base conversion needs an accessible
	// base (an invented public member of it accessible, 11.2p4): each
	// base-specifier on the path, the single base of a class or the one of
	// several that leads to the target (its base-specifier if a direct
	// base). A virtual base may be reached by several paths to its one
	// subobject, the most accessible counting (11.6p1): when the first
	// fails, the invented member's access is decided as a member's (11.2p5's
	// rules over every path).
	if (derived == nullptr || base == nullptr || scope == nullptr)
		return;
	scope = access_point(scope);
	base_leads_.clear();
	base_leads_target_ = base;
	for (const ClassEntity* cls = derived; cls != base;)
	{
		const BaseClass* edge = cls->bases;
		if (edge != nullptr && edge->next != nullptr)
		{
			const BaseClass* direct = base_specifier(cls, base);
			if (direct != nullptr)
				edge = direct;
			while (edge != nullptr && !leads_to_base(static_cast<const ClassEntity*>(edge->base), base))
				edge = edge->next;
		}
		if (edge == nullptr)
			return;
		if (!base_accessible(cls, edge, scope))
		{
			if ((base_path_facts(base, derived, location) & kVirtualBasePath) == 0 ||
				!accessible(derived, base, Access::Public, scope, nullptr, location))
				substitution_error(location, "conversion to an inaccessible base class");
			return;
		}
		cls = static_cast<const ClassEntity*>(edge->base);
	}
}

bool Analyzer::leads_to_base(const ClassEntity* cls, const ClassEntity* base)
{
	// Whether `base` is `cls` or a base of it: down a chain of single bases
	// without the memo, a class of several decided once per query (and the
	// chain's top with it), so check_base_access's choice at each class
	// costs the edges below the derived class once. An access check made
	// meanwhile (base_accessible) may seek another base: the memo is then
	// remade. The classes of several bases under search are frames of
	// base_frames_ (an explicit stack); a base leading to `base` decides
	// them all.
	if (cls == base || cls->bases == nullptr)
		return cls == base;
	if (base != base_leads_target_)
	{
		base_leads_.clear();
		base_leads_target_ = base;
	}
	const uint32_t known = base_leads_.find(cls);
	if (known != PointerMap::kMissing)
		return known != 0;
	const size_t first = base_frames_.size();
	uint32_t leads = leads_chain(cls, base);
	while (leads == kPendingAccess)
	{
		const size_t top = base_frames_.size() - 1;
		const BaseClass* edge = base_frames_[top].next;
		if (edge == nullptr)
		{
			// No base of the class leads to `base`.
			const BaseWalkFrame done = base_frames_[top];
			base_frames_.pop_back();
			base_leads_.set(done.bottom, 0);
			base_leads_.set(done.top, 0);
			if (base_frames_.size() == first)
				leads = 0;
			continue;
		}
		base_frames_[top].next = edge->next;
		const ClassEntity* inner = static_cast<const ClassEntity*>(edge->base);
		uint32_t through = inner == base ? 1 : inner->bases == nullptr ? 0 : base_leads_.find(inner);
		if (through == PointerMap::kMissing)
			through = leads_chain(inner, base);
		if (through == 1)
		{
			for (size_t i = first; i < base_frames_.size(); ++i)
			{
				base_leads_.set(base_frames_[i].bottom, 1);
				base_leads_.set(base_frames_[i].top, 1);
			}
			base_frames_.resize(first);
			leads = 1;
		}
	}
	return leads != 0;
}

uint32_t Analyzer::leads_chain(const ClassEntity* top, const ClassEntity* base)
{
	// leads_to_base down the chain of single bases from `top`: 1 or 0, kept
	// for the chain's top; or kPendingAccess with a frame pushed for the
	// class of several bases it ends at, unless that class is decided.
	const ClassEntity* cls = top;
	while (cls != base && cls->bases != nullptr && cls->bases->next == nullptr)
	{
		++graph_.base_steps;
		cls = static_cast<const ClassEntity*>(cls->bases->base);
	}
	uint32_t leads = cls == base ? 1 : 0;
	if (leads == 0 && cls->bases != nullptr)
	{
		leads = cls != top ? base_leads_.find(cls) : PointerMap::kMissing;
		if (leads == PointerMap::kMissing)
		{
			base_frames_.push_back(BaseWalkFrame{ top, cls, cls->bases, Access::Public, Access::Public, false });
			return kPendingAccess;
		}
	}
	base_leads_.set(top, leads);
	return leads;
}

size_t Analyzer::begin_declaration_access(Scope* scope)
{
	// 11p6: the names of a declaration (its decl-specifiers, declarator and
	// default arguments) are checked in the entity it declares; outside a
	// member's declaration (declaration_class_) that entity, and whether a
	// class befriends it, is known only after the declarator.
	access_deferral_ = scope->kind == ScopeKind::Namespace && declaration_class_ == nullptr && semantic() &&
			template_depth_ == 0
		? scope
		: nullptr;
	return pending_access_.size();
}

bool Analyzer::defer_access(
	const Binding* binding, const ClassEntity* naming, const ClassEntity* object, const Scope* scope, uint32_t location)
{
	// A check at the deferring declaration itself, or in its declarators'
	// prototype scopes (function scopes of no function yet), waits.
	if (access_deferral_ == nullptr)
		return false;
	const Scope* s = scope;
	while (s != nullptr && s->kind == ScopeKind::Function && s->entity == nullptr)
		s = s->parent;
	if (s != access_deferral_)
		return false;
	pending_access_.push_back(PendingAccess{ binding, naming, object, scope, location });
	return true;
}

void Analyzer::settle_declaration_access(size_t first, size_t keep, const Binding* declared)
{
	// Each waiting check is made again as if in the declared function (its
	// befriending classes grant access: 11p7's `A::I g(A::I p = A::x);` for
	// a friend g of A); for any other entity it fails as it did.
	Scope* saved = access_deferral_;
	access_deferral_ = nullptr;
	Entity* entity = declared != nullptr ? declared->entity : nullptr;
	for (size_t i = first; i < pending_access_.size(); ++i)
	{
		const PendingAccess pending = pending_access_[i];
		if (entity == nullptr || entity->kind != EntityKind::Function)
			error(pending.location, "inaccessible member");
		Scope inside = Scope();
		inside.kind = ScopeKind::Function;
		inside.parent = const_cast<Scope*>(pending.scope);
		inside.entity = entity;
		check_access(pending.binding, pending.naming, pending.object, &inside, pending.location);
	}
	pending_access_.resize(keep);
	access_deferral_ = saved;
}

} // namespace sema
