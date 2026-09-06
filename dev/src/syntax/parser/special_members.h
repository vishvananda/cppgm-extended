#ifndef CPPGM_SYNTAX_PARSER_SPECIAL_MEMBERS_H
#define CPPGM_SYNTAX_PARSER_SPECIAL_MEMBERS_H
#include "syntax/model/arena.h"
#include <cctype>
#include <cstddef>
#include <string>
#include <vector>
namespace cppgm
{
namespace syntax
{
// Constructors, destructors and conversion functions: the member
// declarations whose declarator names the class itself (or `operator`),
// parsed speculatively from a class body or an out-of-class definition.
template <class Derived>
class SpecialMemberSyntax
{
protected:
	NodeId ParseSpecialMember(bool)
	{
		Derived& parser = static_cast<Derived&>(*this);
		const auto mark = parser.Checkpoint();
		const std::vector<NodeId> specifiers = parser.ParseSpecialMemberSpecifiers();
		parser.SkipAttributes();
		const std::size_t name_start = parser.position_;
		std::string name;
		TextId terminal_identifier = 0;
		NodeId name_structure = kNoNode;
		if (!parser.ParseName(&name, true, true, true, &name_structure,
			&terminal_identifier) ||
			!parser.At(OP_LPAREN))
		{
			parser.Rollback(mark);
			return kNoNode;
		}
		const bool qualified = name.find("::") != std::string::npos;
		bool special_name = name.find("operator") != std::string::npos;
		if (!special_name && qualified && name_structure != kNoNode)
		{
			std::vector<TextId> components;
			for (std::uint32_t edge = parser.arena_.FirstEdge(name_structure);
				edge != kNoEdge; edge = parser.arena_.NextEdge(edge))
			{
				const NodeId child = parser.arena_.EdgeChild(edge);
				if (parser.arena_.IsTag(child, ::cppgm::syntax::STAG_NAME_COMPONENT))
					components.push_back(parser.arena_.SemanticPayloadId(child));
			}
			if (components.size() > 1)
			{
				const TextId owner = components[components.size() - 2];
				const TextId terminal = components.back();
				special_name = terminal == owner || terminal == parser.strings_.Intern(
					"~" + parser.strings_.Get(owner));
			}
		}
		else if (!special_name && !parser.current_classes_.empty())
		{
			special_name = terminal_identifier == parser.current_classes_.back();
		}
		if (!special_name)
		{
			parser.Rollback(mark);
			return kNoNode;
		}
		if (qualified)
		{
			const std::size_t op = name.rfind("::operator");
			if (op != std::string::npos)
			{
				const std::size_t after = op + std::string("::operator").size();
				if (after < name.size() &&
					name[after] != ' ' &&
					!std::isalnum(static_cast<unsigned char>(name[after])) &&
					name[after] != '_')
				{
					parser.Rollback(mark);
					return kNoNode;
				}
				if (after < name.size() && name[after] != ' ')
					name.insert(after, " ");
			}
		}
		parser.position_ = name_start;
		TextId declarator_name = 0;
		const NodeId declarator = parser.ParseDeclarator(false, &declarator_name);
		if (declarator == kNoNode)
		{
			parser.Rollback(mark);
			return kNoNode;
		}
		// The declarator payload is the exact serialized spelling; this branch
		// still adjusts operator spacing for the special-member payload text.
		std::string declarator_spelling = parser.strings_.Get(declarator_name);
		if (declarator_spelling.find("operator ") != std::string::npos)
			name = declarator_spelling;
		if (qualified)
		{
			const std::size_t op = declarator_spelling.rfind("::operator");
			if (op != std::string::npos)
			{
				const std::size_t after = op + std::string("::operator").size();
				if (after < declarator_spelling.size() &&
					std::isalnum(static_cast<unsigned char>(
						declarator_spelling[after])))
					declarator_spelling.insert(after, " ");
			}
			name = declarator_spelling;
		}
		NodeId ctor_initializer = kNoNode;
		const bool function_try = parser.At(KW_TRY);
		if (!function_try && parser.At(OP_COLON)) ctor_initializer = parser.ParseCtorInitializer();
		const bool has_body = parser.At(OP_LBRACE) || function_try;
		const bool is_declaration = parser.At(OP_SEMICOLON) || parser.At(OP_ASS);
		if (!has_body && !is_declaration)
		{
			parser.Rollback(mark);
			return kNoNode;
		}
		const NodeId member = parser.arena_.Make(has_body ?
			"special-member-definition" : "special-member-declaration", name);
		if (!specifiers.empty())
		{
			const NodeId set = parser.arena_.Make("member-specifiers");
			for (std::size_t i = 0; i < specifiers.size(); ++i)
				parser.arena_.Add(set, specifiers[i]);
			parser.arena_.Add(member, set);
		}
		parser.arena_.Add(member, declarator);
		if (ctor_initializer != kNoNode) parser.arena_.Add(member, ctor_initializer);
		if (parser.Match(OP_ASS))
		{
			const NodeId initializer = parser.arena_.Make("initializer");
			if (parser.Match(KW_DEFAULT))
				parser.arena_.Add(initializer,
					parser.arena_.Make("special-initializer", "default"));
			else if (parser.Match(KW_DELETE))
				parser.arena_.Add(initializer,
					parser.arena_.Make("special-initializer", "delete"));
			else
			{
				const NodeId value = parser.ParseExpression(2);
				if (value == kNoNode)
					throw parser.Error("expected special member initializer");
				parser.arena_.Add(initializer, value);
			}
			parser.arena_.Add(member, initializer);
			parser.Expect(OP_SEMICOLON);
			return member;
		}
		if (parser.Match(OP_SEMICOLON)) return member;
		const std::size_t parameter_fact_mark = parser.name_fact_changes_.size();
		parser.ApplyFunctionParameterFacts(declarator);
		const NodeId body = function_try ? parser.ParseFunctionTryBlock(true) :
			parser.ParseCompoundStatement();
		parser.RestoreNameFacts(parameter_fact_mark);
		if (body == kNoNode) throw parser.Error("expected special member body");
		parser.arena_.Add(member, body);
		return member;
	}
};
}
}
#endif
