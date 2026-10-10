#include "lower/program_lowering.h"

#include <algorithm>
#include <cstring>
#include <vector>

#include "lex/identifier_table.h"
#include "sema/analyzer.h"
#include "sema/placeholder_words.h"
#include "sema/template_records.h"

// The placeholders of a function template's signature in its
// specializations' names (Itanium 5.1.5, 5.1.6): a decltype-specifier
// (DT <expression> E, Dt for an id-expression or member access), a
// value-dependent template argument (X <expression> E) and a dependent array
// bound (A <expression> _) are mangled as the expression they stand for, so
// distinct templates (14.5.6.1p5-p6) get distinct symbols and one template
// gets one in every unit. The expression is read from the structural key
// the analysis kept on the placeholder (sema/placeholder_words.h), whose
// words record each name's resolution: nothing is looked up again. An
// integral constant is its literal (L <type> <value> E), a dependent callee
// its unresolved name, a non-static member named through `this` a member
// access of *this (dt de fpT).
//
// A key the reader cannot mangle leaves the placeholder to its
// specialization's own type (the caller's fallback): an operand the
// analysis did not key (a lambda, a destructor or conversion name, a
// body's local bound; a placeholder substituted with dependent arguments
// and not re-keyed, one naming parameters of several template heads or
// `this`); a string, long double or user-defined literal;
// a qualified member name, an operator member name, a non-static member
// named outside a trailing return type; a pointer or reference template
// argument's constant; and a decltype operand naming no dependent entity or
// type (the host compilers' type, 5.1.6).
namespace lower {

namespace {

using abi_mangle::ABI_NONE;
using abi_mangle::AbiNode;

uint16_t Code(char first, char second)
{
	return abi_mangle::abi_pack_operation_code(first, second);
}

uint16_t TerminalCode(abi_mangle::AbiTerminalKind kind)
{
	const char* code = abi_mangle::abi_terminal_code(kind);
	return code != nullptr && code[0] != '\0' && code[1] != '\0' && code[2] == '\0' ? Code(code[0], code[1]) : 0;
}

// A binary operator's code (5.1.3): its operator function's, `.*` ds.
uint16_t BinaryCode(TokenKind op)
{
	if (op == TokenKind::OP_DOTSTAR)
		return Code('d', 's');
	const abi_mangle::AbiTerminalKind kind = OperatorTerminal(op, false);
	return kind == abi_mangle::ABI_TERMINAL_NONE ? 0 : TerminalCode(abi_mangle::abi_terminal_for_operands(kind, 2));
}

const sema::Type* TypeWord(uint64_t word)
{
	return reinterpret_cast<const sema::Type*>(static_cast<uintptr_t>(word));
}

const sema::Entity* EntityWord(uint64_t word)
{
	return reinterpret_cast<const sema::Entity*>(static_cast<uintptr_t>(word));
}

const Identifier* IdentifierWord(uint64_t word)
{
	return reinterpret_cast<const Identifier*>(static_cast<uintptr_t>(word));
}

// A name's first component as the key recorded it; `self` when it names a
// non-static member through `(*this)` (kWordThis before it).
struct NameStart
{
	uint64_t tag;
	uint64_t word;
	bool self;
};

} // namespace

class ProgramLowering::PlaceholderReader
{
public:
	PlaceholderReader(ProgramLowering& lowering, const sema::TemplateParameterEntity* placeholder)
		: lowering_(lowering), graph_(lowering.abi_graph_), placeholder_(placeholder),
		  at_(placeholder->key + 1 + sema::kPlaceholderKeyPrefix), end_(placeholder->key + 1 + placeholder->key[0]),
		  failed_(false)
	{}

	AbiNode operand()
	{
		const AbiNode node = expression(0);
		return failed_ || at_ != end_ ? ABI_NONE : node;
	}

private:
	uint64_t next()
	{
		if (at_ == end_)
		{
			failed_ = true;
			return sema::kWordAbsent;
		}
		return *at_++;
	}

	uint64_t peek_tag() const { return at_ == end_ ? sema::kWordAbsent : sema::WordTag(*at_); }

	AbiNode fail()
	{
		failed_ = true;
		return ABI_NONE;
	}

	AbiNode string(const Identifier* identifier) { return graph_.string(identifier->spelling, identifier->length); }

	// A type the key names; a placeholder keyed as naming nothing dependent
	// (an instance's member template's `decltype(T())`, T known), which
	// the key does not mangle, leaves the operand to its specialization's
	// type, as the placeholder's own use does.
	AbiNode type(uint64_t word)
	{
		const sema::Type* type = TypeWord(word);
		if (type == nullptr)
			return fail();
		const sema::TemplateParameterEntity* placeholder = type->unqualified->kind == sema::TypeKind::TemplateParameter
			? static_cast<const sema::TemplateParameterEntity*>(type->unqualified->entity)
			: nullptr;
		if (placeholder != nullptr && placeholder->expression != nullptr && placeholder->key != nullptr &&
			placeholder->key[0] > sema::kPlaceholderKeyPrefix && (placeholder->key[1] & 1) == 0)
			return fail();
		return lowering_.abi_type(type);
	}

	// A cast's type as Clang mangles it: the cast expression's (5p5, 5p6),
	// not the type written: a reference's referenced type, a non-class
	// prvalue's without its cv-qualifiers when it is not dependent.
	AbiNode expression_type(uint64_t word)
	{
		const sema::Type* written = TypeWord(word);
		if (written == nullptr)
			return fail();
		const sema::Type* result = written->is_reference() ? written->element : written;
		if (!written->is_reference() && result->cv != 0 && FixedScalar(result->unqualified))
			result = result->unqualified;
		return type(static_cast<uint64_t>(reinterpret_cast<uintptr_t>(result)));
	}

	// Whether a type is a scalar type no template parameter is part of.
	static bool FixedScalar(const sema::Type* type)
	{
		switch (type->kind)
		{
		case sema::TypeKind::Fundamental:
		case sema::TypeKind::Enum:
			return true;
		case sema::TypeKind::Pointer:
		{
			const sema::Type* pointee = type->element->unqualified;
			while (pointee->kind == sema::TypeKind::Pointer)
				pointee = pointee->element->unqualified;
			return pointee->kind == sema::TypeKind::Fundamental || pointee->kind == sema::TypeKind::Enum ||
				(pointee->kind == sema::TypeKind::Class &&
					static_cast<const sema::ClassEntity*>(pointee->entity)->templ == nullptr);
		}
		default:
			return false;
		}
	}

	// A component's template arguments (kWordArguments), on the scratch
	// stack from its current top; their count. A qualifier's are skipped
	// (`build` false: its resolution names the qualifier).
	uint32_t arguments(bool build = true)
	{
		if (peek_tag() != sema::kWordArguments)
			return 0;
		const uint32_t count = static_cast<uint32_t>(sema::WordValue(next()));
		for (uint32_t i = 0; i < count && !failed_; ++i)
		{
			const uint64_t word = next();
			if (build)
				lowering_.abi_scratch_.push_back(type(word));
		}
		return count;
	}

	// Whether the template arguments (kWordArguments) at `at` name a
	// template parameter.
	bool dependent_arguments(const uint64_t* at) const
	{
		if (at == end_ || sema::WordTag(*at) != sema::kWordArguments)
			return false;
		const uint64_t count = sema::WordValue(*at++);
		for (uint64_t i = 0; i < count && at != end_; ++i, ++at)
		{
			const sema::Type* argument = TypeWord(*at);
			if (argument != nullptr && lowering_.analyzer_->dependent(argument))
				return true;
		}
		return false;
	}

	// sizeof or alignof of a type that is not dependent: Itanium 5.1.6 (and
	// Clang) encode its value as an integer literal of std::size_t's type;
	// another type's operator its operation (st, at).
	AbiNode type_query(uint16_t code, uint64_t word, bool size)
	{
		const sema::Type* operand = TypeWord(word);
		if (operand != nullptr && !lowering_.analyzer_->dependent(operand) && Sized(operand))
		{
			const uint64_t value = size ? lowering_.size_of(operand, 0) : lowering_.alignment_of(operand, 0);
			return graph_.literal(
				graph_.builtin(abi_mangle::ABI_BUILTIN_TYPE_UNSIGNED_LONG), static_cast<long long>(value));
		}
		return graph_.type_operation(code, type(word));
	}

	// A complete object type whose size the lowering knows.
	static bool Sized(const sema::Type* type)
	{
		while (type->kind == sema::TypeKind::Array && type->bound != 0)
			type = type->element;
		switch (type->unqualified->kind)
		{
		case sema::TypeKind::Fundamental:
			return type->unqualified->fundamental != FundamentalType::Void;
		case sema::TypeKind::Pointer:
		case sema::TypeKind::MemberPointer:
		case sema::TypeKind::Enum:
		case sema::TypeKind::LValueReference:
		case sema::TypeKind::RValueReference:
			return true;
		case sema::TypeKind::Class:
			return static_cast<const sema::ClassEntity*>(type->unqualified->entity)->complete;
		default:
			return false;
		}
	}

	// An integral constant (kWordConstant): L <type> <value> E, as the host
	// compilers mangle a literal and a substituted constant argument.
	AbiNode constant()
	{
		const sema::Type* constant_type = TypeWord(next());
		const uint64_t bits = next();
		if (failed_ || constant_type == nullptr)
			return fail();
		return lowering_.abi_literal(constant_type, bits);
	}

	AbiNode literal(uint64_t head)
	{
		// The spelling's words, then the type and value the analysis gave it.
		const uint64_t spelling = (sema::WordValue(head) + 7) / 8;
		if (spelling + 2 > static_cast<uint64_t>(end_ - at_))
			return fail();
		at_ += spelling;
		const sema::Type* literal_type = TypeWord(next());
		const uint64_t bits = next();
		if (literal_type == nullptr || literal_type->kind != sema::TypeKind::Fundamental)
			return fail();
		const AbiNode node = lowering_.abi_type(literal_type);
		if (literal_type->fundamental == FundamentalType::Float || literal_type->fundamental == FundamentalType::Double)
			return graph_.floating_literal(node, bits);
		return graph_.literal(node, static_cast<long long>(bits));
	}

	// An entity a name denotes, as an expression.
	AbiNode entity(const sema::Entity* entity)
	{
		switch (entity->kind)
		{
		case sema::EntityKind::TemplateParameter:
		{
			// A non-type parameter (T_), a value placeholder a frame bound
			// the name to (its expression), a dependent member (sr).
			const sema::TemplateParameterEntity* parameter = static_cast<const sema::TemplateParameterEntity*>(entity);
			if (parameter->expression != nullptr)
				return parameter != placeholder_ ? lowering_.placeholder_expression(parameter) : fail();
			if (parameter->qualifier != nullptr)
				return graph_.scoped_member(
					lowering_.unresolved_prefix(parameter->qualifier), string(parameter->name), false);
			return parameter->non_type ? graph_.template_param(parameter->index, false, lowering_.own_level(parameter))
									   : fail();
		}
		case sema::EntityKind::Variable:
		{
			// A non-static member named in a trailing return type is
			// (*this).m (5.1.1p3, 9.3.1p3).
			const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(entity);
			if (!variable->member)
				return graph_.entity_reference(lowering_.variable_name(variable));
			if (placeholder_->this_type == nullptr)
				return fail();
			const AbiNode self = graph_.this_param();
			const AbiNode object = graph_.operation(Code('d', 'e'), &self, 1);
			return graph_.member_access(Code('d', 't'), object, string(variable->name), nullptr, 0);
		}
		case sema::EntityKind::Enumerator:
		{
			// An enumerator is a literal of its enumeration; a frame's
			// constant argument (a non-type parameter's name) of its type.
			const sema::EnumeratorEntity* enumerator = static_cast<const sema::EnumeratorEntity*>(entity);
			if (enumerator->argument != nullptr)
				return fail();
			const sema::Type* literal_type = enumerator->enumeration != nullptr ? enumerator->enumeration->type
																				: enumerator->value.type;
			return lowering_.abi_literal(literal_type, enumerator->value.bits);
		}
		default:
			return fail();
		}
	}

	// An id-expression (after its node word): its name's components and
	// resolution. `call_arguments` is a callee's argument count (an
	// operator-function-id's arity), or -1. A name that denotes a type sets
	// `named_type` (a functional cast's callee).
	AbiNode name(int32_t call_arguments, AbiNode* named_type)
	{
		const uint64_t header = next();
		if (sema::WordTag(header) != sema::kWordName)
			return fail();
		const uint32_t count = static_cast<uint32_t>(sema::WordValue(header) >> 1);
		const bool global = (sema::WordValue(header) & 1) != 0;
		if (count == 0)
			return fail();
		// The first component, after the `this` a non-static member is named
		// through (its type, which the mangling does not spell).
		uint64_t first = next();
		const bool self = !global && sema::WordTag(first) == sema::kWordThis;
		if (self)
		{
			next();
			first = next();
		}
		NameStart start = NameStart{ 0, 0, self };
		const uint64_t tag = sema::WordTag(first);
		if (global || (first & 7) == 0)
			start = NameStart{ 0, first, self };
		else if (tag == sema::kWordName)
			start = NameStart{ sema::kWordName, first, self };
		else if (tag == sema::kWordParameter)
		{
			start = NameStart{ tag, first, self };
			next();
		}
		else if (tag == sema::kWordFreeName || tag == sema::kWordEntity || tag == sema::kWordType ||
			tag == sema::kWordFunction)
			start = NameStart{ tag, next(), self };
		else
			return fail();
		if (count == 1 && !global)
			return unqualified(start, call_arguments, named_type);
		// A qualified name: its components (the last one's arguments kept),
		// then what it denotes.
		const size_t base = lowering_.abi_scratch_.size();
		const uint64_t* qualifier_at = at_;
		uint32_t last_arguments = arguments(count == 1);
		uint64_t last = start.word;
		for (uint32_t i = 1; i < count && !failed_; ++i)
		{
			last = next();
			last_arguments = arguments(i + 1 == count);
		}
		const uint64_t resolution = next();
		AbiNode node = ABI_NONE;
		if (failed_ || (last & 7) != 0 || last == 0)
			node = fail();
		else if (sema::WordTag(resolution) == sema::kWordType)
		{
			// A dependent qualifier (5.1.6: sr ... <base-unresolved-name>),
			// its components as the source wrote them.
			const uint64_t qualifier_type = next();
			const uint64_t* resume = at_;
			at_ = qualifier_at;
			const AbiNode prefix = qualifier(start, global, count - 1, qualifier_type);
			at_ = resume;
			node = failed_ ? ABI_NONE
						   : graph_.scoped_member(prefix, string(IdentifierWord(last)), false,
								 lowering_.abi_scratch_.data() + base, last_arguments);
		}
		else if (sema::WordTag(resolution) == sema::kWordFunction)
		{
			// Functions a qualified name names: one that is no template, its
			// entity (L <encoding> E, as the hosts mangle it); an overload
			// set, its qualifier as written, as Clang mangles it (sr2nsE1f,
			// sr1AE1g, gs1f); through `this` or a template parameter (whose
			// members a key resolves as a dependent qualifier's), GCC's
			// unqualified name.
			const sema::Entity* function = sema::WordValue(resolution) == 1 ? EntityWord(next()) : nullptr;
			const bool parameter = start.tag == sema::kWordEntity &&
				EntityWord(start.word)->kind == sema::EntityKind::TemplateParameter;
			AbiNode prefix = ABI_NONE;
			if (function != nullptr && !start.self && !parameter)
				node = failed_ ? ABI_NONE
							   : graph_.entity_reference(lowering_.function_encoding(
									 static_cast<const sema::FunctionEntity*>(function), EntryVariant::Complete));
			else
			{
				if (count == 1 && global)
					prefix = graph_.unresolved_prefix(ABI_NONE, nullptr, 0, true);
				else if (!start.self && !parameter)
				{
					const uint64_t* resume = at_;
					at_ = qualifier_at;
					prefix = qualifier(start, global, count - 1, 0);
					at_ = resume;
				}
				const AbiNode member = string(IdentifierWord(last));
				node = failed_ ? ABI_NONE
					: prefix == ABI_NONE
					? graph_.unresolved_name(member, lowering_.abi_scratch_.data() + base, last_arguments)
					: graph_.scoped_member(prefix, member, false, lowering_.abi_scratch_.data() + base, last_arguments);
			}
		}
		else if (sema::WordTag(resolution) == sema::kWordEntity)
		{
			const sema::Entity* denoted = EntityWord(next());
			if (denoted->kind == sema::EntityKind::Class || denoted->kind == sema::EntityKind::Enum ||
				denoted->kind == sema::EntityKind::Typedef)
			{
				if (named_type == nullptr)
					node = fail();
				else
					*named_type = lowering_.abi_type(denoted->type);
			}
			else
				node = last_arguments == 0 ? entity(denoted) : fail();
		}
		else
			node = fail();
		lowering_.abi_scratch_.resize(base);
		return node;
	}

	// A qualified name's qualifier of `levels` components (read again from
	// the first one's arguments; `qualifier_type` the dependent type they
	// denote), as Clang mangles it from the nested-name-specifier written
	// (5.1.6): a template parameter, alone (T::x, TT<A>::x) the unresolved
	// type sr <type>, before members srN <type> <level>+ E (T::U::x); any
	// other first component, a namespace, class, class template, alias
	// template or typedef by its name and arguments, a qualifier level, as
	// each later one is: [gs] sr <level>+ E (X<T>::x, ns::X<T>::x,
	// ::X<T>::x, X<T>::type::x). A frame's argument for a type parameter's
	// name (an alias template's C::x of C = X<T>) is the qualifier its type
	// is (see ProgramLowering::qualifier_levels).
	AbiNode qualifier(const NameStart& start, bool global, uint32_t levels, uint64_t qualifier_type)
	{
		if (levels == 0)
			return fail();
		if (start.self)
			return lowering_.unresolved_prefix(TypeWord(qualifier_type));
		const sema::Entity* first = start.tag == sema::kWordEntity ? EntityWord(start.word) : nullptr;
		const bool parameter = !global && first != nullptr && first->kind == sema::EntityKind::TemplateParameter;
		if (parameter && levels == 1)
			return graph_.unresolved_prefix(type(qualifier_type), nullptr, 0, false);
		const size_t base = lowering_.abi_scratch_.size();
		AbiNode unresolved = ABI_NONE;
		if (parameter)
		{
			// The parameter, a template template-parameter's with its
			// arguments (as its specialization's, see name_template_id).
			std::vector<const sema::Type*> given;
			const uint32_t count = peek_tag() == sema::kWordArguments ? static_cast<uint32_t>(sema::WordValue(next()))
																	  : 0;
			for (uint32_t i = 0; i < count && !failed_; ++i)
			{
				given.push_back(TypeWord(next()));
				if (given.back() == nullptr)
					return fail();
			}
			if (failed_)
				return ABI_NONE;
			unresolved = count == 0 ? lowering_.abi_type(first->type)
									: lowering_.name_template_id(first->type, given.data(), count);
		}
		else if (!global && start.tag == sema::kWordType)
		{
			arguments(false);
			const sema::Type* bound = TypeWord(start.word);
			if (bound == nullptr)
				return fail();
			unresolved = lowering_.qualifier_levels(bound);
		}
		else if (global ||
			(first != nullptr && first->name != nullptr && first->kind != sema::EntityKind::Function &&
				first->kind != sema::EntityKind::Variable))
			level(global ? IdentifierWord(start.word) : first->name);
		else
			return fail();
		for (uint32_t i = 1; i < levels && !failed_; ++i)
		{
			const uint64_t word = next();
			if ((word & 7) != 0 || word == 0)
				return fail();
			level(IdentifierWord(word));
		}
		const AbiNode node = failed_ ? ABI_NONE
									 : graph_.unresolved_prefix(unresolved, lowering_.abi_scratch_.data() + base,
										   lowering_.abi_scratch_.size() - base, global);
		lowering_.abi_scratch_.resize(base);
		return node;
	}

	// An <unresolved-qualifier-level> (<simple-id>): a name and the
	// arguments that follow it, onto the scratch stack.
	void level(const Identifier* identifier)
	{
		const size_t base = lowering_.abi_scratch_.size();
		const uint32_t count = arguments();
		const AbiNode node = graph_.unresolved_name(string(identifier), lowering_.abi_scratch_.data() + base, count);
		lowering_.abi_scratch_.resize(base);
		lowering_.abi_scratch_.push_back(node);
	}

	// Functions named by `identifier` (with the template arguments on the
	// scratch stack from `base`): an unresolved name, or through `(*this)`
	// a member access (dt de fpT <name>).
	AbiNode functions(const Identifier* identifier, size_t base, uint32_t count, bool self)
	{
		if (!self)
			return graph_.unresolved_name(string(identifier), lowering_.abi_scratch_.data() + base, count);
		const AbiNode pointer = graph_.this_param();
		const AbiNode object = graph_.operation(Code('d', 'e'), &pointer, 1);
		return graph_.member_access(
			Code('d', 't'), object, string(identifier), lowering_.abi_scratch_.data() + base, count);
	}

	AbiNode unqualified(const NameStart& start, int32_t call_arguments, AbiNode* named_type)
	{
		const size_t base = lowering_.abi_scratch_.size();
		const uint64_t* const arguments_at = at_;
		const uint32_t count = arguments();
		AbiNode node = ABI_NONE;
		switch (start.tag)
		{
		case 0:
		case sema::kWordFreeName:
			// A name found nowhere (argument-dependent lookup), or a dependent
			// callee's functions: an unresolved name.
			node = start.word != 0 ? functions(IdentifierWord(start.word), base, count, start.self) : fail();
			break;
		case sema::kWordName:
		{
			// An operator-function-id callee: on <operator-name>.
			const uint64_t value = sema::WordValue(start.word);
			const abi_mangle::AbiTerminalKind kind = call_arguments < 0
				? abi_mangle::ABI_TERMINAL_NONE
				: abi_mangle::abi_terminal_for_operands(
					  OperatorTerminal(static_cast<TokenKind>(value >> 1), (value & 1) != 0),
					  static_cast<std::size_t>(call_arguments));
			node = kind != abi_mangle::ABI_TERMINAL_NONE && TerminalCode(kind) != 0
				? graph_.unresolved_name(graph_.operator_name(kind), lowering_.abi_scratch_.data() + base, count)
				: fail();
			break;
		}
		case sema::kWordParameter:
			node = count == 0
				? graph_.function_param(sema::ParameterIndex(start.word), sema::ParameterLevel(start.word))
				: fail();
			break;
		case sema::kWordEntity:
		{
			// Functions a non-dependent name finds are named by their name
			// (overload resolution is the substitution's).
			const sema::Entity* denoted = EntityWord(start.word);
			if (denoted->kind == sema::EntityKind::Class || denoted->kind == sema::EntityKind::Enum ||
				denoted->kind == sema::EntityKind::Typedef)
			{
				if (named_type == nullptr || count != 0)
					node = fail();
				else
					*named_type = lowering_.abi_type(denoted->type);
			}
			else if (denoted->kind == sema::EntityKind::Function)
				node = denoted->name != nullptr ? functions(denoted->name, base, count, start.self) : fail();
			else if (count != 0 && denoted->kind == sema::EntityKind::Variable && denoted->name != nullptr &&
				!start.self && dependent_arguments(arguments_at))
			{
				// A variable template's specialization over dependent
				// arguments: its unresolved name (Clang's), the template-id
				// as written.
				node = graph_.unresolved_name(string(denoted->name), lowering_.abi_scratch_.data() + base, count);
			}
			else
				node = count == 0 ? entity(denoted) : fail();
			break;
		}
		case sema::kWordType:
			if (named_type == nullptr || count != 0)
				node = fail();
			else
				*named_type = type(start.word);
			break;
		case sema::kWordFunction:
			// A callee resolved where the operand appears: its function (L
			// <encoding> E, as Clang mangles it), its explicit arguments in
			// its encoding.
			node = start.word != 0 && !start.self
				? graph_.entity_reference(lowering_.function_encoding(
					  static_cast<const sema::FunctionEntity*>(EntityWord(start.word)), EntryVariant::Complete))
				: fail();
			break;
		default:
			node = fail();
			break;
		}
		lowering_.abi_scratch_.resize(base);
		return node;
	}

	// A member access's member (5.1.6: dt/pt <expression> <unresolved-name>):
	// one identifier, with its template arguments.
	AbiNode member(uint16_t code, AbiNode object)
	{
		const uint64_t header = next();
		if (sema::WordTag(header) != sema::kWordName || sema::WordValue(header) != 2)
			return fail();
		const uint64_t word = next();
		if ((word & 7) != 0 || word == 0)
			return fail();
		const size_t base = lowering_.abi_scratch_.size();
		const uint32_t count = arguments();
		const AbiNode node = failed_ ? ABI_NONE
									 : graph_.member_access(code, object, string(IdentifierWord(word)),
										   lowering_.abi_scratch_.data() + base, count);
		lowering_.abi_scratch_.resize(base);
		return node;
	}

	// Operands on the scratch stack; the node built of them.
	AbiNode operation(uint16_t code, uint32_t count, uint32_t depth, bool prefix = false)
	{
		const size_t base = lowering_.abi_scratch_.size();
		for (uint32_t i = 0; i < count && !failed_; ++i)
		{
			const AbiNode operand = expression(depth + 1);
			lowering_.abi_scratch_.push_back(operand);
		}
		const AbiNode node = failed_ || code == 0
			? fail()
			: graph_.operation(code, lowering_.abi_scratch_.data() + base, count, prefix);
		lowering_.abi_scratch_.resize(base);
		return node;
	}

	AbiNode list(uint16_t code, AbiNode list_type, uint32_t count, uint32_t depth)
	{
		const size_t base = lowering_.abi_scratch_.size();
		for (uint32_t i = 0; i < count && !failed_; ++i)
		{
			const AbiNode element = expression(depth + 1);
			lowering_.abi_scratch_.push_back(element);
		}
		const AbiNode node = failed_
			? ABI_NONE
			: graph_.expression_list(code, list_type, lowering_.abi_scratch_.data() + base, count);
		lowering_.abi_scratch_.resize(base);
		return node;
	}

	// A braced-init-list's elements (T{...}'s, whose node follows).
	AbiNode braced(AbiNode list_type, uint32_t depth)
	{
		const uint64_t head = next();
		const uint64_t value = sema::WordValue(head);
		if (sema::WordTag(head) != sema::kWordNode ||
			static_cast<syntax::ExprKind>(value & 0xff) != syntax::ExprKind::BracedInit)
			return fail();
		return list(list_type != ABI_NONE ? Code('t', 'l') : Code('i', 'l'), list_type,
			static_cast<uint32_t>(value >> 8), depth);
	}

	bool at_node(const uint64_t* at, syntax::ExprKind kind) const
	{
		return at < end_ && sema::WordTag(*at) == sema::kWordNode &&
			static_cast<syntax::ExprKind>(sema::WordValue(*at) & 0xff) == kind;
	}

	AbiNode call(uint32_t count, uint32_t depth)
	{
		// The callee: a name (an unresolved name, or a type: a functional
		// cast), or another expression. A parenthesized unqualified name
		// that would use argument-dependent lookup but for the parentheses
		// is cp's (5.1.6: cp <base-unresolved-name> <expression>* E, as
		// Clang writes it; g++ writes cl).
		AbiNode named_type = ABI_NONE;
		AbiNode callee = ABI_NONE;
		const bool parenthesized = at_node(at_, syntax::ExprKind::Paren) && at_node(at_ + 1, syntax::ExprKind::Id);
		if (parenthesized)
			next();
		if (at_node(at_, syntax::ExprKind::Id))
		{
			next();
			callee = name(static_cast<int32_t>(count), &named_type);
		}
		else
			callee = expression(depth + 1);
		if (failed_)
			return ABI_NONE;
		if (named_type != ABI_NONE)
		{
			if (count == 1)
			{
				const AbiNode operand = expression(depth + 1);
				return failed_ ? ABI_NONE : graph_.cast(Code('c', 'v'), named_type, operand);
			}
			return list(Code('c', 'v'), named_type, count, depth);
		}
		const size_t base = lowering_.abi_scratch_.size();
		lowering_.abi_scratch_.push_back(callee);
		for (uint32_t i = 0; i < count && !failed_; ++i)
		{
			const AbiNode argument = expression(depth + 1);
			lowering_.abi_scratch_.push_back(argument);
		}
		const AbiNode node = failed_
			? ABI_NONE
			: graph_.call(lowering_.abi_scratch_.data() + base, count + 1,
				  parenthesized && graph_.kind(callee) == abi_mangle::ABI_NODE_UNRESOLVED_NAME);
		lowering_.abi_scratch_.resize(base);
		return node;
	}

	AbiNode expression(uint32_t depth)
	{
		const uint64_t head = next();
		if (failed_ || depth > 64)
			return fail();
		if (sema::WordTag(head) == sema::kWordLiteral)
			return literal(head);
		if (sema::WordTag(head) == sema::kWordConstant)
			return constant();
		if (sema::WordTag(head) != sema::kWordNode)
			return fail();
		const uint64_t value = sema::WordValue(head);
		const uint64_t above = value >> 8;
		switch (static_cast<syntax::ExprKind>(value & 0xff))
		{
		case syntax::ExprKind::Id:
			return name(-1, nullptr);
		case syntax::ExprKind::KeywordLiteral:
			switch (static_cast<TokenKind>(above))
			{
			case TokenKind::KW_TRUE:
			case TokenKind::KW_FALSE:
				return graph_.literal(graph_.builtin(abi_mangle::ABI_BUILTIN_TYPE_BOOL),
					static_cast<TokenKind>(above) == TokenKind::KW_TRUE ? 1 : 0);
			case TokenKind::KW_NULLPTR:
				return graph_.nullptr_literal();
			case TokenKind::KW_THIS:
				next();
				next();
				return graph_.this_param();
			default:
				return fail();
			}
		case syntax::ExprKind::Paren:
			return expression(depth + 1);
		case syntax::ExprKind::Binary:
		case syntax::ExprKind::Assign:
			return operation(BinaryCode(static_cast<TokenKind>(above)), 2, depth);
		case syntax::ExprKind::Conditional:
			return operation(Code('q', 'u'), 3, depth);
		case syntax::ExprKind::Unary:
		case syntax::ExprKind::Postfix:
		{
			// 5.1.6: a prefix increment or decrement is pp_/mm_, a postfix
			// one pp/mm.
			const TokenKind op = static_cast<TokenKind>(above);
			const abi_mangle::AbiTerminalKind kind = OperatorTerminal(op, false);
			const uint16_t code = kind == abi_mangle::ABI_TERMINAL_NONE
				? 0
				: TerminalCode(abi_mangle::abi_terminal_for_operands(kind, 1));
			const bool step = op == TokenKind::OP_INC || op == TokenKind::OP_DEC;
			return operation(
				code, 1, depth, step && static_cast<syntax::ExprKind>(value & 0xff) == syntax::ExprKind::Unary);
		}
		case syntax::ExprKind::Call:
			return call(static_cast<uint32_t>(above), depth);
		case syntax::ExprKind::FunctionalCast:
		{
			// T(e) is cv T e; T() and T(e, ...) cv T _ ... E; T{...} tl T ... E.
			const uint32_t count = static_cast<uint32_t>(above & 0xffff);
			const bool is_braced = ((above >> 16) & 1) != 0;
			const AbiNode cast_type = type(next());
			if (failed_)
				return ABI_NONE;
			if (is_braced)
				return braced(cast_type, depth);
			if (count == 1)
			{
				const AbiNode operand = expression(depth + 1);
				return failed_ ? ABI_NONE : graph_.cast(Code('c', 'v'), cast_type, operand);
			}
			return list(Code('c', 'v'), cast_type, count, depth);
		}
		case syntax::ExprKind::Subscript:
			return operation(Code('i', 'x'), 2, depth);
		case syntax::ExprKind::Member:
		{
			const uint16_t code = static_cast<TokenKind>(above) == TokenKind::OP_ARROW ? Code('p', 't')
																					   : Code('d', 't');
			const AbiNode object = expression(depth + 1);
			return failed_ ? ABI_NONE : member(code, object);
		}
		case syntax::ExprKind::Cast:
		{
			uint16_t code = 0;
			switch (static_cast<TokenKind>(above))
			{
			case TokenKind::KW_STATIC_CAST:
				code = Code('s', 'c');
				break;
			case TokenKind::KW_DYNAMIC_CAST:
				code = Code('d', 'c');
				break;
			case TokenKind::KW_REINTERPET_CAST:
				code = Code('r', 'c');
				break;
			case TokenKind::KW_CONST_CAST:
				code = Code('c', 'c');
				break;
			case TokenKind::OP_LPAREN:
				code = Code('c', 'v');
				break;
			default:
				return fail();
			}
			const AbiNode cast_type = expression_type(next());
			const AbiNode operand = failed_ ? ABI_NONE : expression(depth + 1);
			return failed_ ? ABI_NONE : graph_.cast(code, cast_type, operand);
		}
		case syntax::ExprKind::Sizeof:
			if (((above >> 16) & 1) != 0)
				return type_query(Code('s', 't'), next(), true);
			return operation(Code('s', 'z'), 1, depth);
		case syntax::ExprKind::TypeTrait:
		{
			// alignof (at/az), noexcept (nx), typeid (ti/te).
			const bool of_type = ((above >> 16) & 1) != 0;
			switch (static_cast<TokenKind>(above & 0xffff))
			{
			case TokenKind::KW_ALIGNOF:
				return of_type ? type_query(Code('a', 't'), next(), false) : operation(Code('a', 'z'), 1, depth);
			case TokenKind::KW_TYPEID:
				return of_type ? graph_.type_operation(Code('t', 'i'), type(next()))
							   : operation(Code('t', 'e'), 1, depth);
			case TokenKind::KW_NOEXCEPT:
				return of_type ? fail() : operation(Code('n', 'x'), 1, depth);
			default:
				return fail();
			}
		}
		case syntax::ExprKind::SizeofPack:
		{
			// sZ of a template parameter pack (T_) or a function parameter
			// pack (fp_).
			AbiNode pack = ABI_NONE;
			if ((above & 1) != 0)
			{
				const uint64_t word = next();
				next();
				pack = graph_.function_param(sema::ParameterIndex(word), sema::ParameterLevel(word));
			}
			else
			{
				const sema::Entity* parameter = EntityWord(next());
				if (parameter == nullptr || parameter->kind != sema::EntityKind::TemplateParameter)
					return fail();
				const sema::TemplateParameterEntity* own = static_cast<const sema::TemplateParameterEntity*>(parameter);
				pack = graph_.template_param(own->index, false, lowering_.own_level(own));
			}
			return failed_ ? ABI_NONE : graph_.operation(Code('s', 'Z'), &pack, 1);
		}
		case syntax::ExprKind::PackExpansion:
			return operation(Code('s', 'p'), 1, depth);
		case syntax::ExprKind::Throw:
			// throw e (tw), a rethrow (tr).
			return operation((above & 1) != 0 ? Code('t', 'r') : Code('t', 'w'), (above & 1) != 0 ? 0 : 1, depth);
		case syntax::ExprKind::BracedInit:
			return list(Code('i', 'l'), ABI_NONE, static_cast<uint32_t>(above), depth);
		case syntax::ExprKind::New:
		{
			// [gs] nw <expression>* _ <type> E, a parenthesized initializer's
			// pi <expression>* E or a braced one's il <expression>* E before
			// the E (5.1.6); an array's na and its element type, its first
			// bound not encoded (as Clang mangles it).
			const uint32_t placement = static_cast<uint32_t>(above & 0xffff);
			const bool global = ((above >> 16) & 1) != 0;
			const bool array = ((above >> 17) & 1) != 0;
			const uint64_t form = (above >> 18) & 3;
			const uint32_t count = static_cast<uint32_t>(above >> 20);
			const AbiNode allocated = type(next());
			if (array && !failed_)
				expression(depth + 1);
			const size_t base = lowering_.abi_scratch_.size();
			for (uint32_t i = 0; i < placement && !failed_; ++i)
			{
				const AbiNode argument = expression(depth + 1);
				lowering_.abi_scratch_.push_back(argument);
			}
			const AbiNode initializer = failed_ || form == 0
				? ABI_NONE
				: list(form == 1 ? Code('p', 'i') : Code('i', 'l'), ABI_NONE, count, depth);
			const AbiNode node = failed_ ? ABI_NONE
										 : graph_.allocation(array ? Code('n', 'a') : Code('n', 'w'), global, allocated,
											   lowering_.abi_scratch_.data() + base, placement, initializer);
			lowering_.abi_scratch_.resize(base);
			return node;
		}
		case syntax::ExprKind::Delete:
		{
			// [gs] dl <expression>, [gs] da <expression>.
			const AbiNode operand = expression(depth + 1);
			return failed_ ? ABI_NONE
						   : graph_.allocation((above & 2) != 0 ? Code('d', 'a') : Code('d', 'l'), (above & 1) != 0,
								 ABI_NONE, &operand, 1, ABI_NONE);
		}
		case syntax::ExprKind::BuiltinTrait:
		{
			// A type transform as a decltype-specifier's whole operand is
			// the vendor type u <source-name> <template-args> (Clang's
			// mangling of `__decay(T)`; placeholder_type takes it as the
			// type).
			const GnuTrait trait = static_cast<GnuTrait>(above & 0xff);
			if (depth != 0 || !IsGnuTypeTransform(trait) || ((above >> 8) & 0xff) != 1)
				return fail();
			const AbiNode argument = type(next());
			const char* spelling = GnuTraitName(trait);
			return failed_ ? ABI_NONE
						   : graph_.vendor_type(graph_.string(spelling, std::strlen(spelling)), &argument, 1);
		}
		default:
			return fail();
		}
	}

	ProgramLowering& lowering_;
	abi_mangle::AbiGraph& graph_;
	const sema::TemplateParameterEntity* placeholder_;
	const uint64_t* at_;
	const uint64_t* end_;
	bool failed_;
};

AbiNode ProgramLowering::unresolved_prefix(const sema::Type* qualifier)
{
	const size_t base = abi_scratch_.size();
	const AbiNode unresolved = qualifier_levels(qualifier);
	const AbiNode prefix = abi_graph_.unresolved_prefix(
		unresolved, abi_scratch_.data() + base, abi_scratch_.size() - base, false);
	abi_scratch_.resize(base);
	return prefix;
}

AbiNode ProgramLowering::qualifier_levels(const sema::Type* qualifier)
{
	// A dependent qualifier known by its type alone (no spelling), as
	// Clang mangles a substituted one: its levels onto the scratch stack,
	// and its unresolved type (ABI_NONE when its root is a level). A
	// dependent member's members are levels after its own qualifier's (Q::U
	// is srN <Q> 1U E, X<T>::U sr 1XIT_E 1U E); a class template's
	// specialization is a level of its name and arguments, unqualified;
	// another type (a template parameter, a template template-parameter's
	// specialization, a decltype) the unresolved type.
	const sema::Type* root = qualifier->unqualified;
	while (root->kind == sema::TypeKind::TemplateParameter &&
		static_cast<const sema::TemplateParameterEntity*>(root->entity)->qualifier != nullptr &&
		static_cast<const sema::TemplateParameterEntity*>(root->entity)->expression == nullptr)
		root = static_cast<const sema::TemplateParameterEntity*>(root->entity)->qualifier->unqualified;
	if (const sema::Type* aliased = root->kind == sema::TypeKind::Class ? analyzer_->aliased_of(root) : nullptr)
		root = aliased->unqualified;
	const sema::ClassTemplateFacts* facts = root->kind == sema::TypeKind::Class
		? static_cast<const sema::ClassEntity*>(root->entity)->templ
		: nullptr;
	AbiNode unresolved = abi_mangle::ABI_NONE;
	if (facts != nullptr && facts->role == sema::TemplateRole::Specialization &&
		facts->pattern->templ->template_name == nullptr && root->entity->name != nullptr)
	{
		const size_t arguments = abi_scratch_.size();
		const uint32_t spelled = analyzer_->qualifier_arguments(
			facts->pattern->templ->head, facts->arguments, facts->argument_count);
		for (uint32_t i = 0; i < spelled; ++i)
		{
			const AbiNode argument = abi_type(facts->arguments[i]);
			abi_scratch_.push_back(argument);
		}
		const char* spelling = root->entity->name->spelling;
		const AbiNode level = abi_graph_.unresolved_name(
			abi_graph_.string(spelling, std::strlen(spelling)), abi_scratch_.data() + arguments, spelled);
		abi_scratch_.resize(arguments);
		abi_scratch_.push_back(level);
	}
	else
		unresolved = abi_type(root);
	// The members' names, innermost last.
	const size_t members = abi_scratch_.size();
	for (const sema::Type* member = qualifier->unqualified; member->kind == sema::TypeKind::TemplateParameter;)
	{
		const sema::TemplateParameterEntity* placeholder = static_cast<const sema::TemplateParameterEntity*>(
			member->entity);
		if (placeholder->qualifier == nullptr || placeholder->expression != nullptr)
			break;
		const char* spelling = placeholder->name->spelling;
		const AbiNode name = abi_graph_.string(spelling, std::strlen(spelling));
		abi_scratch_.push_back(abi_graph_.unresolved_name(name, nullptr, 0));
		member = placeholder->qualifier->unqualified;
	}
	std::reverse(abi_scratch_.begin() + static_cast<std::ptrdiff_t>(members), abi_scratch_.end());
	return unresolved;
}

AbiNode ProgramLowering::placeholder_expression(const sema::TemplateParameterEntity* placeholder)
{
	// Memoized per unit (the graph's nodes are the unit's); ABI_NONE for a
	// placeholder without a key, or of an operand naming no dependent entity
	// or type (see the file comment).
	// Inside a pattern naming a bound pack its parameters are encoded by
	// level (see own_level): not memoized.
	if (placeholder->key == nullptr || placeholder->key[0] <= sema::kPlaceholderKeyPrefix ||
		(placeholder->key[1] & 1) == 0)
		return ABI_NONE;
	if (abi_bound_pattern_)
		return PlaceholderReader(*this, placeholder).operand();
	const uint32_t known = placeholder_nodes_.find(placeholder);
	if (known != PointerMap::kMissing)
		return known;
	// A placeholder naming itself (through a frame) is not a key's.
	placeholder_nodes_.set(placeholder, ABI_NONE);
	const AbiNode node = PlaceholderReader(*this, placeholder).operand();
	placeholder_nodes_.set(placeholder, node);
	return node;
}

AbiNode ProgramLowering::placeholder_type(const sema::TemplateParameterEntity* placeholder)
{
	// A decltype-specifier's type (Dt for an id-expression or class member
	// access, DT for another, 5.1.5.8), or a value-dependent template
	// argument (X <expression> E; a literal or an entity is L ... E alone).
	const AbiNode expression = placeholder_expression(placeholder);
	if (expression == ABI_NONE || abi_graph_.kind(expression) == abi_mangle::ABI_NODE_VENDOR_TYPE)
		return expression;
	if (placeholder->non_type)
	{
		if (abi_graph_.kind(expression) == abi_mangle::ABI_NODE_LITERAL)
			return expression;
		if (abi_graph_.kind(expression) == abi_mangle::ABI_NODE_ENTITY_REFERENCE)
			return abi_graph_.entity_argument(abi_graph_.node(expression).a, false);
		return abi_graph_.expression_argument(expression);
	}
	const uint64_t head = placeholder->key[1 + sema::kPlaceholderKeyPrefix];
	const syntax::ExprKind kind = static_cast<syntax::ExprKind>(sema::WordValue(head) & 0xff);
	const bool named = sema::WordTag(head) == sema::kWordNode &&
		(kind == syntax::ExprKind::Id || kind == syntax::ExprKind::Member);
	return abi_graph_.decltype_of(expression, named);
}

bool ProgramLowering::unmangled_placeholder(const sema::Type* type)
{
	// Whether a template signature's type names a placeholder whose
	// expression the key does not mangle (the specialization's own type
	// then stands in for the whole type).
	switch (type->kind)
	{
	case sema::TypeKind::TemplateParameter:
	{
		const sema::TemplateParameterEntity* parameter = static_cast<const sema::TemplateParameterEntity*>(
			type->entity);
		return parameter->expression != nullptr && placeholder_expression(parameter) == ABI_NONE;
	}
	case sema::TypeKind::Array:
	{
		const sema::TemplateParameterEntity* bound = type->owner != nullptr
			? static_cast<const sema::TemplateParameterEntity*>(type->owner->entity)
			: nullptr;
		return (bound != nullptr && bound->named == nullptr && placeholder_expression(bound) == ABI_NONE) ||
			unmangled_placeholder(type->element);
	}
	case sema::TypeKind::Class:
	{
		// A dependent specialization's arguments.
		const sema::ClassTemplateFacts* facts = static_cast<const sema::ClassEntity*>(type->entity)->templ;
		if (facts == nullptr || facts->role != sema::TemplateRole::Specialization || !facts->dependent)
			return false;
		for (uint32_t i = 0; i < facts->argument_count; ++i)
		{
			if (unmangled_placeholder(facts->arguments[i]))
				return true;
		}
		return false;
	}
	case sema::TypeKind::Pack:
		for (uint32_t i = 0; i < type->param_count; ++i)
		{
			if (unmangled_placeholder(type->params[i]))
				return true;
		}
		return false;
	case sema::TypeKind::Pointer:
	case sema::TypeKind::LValueReference:
	case sema::TypeKind::RValueReference:
	case sema::TypeKind::Expansion:
		return unmangled_placeholder(type->element);
	case sema::TypeKind::MemberPointer:
		return unmangled_placeholder(type->owner) || unmangled_placeholder(type->element);
	case sema::TypeKind::Function:
		for (uint32_t i = 0; i < type->param_count; ++i)
		{
			if (unmangled_placeholder(type->params[i]))
				return true;
		}
		return unmangled_placeholder(type->element);
	default:
		return false;
	}
}

} // namespace lower
