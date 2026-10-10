#include "syntax/parser_state.h"

#include "lex/gnu_builtins.h"
#include "lex/identifier_table.h"
#include "support/stack_budget.h"

#include <algorithm>
#include <cstring>

namespace syntax {

ParserState::ParserState(ParseTokenStream& tokens, IdentifierTable& identifiers, Arena& arena, bool hosted)
	: tokens_(tokens), identifiers_(identifiers), arena_(arena), scopes_(arena), deferred_lambdas_(0),
	  deferred_tokens_(0), peak_run_bytes_(0), function_scope_(nullptr), parameter_scope_(nullptr),
	  declarator_member_of_(nullptr), type_id_scope_(nullptr), angle_(false), definite_type_(false),
	  special_member_clause_(false), friend_declarator_(false), naming_declared_(false), pack_mentions_(0),
	  parameter_mentions_(0), template_parameter_mentions_(0), pending_template_(false), pending_attributes_(nullptr),
	  leading_attributes_(nullptr), tentative_depth_(0), speculation_(0), open_lists_base_(0), abandoning_lists_(false),
	  abandoned_error_position_(0), abandoned_error_location_(0), nesting_(0), peak_nesting_(0), stack_base_(0),
	  stack_budget_(stack_budget_bytes()), error_position_(0), error_location_(0), checkpoints_(0), rewinds_(0),
	  deferred_count_(0), hosted_(hosted), gnu_names_(identifiers)
{
	final_ = identifiers.intern("final", 5);
	coroutine_keywords_[0] = identifiers.intern("co_await", 8);
	coroutine_keywords_[1] = identifiers.intern("co_yield", 8);
	coroutine_keywords_[2] = identifiers.intern("co_return", 9);
	override_ = identifiers.intern("override", 8);
	// Unnamed namespaces are bound under a spelling no identifier has.
	unnamed_namespace_ = identifiers.intern(" unnamed", 8);
	// The course treats std::nullptr_t's unqualified name as predeclared.
	scopes_.declare(
		scopes_.global(), identifiers.intern("nullptr_t", 9), NameBinding{ NameCategory::Type, false, nullptr });
	// GNU alternative keyword spellings (lex/gnu_builtins.h).
	for (unsigned i = 0; i < kGnuKeywordCount; ++i)
	{
		const GnuKeyword& gnu = kGnuKeywords[i];
		Identifier* identifier = identifiers.intern(gnu.spelling, static_cast<uint32_t>(std::strlen(gnu.spelling)));
		if (gnu.dropped)
			tokens.ignore(identifier);
		else
			identifier->keyword = static_cast<uint8_t>(static_cast<uint8_t>(gnu.kind) + 1);
	}
	// A function's predefined __func__ (8.4.1p8), GCC's __FUNCTION__ and
	// __PRETTY_FUNCTION__: variables, whatever the lexical fallback would
	// make of the upper-case names (`sizeof(__PRETTY_FUNCTION__)`).
	for (const char* predefined : { "__func__", "__FUNCTION__", "__PRETTY_FUNCTION__" })
		scopes_.declare(scopes_.global(),
			identifiers.intern(predefined, static_cast<uint32_t>(std::strlen(predefined))),
			NameBinding{ NameCategory::Value, false, nullptr });
	// GNU __null: a null pointer constant of type long (LP64).
	tokens.read_as_literal(identifiers.intern("__null", 6), TokenSpelling{ "0L", 2 });
	// So are GCC's 128-bit integer and extended floating type names
	// (sema/analyzer.cpp), its complex type specifiers (combining with
	// the type keywords, sema/analyzer_types.cpp) and Clang's __bf16.
	unsigned extended = 0;
	for (const char* wide : { "__int128", "__int128_t", "__uint128_t", "_Float16", "_Float32", "_Float64", "_Float128",
			 "_Float32x", "_Float64x", "__float128", "_Complex", "__complex__", "__complex", "__bf16" })
	{
		Identifier* identifier = identifiers.intern(wide, static_cast<uint32_t>(std::strlen(wide)));
		scopes_.declare(scopes_.global(), identifier, NameBinding{ NameCategory::Type, false, nullptr });
		if (extended < sizeof(extended_types_) / sizeof(extended_types_[0]))
			extended_types_[extended++] = identifier;
	}
}

ParserCounters ParserState::counters() const
{
	ParserCounters counters;
	counters.tokens = tokens_.tokens_read();
	counters.checkpoints = checkpoints_;
	counters.rewinds = rewinds_;
	counters.deferred_bodies = deferred_count_;
	counters.lookups = scopes_.lookups();
	counters.bindings = scopes_.binding_count();
	counters.peak_window = tokens_.peak_window();
	counters.peak_nesting = peak_nesting_;
	counters.arena_bytes = arena_.bytes_reserved();
	counters.deferred_tokens = deferred_tokens_;
	counters.peak_run_bytes = peak_run_bytes_;
	telemetry::note_peak(counters.peak_run_bytes, runs_.bytes_reserved());
	counters.deferred_lambdas = deferred_lambdas_;
	counters.kept_searches = scopes_.kept_searches();
	return counters;
}

bool ParserState::at_trait(GnuTrait& trait)
{
	const ParseToken token = tok();
	const uint8_t name = token.is_identifier() ? gnu_names_.kind(token.identifier) : 0;
	if (name == 0 || name >= GnuNames::kFirstSpecial || !at(TokenKind::OP_LPAREN, 1))
		return false;
	trait = static_cast<GnuTrait>(name - 1);
	return true;
}

bool ParserState::at_gnu_decltype(uint32_t ahead)
{
	if (!tok(ahead).is_identifier() || !at(TokenKind::OP_LPAREN, ahead + 1))
		return false;
	const uint8_t name = gnu_names_.kind(tok(ahead).identifier);
	return name == GnuNames::kTypeof ||
		(name != 0 && name < GnuNames::kAttribute && IsGnuTypeTransform(static_cast<GnuTrait>(name - 1)));
}

bool ParserState::at_typeof()
{
	const ParseToken token = tok();
	return token.is_identifier() && gnu_names_.kind(token.identifier) == GnuNames::kTypeof;
}

bool ParserState::accept(TokenKind kind)
{
	if (!at(kind))
		return false;
	next();
	return true;
}

bool ParserState::expect(TokenKind kind)
{
	if (accept(kind))
		return true;
	return fail();
}

bool ParserState::fail()
{
	if (tokens_.absolute_position() >= error_position_)
	{
		error_position_ = tokens_.absolute_position();
		error_location_ = tok().location;
	}
	return false;
}

ParserState::Checkpoint ParserState::save()
{
	++checkpoints_;
	++tentative_depth_;
	++speculation_;
	tokens_.retain();
	scopes_.begin_logging();
	Checkpoint checkpoint;
	checkpoint.position = tokens_.position();
	checkpoint.mark = arena_.mark();
	checkpoint.runs = runs_.mark();
	checkpoint.log = scopes_.log_mark();
	checkpoint.environment = scopes_.environment();
	checkpoint.scope = scopes_.current();
	checkpoint.deferred = deferred_.size();
	checkpoint.lambdas = lambdas_.size();
	checkpoint.classes = class_names_.size();
	checkpoint.function_scope = function_scope_;
	checkpoint.angle = angle_;
	checkpoint.pending_template = pending_template_;
	checkpoint.definite_type = definite_type_;
	checkpoint.special_member_clause = special_member_clause_;
	checkpoint.pack_mentions = pack_mentions_;
	checkpoint.parameter_mentions = parameter_mentions_;
	checkpoint.conflict = conflicts_.statement;
	return checkpoint;
}

void ParserState::restore(const Checkpoint& checkpoint)
{
	++rewinds_;
	tokens_.rewind(checkpoint.position);
	arena_.rewind(checkpoint.mark);
	runs_.rewind(checkpoint.runs);
	scopes_.undo_to(checkpoint.log);
	scopes_.set_environment(checkpoint.environment);
	scopes_.set_current(checkpoint.scope);
	deferred_.resize(checkpoint.deferred);
	lambdas_.resize(checkpoint.lambdas);
	class_names_.resize(checkpoint.classes);
	function_scope_ = checkpoint.function_scope;
	angle_ = checkpoint.angle;
	pending_template_ = checkpoint.pending_template;
	definite_type_ = checkpoint.definite_type;
	special_member_clause_ = checkpoint.special_member_clause;
	pack_mentions_ = checkpoint.pack_mentions;
	parameter_mentions_ = checkpoint.parameter_mentions;
	conflicts_.statement = checkpoint.conflict;
	--speculation_;
	close(checkpoint);
}

void ParserState::commit(const Checkpoint& checkpoint)
{
	// The lambda bodies skipped by the outermost speculation are parsed
	// while its tokens and name log are still retained.
	if (--speculation_ == 0 && lambdas_.size() > checkpoint.lambdas)
		parse_deferred_lambdas(checkpoint.lambdas);
	close(checkpoint);
}

void ParserState::close(const Checkpoint&)
{
	tokens_.release();
	scopes_.end_logging();
	--tentative_depth_;
}

Name* ParserState::make_simple_name(const Identifier* identifier, uint32_t location)
{
	NameComponent* component = make<NameComponent>();
	*component = NameComponent();
	component->kind = NameComponentKind::Identifier;
	component->identifier = identifier;
	component->location = location;
	Name* name = make<Name>();
	name->global = false;
	name->typename_keyword = false;
	name->components = Slice<NameComponent>{ component, 1 };
	name->location = location;
	return name;
}

Scope* ParserState::declaration_scope() const
{
	Scope* scope = scopes_.current();
	while (scope->kind == ScopeKind::Template && scope->parent != nullptr)
		scope = scope->parent;
	return scope;
}

bool ParserState::in_template_definition()
{
	// Kept for the last scope asked about (most names of a declaration or
	// body are parsed in one scope; scope ids are never reused); a
	// reopened class's scope changes its depth with its parent.
	const Scope* current = scopes_.current();
	if (current->id == template_query_id_ && current->depth == template_query_depth_)
		return template_query_result_;
	bool result = false;
	for (const Scope* scope = current; scope != nullptr && !result; scope = scope->parent)
		result = scope->kind == ScopeKind::Template;
	template_query_id_ = current->id;
	template_query_depth_ = current->depth;
	template_query_result_ = result;
	return result;
}

bool ParserState::skip_balanced()
{
	// At an opening bracket: skip to and past its matching closer (the
	// closer stack is reused storage).
	std::vector<TokenKind>& closers = closers_;
	closers.clear();
	for (;;)
	{
		const ParseToken token = tok();
		if (token.is_end())
			return fail();
		if (token.is(TokenKind::OP_LPAREN))
			closers.push_back(TokenKind::OP_RPAREN);
		else if (token.is(TokenKind::OP_LSQUARE))
			closers.push_back(TokenKind::OP_RSQUARE);
		else if (token.is(TokenKind::OP_LBRACE))
			closers.push_back(TokenKind::OP_RBRACE);
		else if (token.is(TokenKind::OP_RPAREN) || token.is(TokenKind::OP_RSQUARE) || token.is(TokenKind::OP_RBRACE))
		{
			if (closers.empty() || !token.is(closers.back()))
				return fail();
			closers.pop_back();
		}
		else if (closers.empty())
		{
			return fail();
		}
		next();
		if (closers.empty())
			return true;
	}
}

bool ParserState::closing_angle_ahead()
{
	// The scan passes over bracketed groups; the maximal runs of depth-0
	// tokens it crosses before its answer become fences with that answer
	// (they are appended in position order, then merged into the list).
	const uint64_t base = tokens_.absolute_position();
	std::vector<AngleFence>::iterator fence = std::upper_bound(angle_fences_.begin(), angle_fences_.end(), base,
		[](uint64_t position, const AngleFence& run) { return position < run.begin; });
	if (fence != angle_fences_.begin() && base <= (fence - 1)->end)
		return (fence - 1)->found;
	const size_t first_new = angle_fences_.size();
	uint64_t run_begin = base;
	int depth = 0;
	bool result = false;
	for (uint32_t ahead = 1;; ++ahead)
	{
		const ParseToken token = tok(ahead);
		const bool opener = token.is(TokenKind::OP_LPAREN) || token.is(TokenKind::OP_LSQUARE) ||
			token.is(TokenKind::OP_LBRACE);
		const bool closer = token.is(TokenKind::OP_RPAREN) || token.is(TokenKind::OP_RSQUARE) ||
			token.is(TokenKind::OP_RBRACE);
		const bool found = depth == 0 && token.is(TokenKind::OP_GT);
		// The input can end inside an unclosed group: the scan stops there
		// too (the depth-0 run before that group already has its fence).
		if (token.is_end() || found || (depth == 0 && (closer || token.is(TokenKind::OP_SEMICOLON))))
		{
			result = found;
			if (depth == 0)
				angle_fences_.push_back(AngleFence{ run_begin, base + ahead - 1, false });
			break;
		}
		if (opener)
		{
			if (depth == 0)
				angle_fences_.push_back(AngleFence{ run_begin, base + ahead, false });
			++depth;
		}
		else if (closer && --depth == 0)
		{
			run_begin = base + ahead;
		}
	}
	for (size_t i = first_new; i < angle_fences_.size(); ++i)
		angle_fences_[i].found = result;
	std::inplace_merge(angle_fences_.begin(), angle_fences_.begin() + first_new, angle_fences_.end(),
		[](const AngleFence& a, const AngleFence& b) { return a.begin < b.begin; });
	return result;
}

bool ParserState::at_attribute(uint32_t ahead)
{
	// The token's fields are read before the next peek (which may move the
	// window).
	const ParseToken token = tok(ahead);
	if (token.is_identifier())
		return gnu_names_.kind(token.identifier) == GnuNames::kAttribute;
	return token.is(TokenKind::KW_ALIGNAS) || (token.is(TokenKind::OP_LSQUARE) && at(TokenKind::OP_LSQUARE, ahead + 1));
}

bool ParserState::skip_attributes()
{
	// attribute-specifier-seq (7.6.1), alignas and GNU __attribute__ are
	// accepted and dropped: the PA5 tree does not represent attributes.
	while (at_attribute())
	{
		if (at(TokenKind::OP_LSQUARE))
		{
			next();
			if (!skip_balanced() || !expect(TokenKind::OP_RSQUARE))
				return false;
			continue;
		}
		next();
		if (!at(TokenKind::OP_LPAREN) || !skip_balanced())
			return fail();
	}
	return true;
}

} // namespace syntax
