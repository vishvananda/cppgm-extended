#include "preprocess/macro/macro_expander.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "lex/gnu_builtins.h"
#include "lex/text_literal.h"
#include "preprocess/lexer/pp_lexer.h"
#include "preprocess/preprocess_error.h"
#include "preprocess/source/source_buffer.h"

namespace {

bool IsPunctuatorToken(const PPToken& token, TokenKind kind)
{
	return token.kind == PPTokenKind::PreprocessingOpOrPunc && IsPunctuator(token.punctuator, kind);
}

bool IsTextLiteral(PPTokenKind kind)
{
	return kind == PPTokenKind::CharacterLiteral || kind == PPTokenKind::UserDefinedCharacterLiteral ||
		kind == PPTokenKind::StringLiteral || kind == PPTokenKind::UserDefinedStringLiteral;
}

// An identifier-like preprocessing-op-or-punc (`and`, `new`, ...).
bool IsIdentifierLikeOperator(const PPToken& token)
{
	return token.kind == PPTokenKind::PreprocessingOpOrPunc && token.length != 0 && token.spelling[0] >= 'a' &&
		token.spelling[0] <= 'z';
}

// __has_cpp_attribute and (gnu) __has_attribute answer 1 for the attributes
// this implementation implements and 0 for every other name: the standard
// attribute no_unique_address, and the GNU and Clang attributes the parser
// accepts and applies (syntax/parser_specifiers.cpp). A header that probes
// for an attribute then uses it, so claiming not to have one that is
// implemented sends the header down a fallback path instead. `__name__`
// spells the same attribute.
bool IsReportedAttribute(const char* spelling, uint32_t length, bool gnu)
{
	if (length > 4 && std::memcmp(spelling, "__", 2) == 0 && std::memcmp(spelling + length - 2, "__", 2) == 0)
	{
		spelling += 2;
		length -= 4;
	}
	static const char* const kStandard[] = { "no_unique_address" };
	static const char* const kGnu[] = { "always_inline", "noinline", "noreturn", "pure", "const", "used", "weak",
		"packed", "aligned", "section", "vector_size", "gnu_inline", "abi_tag", "using_if_exists",
		"exclude_from_explicit_instantiation" };
	const char* const* reported = gnu ? kGnu : kStandard;
	const unsigned count = gnu ? sizeof(kGnu) / sizeof(kGnu[0]) : sizeof(kStandard) / sizeof(kStandard[0]);
	for (unsigned i = 0; i < count; ++i)
	{
		if (std::strlen(reported[i]) == length && std::memcmp(spelling, reported[i], length) == 0)
			return true;
	}
	return false;
}

PPToken EndToken()
{
	PPToken token = PPToken();
	token.kind = PPTokenKind::EndOfFile;
	token.spelling = "";
	return token;
}

} // namespace

MacroExpander::MacroExpander(Arena& arena, IdentifierTable& identifiers, const PreprocessorNames& names,
	ExpansionEnvironment& environment, MacroExpansionState& state)
	: arena_(arena), identifiers_(identifiers), names_(names), environment_(environment), input_(nullptr),
	  controlling_expression_(false), binary_exponents_(false), pending_space_(false), building_head_(EndToken()),
	  building_serial_(0), building_chain_(0), read_serial_(0), read_chain_(0), read_root_(0), read_disabled_(0),
	  current_(EndToken()), state_(state)
{}

// ---- rescanning ---------------------------------------------------------------

const PPToken& MacroExpander::next()
{
	for (;;)
	{
		if (!read(current_))
		{
			// The argument being replaced has ended.
			finish_argument();
			continue;
		}
		if (current_.kind == PPTokenKind::Identifier && start_expansion())
			continue;
		if (invocations_.empty())
			return current_;
		// A token of the argument being replaced.
		expanded_tokens_.push_back(current_);
	}
}

bool MacroExpander::start_expansion()
{
	// Returns whether current_ began an expansion; otherwise it is a
	// replaced token, possibly now painted, a number or a string.
	Identifier* identifier = current_.identifier;
	if (controlling_expression_ && identifier == names_.defined)
	{
		replace_defined();
		return false;
	}
	MacroDefinition* macro = identifier->macro;
	if (macro == nullptr || (current_.flags & kPainted) != 0)
		return false;
	// The names disabled where the token was read apply to every token of an
	// argument being replaced, but in an expansion only to its own tokens:
	// substituted ones carry just their sets.
	const uint32_t disabled = read_serial_ != 0 && (current_.flags & kArgument) != 0 ? 0 : read_disabled_;
	const bool hidden_by_set = hidden(current_, *macro);
	if (hidden_by_set || (disabled != 0 && disabled_here(*macro, disabled)))
	{
		if (hidden_by_set && (current_.flags & (kArgument | kReplacedArgument)) == kArgument)
			++state_.counters.raw_operands_hidden;
		current_.flags |= kPainted;
		return false;
	}
	if (macro->builtin != BuiltinMacro::None)
	{
		replace_builtin(*macro);
		return false;
	}
	const PPToken head = current_;
	const uint32_t context_disabled = read_disabled_;
	const uint32_t head_serial = read_serial_;
	// Read from an argument being replaced (its barrier is the top), not an
	// expansion or the input.
	const bool from_argument = read_serial_ == 0 && !contexts_.empty();
	if (!macro->function_like)
	{
		const Nesting nesting = nesting_of(head, from_argument, false);
		const bool passes = passes_disabled(head, nesting.parent);
		push_expansion(*macro, head, nesting, passes ? context_disabled : 0,
			passes ? begin_inherited(head, head_serial, from_argument) : 0, nullptr);
		return true;
	}
	// A function-like macro name is replaced only when the next token is
	// `(` ([cpp.replace] 16.3p10).
	if (!peek_is_left_paren())
		return false;
	begin_invocation(*macro, head, context_disabled, begin_inherited(head, head_serial, from_argument), from_argument);
	return true;
}

bool MacroExpander::hidden(const PPToken& token, const MacroDefinition& macro) const
{
	// Whether the token's hide set holds the macro's name. A replacement-list
	// token read from its own context C, the top of the stack, carries C's
	// chain. Every open context between C's root and C is on that chain
	// (another one would have been popped before the next link's name was
	// read), so the macro is on the chain exactly when its newest open
	// context is at or above the root: no walk of the set. The same holds
	// while C is open below the top, for macros whose newest open context
	// is not above C. A chain with an ended context has no root.
	if (read_serial_ != 0 && read_root_ != 0 && token.origin == read_serial_ && token.hide_set == read_chain_)
		return in_open_chain(macro, read_root_);
	const Context* origin = token.origin != 0 && token.hide_set != 0 ? find_open_context(token.origin) : nullptr;
	if (origin != nullptr && origin->expansion.root != 0 && macro.open_serial <= origin->serial &&
		origin->expansion.chain == token.hide_set)
	{
		return in_open_chain(macro, origin->expansion.root);
	}
	return hide_sets_.contains(token.hide_set, macro.name);
}

bool MacroExpander::disabled_here(const MacroDefinition& macro, uint32_t disabled) const
{
	// Whether the macro is in the set disabled for the token just read. For
	// an argument's own tokens the set is often the chain of the expansion
	// the invocation is nested in, open below the barrier: the macro is on
	// it exactly when its newest open context not above that expansion is
	// at or above the root, the test of hidden(); a context of the macro
	// opened inside the argument leaves the set to decide.
	const Context& from = contexts_.back();
	if (from.macro == nullptr && from.barrier.disabled_top != 0 && disabled == from.disabled &&
		macro.open_serial <= from.barrier.disabled_top)
	{
		return in_open_chain(macro, from.barrier.disabled_root);
	}
	return hide_sets_.contains(disabled, macro.name);
}

bool MacroExpander::read(PPToken& token)
{
	// False at the end of the argument being replaced.
	for (;;)
	{
		if (contexts_.empty())
		{
			token = input_->next();
			read_serial_ = 0;
			read_disabled_ = 0;
			break;
		}
		Context& context = contexts_.back();
		if (context.cursor < context.end)
		{
			token = tokens_of(context)[context.cursor++];
			if (context.macro != nullptr)
			{
				read_serial_ = context.serial;
				read_chain_ = context.expansion.chain;
				read_root_ = context.expansion.root;
				read_disabled_ = context.disabled;
			}
			else
			{
				read_serial_ = 0;
				read_disabled_ = (token.flags & kFromArgument) != 0 ? context.barrier.outer : context.disabled;
			}
			break;
		}
		if (context.macro == nullptr)
		{
			token = EndToken();
			return false;
		}
		pop_context();
	}
	if (pending_space_)
	{
		token.flags |= kLeadingSpace;
		pending_space_ = false;
	}
	return true;
}

bool MacroExpander::peek_is_left_paren()
{
	// Exhausted contexts are popped on the way, as the read that follows
	// would pop them.
	for (;;)
	{
		if (contexts_.empty())
			return IsPunctuatorToken(input_->peek(), TokenKind::OP_LPAREN);
		Context& context = contexts_.back();
		if (context.cursor < context.end)
			return IsPunctuatorToken(tokens_of(context)[context.cursor], TokenKind::OP_LPAREN);
		if (context.macro == nullptr)
			return false;
		pop_context();
	}
}

void MacroExpander::pop_context()
{
	Context& context = contexts_.back();
	if (context.macro != nullptr)
	{
		context.macro->open_serial = context.expansion.previous_open;
		context_tokens_.resize(context.begin);
	}
	contexts_.pop_back();
}

const MacroExpander::Context* MacroExpander::find_open_context(uint32_t serial) const
{
	// Serials never decrease up the stack, and the barriers pushed after the
	// expansion with this serial have serials no smaller, so that expansion,
	// if still open, is the lowest context whose serial is not below it. The
	// one sought is usually within a few contexts of the top; below those a
	// search keeps stacked argument barriers from costing a walk per lookup.
	const std::size_t size = contexts_.size();
	std::size_t below = size;
	for (std::size_t steps = 0; below != 0 && steps != kTopProbe; ++steps)
	{
		const Context& context = contexts_[below - 1];
		if (context.serial < serial)
			return nullptr;
		if (context.serial == serial && context.macro != nullptr)
			return &context;
		--below;
	}
	auto found = std::lower_bound(contexts_.begin(), contexts_.begin() + below, serial,
		[](const Context& context, uint32_t value) { return context.serial < value; });
	return found != contexts_.begin() + below && found->macro != nullptr && found->serial == serial ? &*found : nullptr;
}

MacroExpander::Nesting MacroExpander::nesting_of(const PPToken& head, bool from_argument, bool function_like) const
{
	// An invocation is nested in the context that produced its name token if
	// it begins there: the name is an object-like macro's or its `(` comes
	// from that context. That is whether the context is still open (for a
	// function-like macro, when its `(` is found); a name substituted for a
	// parameter is never nested. The expansion's chain extends that
	// context's chain, which names only the contexts it is nested in: a
	// set a placemarker paste merged into the name token hides that token
	// alone.
	if ((head.flags & kArgument) != 0 || head.origin == 0)
		return Nesting{ 0, 0 };
	if (const Context* context = find_open_context(head.origin))
		return Nesting{ context->serial, context->expansion.chain };
	// A name read from an argument being replaced, collected from a context
	// that has ended since: it began there if the collection says so, and
	// its token carries that context's chain.
	if (from_argument)
	{
		const Context& barrier = contexts_.back();
		if (!function_like || (argument_tokens_[barrier.cursor].flags & kFollowsInContext) != 0)
			return Nesting{ head.origin, head.hide_set };
	}
	return Nesting{ 0, 0 };
}

uint32_t MacroExpander::begin_inherited(const PPToken& head, uint32_t head_serial, bool from_argument) const
{
	// What an invocation inherits comes from the context it begins in, the
	// top of the stack once its `(` is found; a name substituted as written
	// into that expansion inherits nothing, and one collected into the
	// argument from an enclosing argument inherits what is disabled there.
	if (contexts_.empty())
		return 0;
	const Context& top = contexts_.back();
	if (top.macro == nullptr)
		return from_argument && (head.flags & kFromArgument) != 0 ? top.barrier.outer : top.inherited;
	if (top.serial == head_serial && (head.flags & (kArgument | kReplacedArgument)) == kArgument)
		return 0;
	return top.inherited;
}

uint32_t MacroExpander::chain_root(uint32_t parent) const
{
	// A nested expansion shares its parent's root while every context on the
	// chain is open. The parent can end while the arguments are collected;
	// the chain is then only a set (root 0: no open-context test). An open
	// parent is the top: what the arguments' replacement pushed is gone.
	if (parent == 0)
		return building_serial_;
	const Context* top = contexts_.empty() ? nullptr : &contexts_.back();
	return top != nullptr && top->macro != nullptr && top->serial == parent ? top->expansion.root : 0;
}

// ---- invocation -----------------------------------------------------------------
//
// An invocation's arguments are collected, then each argument used as a
// plain parameter is macro-replaced once, as if it were the rest of the
// file, and copied at each use (16.3.1p1). Replacement reads the argument
// through a barrier context and appends what next() produces to
// expanded_tokens_; invocations inside the argument start their own
// arguments on the stack invocations_, so nesting costs no native stack.

void MacroExpander::begin_invocation(
	MacroDefinition& macro, const PPToken& head, uint32_t context_disabled, uint32_t inherited, bool from_argument)
{
	Arguments arguments;
	arguments.bounds = static_cast<uint32_t>(argument_bounds_.size());
	arguments.expanded = static_cast<uint32_t>(expanded_bounds_.size());
	arguments.token_mark = static_cast<uint32_t>(argument_tokens_.size());
	arguments.expanded_mark = static_cast<uint32_t>(expanded_tokens_.size());
	// While an argument is replaced, the names inherited where the name token
	// was read stay disabled, along with the chain of an invocation nested
	// there. A substituted function name's set applies to its replacement,
	// not to the arguments it collects after that expansion has ended.
	const Nesting nesting = nesting_of(head, from_argument, true);
	arguments.disabled = hide_sets_.merge(nesting.parent != 0 ? head.hide_set : 0, inherited);
	arguments.outer = arguments.disabled;
	arguments.disabled_top = 0;
	arguments.disabled_root = 0;
	if (!collect_nested_arguments(macro, arguments))
	{
		PPToken left_paren;
		read(left_paren);
		collect_arguments(macro, head, arguments);
	}
	// The set is the parent's chain; the parent is below the barriers while
	// it outlasted the collection.
	if (nesting.parent != 0 && arguments.disabled == nesting.chain)
	{
		const Context* parent = find_open_context(nesting.parent);
		if (parent != nullptr && parent->expansion.root != 0)
		{
			arguments.disabled_top = parent->serial;
			arguments.disabled_root = parent->expansion.root;
		}
	}
	check_argument_count(macro, head, arguments);
	invocations_.push_back(
		Invocation{ &macro, head, nesting, passes_disabled(head, nesting.parent) ? context_disabled : 0,
			passes_disabled(head, nesting.parent) ? inherited : 0, arguments, 0, pending_space_ });
	telemetry::note_peak(state_.counters.peak_invocations, invocations_.size());
	advance_arguments();
}

void MacroExpander::collect_arguments(const MacroDefinition& macro, const PPToken& head, Arguments& arguments)
{
	// Arguments are separated by commas outside nested parentheses; the
	// variable arguments take the rest, commas included (16.3p11-p12). The
	// tokens are copied, and each nested `(` and comma is linked to the next
	// comma or `)` at its level, so an invocation inside the argument can
	// later take its arguments without scanning or copying them.
	const uint32_t variable = macro.variadic ? macro.parameter_count - 1u : UINT32_MAX;
	uint32_t index = 0;
	uint32_t depth = 0;
	uint32_t start = arguments.token_mark;
	link_stack_.clear();
	uint32_t previous_serial = 0;
	PPToken token;
	for (;;)
	{
		if (!read(token) || token.kind == PPTokenKind::EndOfFile)
		{
			if (token.kind == PPTokenKind::EndOfFile && environment_.continue_macro_arguments())
			{
				previous_serial = 0;
				continue;
			}
			throw PreprocessError(head.begin, "unterminated macro invocation");
		}
		if (read_serial_ != 0 && read_serial_ == previous_serial)
			token.flags |= kFollowsInContext;
		previous_serial = read_serial_;
		if (read_serial_ == 0 && !contexts_.empty())
		{
			// Read from the argument being replaced below the expansions the
			// invocation began in: what is disabled there stays disabled.
			arguments.outer = hide_sets_.merge(arguments.outer, read_disabled_);
			token.flags |= kFromArgument;
		}
		const uint32_t position = static_cast<uint32_t>(argument_tokens_.size());
		if (token.kind == PPTokenKind::PreprocessingOpOrPunc)
		{
			if (IsPunctuator(token.punctuator, TokenKind::OP_LPAREN))
			{
				++depth;
				link_stack_.push_back(position);
			}
			else if (IsPunctuator(token.punctuator, TokenKind::OP_RPAREN))
			{
				if (depth == 0)
					break;
				--depth;
				argument_links_[link_stack_.back()] = position;
				link_stack_.pop_back();
			}
			else if (IsPunctuator(token.punctuator, TokenKind::OP_COMMA))
			{
				if (depth != 0)
				{
					argument_links_[link_stack_.back()] = position;
					link_stack_.back() = position;
				}
				else if (index != variable)
				{
					argument_bounds_.push_back(start);
					argument_bounds_.push_back(position);
					start = position;
					++index;
					continue;
				}
			}
		}
		argument_tokens_.push_back(token);
		argument_links_.push_back(UINT32_MAX);
	}
	argument_bounds_.push_back(start);
	argument_bounds_.push_back(static_cast<uint32_t>(argument_tokens_.size()));
	arguments.count = index + 1;
	state_.counters.argument_tokens += argument_tokens_.size() - arguments.token_mark;
}

bool MacroExpander::collect_nested_arguments(const MacroDefinition& macro, Arguments& arguments)
{
	// An invocation read from an argument being replaced takes its
	// arguments as ranges of that argument by following the links, without
	// copying: collection changes no token, and what is disabled is decided
	// as each one is read.
	if (contexts_.empty())
		return false;
	Context& context = contexts_.back();
	if (context.macro != nullptr)
		return false;
	++state_.counters.nested_collections;
	arguments.outer = hide_sets_.merge(arguments.disabled, context.barrier.outer);
	const uint32_t variable = macro.variadic ? macro.parameter_count - 1u : UINT32_MAX;
	const uint32_t open = context.cursor;
	uint32_t index = 0;
	uint32_t start = open + 1;
	uint32_t at = argument_links_[open];
	while (!IsPunctuatorToken(argument_tokens_[at], TokenKind::OP_RPAREN))
	{
		if (index != variable)
		{
			argument_bounds_.push_back(start);
			argument_bounds_.push_back(at);
			start = at + 1;
			++index;
		}
		at = argument_links_[at];
	}
	argument_bounds_.push_back(start);
	argument_bounds_.push_back(at);
	arguments.count = index + 1;
	context.cursor = at + 1;
	return true;
}

void MacroExpander::check_argument_count(const MacroDefinition& macro, const PPToken& head, Arguments& arguments)
{
	const uint32_t parameters = macro.parameter_count;
	if (parameters == 0)
	{
		// `()` is one empty argument, which a macro without parameters takes.
		if (arguments.count != 1 || argument_bounds_[arguments.bounds] != argument_bounds_[arguments.bounds + 1])
			throw PreprocessError(head.begin, "too many macro arguments");
		arguments.count = 0;
	}
	else if (arguments.count + 1 == parameters && macro.variadic)
	{
		// No variable arguments at all: they are empty (as the course
		// reference and C++20 16.3p12 allow).
		const uint32_t end = argument_bounds_.back();
		argument_bounds_.push_back(end);
		argument_bounds_.push_back(end);
		arguments.count = parameters;
	}
	else if (arguments.count != parameters)
	{
		throw PreprocessError(
			head.begin, arguments.count < parameters ? "too few macro arguments" : "too many macro arguments");
	}
}

void MacroExpander::advance_arguments()
{
	// Starts replacing the next argument used as a plain parameter, or
	// completes the invocation when none is left.
	Invocation& invocation = invocations_.back();
	const Arguments& arguments = invocation.arguments;
	while (invocation.argument < arguments.count)
	{
		const uint32_t i = invocation.argument;
		if ((invocation.macro->parameter_uses[i] & kParameterExpanded) != 0)
		{
			++state_.counters.argument_expansions;
			expanded_bounds_.push_back(static_cast<uint32_t>(expanded_tokens_.size()));
			const uint32_t begin = argument_bounds_[arguments.bounds + 2 * i];
			const uint32_t end = argument_bounds_[arguments.bounds + 2 * i + 1];
			push_barrier(arguments, begin, end);
			pending_space_ = false;
			return;
		}
		expanded_bounds_.push_back(0);
		expanded_bounds_.push_back(0);
		++invocation.argument;
	}
	complete_invocation();
}

void MacroExpander::push_barrier(const Arguments& arguments, uint32_t begin, uint32_t end)
{
	Context context;
	context.begin = begin;
	context.cursor = begin;
	context.end = end;
	context.serial = state_.serial;
	context.macro = nullptr;
	context.disabled = arguments.disabled;
	context.inherited = arguments.disabled;
	context.barrier.outer = arguments.outer;
	context.barrier.disabled_top = arguments.disabled_top;
	context.barrier.disabled_root = arguments.disabled_root;
	contexts_.push_back(context);
}

void MacroExpander::finish_argument()
{
	// Contexts above the barrier were popped by the read that reached it.
	contexts_.pop_back();
	expanded_bounds_.push_back(static_cast<uint32_t>(expanded_tokens_.size()));
	++invocations_.back().argument;
	advance_arguments();
}

void MacroExpander::complete_invocation()
{
	// The expansion is pushed while the invocation's record still holds its
	// arguments; the record goes after.
	const Invocation& invocation = invocations_.back();
	pending_space_ = invocation.pending_space;
	push_expansion(*invocation.macro, invocation.head, invocation.nesting, invocation.list_disabled,
		invocation.inherited, &invocation.arguments);
	const Arguments& arguments = invocation.arguments;
	argument_tokens_.resize(arguments.token_mark);
	argument_links_.resize(arguments.token_mark);
	argument_bounds_.resize(arguments.bounds);
	expanded_tokens_.resize(arguments.expanded_mark);
	expanded_bounds_.resize(arguments.expanded);
	invocations_.pop_back();
}

// ---- substitution -----------------------------------------------------------------

void MacroExpander::push_expansion(MacroDefinition& macro, const PPToken& head, const Nesting& nesting,
	uint32_t disabled, uint32_t inherited, const Arguments* arguments)
{
	++state_.counters.invocations;
	// No context or hide set can be observed when the replacement is empty.
	if (macro.body_size == 0)
	{
		pending_space_ = pending_space_ || (head.flags & kLeadingSpace) != 0;
		return;
	}
	const uint32_t begin = static_cast<uint32_t>(context_tokens_.size());
	// The arguments are hidden from the name token's set if the name is an
	// argument token.
	const uint32_t argument_hide_set = arguments != nullptr
		? hide_sets_.add((head.flags & kArgument) != 0 ? head.hide_set : 0, macro.name)
		: 0;
	// Serials are not reused, so they order contexts and name each one for
	// the translation unit.
	if (state_.serial == UINT32_MAX)
		throw PreprocessError(head.begin, "too many macro invocations in one translation unit");
	building_head_ = head;
	building_serial_ = ++state_.serial;
	building_chain_ = hide_sets_.add(nesting.chain, macro.name);
	bool paste = false;
	bool has_placemarkers = false;
	bool comma_paste = false;
	bool first_item_emitted = false;
	for (uint32_t i = 0; i < macro.body_size; ++i)
	{
		const ReplacementToken& replacement = macro.body[i];
		switch (replacement.role)
		{
		case ReplacementRole::Token:
		{
			PPToken token = replacement.token;
			token.begin = head.begin;
			token.end = head.end;
			token.hide_set = building_chain_;
			token.origin = building_serial_;
			emit(token, paste);
			break;
		}
		case ReplacementRole::Parameter:
		{
			const uint32_t index = arguments->expanded + 2u * replacement.parameter;
			substitute(expanded_tokens_, expanded_bounds_[index], expanded_bounds_[index + 1], replacement,
				argument_hide_set, true, paste);
			break;
		}
		case ReplacementRole::RawParameter:
		{
			const uint32_t raw_begin = argument_bounds_[arguments->bounds + 2u * replacement.parameter];
			const uint32_t raw_end = argument_bounds_[arguments->bounds + 2u * replacement.parameter + 1];
			if (comma_paste)
			{
				// Empty variable arguments remove the comma, and are still a
				// placemarker for a ## that follows.
				comma_paste = false;
				if (raw_begin == raw_end)
					context_tokens_.pop_back();
			}
			if (raw_begin == raw_end)
			{
				has_placemarkers = true;
				PPToken placemarker = PPToken();
				placemarker.kind = PPTokenKind::Placemarker;
				placemarker.spelling = "";
				placemarker.begin = head.begin;
				placemarker.end = head.end;
				emit(placemarker, paste);
				break;
			}
			substitute(argument_tokens_, raw_begin, raw_end, replacement, 0, false, paste);
			break;
		}
		case ReplacementRole::Stringize:
		{
			const uint32_t raw_begin = argument_bounds_[arguments->bounds + 2u * replacement.parameter];
			const uint32_t raw_end = argument_bounds_[arguments->bounds + 2u * replacement.parameter + 1];
			emit(stringize(raw_begin, raw_end, replacement, head), paste);
			break;
		}
		case ReplacementRole::StringizedParameter:
			break;
		case ReplacementRole::Paste:
			paste = true;
			break;
		case ReplacementRole::CommaPaste:
			comma_paste = true;
			break;
		}
		if (i == 0)
			first_item_emitted = context_tokens_.size() != begin;
	}
	// The first item's token, with the ones pasted to it, stands where the
	// macro name stood.
	first_item_emitted = first_item_emitted && context_tokens_[begin].kind != PPTokenKind::Placemarker;
	// Only an empty raw operand of ## emits a placemarker. Ordinary
	// substitution and nonempty pastes need no second pass over the tokens.
	if (has_placemarkers)
		remove_placemarkers(begin);

	const uint32_t end = static_cast<uint32_t>(context_tokens_.size());
	state_.counters.produced_tokens += end - begin;
	if (begin == end)
	{
		if ((head.flags & kLeadingSpace) != 0)
			pending_space_ = true;
		return;
	}
	// The expansion takes the place of the macro name, and its white space:
	// the first item's token takes the name's instead of its own; after
	// items that produced nothing, the first token keeps its own too.
	PPToken& first = context_tokens_[begin];
	const uint8_t own = first_item_emitted ? 0 : first.flags;
	first.flags = static_cast<uint8_t>(
		(first.flags & ~(kLeadingSpace | kStartOfLine)) | ((head.flags | own) & (kLeadingSpace | kStartOfLine)));
	Context context;
	context.begin = begin;
	context.cursor = begin;
	context.end = end;
	context.serial = building_serial_;
	context.macro = &macro;
	context.disabled = disabled;
	context.inherited = inherited;
	context.expansion.chain = building_chain_;
	context.expansion.root = chain_root(nesting.parent);
	context.expansion.previous_open = macro.open_serial;
	contexts_.push_back(context);
	macro.open_serial = building_serial_;
	telemetry::note_peak(state_.counters.peak_contexts, contexts_.size());
	telemetry::note_peak(state_.counters.peak_context_tokens, context_tokens_.size());
}

void MacroExpander::substitute(const std::vector<PPToken>& source, uint32_t begin, uint32_t end,
	const ReplacementToken& parameter, uint32_t hide_set, bool replace_hide_set, bool& paste)
{
	for (uint32_t i = begin; i < end; ++i)
	{
		PPToken token = source[i];
		// A macro-replaced argument's hide set is the macro plus, for an
		// argument name token, that token's set; used as written, a token
		// substituted that way loses that macro again (course reference).
		if (replace_hide_set)
		{
			token.hide_set = hide_set;
			token.origin = 0;
		}
		else if ((token.flags & kReplacedArgument) != 0)
			token.hide_set = hide_sets_.without_last(token.hide_set);
		token.flags = static_cast<uint8_t>(
			(token.flags & ~(kStartOfLine | kReplacedArgument | kFollowsInContext | kFromArgument)) | kArgument |
			(replace_hide_set ? kReplacedArgument : 0));
		// The argument takes the place of the parameter, and its white space.
		if (i == begin)
			token.flags = static_cast<uint8_t>(
				(token.flags & ~kLeadingSpace) | (parameter.token.flags & kLeadingSpace));
		emit(token, paste);
	}
}

void MacroExpander::emit(const PPToken& token, bool& paste)
{
	if (!paste)
	{
		context_tokens_.push_back(token);
		return;
	}
	paste = false;
	PPToken& left = context_tokens_.back();
	left = paste_tokens(left, token);
}

PPToken MacroExpander::stringize(
	uint32_t begin, uint32_t end, const ReplacementToken& operator_token, const PPToken& head)
{
	// White space between the argument's tokens becomes one space; `"` and
	// `\` in character and string literals are escaped (16.3.2p2).
	++state_.counters.stringizes;
	scratch_.assign(1, '"');
	for (uint32_t i = begin; i < end; ++i)
	{
		const PPToken& token = argument_tokens_[i];
		if (i != begin && (token.flags & kLeadingSpace) != 0)
			scratch_.push_back(' ');
		if (!IsTextLiteral(token.kind))
		{
			scratch_.append(token.spelling, token.length);
			continue;
		}
		for (uint32_t k = 0; k < token.length; ++k)
		{
			const char c = token.spelling[k];
			if (c == '"' || c == '\\')
				scratch_.push_back('\\');
			scratch_.push_back(c);
		}
	}
	scratch_.push_back('"');

	PPToken token = PPToken();
	token.kind = PPTokenKind::StringLiteral;
	token.flags = operator_token.token.flags & kLeadingSpace;
	token.begin = head.begin;
	token.end = head.end;
	token.spelling = arena_.copy_string(scratch_.data(), scratch_.size());
	token.length = static_cast<uint32_t>(scratch_.size());
	return token;
}

PPToken MacroExpander::paste_tokens(const PPToken& left, const PPToken& right)
{
	// A placemarker operand leaves the other operand as it is (16.3.3p3).
	// Either way the result is a token of the replacement list again: it
	// carries the context's chain, the operand also its own hide set.
	const PPToken& head = building_head_;
	if (left.kind == PPTokenKind::Placemarker || right.kind == PPTokenKind::Placemarker)
	{
		PPToken result = left.kind == PPTokenKind::Placemarker ? right : left;
		result.flags &= static_cast<uint8_t>(~(kArgument | kReplacedArgument | kFollowsInContext | kFromArgument));
		if (result.kind != PPTokenKind::Placemarker)
			result.hide_set = hide_sets_.merge(building_chain_, result.hide_set);
		result.origin = building_serial_;
		return result;
	}

	// The spellings are joined and lexed again; they shall form exactly one
	// preprocessing token (16.3.3p3; an error under the course rules).
	++state_.counters.pastes;
	scratch_.assign(left.spelling, left.length);
	scratch_.append(right.spelling, right.length);
	const SourceBuffer text = SourceBuffer::ForTokenText(scratch_);
	PPLexerOptions options;
	options.header_names_on_request = true;
	options.token_text = true;
	options.binary_exponents = binary_exponents_;
	PPLexer lexer(text, options);
	PPToken result = PPToken();
	try
	{
		const PPToken& token = lexer.next();
		if (token.kind == PPTokenKind::WhitespaceSequence || token.kind == PPTokenKind::NewLine ||
			token.end != scratch_.size())
		{
			throw PreprocessError(head.begin, "token pasting does not form one preprocessing token");
		}
		result.kind = token.kind;
		result.punctuator = token.punctuator;
		if (token.kind == PPTokenKind::Identifier)
		{
			result.identifier = identifiers_.intern(token.spelling, token.length);
			result.spelling = result.identifier->spelling;
			if (result.identifier == names_.va_args)
				throw PreprocessError(head.begin, "__VA_ARGS__ formed by token pasting");
		}
		else
		{
			result.spelling = arena_.copy_string(token.spelling, token.length);
		}
		result.length = token.length;
	}
	catch (const SourceError&)
	{
		throw PreprocessError(head.begin, "token pasting does not form a valid preprocessing token");
	}
	result.flags = left.flags & (kLeadingSpace | kStartOfLine);
	result.begin = head.begin;
	result.end = head.end;
	result.hide_set = building_chain_;
	result.origin = building_serial_;
	return result;
}

void MacroExpander::remove_placemarkers(uint32_t begin)
{
	uint32_t out = begin;
	while (out < context_tokens_.size() && context_tokens_[out].kind != PPTokenKind::Placemarker)
		++out;
	if (out == context_tokens_.size())
		return;
	for (uint32_t i = out + 1; i < context_tokens_.size(); ++i)
	{
		if (context_tokens_[i].kind != PPTokenKind::Placemarker)
			context_tokens_[out++] = context_tokens_[i];
	}
	context_tokens_.resize(out);
}

// ---- builtin macros and controlling-expression operators ---------------------------

void MacroExpander::replace_builtin(const MacroDefinition& macro)
{
	switch (macro.builtin)
	{
	case BuiltinMacro::Line:
	{
		char digits[24];
		const int length = std::snprintf(
			digits, sizeof digits, "%llu", static_cast<unsigned long long>(environment_.presumed_line(current_.begin)));
		set_number(arena_.copy_string(digits, static_cast<std::size_t>(length)), static_cast<uint32_t>(length));
		return;
	}
	case BuiltinMacro::Counter:
	{
		char digits[24];
		const int length = std::snprintf(
			digits, sizeof digits, "%llu", static_cast<unsigned long long>(state_.counter++));
		set_number(arena_.copy_string(digits, static_cast<std::size_t>(length)), static_cast<uint32_t>(length));
		return;
	}
	case BuiltinMacro::File:
	{
		const TokenSpelling literal = environment_.presumed_file_literal(current_.begin);
		current_.kind = PPTokenKind::StringLiteral;
		current_.identifier = nullptr;
		current_.spelling = literal.text;
		current_.length = literal.length;
		current_.hide_set = 0;
		return;
	}
	case BuiltinMacro::HasCppAttribute:
	case BuiltinMacro::HasAttribute:
		// An operator of controlling expressions; elsewhere an identifier.
		if (controlling_expression_)
			replace_has_cpp_attribute(macro.builtin == BuiltinMacro::HasAttribute);
		return;
	case BuiltinMacro::HasInclude:
	case BuiltinMacro::HasIncludeNext:
		if (controlling_expression_)
			replace_has_include(macro.builtin == BuiltinMacro::HasIncludeNext);
		return;
	case BuiltinMacro::HasBuiltin:
		if (controlling_expression_)
			replace_has_builtin();
		return;
	case BuiltinMacro::HasFeature:
	case BuiltinMacro::HasExtension:
	case BuiltinMacro::HasDeclspecAttribute:
	case BuiltinMacro::HasConstexprBuiltin:
	case BuiltinMacro::HasRmwBuiltin:
	case BuiltinMacro::BuildingModule:
	case BuiltinMacro::IsIdentifier:
		if (controlling_expression_)
			replace_identifier_probe(macro.builtin);
		return;
	case BuiltinMacro::HasWarning:
		if (controlling_expression_)
			replace_has_warning();
		return;
	case BuiltinMacro::None:
		return;
	}
}

void MacroExpander::replace_defined()
{
	// `defined identifier` or `defined ( identifier )`; the identifier is not
	// macro-replaced (16.1p1, p4). An identifier-like operator such as `and`
	// names no macro (course fixture); other operands are errors. `defined`
	// produced by macro replacement is evaluated the same way.
	const PPToken defined = current_;
	PPToken operand;
	read(operand);
	const bool parenthesized = IsPunctuatorToken(operand, TokenKind::OP_LPAREN);
	if (parenthesized)
		read(operand);
	bool is_defined = false;
	if (operand.kind == PPTokenKind::Identifier)
		is_defined = operand.identifier->macro != nullptr;
	else if (!IsIdentifierLikeOperator(operand))
		throw PreprocessError(defined.begin, "`defined` requires an identifier");
	if (parenthesized)
	{
		PPToken close;
		read(close);
		if (!IsPunctuatorToken(close, TokenKind::OP_RPAREN))
			throw PreprocessError(defined.begin, "missing `)` after `defined`");
	}
	current_ = defined;
	set_number(is_defined ? "1" : "0", 1);
}

void MacroExpander::replace_has_cpp_attribute(bool gnu)
{
	// `__has_cpp_attribute ( attribute-token )` (or `__has_attribute`),
	// where the attribute-token is an identifier or `identifier ::
	// identifier`, read without replacement as the course reference does.
	const PPToken operator_token = current_;
	PPToken token;
	read(token);
	if (!IsPunctuatorToken(token, TokenKind::OP_LPAREN))
		throw PreprocessError(operator_token.begin, "expected `(` after __has_cpp_attribute");
	read(token);
	if (token.kind != PPTokenKind::Identifier && !IsIdentifierLikeOperator(token))
		throw PreprocessError(operator_token.begin, "expected an attribute name");
	bool reported = IsReportedAttribute(token.spelling, token.length, gnu);
	read(token);
	if (IsPunctuatorToken(token, TokenKind::OP_COLON2))
	{
		read(token);
		if (token.kind != PPTokenKind::Identifier && !IsIdentifierLikeOperator(token))
			throw PreprocessError(operator_token.begin, "expected an attribute name");
		reported = false;
		read(token);
	}
	if (!IsPunctuatorToken(token, TokenKind::OP_RPAREN))
		throw PreprocessError(operator_token.begin, "expected `)` after the attribute name");
	current_ = operator_token;
	set_number(reported ? "1" : "0", 1);
}

void MacroExpander::replace_has_include(bool next)
{
	// `__has_include ( header-name )` (GNU, C++17's 16.1p1): a string
	// literal, or the tokens `<` ... `>` spelled as #include combines them.
	// A directly written header is not replaced; other operands are rescanned
	// with the ordinary macro machinery, as for a computed #include.
	const PPToken operator_token = current_;
	PPToken token;
	read(token);
	if (!IsPunctuatorToken(token, TokenKind::OP_LPAREN))
		throw PreprocessError(operator_token.begin, "expected `(` after __has_include");
	read(token);
	const bool expand = token.kind != PPTokenKind::StringLiteral && !IsPunctuatorToken(token, TokenKind::OP_LT);
	if (expand)
		token = expand_include_token(token);
	std::string name;
	bool angle = false;
	TextLiteralParts parts;
	if (token.kind == PPTokenKind::StringLiteral && SplitTextLiteral(token.spelling, token.length, parts) &&
		parts.prefix == EncodingPrefix::None && !parts.raw)
	{
		name.assign(token.spelling + parts.body_begin, parts.body_end - parts.body_begin);
	}
	else if (IsPunctuatorToken(token, TokenKind::OP_LT))
	{
		angle = true;
		bool first = true;
		for (read(token);; read(token), first = false)
		{
			if (expand)
				token = expand_include_token(token);
			if (IsPunctuatorToken(token, TokenKind::OP_GT))
				break;
			if (token.kind == PPTokenKind::EndOfFile || token.kind == PPTokenKind::NewLine)
				throw PreprocessError(operator_token.begin, "missing `>` in __has_include");
			if (!first && (token.flags & kLeadingSpace) != 0)
				name.push_back(' ');
			name.append(token.spelling, token.length);
		}
	}
	else
	{
		throw PreprocessError(operator_token.begin, "expected a header name after __has_include");
	}
	read(token);
	if (expand)
		token = expand_include_token(token);
	if (!IsPunctuatorToken(token, TokenKind::OP_RPAREN))
		throw PreprocessError(operator_token.begin, "expected `)` after the header name");
	const bool found = environment_.can_include(name, angle, next);
	current_ = operator_token;
	set_number(found ? "1" : "0", 1);
}

PPToken MacroExpander::expand_include_token(PPToken token)
{
	const std::size_t depth = invocations_.size();
	for (;;)
	{
		current_ = token;
		if (current_.kind == PPTokenKind::Identifier && start_expansion())
		{
			// Continue rescanning the expansion it opened.
		}
		else if (invocations_.size() == depth)
		{
			return current_;
		}
		else
		{
			expanded_tokens_.push_back(current_);
		}
		while (!read(token))
		{
			if (invocations_.size() == depth)
				return token;
			finish_argument();
		}
	}
}

void MacroExpander::replace_has_builtin()
{
	// `__has_builtin ( identifier )` (GNU): whether the identifier names a
	// builtin function, trait or type this implementation provides.
	const PPToken operator_token = current_;
	const PPToken operand = read_probe_operand(operator_token);
	if (operand.kind != PPTokenKind::Identifier && !IsIdentifierLikeOperator(operand))
		throw PreprocessError(operator_token.begin, "expected an identifier after __has_builtin");
	const bool provided = operand.kind == PPTokenKind::Identifier &&
		IsProvidedBuiltin(operand.spelling, operand.length);
	current_ = operator_token;
	set_number(provided ? "1" : "0", 1);
}

void MacroExpander::replace_identifier_probe(BuiltinMacro probe)
{
	// A probe answers for its whole family: `__has_feature` and
	// `__has_extension` by the features this implementation provides;
	// `__is_identifier ( token )` (Clang) whether the token is an identifier
	// the parser reads as one, not a keyword, a GNU keyword spelling or a
	// trait; the guarantees this implementation does not make -- Microsoft
	// declspec attributes, constant evaluation or read-modify-write forms
	// promised for builtins, Clang modules -- are never present.
	const PPToken operator_token = current_;
	const PPToken operand = read_probe_operand(operator_token);
	bool answer = false;
	if (probe == BuiltinMacro::IsIdentifier)
	{
		GnuTrait trait;
		answer = operand.kind == PPTokenKind::Identifier && operand.identifier->keyword == 0 &&
			!IsGnuKeyword(operand.spelling, operand.length) && !FindGnuTrait(operand.spelling, operand.length, trait);
	}
	else
	{
		if (operand.kind != PPTokenKind::Identifier && !IsIdentifierLikeOperator(operand))
			throw PreprocessError(operator_token.begin, "expected an identifier operand");
		if (probe == BuiltinMacro::HasFeature || probe == BuiltinMacro::HasExtension)
			answer = environment_.has_feature(operand.spelling, operand.length);
	}
	current_ = operator_token;
	set_number(answer ? "1" : "0", 1);
}

void MacroExpander::replace_has_warning()
{
	// `__has_warning ( string-literal )` (Clang): no warning option is
	// recognized.
	const PPToken operator_token = current_;
	const PPToken operand = read_probe_operand(operator_token);
	if (operand.kind != PPTokenKind::StringLiteral)
		throw PreprocessError(operator_token.begin, "expected a string literal after __has_warning");
	current_ = operator_token;
	set_number("0", 1);
}

PPToken MacroExpander::read_probe_operand(const PPToken& probe)
{
	PPToken token;
	read(token);
	if (!IsPunctuatorToken(token, TokenKind::OP_LPAREN))
		throw PreprocessError(probe.begin, "expected `(` after the probe");
	PPToken operand;
	read(operand);
	if (operand.kind == PPTokenKind::EndOfFile || operand.kind == PPTokenKind::NewLine)
		throw PreprocessError(probe.begin, "expected an operand of the probe");
	read(token);
	if (!IsPunctuatorToken(token, TokenKind::OP_RPAREN))
		throw PreprocessError(probe.begin, "expected `)` after the probe's operand");
	return operand;
}

void MacroExpander::set_number(const char* spelling, uint32_t length)
{
	current_.kind = PPTokenKind::PPNumber;
	current_.identifier = nullptr;
	current_.punctuator = kNoPunctuator;
	current_.spelling = spelling;
	current_.length = length;
	current_.hide_set = 0;
	current_.flags &= static_cast<uint8_t>(kLeadingSpace | kStartOfLine);
}
