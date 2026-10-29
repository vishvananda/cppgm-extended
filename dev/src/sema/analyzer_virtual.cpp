#include "sema/analyzer.h"

#include <algorithm>

#include "lex/identifier_table.h"

// Virtual functions (10.3, 10.4, 12.4p9, p12). A member function's
// overriding is found where it is declared (its bases are complete): each
// polymorphic base class is probed through the overload index for the
// function's name and signature, stopping on each path at the first class
// declaring one (that function overrides the ones below it). The
// virt-specifier and covariance rules are checked there. At the class's
// completion its virtual table is laid out once (Itanium C++ ABI 2.5.2):
// the primary base's slots, overriders replacing their entries, then a
// slot per virtual function the class introduces in declaration order (a
// virtual destructor's complete and deleting entries), with the facts the
// lowering reads: the final overriders, abstractness, the key function
// and a virtual destructor's deallocation function.
namespace sema {

namespace {

const ClassEntity* BaseClassOf(const BaseClass* base)
{
	return static_cast<const ClassEntity*>(base->base);
}

// `base` is one of `cls`'s virtual bases.
bool HasVirtualBase(const ClassEntity* cls, const ClassEntity* base)
{
	for (uint32_t i = 0; i < VirtualBaseCount(cls); ++i)
	{
		if (VirtualBases(cls)[i].base->base == base)
			return true;
	}
	return false;
}

// The offset of `base` in `derived` through non-virtual bases (the
// layout's offsets); false when no such path reaches it.
bool NonVirtualBaseOffset(const ClassEntity* derived, const ClassEntity* base, uint64_t& offset)
{
	if (derived == base)
	{
		offset = 0;
		return true;
	}
	for (const BaseClass* edge = derived->bases; edge != nullptr; edge = edge->next)
	{
		uint64_t below = 0;
		if (!edge->is_virtual && NonVirtualBaseOffset(BaseClassOf(edge), base, below))
		{
			offset = edge->offset + below;
			return true;
		}
	}
	return false;
}

// The class's primary base when it is non-virtual (the table group's
// chains: a virtual primary base has a segment of its own).
const ClassEntity* NonVirtualPrimary(const ClassEntity* cls)
{
	const BaseClass* primary = cls->virtuals->primary_base;
	return primary != nullptr && !primary->is_virtual ? BaseClassOf(primary) : nullptr;
}

bool IsPureSpecifier(const syntax::Initializer* initializer)
{
	// 9.2: pure-specifier: `= 0`, the literal token 0.
	if (initializer == nullptr || initializer->kind != syntax::InitializerKind::Assign)
		return false;
	const syntax::Expr* e = initializer->expression;
	if (e == nullptr || e->kind != syntax::ExprKind::Literal)
		return false;
	const TokenSpelling& spelling = static_cast<const syntax::LiteralExpr*>(e)->spelling;
	return spelling.length == 1 && spelling.text[0] == '0';
}

} // namespace

void Analyzer::virtual_declaration(
	FunctionEntity* function, const Specifiers& specifiers, const DeclaratorInfo& info, bool first, uint32_t location)
{
	bool is_override = false, is_final = false;
	if (info.virt_suffix != nullptr)
	{
		for (const syntax::FunctionQualifier& qualifier : info.virt_suffix->qualifiers)
		{
			if (qualifier.kind != syntax::FunctionQualifierKind::Virt)
				continue;
			bool& seen = qualifier.virt == final_key_ ? is_final : is_override;
			// 9.2p9: at most one of each virt-specifier.
			if (seen)
				error(qualifier.location, "a repeated virt-specifier");
			seen = true;
		}
	}
	if (!first || !function->member || specifiers.is_friend)
	{
		// 7.1.2p5: `virtual` only in the initial declaration of a non-static
		// class member function; 9.2p9: virt-specifiers only in a member
		// declaration.
		if (specifiers.is_virtual)
			error(location, "virtual outside the initial declaration of a class member function");
		if (is_override || is_final)
			error(location, "a virt-specifier outside the declaration of a class member function");
		return;
	}
	ClassEntity* cls = static_cast<ClassEntity*>(function->owner->entity);
	// 9.4.1p2: a static member function is not virtual; 12.1p4: nor is a
	// constructor; 9.5p2: a union has no virtual functions.
	if (specifiers.is_virtual && function->is_static)
		error(location, "a static member function declared virtual");
	if (specifiers.is_virtual && function->constructor)
		error(location, "a constructor declared virtual");
	if (specifiers.is_virtual && cls->key == TokenKind::KW_UNION)
		error(location, "a union with a virtual function");
	function->is_override = is_override;
	function->is_final = is_final;
	override_facts(function, cls, location);
	// 9.2p9: a virt-specifier only on a virtual function; 10.3p5: one
	// marked override overrides a base class member.
	if (is_override && function->overridden == nullptr && !function->overrides_other_base)
		error(location, "a function marked override overrides no base class member function");
	if (is_final && !function->is_virtual)
		error(location, "a function marked final is not virtual");
	// 7.1.5p3: a constexpr function is not virtual (nor overrides one).
	if (function->is_virtual && function->is_constexpr)
		error(location, "a constexpr virtual function");
}

void Analyzer::override_facts(FunctionEntity* function, ClassEntity* cls, uint32_t location)
{
	// 10.3p2: a member function with the name, parameter-type-list,
	// cv-qualification and ref-qualifier of a base's virtual function
	// overrides it and is virtual (the signature interns all four; a
	// conversion function's includes its type, which is part of its name,
	// 12.3.2p1); 10.3p6: a destructor overrides a virtual destructor. The
	// bases are searched only for a name and a signature some virtual
	// functions of the unit have (both are needed to override). Found on
	// the path of primary bases, a function takes the overridden function's
	// slot of the table they share. A class is searched once off that path
	// (visit 1) and once on it (visit 2): reached first through another
	// subobject of it (a virtual base listed before the primary base), it
	// is searched again when the primary path reaches it.
	if (function->constructor || function->templated)
		return;
	if (function->destructor ||
		(virtual_names_.find(function->name) != PointerMap::kMissing &&
			virtual_signatures_.find(function->signature) != PointerMap::kMissing))
	{
		std::vector<OverrideStep>& stack = override_stack_;
		stack.clear();
		override_visits_.clear();
		// The class's primary base, known from its bases (its virtual
		// facts wait for its completion; it may be a virtual base, see
		// layout_primary).
		bool primary_virtual = false;
		const ClassEntity* primary = itanium_primary(cls, primary_virtual);
		const BaseClass* direct = nullptr;
		for (const BaseClass* base = cls->bases; base != nullptr && direct == nullptr; base = base->next)
		{
			if (!base->is_virtual && BaseClassOf(base)->dynamic)
				direct = base;
		}
		push_override_bases(cls, direct, primary_virtual ? primary : nullptr);
		while (!stack.empty())
		{
			const OverrideStep step = stack.back();
			stack.pop_back();
			const ClassEntity* candidate = step.entity;
			const uint32_t visit = step.primary ? 2 : 1;
			const uint32_t visited = override_visits_.find(candidate);
			if (!candidate->polymorphic || (visited != PointerMap::kMissing && visited >= visit))
				continue;
			override_visits_.set(candidate, visit);
			++graph_.override_steps;
			// A class's destructor (implicit ones have no binding) is its own.
			const FunctionList* node = function->destructor
				? nullptr
				: names_.overload(candidate->scope, function->name, function->signature);
			const FunctionEntity* found = function->destructor ? candidate->destructor
				: node != nullptr ? static_cast<const FunctionEntity*>(node->binding->entity)
								  : nullptr;
			if (found == nullptr || found->owner != candidate->scope || found->templated)
			{
				const BaseClass* primary = step.primary && candidate->virtuals != nullptr
					? candidate->virtuals->primary_base
					: nullptr;
				push_override_bases(candidate, primary != nullptr && !primary->is_virtual ? primary : nullptr,
					primary != nullptr && primary->is_virtual ? BaseClassOf(primary) : nullptr);
				continue;
			}
			if (found->is_virtual)
				overrides(function, found, step.primary, cls, location);
		}
	}
	if (function->is_virtual)
	{
		cls->polymorphic = true;
		virtual_names_.set(function->name, 1);
		virtual_signatures_.set(function->signature, 1);
	}
}

void Analyzer::push_override_bases(
	const ClassEntity* entity, const BaseClass* primary, const ClassEntity* virtual_primary)
{
	// The bases, first on top; the primary base (on the primary path)
	// marked: `primary`, or a virtual primary base, on top when it is an
	// indirect virtual base.
	std::vector<OverrideStep>& stack = override_stack_;
	const size_t mark = stack.size();
	bool met = false;
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		const bool on_path = base == primary || (base->is_virtual && BaseClassOf(base) == virtual_primary);
		met = met || on_path;
		stack.push_back(OverrideStep{ BaseClassOf(base), on_path });
	}
	std::reverse(stack.begin() + static_cast<std::ptrdiff_t>(mark), stack.end());
	if (virtual_primary != nullptr && !met)
		stack.push_back(OverrideStep{ virtual_primary, true });
}

void Analyzer::overrides(
	FunctionEntity* function, const FunctionEntity* found, bool primary, ClassEntity* cls, uint32_t location)
{
	// 9.4.1p2: a static member function cannot be virtual.
	if (function->is_static)
		error(location, "a static member function overrides a virtual function");
	// 10.3p4: a final function is not overridden; 10.3p16: deleted and
	// non-deleted functions do not override each other.
	if (found->is_final)
		error(location, "overriding a final virtual function");
	// (A defaulted destructor's deletedness is known at completion, see
	// virtual_facts.)
	if (found->deleted != function->deleted && !function->destructor)
		error(location, "a deleted function and a function not deleted override one another");
	if (!function->destructor)
		check_override_result(function, found, cls, location);
	// 15.4p5: no more exceptions allowed than the overridden function
	// allows, by the values of noexcept-specifications (15.4p1, DR 1330:
	// the comparison needs them; an overrider's waits for its class, 9.2p2,
	// until the end of the unit). A destructor without an exception-
	// specification has its implicit one once its class completes (see
	// virtual_facts).
	if ((function->exception_spec || !function->destructor) && function_nothrow(found))
	{
		if (function->noexcept_pending)
			pending_overrides_.push_back(PendingOverride{ function, location });
		else if (!function->nothrow)
			error(location, "an overrider allows exceptions its overridden function does not");
	}
	note_exception_override(function, found, location);
	function->is_virtual = true;
	// The first found on the primary path is that table's final overrider.
	if (primary && function->overridden == nullptr)
	{
		function->overridden = found;
		function->vtable_slot = found->vtable_slot;
	}
	else if (!primary)
		function->overrides_other_base = true;
}

void Analyzer::check_override_result(
	const FunctionEntity* function, const FunctionEntity* overridden, const ClassEntity* cls, uint32_t location)
{
	// 10.3p7: the return types are the same, or covariant: both pointers,
	// both lvalue references or both rvalue references to classes, the
	// overridden function's class the same as or an unambiguous and
	// accessible base of the overrider's (accessible in the overrider's
	// class), no more cv-qualified; p8: that class complete where the
	// overrider is declared, unless it is the class being defined.
	const Type* mine = function->type->canonical->element;
	const Type* theirs = overridden->type->canonical->element;
	if (mine == theirs)
		return;
	const bool shaped = mine->kind == theirs->kind && mine->cv == theirs->cv &&
		(mine->kind == TypeKind::Pointer || mine->kind == TypeKind::LValueReference ||
			mine->kind == TypeKind::RValueReference);
	const ClassEntity* derived = shaped ? ClassOf(mine->element) : nullptr;
	const ClassEntity* base = shaped ? ClassOf(theirs->element) : nullptr;
	if (derived == nullptr || base == nullptr)
		error(location, "an overrider's return type is neither the overridden function's nor covariant with it");
	if ((mine->element->cv & ~theirs->element->cv) != 0)
		error(location, "a covariant return type more cv-qualified than the overridden function's");
	if (derived == base)
		return;
	if (derived != cls && !require_complete(derived))
		error(location, "a covariant return type of an incomplete class");
	if (!conversions_.is_base_of(base, derived))
		error(location, "a covariant return type whose class is not derived from the overridden function's");
	if ((base_path_facts(base, derived, location) & kBasePathCount) > 1)
		error(location, "a covariant return type whose class has the overridden function's as an ambiguous base");
	check_base_access(derived, base, cls->scope, location);
}

bool Analyzer::result_needs_adjustment(
	const FunctionEntity* function, const FunctionEntity* overridden, uint32_t location)
{
	// 10.3p7: a covariant result whose conversion to the overridden
	// function's result is not the identity on its address: the
	// overridden function's class a virtual base of the overrider's (or
	// within one), or at a nonzero offset. The overrider's class is
	// complete (10.3p8), its layout final, or the class completing now.
	const Type* mine = function->type->canonical->element;
	const Type* theirs = overridden->type->canonical->element;
	if (mine == theirs || !(mine->kind == TypeKind::Pointer || mine->is_reference()))
		return false;
	const ClassEntity* derived = ClassOf(mine->element);
	const ClassEntity* base = ClassOf(theirs->element);
	if (derived == nullptr || base == nullptr || derived == base)
		return false;
	uint64_t size = 0, alignment = 0, offset = 0;
	if (!type_size(derived->type, size, alignment, location))
		return false;
	return !NonVirtualBaseOffset(derived, base, offset) || offset != 0;
}

void Analyzer::pure_specifier(
	FunctionEntity* function, const syntax::Initializer* initializer, bool member, uint32_t location)
{
	// 10.4p2, 9.2: a pure-specifier only in the member declaration of a
	// virtual function in its class (its first declaration, 9.2p1), not in
	// a redeclaration or a friend declaration.
	if (initializer == nullptr || initializer->kind != syntax::InitializerKind::Assign)
		return;
	if (!IsPureSpecifier(initializer) || !function->is_virtual || !member)
		error(location, "a pure-specifier on a declaration that is not of a virtual function");
	function->pure = true;
}

void Analyzer::virtual_facts(ClassEntity* entity, uint32_t location)
{
	// At completion: the slots of the functions the class introduces (an
	// overrider's is its overridden function's), the table's size, whether
	// a final overrider is pure (10.4p2), the key function and a virtual
	// destructor's deallocation function. The table itself is built only
	// where a unit emits it (see virtual_slots).
	if (!entity->dynamic)
		return;
	// A class with virtual bases has its facts already (with its virtual
	// bases, see virtual_base_list).
	VirtualFacts* facts = entity->virtuals;
	if (facts == nullptr)
	{
		facts = static_cast<VirtualFacts*>(arena_.allocate(sizeof(VirtualFacts), alignof(VirtualFacts)));
		*facts = VirtualFacts{ nullptr, 0, 0, nullptr, nullptr, nullptr, false, nullptr, nullptr, 0, 0 };
		entity->virtuals = facts;
	}
	facts->primary_base = layout_primary(entity);
	const VirtualFacts* inherited = facts->primary_base != nullptr ? BaseClassOf(facts->primary_base)->virtuals
																   : nullptr;
	facts->secondary_bases = inherited != nullptr && inherited->secondary_bases;
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
		facts->secondary_bases = facts->secondary_bases ||
			(base != facts->primary_base && !base->is_virtual && BaseClassOf(base)->dynamic);
	uint32_t next = inherited != nullptr ? inherited->vtable_size : 0;
	uint32_t pure = inherited != nullptr ? inherited->pure_count : 0;
	FunctionEntity* key = nullptr;
	FunctionEntity* destructor = entity->destructor;
	for (const Binding* binding = entity->scope->bindings; binding != nullptr; binding = binding->next)
	{
		if (binding->kind != BindingKind::Function || binding->entity->owner != entity->scope)
			continue;
		FunctionEntity* function = static_cast<FunctionEntity*>(binding->entity);
		if (!function->is_virtual || (function->destructor && function->implicit))
			continue;
		// An overrider whose covariant result needs adjustment takes a
		// slot of its own too (Itanium 2.5.2): the overridden function's
		// slots hold result-adjusting thunks to it.
		if (function->overridden == nullptr ||
			(!function->destructor && result_needs_adjustment(function, function->overridden, location)))
		{
			function->vtable_slot = next;
			next += function->destructor ? 2 : 1;
		}
		pure += (function->pure ? 1 : 0) - (function->overridden != nullptr && function->overridden->pure ? 1 : 0);
		// Itanium 5.2.3: the first non-pure virtual function not inline
		// where the class is defined (a defaulted or deleted one is).
		if (key == nullptr && !function->pure && !function->is_inline && !function->defaulted && !function->deleted)
			key = function;
	}
	// An implicit destructor or assignment operator (no binding of the
	// class's list) is virtual only by overriding (12.4p9, 10.3p2); one
	// overriding only functions of bases off the primary chain takes new
	// slots after the declared ones (as GCC orders it).
	FunctionEntity* const implicit[] = { destructor, entity->copy_assignment, entity->move_assignment };
	for (FunctionEntity* member : implicit)
	{
		if (member == nullptr || !member->implicit || !member->is_virtual)
			continue;
		if (member->overridden != nullptr)
			pure -= member->overridden->pure ? 1 : 0;
		else
		{
			member->vtable_slot = next;
			next += member->destructor ? 2 : 1;
		}
	}
	if (destructor != nullptr && destructor->overridden != nullptr && !destructor->noexcept_pending &&
		function_nothrow(destructor->overridden) && !destructor->nothrow)
		error(location, "a destructor allows exceptions the destructor it overrides does not");
	facts->vtable_size = next;
	facts->pure_count = pure;
	// A class template specialization's members are defined wherever they
	// are needed (vague linkage): it has no key function, and each unit
	// needing its table defines it (Itanium 5.2.3), with the virtual
	// functions its slots name.
	facts->key_function = instance_class(entity->scope) ? nullptr : key;
	// 12.4p12: a virtual destructor's deallocation function is looked up
	// where it is defined: a defaulted one's here, a failed lookup making
	// it deleted (12.4p5); a user-provided one's at its definition, here
	// when defined in the class (else see virtual_definition).
	if (destructor != nullptr && destructor->is_virtual && !destructor->deleted)
	{
		if (destructor->implicit || destructor->defaulted)
		{
			facts->deleting_deallocator = virtual_deallocation(entity, false, location);
			destructor->deleted = facts->deleting_deallocator == nullptr;
		}
		else if (destructor->defined)
			facts->deleting_deallocator = virtual_deallocation(entity, true, location);
	}
	// 10.3p16 once a defaulted destructor's deletedness is known.
	if (destructor != nullptr && destructor->overridden != nullptr &&
		destructor->overridden->deleted != destructor->deleted)
		error(location, "a deleted destructor and a destructor not deleted override one another");
	if (facts->secondary_bases || VirtualBaseCount(entity) != 0)
	{
		vtable_group(entity, location);
		grouped_classes_.push_back(entity);
	}
}

FunctionEntity* Analyzer::declared_overrider(const ClassEntity* cls, const FunctionEntity* function)
{
	// 10.3p2: a member function of `cls` with the name, parameter-type-list,
	// cv-qualification and ref-qualifier of `function` (a virtual function
	// of a base) overrides it; 10.3p6: so does any destructor.
	if (function->destructor)
		return cls->destructor;
	const FunctionList* node = names_.overload(cls->scope, function->name, function->signature);
	FunctionEntity* found = node != nullptr ? static_cast<FunctionEntity*>(node->binding->entity) : nullptr;
	if (found != nullptr && found->owner == cls->scope && !found->templated && found->is_virtual)
		return found;
	FunctionEntity* const implicit[] = { cls->copy_assignment, cls->move_assignment };
	for (FunctionEntity* member : implicit)
	{
		if (member != nullptr && member->implicit && member->is_virtual && member->name == function->name &&
			member->signature == function->signature)
			return member;
	}
	return nullptr;
}

void Analyzer::vtable_group(ClassEntity* entity, uint32_t location)
{
	// Segment 0 is the class's own table. The non-virtual subobjects
	// follow depth first (bases in declaration order), then each virtual
	// base's (inheritance graph order): the primary chain's share a segment
	// (segment 0, or the virtual base's), every other dynamic subobject has
	// one of its class's slots. A slot's final overrider (10.3p2) is the
	// function overriding it declared nearest the complete class on the
	// path to the subobject; within a virtual base, a class deriving from
	// that base dominates the path, and of several such the one deriving
	// from the others must be unique (see group_outside). A final overrider
	// that is pure makes the class abstract (10.4p2). The subobjects below
	// a direct base come from the base's own group, its final overriders
	// relative to the base (see group_splice): a group costs a lookup in
	// the complete class per entry, not one per class on each path.
	VirtualFacts* facts = entity->virtuals;
	std::vector<VtableSegment>& segments = group_segments_;
	std::vector<VtableEntry>& entries = group_entries_;
	segments.clear();
	entries.clear();
	const VirtualSlot* own = virtual_slots(entity);
	segments.push_back(VtableSegment{ entity, nullptr, 0, 0, false, 0 });
	for (uint32_t i = 0; i < facts->vtable_size; ++i)
		entries.push_back(VtableEntry{ own[i].function, own[i].kind, 0 });
	// The primary base's subobjects are on the class's chain (its primary
	// chain shares segment 0); another base has a segment of its own.
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		const ClassEntity* cls = BaseClassOf(base);
		if (!cls->dynamic || base->is_virtual)
			continue;
		if (base == facts->primary_base)
		{
			group_splice(entity, cls, 0, true);
			continue;
		}
		const uint32_t segment = static_cast<uint32_t>(segments.size());
		segments.push_back(VtableSegment{ cls, base, 0, static_cast<uint32_t>(entries.size()), false, 0 });
		group_slots(entity, cls, segment);
		group_below(entity, cls, segment, 0);
	}
	// A virtual base's subobjects, their overriders first found within it.
	for (uint32_t i = 0; i < VirtualBaseCount(entity); ++i)
	{
		BaseClass* base = VirtualBases(entity)[i].base;
		const ClassEntity* cls = static_cast<const ClassEntity*>(base->base);
		if (!cls->dynamic)
			continue;
		const uint32_t segment = static_cast<uint32_t>(segments.size());
		segments.push_back(VtableSegment{ cls, base, 0, static_cast<uint32_t>(entries.size()), false, segment });
		group_slots(nullptr, cls, segment);
		group_below(nullptr, cls, segment, segment);
	}
	group_outside(location);
	share_virtual_primaries(entity, location);
	uint32_t pure = 0;
	for (const VtableEntry& entry : entries)
		pure += entry.function->pure ? 1 : 0;
	facts->pure_count = pure;
	VtableSegment* stored_segments = static_cast<VtableSegment*>(
		arena_.allocate(sizeof(VtableSegment) * segments.size(), alignof(VtableSegment)));
	std::copy(segments.begin(), segments.end(), stored_segments);
	VtableEntry* stored_entries = static_cast<VtableEntry*>(
		arena_.allocate(sizeof(VtableEntry) * std::max<size_t>(entries.size(), 1), alignof(VtableEntry)));
	std::copy(entries.begin(), entries.end(), stored_entries);
	VtableGroup* group = static_cast<VtableGroup*>(arena_.allocate(sizeof(VtableGroup), alignof(VtableGroup)));
	*group = VtableGroup{ stored_segments, static_cast<uint32_t>(segments.size()), stored_entries,
		static_cast<uint32_t>(entries.size()) };
	facts->group = group;
}

void Analyzer::group_slots(const ClassEntity* top, const ClassEntity* cls, uint32_t segment)
{
	// The entries of a segment whose subobject is a direct base of `top`
	// (the complete class; null for a virtual base, the top of its own
	// walk): its class's slots, each overridden by `top`'s declaration.
	const VirtualSlot* slots = virtual_slots(cls);
	const uint32_t size = cls->virtuals->vtable_size;
	for (uint32_t i = 0; i < size; ++i)
	{
		FunctionEntity* overrider = top != nullptr ? declared_overrider(top, slots[i].function) : nullptr;
		group_entries_.push_back(overrider != nullptr ? VtableEntry{ overrider, slots[i].kind, 0 }
													  : VtableEntry{ slots[i].function, slots[i].kind, segment });
	}
}

void Analyzer::group_below(const ClassEntity* top, const ClassEntity* cls, uint32_t segment, uint32_t within)
{
	// The subobjects below a subobject off the complete class's primary
	// chain (`cls`'s, segment `segment`, its entries made; `within`: the
	// virtual base's segment it is or is in, 0 for none). Its primary
	// chain shares its table pointer with a shared segment each, the first
	// the walk reaches below it: a chain class's slot is overridden by the
	// classes above its parent as its parent's slot is (both have its name
	// and signature), else by the parent's own declaration. Then the
	// subobjects below the chain, from `cls`'s group, each chain class
	// standing for its segment there (its segment 0; group_chain_ maps the
	// chain classes' scopes and their base-specifiers to their segments).
	std::vector<VtableSegment>& segments = group_segments_;
	std::vector<VtableEntry>& entries = group_entries_;
	const bool grouped = cls->virtuals->group != nullptr;
	group_chain_.clear();
	uint32_t parent = segment;
	for (const ClassEntity* link = cls;;)
	{
		if (grouped)
		{
			group_chain_.set(link->scope, parent);
			for (const BaseClass* base = link->bases; base != nullptr; base = base->next)
				group_chain_.set(base, parent);
		}
		// A virtual primary base has a segment of its own.
		const BaseClass* primary = link->virtuals->primary_base;
		if (primary == nullptr || primary->is_virtual)
			break;
		const ClassEntity* next = BaseClassOf(primary);
		const uint32_t own = static_cast<uint32_t>(segments.size());
		const uint32_t above = segments[parent].first_entry;
		segments.push_back(VtableSegment{ next, primary, parent, static_cast<uint32_t>(entries.size()), true, within });
		const VirtualSlot* slots = virtual_slots(next);
		for (uint32_t i = 0; i < next->virtuals->vtable_size; ++i)
		{
			VtableEntry entry = entries[above + i];
			entry.kind = slots[i].kind;
			if (entry.overrider == parent)
			{
				FunctionEntity* overrider = declared_overrider(link, slots[i].function);
				entry = overrider != nullptr ? VtableEntry{ overrider, slots[i].kind, parent }
											 : VtableEntry{ slots[i].function, slots[i].kind, own };
			}
			entries.push_back(entry);
		}
		link = next;
		parent = own;
	}
	if (grouped)
		group_splice(top, cls, within, false);
}

void Analyzer::group_splice(const ClassEntity* top, const ClassEntity* cls, uint32_t within, bool chained)
{
	// The subobjects below `cls`'s primary chain, from its group's
	// non-virtual segments (the walk below `cls` reaches them in the order
	// its own walk did): `cls`'s chain classes, its segment 0, are on the
	// complete class's chain (`chained`: segment 0 too) or each stands for
	// its segment (group_chain_); an entry's final overrider is `top`'s
	// declaration (the complete class's, null within a virtual base), else
	// the one relative to `cls`.
	const VtableGroup* group = cls->virtuals->group;
	if (group == nullptr)
		return;
	std::vector<VtableSegment>& segments = group_segments_;
	std::vector<VtableEntry>& entries = group_entries_;
	const uint32_t first = static_cast<uint32_t>(segments.size()) - 1;
	for (uint32_t j = 1; j < group->segment_count && group->segments[j].within == 0; ++j)
	{
		const VtableSegment& from = group->segments[j];
		const uint32_t parent = from.parent != 0 ? first + from.parent : chained ? 0 : group_chain_.find(from.base);
		segments.push_back(
			VtableSegment{ from.cls, from.base, parent, static_cast<uint32_t>(entries.size()), from.shared, within });
		const VtableEntry* slots = group->entries + from.first_entry;
		const uint32_t size = from.cls->virtuals->vtable_size;
		for (uint32_t i = 0; i < size; ++i)
		{
			FunctionEntity* overrider = top != nullptr ? declared_overrider(top, slots[i].function) : nullptr;
			if (overrider != nullptr)
				entries.push_back(VtableEntry{ overrider, slots[i].kind, 0 });
			else
				entries.push_back(VtableEntry{ slots[i].function, slots[i].kind,
					slots[i].overrider != 0 ? first + slots[i].overrider
						: chained           ? 0
											: group_chain_.find(slots[i].function->owner) });
		}
	}
}

void Analyzer::group_outside(uint32_t location)
{
	// The overriders of a virtual base's subobjects from outside that base:
	// a class holds the shared subobject only by having it as a virtual
	// base (a non-virtual base of its class is another one), so each
	// virtual base lists once the subobjects whose class has it (the
	// complete class standing for its primary chain, then each segment's,
	// in walk order; group_shared_ numbers the virtual bases with
	// segments). The chain's candidate is its first class declaring an
	// overrider (chain_overrider): it dominates the chain's others.
	std::vector<VtableSegment>& segments = group_segments_;
	std::vector<VtableEntry>& entries = group_entries_;
	PointerMap& shared = group_shared_;
	shared.clear();
	group_depths_.clear();
	uint32_t count = 0;
	for (uint32_t s = 1; s < segments.size(); ++s)
	{
		if (segments[s].within == s)
			shared.set(segments[s].cls, count++);
	}
	if (count == 0)
		return;
	std::vector<GroupSubobject>& subobjects = group_subobjects_;
	subobjects.clear();
	if (VirtualBaseCount(segments[0].cls) != 0)
		subobjects.push_back(GroupSubobject{ segments[0].cls, 0, 0 });
	for (uint32_t s = 1; s < segments.size(); ++s)
	{
		if (VirtualBaseCount(segments[s].cls) != 0)
			subobjects.push_back(GroupSubobject{ segments[s].cls, s, segments[s].within });
	}
	// The lists in one array, a virtual base's from starts[k] to
	// starts[k + 1].
	std::vector<uint32_t>& starts = group_starts_;
	std::vector<uint32_t>& members = group_members_;
	starts.assign(count + 1, 0);
	for (const GroupSubobject& subobject : subobjects)
	{
		for (uint32_t i = 0; i < VirtualBaseCount(subobject.cls); ++i)
		{
			const uint32_t k = shared.find(VirtualBases(subobject.cls)[i].base->base);
			if (k != PointerMap::kMissing)
				++starts[k + 1];
		}
	}
	for (uint32_t k = 0; k < count; ++k)
		starts[k + 1] += starts[k];
	members.resize(starts[count]);
	for (uint32_t m = 0; m < subobjects.size(); ++m)
	{
		const ClassEntity* cls = subobjects[m].cls;
		for (uint32_t i = 0; i < VirtualBaseCount(cls); ++i)
		{
			const uint32_t k = shared.find(VirtualBases(cls)[i].base->base);
			if (k != PointerMap::kMissing)
				members[starts[k]++] = m;
		}
	}
	for (uint32_t k = count; k > 0; --k)
		starts[k] = starts[k - 1];
	starts[0] = 0;
	for (uint32_t s = 1; s < segments.size(); ++s)
	{
		const VtableSegment& segment = segments[s];
		if (segment.within == 0)
			continue;
		const uint32_t k = shared.find(segments[segment.within].cls);
		const uint32_t size = segment.cls->virtuals->vtable_size;
		for (uint32_t i = 0; i < size && starts[k] != starts[k + 1]; ++i)
		{
			VtableEntry& entry = entries[segment.first_entry + i];
			const VtableEntry* chosen = nullptr;
			uint32_t found = 0;
			group_candidates_.clear();
			for (uint32_t m = starts[k]; m < starts[k + 1]; ++m)
			{
				const GroupSubobject& subobject = subobjects[members[m]];
				if (subobject.within == segment.within)
					continue;
				FunctionEntity* overrider = subobject.segment == 0
					? chain_overrider(subobject.cls, entry.function, segments[segment.within].cls)
					: declared_overrider(subobject.cls, entry.function);
				if (overrider != nullptr)
					group_candidates_.push_back(VtableEntry{ overrider, entry.kind, subobject.segment });
			}
			for (const VtableEntry& candidate : group_candidates_)
			{
				bool dominated = false;
				for (const VtableEntry& other : group_candidates_)
					dominated = dominated || group_dominates(other, candidate);
				if (dominated ||
					(chosen != nullptr && chosen->function == candidate.function &&
						chosen->overrider == candidate.overrider))
					continue;
				chosen = &candidate;
				++found;
			}
			if (found > 1)
				error(location, "a virtual function has no unique final overrider");
			if (chosen != nullptr)
				entry = *chosen;
		}
	}
}

void Analyzer::share_virtual_primaries(ClassEntity* entity, uint32_t location)
{
	// Host layouts: a virtual base at the offset of a subobject with a
	// table pointer of its own shares that pointer (Itanium 2.4 II.3, as
	// Clang's DeterminePrimaryVirtualBases decides it: equal offsets). Its
	// segment shares its parent's: the segment there whose class has it as
	// its primary base (segment 0 for the class's chain).
	std::vector<VtableSegment>& segments = group_segments_;
	std::vector<VtableEntry>& entries = group_entries_;
	bool any = false;
	bool is_virtual = false;
	itanium_primary(entity, is_virtual);
	any = is_virtual;
	for (uint32_t s = 1; s < segments.size() && !any; ++s)
	{
		if (segments[s].within == s)
			itanium_primary(segments[s].cls, is_virtual);
		any = any || (segments[s].within == s && is_virtual);
	}
	any = any || has_indirect_primaries(entity);
	uint64_t size = 0, alignment = 0;
	if (!any || !type_size(entity->type, size, alignment, location))
		return;
	// Sharing (equal offsets in the class's layout).
	std::vector<uint64_t>& offsets = share_offsets_;
	offsets.assign(segments.size(), 0);
	for (uint32_t s = 1; s < segments.size(); ++s)
		offsets[s] = (segments[s].within == s ? 0 : offsets[segments[s].parent]) + segments[s].base->offset;
	// The segments whose class has a virtual primary base, listed by that
	// base (share_heads_, share_next_), and the virtual primary bases of
	// the complete class's chain (segment 0's classes).
	PointerMap& heads = share_heads_;
	heads.clear();
	std::vector<uint32_t>& next = share_next_;
	next.assign(segments.size(), PointerMap::kMissing);
	for (uint32_t k = segments.size(); k-- > 1;)
	{
		const BaseClass* primary = segments[k].cls->virtuals->primary_base;
		if (primary == nullptr || !primary->is_virtual)
			continue;
		next[k] = heads.find(primary->base);
		heads.set(primary->base, k);
	}
	PointerMap& chained = share_chain_;
	chained.clear();
	for (const ClassEntity* link = entity; link != nullptr; link = NonVirtualPrimary(link))
	{
		const BaseClass* primary = link->virtuals->primary_base;
		if (primary != nullptr && primary->is_virtual)
			chained.set(primary->base, 1);
	}
	for (uint32_t s = 1; s < segments.size(); ++s)
	{
		if (segments[s].within != s)
			continue;
		const ClassEntity* shared = segments[s].cls;
		uint32_t parent = PointerMap::kMissing;
		for (uint32_t k = heads.find(shared); k != PointerMap::kMissing && parent == PointerMap::kMissing; k = next[k])
		{
			if (k != s && offsets[k] == offsets[s])
				parent = k;
		}
		if (parent == PointerMap::kMissing && offsets[s] == 0 && chained.find(shared) != PointerMap::kMissing)
			parent = 0;
		if (parent == PointerMap::kMissing)
			continue;
		segments[s].shared = true;
		segments[s].parent = parent;
	}
	// A slot whose function the chain declares at or below a virtual
	// primary base (Clang's MethodInfo there) is that base's: its final
	// overrider is the virtual base's own entry's (overriders in the
	// classes sharing the base), whichever segment's table holds it (a
	// shared one's parent's included).
	PointerMap& levels = share_levels_;
	levels.clear();
	for (uint32_t s = 1; s < segments.size(); ++s)
	{
		if (segments[s].within == s)
			levels.set(segments[s].cls, s);
	}
	for (uint32_t s = 0; s < segments.size(); ++s)
	{
		const ClassEntity* cls = segments[s].cls;
		const ClassEntity* const* slot_level = slot_levels(cls);
		for (uint32_t i = 0; slot_level != nullptr && i < cls->virtuals->vtable_size; ++i)
		{
			const uint32_t level = slot_level[i] != nullptr ? levels.find(slot_level[i]) : PointerMap::kMissing;
			if (level != PointerMap::kMissing && level != s)
				entries[segments[s].first_entry + i] = entries[segments[level].first_entry + i];
		}
	}
}

bool Analyzer::has_virtual_base(const ClassEntity* cls, const ClassEntity* base)
{
	// O(1) after one pass over the class's virtual bases (a class with
	// thousands of them is asked once per base and slot).
	if (virtual_base_indexed_.find(cls) == PointerMap::kMissing)
	{
		virtual_base_indexed_.set(cls, 1);
		for (uint32_t i = 0; i < VirtualBaseCount(cls); ++i)
			virtual_base_index_.insert(VirtualBaseKey{ cls, BaseClassOf(VirtualBases(cls)[i].base) }, true);
	}
	return virtual_base_index_.find(VirtualBaseKey{ cls, base }) != nullptr;
}

FunctionEntity* Analyzer::chain_overrider(
	const ClassEntity* head, const FunctionEntity* function, const ClassEntity* vbase)
{
	// (The caller asks only about a virtual base `head` has.)
	// The first class of `head`'s non-virtual primary chain declaring an
	// overrider of `function` (a virtual base `vbase`'s), among the chain's
	// classes having that base: once per class, function and base (the
	// classes walked take the answer found below them).
	std::vector<const ClassEntity*>& walk = chain_walk_;
	walk.clear();
	FunctionEntity* found = nullptr;
	for (const ClassEntity* link = head; link != nullptr; link = NonVirtualPrimary(link))
	{
		if (FunctionEntity* const* known = chain_overriders_.find(ChainKey{ link, function, vbase }))
		{
			found = *known;
			break;
		}
		if (link != head && !has_virtual_base(link, vbase))
			break;
		walk.push_back(link);
		found = declared_overrider(link, function);
		if (found != nullptr)
			break;
	}
	for (const ClassEntity* link : walk)
		chain_overriders_.insert(ChainKey{ link, function, vbase }, found);
	return found;
}

const ClassEntity* const* Analyzer::slot_levels(const ClassEntity* cls)
{
	// Per slot of the class's table, the deepest virtual primary base on
	// its primary chain at or above the class declaring the slot's function
	// nearest it (the function sharing the slot; Clang's MethodInfo key),
	// null for none; null for a chain without a virtual primary base. Once
	// per class, each from its primary base's (the chain walked once).
	const uint32_t known = slot_level_index_.find(cls);
	if (known != PointerMap::kMissing)
		return known == 0 ? nullptr : slot_level_store_.data() + (known - 1);
	std::vector<const ClassEntity*>& chain = slot_level_chain_;
	chain.clear();
	for (const ClassEntity* link = cls; link != nullptr && slot_level_index_.find(link) == PointerMap::kMissing;)
	{
		chain.push_back(link);
		const BaseClass* primary = link->virtuals != nullptr ? link->virtuals->primary_base : nullptr;
		link = primary != nullptr ? BaseClassOf(primary) : nullptr;
	}
	for (size_t c = chain.size(); c-- > 0;)
	{
		const ClassEntity* link = chain[c];
		const BaseClass* primary = link->virtuals != nullptr ? link->virtuals->primary_base : nullptr;
		const uint32_t below = primary != nullptr ? slot_level_index_.find(BaseClassOf(primary)) : 0;
		const bool virtual_edge = primary != nullptr && primary->is_virtual;
		if (!virtual_edge && below == 0)
		{
			slot_level_index_.set(link, 0);
			continue;
		}
		const uint32_t size = link->virtuals->vtable_size;
		const VirtualSlot* slots = virtual_slots(link);
		const uint32_t first = static_cast<uint32_t>(slot_level_store_.size());
		for (uint32_t i = 0; i < size; ++i)
		{
			const uint32_t base_slot = slots[i].kind == SlotKind::DeletingDestructor ? i - 1 : i;
			const FunctionEntity* function = slots[i].function;
			const ClassEntity* level = nullptr;
			if (function->owner != link->scope || function->vtable_slot != base_slot)
			{
				level = below != 0 && i < BaseClassOf(primary)->virtuals->vtable_size ? slot_level_store_[below - 1 + i]
																					  : nullptr;
				if (level == nullptr && virtual_edge)
					level = BaseClassOf(primary);
			}
			slot_level_store_.push_back(level);
		}
		slot_level_index_.set(link, first + 1);
	}
	const uint32_t index = slot_level_index_.find(cls);
	return index == 0 ? nullptr : slot_level_store_.data() + (index - 1);
}

bool Analyzer::group_dominates(const VtableEntry& over, const VtableEntry& under)
{
	// `over`'s function overrides `under`'s for the subobject in question
	// when `under`'s subobject is a base subobject of `over`'s (10.3p2; its
	// class a base of `over`'s): in a virtual base `over`'s class has, or
	// up `under`'s chain of segments to `over`'s. Segment 0 holds the
	// complete class's primary chain: a subobject reaching it through a
	// chain class's base-specifier is within that chain class and the ones
	// above it only, not those below (another subobject of their bases).
	const ClassEntity* derived = ClassOf(over.function->owner->entity->type);
	const ClassEntity* base = ClassOf(under.function->owner->entity->type);
	if (derived == base || !conversions_.is_base_of(base, derived))
		return false;
	const uint32_t within = group_segments_[under.overrider].within;
	if (within != 0 && HasVirtualBase(derived, group_segments_[within].cls))
		return true;
	if (under.overrider == over.overrider)
		return true;
	for (uint32_t segment = under.overrider; segment != 0 && group_segments_[segment].within != segment;)
	{
		const uint32_t parent = group_segments_[segment].parent;
		if (parent == over.overrider)
			return parent != 0 || group_chain_depth(group_segments_[segment].base) >= group_chain_depth(derived);
		segment = parent;
	}
	return false;
}

uint32_t Analyzer::group_chain_depth(const void* key)
{
	// The depth on the complete class's primary chain (0: the class
	// itself) of a chain class, or of the chain class a base-specifier
	// `key` is of; indexed at a group's first query (group_depths_).
	if (group_depths_.size() == 0)
	{
		uint32_t depth = 0;
		for (const ClassEntity* link = group_segments_[0].cls; link != nullptr; ++depth, link = NonVirtualPrimary(link))
		{
			group_depths_.set(link, depth);
			for (const BaseClass* base = link->bases; base != nullptr; base = base->next)
				group_depths_.set(base, depth);
		}
	}
	return group_depths_.find(key);
}

void Analyzer::virtual_definition(FunctionEntity* function, uint32_t location)
{
	// A user-provided virtual destructor defined outside its class: its
	// deallocation function (12.4p12).
	const ClassEntity* cls = function->member ? ClassOf(function->owner->entity->type) : nullptr;
	VirtualFacts* facts = cls != nullptr ? cls->virtuals : nullptr;
	if (facts != nullptr && function->destructor && function->is_virtual && !function->deleted &&
		facts->deleting_deallocator == nullptr)
		facts->deleting_deallocator = virtual_deallocation(const_cast<ClassEntity*>(cls), true, location);
}

const VirtualSlot* Analyzer::virtual_slots(const ClassEntity* entity)
{
	// Built once per class on demand, top-down along the primary chain in
	// a scratch table (each class's starts as its primary base's). A
	// base's own table is kept only where this unit may emit it too (no
	// key function, or the key function defined here, Itanium 5.2.3):
	// another unit's is never demanded here.
	const VirtualFacts* facts = entity->virtuals;
	if (facts == nullptr || facts->vtable != nullptr || facts->vtable_size == 0)
		return facts != nullptr ? facts->vtable : nullptr;
	std::vector<const ClassEntity*>& chain = slot_chain_;
	chain.clear();
	for (const ClassEntity* cls = entity; cls != nullptr && cls->virtuals != nullptr &&
		cls->virtuals->vtable == nullptr && cls->virtuals->vtable_size != 0;
		cls = cls->virtuals->primary_base != nullptr ? BaseClassOf(cls->virtuals->primary_base) : nullptr)
		chain.push_back(cls);
	std::vector<VirtualSlot>& work = slot_work_;
	work.clear();
	const BaseClass* above = chain.back()->virtuals->primary_base;
	const VirtualFacts* built = above != nullptr ? BaseClassOf(above)->virtuals : nullptr;
	if (built != nullptr && built->vtable != nullptr)
		work.assign(built->vtable, built->vtable + built->vtable_size);
	for (size_t i = chain.size(); i-- > 0;)
	{
		const ClassEntity* cls = chain[i];
		VirtualFacts* own = cls->virtuals;
		work.resize(own->vtable_size);
		for (const Binding* binding = cls->scope->bindings; binding != nullptr; binding = binding->next)
		{
			if (binding->kind == BindingKind::Function && binding->entity->owner == cls->scope)
				place_virtual(static_cast<FunctionEntity*>(binding->entity), work.data());
		}
		FunctionEntity* const implicit[] = { cls->destructor, cls->copy_assignment, cls->move_assignment };
		for (FunctionEntity* member : implicit)
		{
			if (member != nullptr && member->implicit)
				place_virtual(member, work.data());
		}
		if (i != 0 && own->key_function != nullptr && !own->key_function->defined)
			continue;
		VirtualSlot* slots = static_cast<VirtualSlot*>(
			arena_.allocate(sizeof(VirtualSlot) * own->vtable_size, alignof(VirtualSlot)));
		std::copy(work.begin(), work.end(), slots);
		own->vtable = slots;
		graph_.virtual_slots += own->vtable_size;
	}
	return facts->vtable;
}

void Analyzer::place_virtual(FunctionEntity* function, VirtualSlot* slots)
{
	// A virtual function's entries, a destructor's two: its own slot and
	// those of the functions it overrides along the primary chain (one
	// slot unless a covariant result needing adjustment added its own).
	// A function taking a new slot is its introducer; the functions below
	// an introducer take another slot, so the walk steps from each slot's
	// introducer to the function it overrides: a step per distinct slot,
	// not per function of the chain.
	if (!function->is_virtual)
		return;
	const SlotKind kind = function->destructor ? SlotKind::CompleteDestructor : SlotKind::Function;
	if (function->overridden == nullptr || function->overridden->vtable_slot != function->vtable_slot)
	{
		slots[function->vtable_slot].introducer = function;
		if (function->destructor)
			slots[function->vtable_slot + 1].introducer = function;
	}
	for (const FunctionEntity* overridden = function; overridden != nullptr;)
	{
		const uint32_t slot = overridden->vtable_slot;
		const FunctionEntity* introducer = slots[slot].introducer != nullptr ? slots[slot].introducer : overridden;
		// The function's own slot returns its result; another keeps the
		// result of the function sharing it nearest the class.
		const FunctionEntity* adjusted = slot == function->vtable_slot ? function
			: slots[slot].adjusted != nullptr                          ? slots[slot].adjusted
																	   : introducer;
		slots[slot] = VirtualSlot{ function, kind, introducer, adjusted };
		if (function->destructor)
			slots[slot + 1] = VirtualSlot{ function, SlotKind::DeletingDestructor, introducer, adjusted };
		overridden = introducer->overridden;
	}
}

const FunctionEntity* Analyzer::virtual_deallocation(ClassEntity* entity, bool fatal, uint32_t location)
{
	// 12.4p12, 12.5p4: a virtual destructor's deallocation function is
	// looked up where it is defined, in its class's scope (members of its
	// bases included), else the global one: operator delete(void*) before
	// a member operator delete(void*, std::size_t). Not `fatal` (a
	// defaulted destructor), a failed lookup (none, deleted, inaccessible
	// from the destructor) is null.
	const Identifier* key = operator_key(TokenKind::KW_DELETE);
	LookupResult found = names_.qualified(entity->scope, key, LookupFilter::Ordinary);
	const ClassEntity* naming = found.found() ? entity : nullptr;
	if (!found.found())
		found = names_.qualified(graph_.global, key, LookupFilter::Ordinary);
	const FunctionEntity* function = found.found() && found.binding->kind == BindingKind::Function
		? usual_deallocation(found.functions)
		: nullptr;
	if (!fatal)
	{
		if (function == nullptr || function->deleted)
			return nullptr;
		for (const FunctionList* node = found.functions; naming != nullptr && node != nullptr; node = node->next)
		{
			const Binding* binding = node->binding;
			if (binding->entity != function || binding->declared_in == nullptr ||
				binding->declared_in->kind != ScopeKind::Class)
				continue;
			const ClassEntity* declaring = static_cast<const ClassEntity*>(binding->declared_in->entity);
			const ClassEntity* via = nullptr;
			if (!accessible(naming, declaring, binding->access, entity->scope, &via, location))
				return nullptr;
		}
		return function;
	}
	if (!found.found() || found.binding->kind != BindingKind::Function)
		error(location, "no deallocation function for a virtual destructor");
	if (function == nullptr)
		error(location, "no usual deallocation function for a virtual destructor");
	check_not_deleted(function, location);
	check_function_access(function, found.functions, naming, nullptr, entity->scope, location);
	return function;
}

bool Analyzer::check_abstract_object(const Type* type, uint32_t location)
{
	// 10.4p2-p3: no object of an abstract class is created but as a base
	// subobject; it is no parameter or return type. False for a
	// substitution's failure (14.8.2p8).
	while (type->kind == TypeKind::Array)
		type = type->element;
	const ClassEntity* entity = type->is_reference() ? nullptr : ClassOf(type);
	if (entity != nullptr && entity->complete && entity->virtuals != nullptr && entity->virtuals->pure_count != 0)
		return substitution_error(location, "an object of an abstract class type");
	return true;
}

} // namespace sema
