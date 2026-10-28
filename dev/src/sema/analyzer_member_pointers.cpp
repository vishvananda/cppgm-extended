#include "sema/analyzer.h"

#include <utility>

// Pointers to members across base classes: the operands of `.*`
// and `->*` (5.5), casts between pointers to members of related classes
// (5.2.9p12), the base paths that make a conversion between a class and
// its base unambiguous (10.2, 4.10p3, 4.11p2), the checks a recorded
// derived-to-base or member-to-derived conversion needs where applied,
// and the composite type of two pointers or pointers to members compared
// or chosen by ?: (5.9p2, 5.10p2, 5.16p6).
namespace sema {

namespace {

bool IsPointer(const Type* type)
{
	return type->unqualified->kind == TypeKind::Pointer;
}

bool IsMemberPointer(const Type* type)
{
	return type->unqualified->kind == TypeKind::MemberPointer;
}

bool IsNullptrType(const Type* type)
{
	type = type->unqualified;
	return type->kind == TypeKind::Fundamental && type->fundamental == FundamentalType::NullptrT;
}

// build_base_table's and base_paths' mark, beside a class's facts, of a
// virtual base whose subobject has been counted (one however many
// base-specifiers name it, 10.1p4).
const uint32_t kCountedVirtualBase = 16;

} // namespace

ExprFacts* Analyzer::member_pointer_access(const syntax::BinaryExpr& expr, ExprFacts* left, ExprFacts* right)
{
	// 5.5: `.*` takes a class object, `->*` a pointer to one; the right
	// operand is a pointer to a member of that class or of a base. Invalid
	// operands fail a substitution (14.8.2p8).
	if (!check_resolved(*expr.left))
		return nullptr;
	const Type* member_pointer = decayed(right);
	if (!IsMemberPointer(member_pointer))
	{
		substitution_error(expr.location, "right operand of .* or ->* is not a pointer to member");
		return nullptr;
	}
	convert_operand(*expr.right, member_pointer);
	const Type* object = left->type;
	ValueCategory category = left->category;
	if (expr.op == TokenKind::OP_ARROWSTAR)
	{
		const Type* pointer = decayed(left);
		if (!IsPointer(pointer))
		{
			substitution_error(expr.location, "left operand of ->* is not a pointer");
			return nullptr;
		}
		convert_operand(*expr.left, pointer);
		object = pointer->element;
		category = ValueCategory::LValue;
	}
	const ClassEntity* owner = ClassOf(member_pointer->owner);
	const ClassEntity* cls = ClassOf(object);
	if (cls == nullptr || (cls != owner && !conversions_.is_base_of(owner, cls)))
	{
		substitution_error(expr.location, "object and pointer to member classes do not match");
		return nullptr;
	}
	// 5.5p2-p3: the pointer's class is the object's or an unambiguous
	// accessible base of it.
	if (cls != owner)
	{
		if ((base_path_facts(owner, cls, expr.location) & kBasePathCount) > 1)
		{
			substitution_error(expr.location, "a pointer to member of an ambiguous base class of the object");
			return nullptr;
		}
		check_base_access(cls, owner, expression_scope_, expr.location);
		if (failed())
			return nullptr;
	}
	const Type* member = member_pointer->element;
	if (member->kind == TypeKind::Function)
	{
		ExprFacts* facts = new_facts(expr, member, ValueCategory::PRValue);
		facts->flags |= kBoundMember;
		return facts;
	}
	// 5.5p6: the data member is an lvalue of an lvalue object and, as
	// E1.E2 (5.2.5p4), an xvalue of another (CWG 616's reading, which
	// g++ and clang++ apply to C++11): a prvalue object's member is that
	// temporary's subobject, not a copy.
	return new_facts(expr, types_.qualified(member, object->cv),
		category == ValueCategory::LValue ? ValueCategory::LValue : ValueCategory::XValue);
}

ExprFacts* Analyzer::member_pointer_cast(
	const syntax::Expr& expr, ExprFacts* source, const Type* from, const Type* goal, bool c_style)
{
	// A member pointer converts implicitly (4.11), or to a pointer to
	// member of a base class B of its class D when B's converts to D's
	// (5.2.9p12: an unambiguous base that is no virtual base, the member
	// type the same but for added cv-qualifiers); 5.4p4: a C-style cast
	// ignores access (not ambiguity), and may then cast away constness (a
	// static_cast followed by a const_cast). The view shows the value
	// itself.
	Conversion conversion = Conversion();
	uint32_t hierarchy = 0;
	const Type* kept = c_style && IsMemberPointer(from) ? types_.member_pointer(goal->owner, from->element) : goal;
	if (conversions_.implicit(operand_of(source), goal, true, conversion))
		check_conversion_access(source, conversion, expr.location, !c_style);
	else if (kept != goal && conversions_.implicit(operand_of(source), kept, true, conversion))
	{
		check_conversion_access(source, conversion, expr.location, false);
		conversion.target = goal->canonical;
		conversion.result = goal->canonical;
	}
	else
	{
		const ClassEntity* derived = IsMemberPointer(from) ? ClassOf(from->owner) : nullptr;
		const ClassEntity* base = ClassOf(goal->owner);
		if (derived == nullptr || base == nullptr || !conversions_.is_base_of(base, derived) ||
			from->element->unqualified != goal->element->unqualified ||
			(!c_style && (from->element->cv & ~goal->element->cv) != 0))
		{
			substitution_error(expr.location, "invalid conversion to a pointer to member");
			return nullptr;
		}
		const uint32_t paths = base_path_facts(base, derived, expr.location);
		if ((paths & kBasePathCount) != 1 || (paths & kVirtualBasePath) != 0)
		{
			substitution_error(expr.location, "a cast to a pointer to member of an ambiguous or virtual base class");
			return nullptr;
		}
		if (!c_style)
			check_base_access(derived, base, expression_scope_, expr.location);
		conversion.target = goal;
		conversion.result = goal;
		conversion.rank = ConversionRank::Conversion;
		hierarchy = kBaseCast;
	}
	if (failed())
		return nullptr;
	record_conversion(source, conversion);
	ExprFacts* facts = new_facts(expr, goal, ValueCategory::PRValue);
	facts->flags |= kReferenceCast | hierarchy;
	facts->target = goal;
	return facts;
}

bool Analyzer::static_member_pointer_cast(const ExprFacts* source, const Type* from, const Type* goal)
{
	// 5.4p4's first applicable interpretation of a C-style cast to a pointer
	// to member is a static_cast (perhaps followed by a const_cast) when it
	// converts implicitly or between members of one type (but for cv) of
	// classes related by derivation (4.11, 5.2.9p12: ill-formed through an
	// ambiguous or virtual base, not then a reinterpret_cast); a const_cast
	// when only qualifiers differ; otherwise a reinterpret_cast (5.2.10p9).
	Conversion conversion = Conversion();
	if (!IsMemberPointer(from) || conversions_.implicit(operand_of(source), goal, true, conversion))
		return true;
	if (ConversionRules::const_castable(from, goal))
		return false;
	const ClassEntity* derived = ClassOf(from->owner);
	const ClassEntity* base = ClassOf(goal->owner);
	return derived != nullptr && base != nullptr && from->element->unqualified == goal->element->unqualified &&
		(derived == base || conversions_.is_base_of(base, derived) || conversions_.is_base_of(derived, base));
}

bool Analyzer::member_subobject(const syntax::Expr& expr, const Entity* member, const ClassEntity* cls,
	const ClassEntity* naming, uint32_t location)
{
	// A non-static member named in class `naming` of an object of class
	// `cls`: the object converts to the naming class (11.2p6), and the class
	// of which the member is directly a member is an unambiguous base of
	// the naming class (5.2.5p5; 10.2: one member of several subobjects of
	// its class is ambiguous). A naming class between the two is recorded
	// for the lowering's path (member_naming_class). False on failure.
	const ClassEntity* home = MemberClassOf(member);
	if ((naming != cls && (base_path_facts(naming, cls, location) & kBasePathCount) > 1) ||
		(home != naming && (base_path_facts(home, naming, location) & kBasePathCount) > 1))
	{
		substitution_error(location, "a non-static member of an ambiguous base class subobject");
		return false;
	}
	// 11.2p6: the object (as a pointer) converts to a pointer to the naming
	// class at the point: an accessible base.
	if (naming != cls)
	{
		check_base_access(cls, naming, expression_scope_, location);
		if (failed())
			return false;
	}
	if (naming != cls && naming != home && naming_index_.find(&expr) == PointerMap::kMissing)
	{
		naming_index_.set(&expr, static_cast<uint32_t>(namings_.size()));
		namings_.push_back(naming);
	}
	return true;
}

void Analyzer::check_conversion_access(
	const ExprFacts* from, const Conversion& conversion, uint32_t location, bool access)
{
	// 4.10p3, 8.5.3p5, 13.3.3.1.4p1: a conversion to a base (of a pointer,
	// a reference binding or an object) needs an accessible unambiguous
	// base; 4.11p2: a pointer to member's conversion to a derived class's,
	// an accessible unambiguous base that is no virtual base nor a base of
	// one. An array operand converts as the pointer it decays to (4.2).
	if ((conversion.second != ValueConversion::DerivedToBase &&
			conversion.second != ValueConversion::MemberToDerived) ||
		conversion.result == nullptr)
		return;
	const Type* source = from->type->unqualified;
	const Type* target = conversion.result->unqualified;
	const bool member = conversion.second == ValueConversion::MemberToDerived;
	if (member)
	{
		std::swap(source, target);
		source = source->owner;
		target = target->owner;
	}
	else if ((source->kind == TypeKind::Pointer || source->kind == TypeKind::Array) &&
		target->kind == TypeKind::Pointer)
	{
		source = source->element;
		target = target->element;
	}
	const ClassEntity* derived = ClassOf(source);
	const ClassEntity* base = ClassOf(target);
	if (derived == nullptr || base == nullptr || derived == base)
		return;
	if (!member)
	{
		upcast_base(derived, base, access, location);
		return;
	}
	const uint32_t paths = base_path_facts(base, derived, location);
	if ((paths & kBasePathCount) > 1)
	{
		substitution_error(location, "conversion to an ambiguous base class");
		return;
	}
	if ((paths & kVirtualBasePath) != 0)
	{
		substitution_error(location, "a pointer to member of a virtual base class converted to a derived class's");
		return;
	}
	if (access)
		check_base_access(derived, base, expression_scope_, location);
}

bool Analyzer::upcast_base(const ClassEntity* derived, const ClassEntity* base, bool access, uint32_t location)
{
	// 4.10p3, 8.5.3p5, 5.2.9p2/p4: a derived class object (or pointer)
	// converted to a base class's needs an unambiguous base (10.2p12),
	// accessible at the point (11.2p4) unless `access` is false: a C-style
	// cast waives access only (5.4p4). False (a substitution's failure)
	// otherwise.
	if (derived == base)
		return true;
	if ((base_path_facts(base, derived, location) & kBasePathCount) > 1)
	{
		substitution_error(location, "conversion to an ambiguous base class");
		return false;
	}
	if (access)
		check_base_access(derived, base, expression_scope_, location);
	return !failed();
}

bool Analyzer::downcast_base(const ClassEntity* base, const ClassEntity* derived, bool access, uint32_t location)
{
	// 5.2.9p2, p11: a static_cast from a base to its derived class needs a
	// base that is unambiguous and neither a virtual base nor a base of
	// one, and accessible (a valid standard conversion from a pointer to
	// the derived class to one to the base exists, 11.2p4) unless `access`
	// is false (a C-style cast, 5.4p4); a C-style cast so interpreted is
	// ill-formed too (5.4p5), not a reinterpret_cast. False (a
	// substitution's failure) otherwise.
	const uint32_t facts = base_path_facts(base, derived, location);
	if ((facts & kVirtualBasePath) == 0 && (facts & kBasePathCount) <= 1)
	{
		if (access)
			check_base_access(derived, base, expression_scope_, location);
		return !failed();
	}
	substitution_error(location,
		(facts & kVirtualBasePath) != 0 ? "a cast from a virtual base class to its derived class"
										: "a cast from an ambiguous base class to its derived class");
	return false;
}

bool Analyzer::simple_hierarchy(const ClassEntity* cls)
{
	// No virtual base-specifier below `cls` and no class reached twice: each
	// base is one subobject. A chain of single non-virtual bases shares the
	// fact of the class it reaches (no base, several, or one known), each
	// complete class's fact kept, so a chain costs its length once.
	SmallList<const ClassEntity*, 16> chain;
	const ClassEntity* current = cls;
	bool simple = true;
	bool known = false;
	for (;;)
	{
		const uint32_t fact = simple_hierarchies_.find(current);
		if (fact != PointerMap::kMissing)
		{
			simple = fact == 1;
			known = true;
			break;
		}
		const BaseClass* node = current->bases;
		if (node == nullptr)
			break;
		if (node->next == nullptr)
		{
			if (node->base == nullptr || node->base->kind != EntityKind::Class || node->is_virtual)
			{
				simple = false;
				break;
			}
			chain.push_back(current);
			current = static_cast<const ClassEntity*>(node->base);
			continue;
		}
		base_path_memo_.clear();
		SmallList<const ClassEntity*, 16> stack;
		stack.push_back(current);
		while (simple && !stack.empty())
		{
			const ClassEntity* next = stack.back();
			stack.pop_back();
			for (const BaseClass* edge = next->bases; edge != nullptr; edge = edge->next)
			{
				if (edge->base == nullptr || edge->base->kind != EntityKind::Class || edge->is_virtual ||
					base_path_memo_.find(edge->base) != PointerMap::kMissing)
				{
					simple = false;
					break;
				}
				base_path_memo_.set(edge->base, 1);
				stack.push_back(static_cast<const ClassEntity*>(edge->base));
			}
		}
		break;
	}
	if (!known && current->complete)
		simple_hierarchies_.set(current, simple ? 1 : 2);
	for (size_t k = 0; k < chain.size(); ++k)
	{
		if (chain[k]->complete)
			simple_hierarchies_.set(chain[k], simple ? 1 : 2);
	}
	return simple;
}

uint32_t Analyzer::base_path_facts(const ClassEntity* base, const ClassEntity* derived, uint32_t location)
{
	// The facts of `base` (the class or one of its bases; callers ask only
	// about bases) in `derived`: a hierarchy without repeated classes or
	// virtual bases has one path to each base. Otherwise the chain of single
	// non-virtual bases from `derived` reaches a class with several bases
	// (or a virtual one) whose table holds the facts of every class below it
	// (base_table_fact); a class on the chain above it has one path.
	const uint32_t one = 1 | kNonVirtualBasePath;
	if (base == derived || simple_hierarchy(derived))
		return one;
	const ClassEntity* bottom = chain_bottom(derived);
	if (base == bottom)
		return one;
	const int32_t fact = base_table_fact(bottom, base, location);
	if (fact >= 0)
		return fact != 0 ? static_cast<uint32_t>(fact) : bottom != derived ? one : 0;
	// No table (an incomplete class, or past the tables' budget): a chain
	// of classes of one base-specifier each is followed without a memo. A
	// base below the class of several it reaches has the subobjects it has
	// in an object of that class (the chain's classes are each one
	// subobject), within a virtual base if a base-specifier of the chain is
	// virtual.
	bool virtual_chain = false;
	for (const ClassEntity* cls = derived; cls != base;)
	{
		const BaseClass* node = cls->bases;
		if (node == nullptr || node->base == nullptr || node->base->kind != EntityKind::Class)
			return 0;
		if (node->next != nullptr)
		{
			// The memo is the analysis's scratch, reset per query (only the
			// slots written are cleared).
			base_path_memo_.clear();
			uint32_t shared = 0;
			const uint32_t below = base_paths(base, cls, base_path_memo_, shared, location);
			if (below == 0)
				return 0;
			const uint32_t paths = (below & kBasePathCount) + shared;
			return (paths < 2 ? paths : 2) |
				(virtual_chain ? kVirtualBasePath : below & (kVirtualBasePath | kNonVirtualBasePath));
		}
		virtual_chain = virtual_chain || node->is_virtual;
		cls = static_cast<const ClassEntity*>(node->base);
	}
	return virtual_chain ? 1 | kVirtualBasePath : one;
}

const ClassEntity* Analyzer::chain_bottom(const ClassEntity* cls)
{
	// The first class down the chain of single non-virtual bases from `cls`
	// that has none, several or a virtual one (`cls` itself if so); each
	// complete class of the chain keeps it, so a chain costs its length once.
	SmallList<const ClassEntity*, 16> chain;
	const ClassEntity* current = cls;
	const ClassEntity* bottom = nullptr;
	for (;;)
	{
		const uint32_t known = chain_bottoms_.find(current);
		if (known != PointerMap::kMissing)
		{
			bottom = chain_bottom_list_[known];
			break;
		}
		const BaseClass* node = current->bases;
		if (node == nullptr || node->next != nullptr || node->is_virtual || node->base == nullptr ||
			node->base->kind != EntityKind::Class)
		{
			bottom = current;
			break;
		}
		chain.push_back(current);
		current = static_cast<const ClassEntity*>(node->base);
	}
	const uint32_t index = static_cast<uint32_t>(chain_bottom_list_.size());
	bool kept = false;
	for (size_t k = 0; k < chain.size(); ++k)
	{
		if (chain[k]->complete)
		{
			chain_bottoms_.set(chain[k], index);
			kept = true;
		}
	}
	if (kept)
		chain_bottom_list_.push_back(bottom);
	return bottom;
}

int32_t Analyzer::base_table_fact(const ClassEntity* cls, const ClassEntity* base, uint32_t location)
{
	// The base_path_facts of `base` below `cls` (0: not below it) from
	// cls's table, built at its first need for a complete class: the classes
	// below it in topological order, the subobjects counted from cls down,
	// so a table costs the edges below its class once. The tables' entries
	// have a budget (each class of a deep chain of such classes queried
	// would otherwise keep a table of all below it): -1 without a table.
	if (!cls->complete)
		return -1;
	const uint32_t state = base_tables_.find(cls);
	if (state == 2 || (state == PointerMap::kMissing && !build_base_table(cls, location)))
		return -1;
	const uint8_t* fact = base_table_.find(EntityPair{ cls, base });
	return fact != nullptr ? *fact : 0;
}

bool Analyzer::build_base_table(const ClassEntity* cls, uint32_t location)
{
	// The classes below cls, each once, in post-order (an explicit stack),
	// then counted in reverse post-order (each class after every class it
	// is a base of): a class's subobjects in a cls object are one if it is
	// a virtual base, plus those of each class it is a non-virtual base of
	// (the direct base subobjects of theirs). False (and kept so) when past
	// the budget.
	check_stack(location);
	base_path_memo_.clear();
	std::vector<const ClassEntity*>& order = table_order_;
	std::vector<std::pair<const ClassEntity*, const BaseClass*>>& stack = table_stack_;
	order.clear();
	stack.clear();
	stack.push_back(std::make_pair(cls, cls->bases));
	base_path_memo_.set(cls, 0);
	while (!stack.empty())
	{
		const BaseClass* edge = stack.back().second;
		if (edge == nullptr)
		{
			order.push_back(stack.back().first);
			stack.pop_back();
			continue;
		}
		stack.back().second = edge->next;
		if (edge->base == nullptr || edge->base->kind != EntityKind::Class ||
			base_path_memo_.find(edge->base) != PointerMap::kMissing)
			continue;
		const ClassEntity* next = static_cast<const ClassEntity*>(edge->base);
		base_path_memo_.set(next, 0);
		stack.push_back(std::make_pair(next, next->bases));
	}
	graph_.base_steps += order.size();
	static const size_t kBudget = 1u << 18;
	if (base_table_.size() + order.size() > kBudget)
	{
		base_tables_.set(cls, 2);
		return false;
	}
	const size_t count = order.size();
	for (size_t k = 0; k < count; ++k)
		base_path_memo_.set(order[k], static_cast<uint32_t>(count - 1 - k));
	std::vector<uint8_t>& facts = table_facts_;
	facts.assign(count, 0);
	facts[0] = 1 | kNonVirtualBasePath;
	for (size_t k = count; k-- > 0;)
	{
		const uint8_t above = facts[count - 1 - k];
		for (const BaseClass* edge = order[k]->bases; edge != nullptr; edge = edge->next)
		{
			if (edge->base == nullptr || edge->base->kind != EntityKind::Class)
				continue;
			uint8_t& below = facts[base_path_memo_.find(edge->base)];
			uint32_t paths = below & kBasePathCount;
			uint32_t flags = below & ~kBasePathCount;
			if (edge->is_virtual)
			{
				paths += (flags & kCountedVirtualBase) != 0 ? 0 : 1;
				flags |= kVirtualBasePath | kCountedVirtualBase;
			}
			else
			{
				paths += above & kBasePathCount;
				flags |= above & (kVirtualBasePath | kNonVirtualBasePath);
			}
			below = static_cast<uint8_t>((paths < 2 ? paths : 2) | flags);
		}
	}
	for (size_t k = 0; k + 1 < count; ++k)
		base_table_.insert(
			EntityPair{ cls, order[k] }, static_cast<uint8_t>(facts[count - 1 - k] & ~kCountedVirtualBase));
	base_tables_.set(cls, 1);
	return true;
}

uint32_t Analyzer::base_paths(
	const ClassEntity* base, const ClassEntity* cls, PointerMap& memo, uint32_t& shared, uint32_t location)
{
	// The paths from `cls` to its base class `base`: how many pass through
	// no virtual base-specifier (each a distinct subobject; counted up to
	// 2, kNonVirtualBasePath with any) and whether any passes through one
	// (kVirtualBasePath), once per class of the hierarchy (memo), so a
	// lattice costs its edges. `shared` adds (up to 2) the subobjects in the
	// virtual bases met: each one's paths, once (its memo entry marked;
	// `base`'s entry is read by no walk).
	const uint32_t known = memo.find(cls);
	if (known != PointerMap::kMissing)
		return known & ~kCountedVirtualBase;
	check_stack(location);
	++graph_.base_steps;
	uint32_t count = 0, facts = 0;
	for (const BaseClass* node = cls->bases; node != nullptr; node = node->next)
	{
		if (node->base == nullptr || node->base->kind != EntityKind::Class)
			continue;
		const ClassEntity* next = static_cast<const ClassEntity*>(node->base);
		const uint32_t below = next == base ? 1 | kNonVirtualBasePath : base_paths(base, next, memo, shared, location);
		if (!node->is_virtual)
		{
			count += below & kBasePathCount;
			facts |= below & (kVirtualBasePath | kNonVirtualBasePath);
			continue;
		}
		if (below == 0)
			continue;
		facts |= kVirtualBasePath;
		const uint32_t mark = memo.find(next);
		if (mark != PointerMap::kMissing && (mark & kCountedVirtualBase) != 0)
			continue;
		shared += below & kBasePathCount;
		shared = shared < 2 ? shared : 2;
		memo.set(next, below | kCountedVirtualBase);
	}
	facts |= count < 2 ? count : 2;
	memo.set(cls, facts);
	return facts;
}

const Type* Analyzer::cv_combined(const Type* a, const Type* b)
{
	// Of similar pointer and member pointer types (4.4p4: the same levels,
	// member pointers of one class, down to one type): the type whose
	// qualifiers below each level are both types' there, with const added
	// at every level above one where they differ (the composite type of
	// 5.9p2 and 5.10p2, as CWG 1512 states it so both convert to it); null
	// if not similar.
	SmallList<const Type*, 8> levels;
	SmallList<uint8_t, 8> qualifiers;
	a = a->unqualified;
	b = b->unqualified;
	while ((a->kind == TypeKind::Pointer || a->kind == TypeKind::MemberPointer) && a->kind == b->kind)
	{
		if (a->kind == TypeKind::MemberPointer && a->owner != b->owner)
			return nullptr;
		const uint8_t united = a->element->cv | b->element->cv;
		if (united != a->element->cv || united != b->element->cv)
		{
			for (size_t k = 0; k < qualifiers.size(); ++k)
				qualifiers[k] = static_cast<uint8_t>(qualifiers[k] | kConst);
		}
		levels.push_back(a);
		qualifiers.push_back(united);
		a = a->element->unqualified;
		b = b->element->unqualified;
	}
	if (levels.empty() || a != b || a->kind == TypeKind::Array)
		return nullptr;
	const Type* result = a;
	for (size_t k = levels.size(); k-- > 0;)
	{
		const Type* element = types_.qualified(result, qualifiers[k]);
		result = levels[k]->kind == TypeKind::Pointer ? types_.pointer(element)
													  : types_.member_pointer(levels[k]->owner, element);
	}
	return result;
}

const Type* Analyzer::composite_pointer(const ExprFacts* a, const ExprFacts* b, bool comparison)
{
	// 5.9p2 for the procedural slice: a null pointer constant takes the
	// other operand's type; otherwise the operands are similar pointers (or
	// one is void*, or they point to classes related by derivation) and the
	// result is qualified by both at the first level below the top.
	const Type* ta = a->type->kind == TypeKind::Array || a->type->kind == TypeKind::Function
		? types_.pointer(a->type->kind == TypeKind::Array ? a->type->element : a->type)
		: a->type->unqualified;
	const Type* tb = b->type->kind == TypeKind::Array || b->type->kind == TypeKind::Function
		? types_.pointer(b->type->kind == TypeKind::Array ? b->type->element : b->type)
		: b->type->unqualified;
	const bool a_null = (a->flags & kNullLiteral) != 0 || IsNullptrType(ta);
	const bool b_null = (b->flags & kNullLiteral) != 0 || IsNullptrType(tb);
	if (a_null && b_null)
		return IsNullptrType(ta) ? ta : tb;
	if (a_null)
		return IsPointer(tb) || IsMemberPointer(tb) ? tb : nullptr;
	if (b_null)
		return IsPointer(ta) || IsMemberPointer(ta) ? ta : nullptr;
	if (ta == tb)
		return ta;
	if (IsMemberPointer(ta) && IsMemberPointer(tb))
	{
		// 5p13: pointers to members of classes related by derivation take
		// the more derived class (4.11p2).
		const ClassEntity* ca = ClassOf(ta->owner);
		const ClassEntity* cb = ClassOf(tb->owner);
		if (ca != cb && ca != nullptr && cb != nullptr)
		{
			if (conversions_.is_base_of(ca, cb))
				ta = types_.member_pointer(tb->owner, ta->element);
			else if (conversions_.is_base_of(cb, ca))
				tb = types_.member_pointer(ta->owner, tb->element);
			else
				return nullptr;
			if (ta == tb)
				return ta;
		}
		if (ConversionRules::qualification_convertible(ta, tb))
			return tb;
		if (ConversionRules::qualification_convertible(tb, ta))
			return ta;
		// 5.10p2: compared, both qualifiers (5.16p6: as ?: operands, the
		// common type's are one operand's).
		return comparison ? cv_combined(ta, tb) : nullptr;
	}
	if (!IsPointer(ta) || !IsPointer(tb))
		return nullptr;
	const Type* pa = ta->element;
	const Type* pb = tb->element;
	const uint8_t cv = pa->cv | pb->cv;
	if (IsVoid(pa) || IsVoid(pb))
	{
		if ((IsVoid(pa) || pa->kind != TypeKind::Function) && (IsVoid(pb) || pb->kind != TypeKind::Function))
			return types_.pointer(types_.qualified(types_.fundamental(FundamentalType::Void), cv));
		return nullptr;
	}
	if (pa->unqualified == pb->unqualified)
		return types_.pointer(types_.qualified(pa->unqualified, cv));
	if (ConversionRules::qualification_convertible(ta, tb))
		return tb;
	if (ConversionRules::qualification_convertible(tb, ta))
		return ta;
	const ClassEntity* ca = ClassOf(pa);
	const ClassEntity* cb = ClassOf(pb);
	if (ca != nullptr && cb != nullptr)
	{
		if (conversions_.is_base_of(ca, cb))
			return types_.pointer(types_.qualified(pa->unqualified, cv));
		if (conversions_.is_base_of(cb, ca))
			return types_.pointer(types_.qualified(pb->unqualified, cv));
	}
	// Similar multi-level pointers: both qualifiers at each level.
	return cv_combined(ta, tb);
}

} // namespace sema
