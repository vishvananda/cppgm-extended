#pragma once

#include <cstdint>

#include "syntax/ast.h"
#include "syntax/name_scopes.h"

// The parser's record of names whose category the specializations of the
// template they are members of disagree on (NameBinding::conflicted, see
// NameScopes::add_specialization), for the statements read once per
// category (see Parser::parse_declaration_or_expression_statement).
namespace syntax {

inline ReadingCategory ReadingOf(NameCategory category)
{
	return category == NameCategory::TypeTemplate || category == NameCategory::FunctionTemplate
		? ReadingCategory::Template
		: category == NameCategory::Type ? ReadingCategory::Type
		: category == NameCategory::None ? ReadingCategory::Unknown
										 : ReadingCategory::Value;
}

// The first such name of the statement being parsed: the name, its
// component, the category the reading took it with and its other ones (a
// CategoryBit each).
struct StatementConflict
{
	const Name* name;
	uint32_t component;
	NameCategory category;
	uint8_t alternatives;
};

struct NameConflicts
{
	// Lookups that found such a name; the category such names take where
	// they have it (a statement's other reading; None: the found one); the
	// last one's category and other ones.
	uint32_t uses = 0;
	NameCategory take = NameCategory::None;
	// The categories the ones the statement being parsed met could take
	// besides their found ones.
	uint8_t seen = 0;
	NameCategory last = NameCategory::None;
	uint8_t last_alternatives = 0;
	StatementConflict statement = StatementConflict{ nullptr, 0, NameCategory::None, 0 };
	// The component of the name being parsed that was one (-1: none), and
	// its categories.
	int32_t name_component = -1;
	NameCategory name_category = NameCategory::None;
	uint8_t name_alternatives = 0;
};

// A region whose names take their found categories (a statement, or a body
// inside a statement read again), with its own conflicted names.
class ConflictScope
{
public:
	explicit ConflictScope(NameConflicts& conflicts)
		: conflicts_(conflicts), take_(conflicts.take), seen_(conflicts.seen), statement_(conflicts.statement)
	{
		conflicts.take = NameCategory::None;
		conflicts.seen = 0;
		conflicts.statement.name = nullptr;
	}
	~ConflictScope()
	{
		conflicts_.take = take_;
		conflicts_.seen = seen_;
		conflicts_.statement = statement_;
	}
	ConflictScope(const ConflictScope&) = delete;
	ConflictScope& operator=(const ConflictScope&) = delete;

private:
	NameConflicts& conflicts_;
	NameCategory take_;
	uint8_t seen_;
	StatementConflict statement_;
};

} // namespace syntax
