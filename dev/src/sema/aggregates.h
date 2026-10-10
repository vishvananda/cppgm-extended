#pragma once

#include "sema/conversions.h"
#include "syntax/ast.h"

// The structure of aggregate initialization (8.5.1) shared by its analysis
// and its lowering: which list element initializes which member.
namespace sema {

// 8.5.1p1: no user-provided constructors, no private or protected
// non-static data members, no brace-or-equal-initializers, no base classes
// and no virtual functions.
inline bool IsAggregateClass(const ClassEntity* entity)
{
	return !entity->not_aggregate && entity->bases == nullptr && !entity->polymorphic;
}

// A member's brace-or-equal-initializer, or one of an anonymous struct's or
// union's member object's members' (which initializes that object, 9.5p5,
// 12.6.2p8: the object is then the variant member with an initializer,
// 9.5p8).
inline bool MemberInitialized(const DataMember* member)
{
	if (member->variable->kind != EntityKind::Variable)
		return false;
	const VariableEntity* variable = static_cast<const VariableEntity*>(member->variable);
	if (variable->member_initializer != nullptr)
		return true;
	const ClassEntity* cls = variable->name == nullptr && !member->bit_field ? ClassOf(variable->type) : nullptr;
	return cls != nullptr && cls->anonymous && cls->members_initialized;
}

// The member an aggregate union's list initializes: its first named one
// (8.5.1p15), from an empty list the variant member with a brace-or-equal-
// initializer if there is one (CWG 1460; in an aggregate only an anonymous
// member object's members have them). Null for a union without members.
inline const DataMember* UnionListMember(const ClassEntity* entity, bool empty)
{
	const DataMember* first = nullptr;
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		if (member->bit_field && member->variable->name == nullptr)
			continue;
		if (!empty)
			return member;
		if (MemberInitialized(member))
			return member;
		if (first == nullptr)
			first = member;
	}
	return first;
}

// The member a union's list initializes: the one a designated list named
// (`{ .m = x }`, see syntax::BracedInitList::union_member), else
// UnionListMember's.
inline const DataMember* ListUnionMember(const ClassEntity* entity, const syntax::BracedInitList* list, uint32_t next)
{
	if (list != nullptr && list->union_member != nullptr && list->union_member->variable->owner == entity->scope)
		return list->union_member;
	return UnionListMember(entity, list == nullptr || next >= list->elements.size);
}

// An array or an aggregate class: a member whose initializer may omit its
// braces (8.5.1p11).
inline bool IsSubaggregate(const Type* type)
{
	if (type->is_reference())
		return false;
	if (type->kind == TypeKind::Array)
		return true;
	const ClassEntity* entity = ClassOf(type);
	return entity != nullptr && IsAggregateClass(entity);
}

// 8.5.1p11: an element that is an expression initializes a subaggregate
// member itself when it has the member's class or, for a character array,
// is a string literal; otherwise the member's elements come from the
// enclosing list (brace elision). The element's facts must be known.
inline bool InitializesSubaggregate(const Type* member, const syntax::Expr& element)
{
	const syntax::Expr* e = &element;
	while (e->kind == syntax::ExprKind::Paren)
		e = static_cast<const syntax::ParenExpr*>(e)->inner;
	const Type* type = e->facts != nullptr ? e->facts->type : nullptr;
	if (type == nullptr)
		return false;
	if (member->kind == TypeKind::Array)
		return e->kind == syntax::ExprKind::Literal && type->kind == TypeKind::Array;
	return ClassOf(type) != nullptr && ClassOf(type) == ClassOf(member);
}

// GCC's list of a complex value's two parts, `{re, im}`: each part a
// part-typed scalar's element, the value built part by part as a vector's
// lanes are (a list of one element or none initializes it as a scalar's).
inline bool PartList(const Type* type, const syntax::BracedInitList& list)
{
	return !type->is_reference() && IsComplex(type) && list.elements.size == 2;
}

// A list giving a value's lanes or parts one by one: a GNU vector's, or a
// complex value's two parts (see PartList).
inline bool LaneList(const Type* type, const syntax::BracedInitList& list)
{
	return !type->is_reference() && (type->unqualified->kind == TypeKind::Vector || PartList(type, list));
}

} // namespace sema
