#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "lex/identifier_table.h"
#include "lex/token_spelling.h"
#include "preprocess/lexer/pp_token.h"
#include "preprocess/macro/hide_sets.h"
#include "preprocess/macro/macro_definition.h"
#include "preprocess/preprocessor_names.h"
#include "support/memory/arena.h"
#include "support/telemetry.h"

// Where macro replacement reads once no expansion context is open: the text
// of the translation unit, or the rest of a directive line.
class ExpansionInput
{
public:
	// The next token, or EndOfFile where the input ends for macro
	// replacement: at a directive in the text (a text-sequence ends there,
	// macros.md), at the new-line ending a directive, or at the end of the
	// translation unit.
	virtual const PPToken& next() = 0;
	// The token next() will return, left unread.
	virtual const PPToken& peek() = 0;

protected:
	~ExpansionInput() {}
};

// What replacing __FILE__ and __LINE__ needs from the preprocessor.
class ExpansionEnvironment
{
public:
	// The presumed line number at a location ([cpp.predefined] 16.8p1).
	virtual uint64_t presumed_line(uint32_t location) = 0;
	// __FILE__ at a location: the presumed file name as the spelling of a
	// character string literal.
	virtual TokenSpelling presumed_file_literal(uint32_t location) = 0;
	// Whether `#include` (`#include_next` with `next`) of the name would
	// find a file (the hosted __has_include operators).
	virtual bool can_include(const std::string& name, bool angle, bool next) = 0;
	// Whether the language feature is provided (`__has_feature` and
	// `__has_extension`, preprocess/macro/feature_probes.h).
	virtual bool has_feature(const char* spelling, uint32_t length) = 0;

protected:
	~ExpansionEnvironment() {}
};

// Work counters (spec §9 telemetry); they only observe work already done.
struct MacroExpansionCounters
{
	telemetry::Counter invocations = 0;
	telemetry::Counter produced_tokens = 0;
	telemetry::Counter argument_tokens = 0;
	telemetry::Counter argument_expansions = 0;
	telemetry::Counter nested_collections = 0;
	telemetry::Counter pastes = 0;
	telemetry::Counter stringizes = 0;
	// ## operands used as written that their own hide set hides.
	telemetry::Counter raw_operands_hidden = 0;
	telemetry::Counter peak_contexts = 0;
	telemetry::Counter peak_context_tokens = 0;
	telemetry::Counter peak_invocations = 0;
};

// Macro replacement ([cpp.replace] 16.3) as a pull cursor over an input.
//
// Recursion follows the course rule (macros.md) as the course reference
// implements it. Every token carries a hide set of macro names it may not
// invoke; a name found in its own token's set is painted and never
// replaced. An expansion's replacement-list tokens and pasted tokens carry
// the context's chain: the macro plus the chain of the context its name
// token came from, if that token was a replacement-list token of a context
// still open when the invocation begins (an invocation begun inside a
// context is nested in it; contexts are popped lazily, when a read goes
// past their end). An invocation named by an argument token is not nested:
// nesting does not survive parameter substitution. Substituted
// macro-replaced arguments carry the macro plus, if the name token is an
// argument token, the name token's set (replacing their own); operands of
// ## keep theirs, except that a token last substituted as a macro-replaced
// argument drops the macro it was substituted into.
//
// While an invocation's argument is macro-replaced, more names are
// disabled: those of its name token's set if the invocation is nested,
// and those inherited where the name token was
// read. They apply to every token of the argument, and to the
// replacement-list tokens of an expansion named by a token of the argument
// or of such a list, to any depth, if the name came from the source text or
// the invocation is nested; tokens substituted into those expansions carry
// only their own sets. The replacement of an argument of an invocation
// begun inside the argument, or inside such an expansion, inherits them,
// except for one named by a token substituted as written into the
// expansion it begins in; tokens that invocation collects from the
// argument keep them anyway. An invocation begins when an object-like name
// is read, or when a function-like name's `(` is found.
//
// Tokens being rescanned live in three LIFO buffers: expansion contexts,
// collected arguments and macro-replaced arguments. Each invocation's
// storage is released when its expansion is pushed (arguments) or popped
// (contexts), so memory follows the nesting depth, not the output length.
// An invocation inside an argument takes its arguments as ranges of that
// argument, so nested invocations cost time and memory in proportion to
// their own tokens.
class MacroExpander
{
public:
	MacroExpander(
		Arena& arena, IdentifierTable& identifiers, const PreprocessorNames& names, ExpansionEnvironment& environment);

	void set_input(ExpansionInput* input) { input_ = input; }
	// In a controlling expression, `defined` and __has_cpp_attribute are
	// evaluated as they are rescanned ([cpp.cond] 16.1p4).
	void set_controlling_expression(bool enabled) { controlling_expression_ = enabled; }
	// Pasted spellings are lexed as hosted pp-numbers
	// (PPLexerOptions::binary_exponents).
	void set_binary_exponents(bool enabled) { binary_exponents_ = enabled; }

	// The next token of the macro-replaced input, with the input's EndOfFile
	// at its end. Valid until the next call. Throws PreprocessError.
	const PPToken& next();

	const MacroExpansionCounters& counters() const { return counters_; }

private:
	// An expansion, reading [begin, end) of context_tokens_, or the barrier
	// at the end of an argument being replaced, reading argument_tokens_.
	struct Context
	{
		uint32_t begin;
		uint32_t cursor;
		uint32_t end;
		// An expansion's serial number; a barrier's is the last one given
		// out when it was pushed. Serials never decrease up the stack.
		uint32_t serial;
		// Null for a barrier.
		MacroDefinition* macro;
		// Names disabled for the argument's tokens (a barrier) or for the
		// replacement-list and pasted tokens (an expansion), and the names
		// an invocation named by a token read here keeps disabled while its
		// arguments are replaced.
		uint32_t disabled;
		uint32_t inherited;
		union
		{
			// The expansion's chain, the serial of the first context of its
			// nesting chain (0 if one of them has ended), and the macro's
			// previous open context.
			struct
			{
				uint32_t chain;
				uint32_t root;
				uint32_t previous_open;
			} expansion;
			// The names disabled instead for tokens collected from an
			// enclosing argument (kFromArgument); and if `disabled` is the
			// chain of an expansion open below, with a root, that
			// expansion's serial and root (else zeros), so membership takes
			// the open-context test.
			struct
			{
				uint32_t outer;
				uint32_t disabled_top;
				uint32_t disabled_root;
			} barrier;
		};
	};

	// One invocation's arguments. Argument i is the range
	// [argument_bounds_[bounds + 2i], argument_bounds_[bounds + 2i + 1]) of
	// argument_tokens_, and its replacement the same pair of
	// expanded_bounds_ at `expanded`. The marks restore the buffers.
	struct Arguments
	{
		uint32_t count;
		uint32_t bounds;
		uint32_t expanded;
		uint32_t token_mark;
		uint32_t expanded_mark;
		// The names disabled while an argument is macro-replaced, and for
		// its tokens collected from an enclosing argument; Context's
		// disabled_top and disabled_root.
		uint32_t disabled;
		uint32_t outer;
		uint32_t disabled_top;
		uint32_t disabled_root;
	};

	// The context an invocation is nested in, taken when it begins: its
	// serial and chain, or zeros.
	struct Nesting
	{
		uint32_t parent;
		uint32_t chain;
	};

	// A function-like invocation whose arguments are being replaced.
	struct Invocation
	{
		MacroDefinition* macro;
		PPToken head;
		// The context the invocation is nested in, and the expansion's
		// Context::disabled and Context::inherited.
		Nesting nesting;
		uint32_t list_disabled;
		uint32_t inherited;
		Arguments arguments;
		uint32_t argument;
		bool pending_space;
	};

	// find_open_context looks at this many contexts below the top before
	// searching.
	static const std::size_t kTopProbe = 4;

	bool read(PPToken& token);
	bool peek_is_left_paren();
	void pop_context();
	const Context* find_open_context(uint32_t serial) const;
	Nesting nesting_of(const PPToken& head, bool from_argument, bool function_like) const;
	uint32_t chain_root(uint32_t parent) const;
	uint32_t begin_inherited(const PPToken& head, uint32_t head_serial, bool from_argument) const;
	// Whether an expansion passes on what is disabled where its name token
	// was read (Context::disabled) and what that invocation inherits
	// (Context::inherited): if the invocation is nested or the name came
	// from the source text, not for a name substituted for a parameter or a
	// replacement-list name read after its expansion ended.
	static bool passes_disabled(const PPToken& head, uint32_t parent)
	{
		return (head.flags & kArgument) == 0 && (parent != 0 || head.origin == 0);
	}
	bool in_open_chain(const MacroDefinition& macro, uint32_t root) const
	{
		return macro.open_serial != 0 && macro.open_serial >= root;
	}
	bool start_expansion();
	bool hidden(const PPToken& token, const MacroDefinition& macro) const;
	bool disabled_here(const MacroDefinition& macro, uint32_t disabled) const;

	void begin_invocation(
		MacroDefinition& macro, const PPToken& head, uint32_t context_disabled, uint32_t inherited, bool from_argument);
	void collect_arguments(const MacroDefinition& macro, const PPToken& head, Arguments& arguments);
	bool collect_nested_arguments(const MacroDefinition& macro, Arguments& arguments);
	void check_argument_count(const MacroDefinition& macro, const PPToken& head, Arguments& arguments);
	void advance_arguments();
	void finish_argument();
	void complete_invocation();
	void push_expansion(MacroDefinition& macro, const PPToken& head, const Nesting& nesting, uint32_t disabled,
		uint32_t inherited, const Arguments* arguments);
	void push_barrier(const Arguments& arguments, uint32_t begin, uint32_t end);
	const std::vector<PPToken>& tokens_of(const Context& context) const
	{
		return context.macro != nullptr ? context_tokens_ : argument_tokens_;
	}
	void substitute(const std::vector<PPToken>& source, uint32_t begin, uint32_t end, const ReplacementToken& parameter,
		uint32_t hide_set, bool replace_hide_set, bool& paste);
	void emit(const PPToken& token, bool& paste);
	PPToken stringize(uint32_t begin, uint32_t end, const ReplacementToken& operator_token, const PPToken& head);
	PPToken paste_tokens(const PPToken& left, const PPToken& right);
	void remove_placemarkers(uint32_t begin);

	void replace_builtin(const MacroDefinition& macro);
	void replace_defined();
	void replace_has_cpp_attribute(bool gnu);
	void replace_has_include(bool next);
	// Rescan an already-read probe operand without consuming an enclosing
	// invocation's argument. Uses the ordinary expansion contexts and hides.
	PPToken expand_include_token(PPToken token);
	void replace_has_builtin();
	// The probes of one identifier (`__is_identifier`: one token) and
	// `__has_warning ( string-literal )`.
	void replace_identifier_probe(BuiltinMacro probe);
	void replace_has_warning();
	// Reads `( token )` after the probe name; the operand's token.
	PPToken read_probe_operand(const PPToken& probe);
	void set_number(const char* spelling, uint32_t length);

	Arena& arena_;
	IdentifierTable& identifiers_;
	const PreprocessorNames& names_;
	ExpansionEnvironment& environment_;
	ExpansionInput* input_;
	HideSets hide_sets_;
	bool controlling_expression_;
	bool binary_exponents_;
	// An expansion that produced nothing had white space before its name;
	// the next token read inherits it.
	bool pending_space_;
	uint64_t counter_;
	uint32_t next_serial_;
	// The expansion being built: its name token, serial and chain.
	PPToken building_head_;
	uint32_t building_serial_;
	uint32_t building_chain_;
	// The expansion context the last token was read from (zero serial for
	// the input and barriers): its serial, chain and root; and the names
	// disabled in the context (none for the input).
	uint32_t read_serial_;
	uint32_t read_chain_;
	uint32_t read_root_;
	uint32_t read_disabled_;
	PPToken current_;
	std::vector<Context> contexts_;
	std::vector<PPToken> context_tokens_;
	std::vector<PPToken> argument_tokens_;
	// For each argument token that is a nested `(` or comma: the next comma
	// or `)` at its level.
	std::vector<uint32_t> argument_links_;
	std::vector<uint32_t> link_stack_;
	std::vector<uint32_t> argument_bounds_;
	std::vector<Invocation> invocations_;
	std::vector<PPToken> expanded_tokens_;
	std::vector<uint32_t> expanded_bounds_;
	std::string scratch_;
	MacroExpansionCounters counters_;
};
