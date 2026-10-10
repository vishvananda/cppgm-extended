#pragma once

// The canonical typed ABI entity graph shared by the PA9 fact adapter and
// later compiler stages. Names, types, template arguments, expressions,
// function encodings and entity references are hash-consed nodes:
// structurally identical facts receive the same dense id, so equality and
// substitution matching are id comparisons (5.1.10 compares the represented
// constructs, not their spellings). The graph never renders text; only the
// encoder does.

#include "abi/itanium/abi_expression_vocabulary.h"
#include "abi/itanium/abi_terminal_vocabulary.h"
#include "abi/itanium/abi_type_vocabulary.h"
#include "support/flat_map.h"
#include "support/memory/arena.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace abi_mangle {

// Builtin types the fixed vocabulary (abi_type_vocabulary.h) lacks,
// numbered after its kinds, which the vocabulary's functions answer for
// too (abi_vocabulary.cpp): Clang's __bf16, DF16b (std::bfloat16_t's code).
const AbiBuiltinTypeKind ABI_BUILTIN_TYPE_BFLOAT16 = static_cast<AbiBuiltinTypeKind>(ABI_BUILTIN_TYPE_COUNT);
const unsigned kAbiBuiltinTypeCount = ABI_BUILTIN_TYPE_COUNT + 1;

// A dense node identity; 0 is "no node".
typedef std::uint32_t AbiNode;
const AbiNode ABI_NONE = 0;

// Fields not listed for a kind are zero.
enum AbiNodeKind : std::uint8_t
{
	ABI_NODE_NONE,
	ABI_NODE_STRING, // a = text index (source names, tags, raw fragments)

	// Scopes and names.
	ABI_NODE_STD,                   // ::std; the St prefix, never a candidate
	ABI_NODE_FUNCTION_SCOPE,        // a = function encoding, or raw "Z...E" STRING when flags=RAW,
									//   b = a default argument's parameter number + 1 (d [n] _,
									//   0 none: 5.1.8)
	ABI_NODE_NAME,                  // a = parent scope (0 global), b = unqualified name,
									//   c = local discriminator + 1 (under a function scope),
									//   list = ABI tags (sorted STRINGs), flags = INTERNAL,
									//   HIDDEN_TAGS (an inline namespace's: not encoded),
									//   DATA_MEMBER (a <closure-prefix>'s member: <name> M)
	ABI_NODE_OPERATOR_NAME,         // code = fixed-code AbiTerminalKind
	ABI_NODE_CTOR_DTOR_NAME,        // code = constructor/destructor AbiTerminalKind,
									//   a = an inheriting constructor's base class type (CI1/CI2)
	ABI_NODE_CONVERSION_NAME,       // a = target type
	ABI_NODE_LITERAL_OPERATOR_NAME, // a = suffix STRING
	ABI_NODE_CLOSURE_NAME,          // a = closure number + 1 (0 omitted), list = lambda-sig types, VARIADIC: z
	ABI_NODE_UNNAMED_TYPE_NAME,     // a = number + 1 (0 omitted)
	ABI_NODE_DECOMPOSITION_NAME,    // list = a structured binding declaration's names (source-name
									//   STRINGs; DC <source-name>+ E, 5.1.4)
	ABI_NODE_TEMPLATE_ID,           // a = template (NAME, TEMPLATE_PARAM, STD_ABBREVIATION), list = args
	ABI_NODE_STD_ABBREVIATION,      // code = AbiStandardSubstitutionKind, list = ABI tags (sorted STRINGs)

	// Types (class and enum types are NAME / TEMPLATE_ID / STD_ABBREVIATION).
	ABI_NODE_BUILTIN,          // code = AbiBuiltinTypeKind
	ABI_NODE_VENDOR_TYPE,      // a = source-name STRING, list = template args
	ABI_NODE_POINTER,          // a = pointee
	ABI_NODE_LVALUE_REFERENCE, // a = referee
	ABI_NODE_RVALUE_REFERENCE, // a = referee
	ABI_NODE_PACK_EXPANSION,   // a = pattern type
	ABI_NODE_COMPLEX,          // a = part type (C, a C99 complex pair)
	ABI_NODE_QUALIFIED,        // a = cv-unqualified type, code = AbiQualifier bits
	ABI_NODE_VENDOR_QUALIFIED, // a = type, b = qualifier source-name STRING
	ABI_NODE_ARRAY,            // a = element, flags = AbiArrayBound, b/c = bound value or b = expression
	ABI_NODE_FUNCTION_TYPE,    // a = result, list = parameters, flags = VARIADIC,
							   //   code = AbiQualifier bits of the function type,
							   //   b = class of a member function type (0 otherwise)
	ABI_NODE_MEMBER_POINTER,   // a = class type, b = member type
	ABI_NODE_TEMPLATE_PARAM,   // a = index, flags = SUBSTITUTABLE (types, template names, expressions)
	ABI_NODE_DECLTYPE,         // a = expression, flags = ID_EXPRESSION (Dt) or 0 (DT)

	// Template arguments (type and template arguments are the nodes above).
	ABI_NODE_LITERAL,             // a = type, b/c = value (low/high 32 bits); also an expression;
								  //   flags = FLOATING (the value is the type's IEEE bits),
								  //   UNSIGNED_VALUE (an unsigned value of a type that is no builtin)
								  //   or NO_VALUE (the nullptr keyword, L <type> E)
	ABI_NODE_TEMPLATE_PARAM_DECL, // code = AbiTemplateParamDeclKind, flags = PACK (Tp), a = a non-type
								  //   parameter's type, list = a template parameter's own declarations
	ABI_NODE_DECLARED_ARGUMENT,   // a = argument, list = its parameter's TEMPLATE_PARAM_DECLs
								  //   (<template-param-decl>+ <template-arg>)
	ABI_NODE_EXPRESSION_ARGUMENT, // a = expression
	ABI_NODE_ARGUMENT_PACK,       // list = arguments
	ABI_NODE_ENTITY_ARGUMENT,     // a = entity, flags = ADDRESS_OF

	// Expressions.
	ABI_NODE_FUNCTION_PARAM,    // a = index, b = level (fL<b-1>p), flags = THIS (fpT)
	ABI_NODE_OPERATION,         // code = packed operation, list = operands, flags = PREFIX (pp_, mm_)
	ABI_NODE_CALL,              // list = callee, arguments; flags = PARENTHESIZED (cp: a parenthesized
								//   <base-unresolved-name> callee)
	ABI_NODE_CAST,              // code = packed cast operation, a = type, b = operand
	ABI_NODE_UNRESOLVED_NAME,   // a = source-name STRING or OPERATOR_NAME (on), list = template args
	ABI_NODE_VENDOR_EXPRESSION, // a = source-name STRING, list = template args
	ABI_NODE_SIZEOF_TYPE,       // a = type, code = packed st, at or ti (0: st)
	ABI_NODE_SCOPED_MEMBER,     // a = qualifier type or UNRESOLVED_PREFIX, b = member STRING,
								//   flags = CLOSE_QUALIFIER, list = the member's template args
	ABI_NODE_UNRESOLVED_PREFIX, // a = <unresolved-type> (0 none), list = <unresolved-qualifier-level>s
								//   (UNRESOLVED_NAME simple-ids), flags = GLOBAL (gs)
	ABI_NODE_MEMBER_ACCESS,     // code = packed dt/pt, a = object, b = member STRING, list = template args
	ABI_NODE_ENTITY_REFERENCE,  // a = entity
	ABI_NODE_EXPRESSION_LIST,   // code = packed cv (cv <type> _ ... E), tl or il, a = type (0 for il),
								//   list = expressions
	ABI_NODE_ALLOCATION,        // code = packed nw, na, dl or da, flags = GLOBAL (gs); a new-expression's
								//   a = type, b = its initializer's EXPRESSION_LIST (pi or il; 0 none),
								//   list = placement arguments; a delete-expression's list = its operand

	// Entities.
	ABI_NODE_FUNCTION_ENCODING, // a = name, b = result type (0 none), list = parameters,
								//   flags = VARIADIC, code = AbiQualifier bits of the member function
	ABI_NODE_EXTERNAL_SYMBOL,   // a = mangled-name STRING (an entity known only by its symbol)
	ABI_NODE_KIND_COUNT
};

enum AbiNodeFlag : std::uint8_t
{
	ABI_FLAG_RAW = 1,
	ABI_FLAG_INTERNAL = 1,
	ABI_FLAG_VARIADIC = 1,
	ABI_FLAG_SUBSTITUTABLE = 1,
	ABI_FLAG_ID_EXPRESSION = 1,
	ABI_FLAG_ADDRESS_OF = 1,
	ABI_FLAG_CLOSE_QUALIFIER = 1,
	ABI_FLAG_FLOATING = 1,
	ABI_FLAG_THIS = 1,
	ABI_FLAG_PREFIX = 1,
	ABI_FLAG_PACK = 1,
	ABI_FLAG_GLOBAL = 1,
	ABI_FLAG_PARENTHESIZED = 1,
	ABI_FLAG_HIDDEN_TAGS = 2,
	ABI_FLAG_DATA_MEMBER = 4,
	ABI_FLAG_UNSIGNED_VALUE = 2,
	ABI_FLAG_NO_VALUE = 4
};

// The forms of a <template-param-decl>: Ty, Tn <type>, Tt <template-param-decl>* E.
enum AbiTemplateParamDeclKind : std::uint16_t
{
	ABI_PARAM_DECL_TYPE,
	ABI_PARAM_DECL_NON_TYPE,
	ABI_PARAM_DECL_TEMPLATE
};

enum AbiQualifier : std::uint16_t
{
	ABI_QUALIFIER_CONST = 1,
	ABI_QUALIFIER_VOLATILE = 2,
	ABI_QUALIFIER_RESTRICT = 4,
	ABI_QUALIFIER_LVALUE_REF = 8,
	ABI_QUALIFIER_RVALUE_REF = 16
};

enum AbiArrayBound : std::uint8_t
{
	ABI_ARRAY_BOUND_NONE,
	ABI_ARRAY_BOUND_VALUE,
	ABI_ARRAY_BOUND_EXPRESSION,
	// A vendor vector of that many lanes: Dv <number> _ <element>.
	ABI_ARRAY_BOUND_VECTOR
};

struct AbiNodeData
{
	AbiNodeKind kind;
	std::uint8_t flags;
	std::uint16_t code;
	std::uint32_t a;
	std::uint32_t b;
	std::uint32_t c;
	std::uint32_t list_begin;
	std::uint32_t list_size;
};

struct AbiText
{
	const char* text;
	std::uint32_t length;
};

// Owns the nodes of one translation unit or tool run. Construction is the
// only mutation; a node never changes once created, so an encoder reads the
// graph with nothing to invalidate. Pointers returned by list() are valid
// until the next construction.
class AbiGraph
{
public:
	AbiGraph();
	AbiGraph(const AbiGraph&) = delete;
	AbiGraph& operator=(const AbiGraph&) = delete;

	// Releases every node in bulk (a tool whose inputs share no facts
	// resets between them); ids restart and earlier ids become invalid.
	void reset();

	const AbiNodeData& node(AbiNode id) const { return nodes_[id]; }
	AbiNodeKind kind(AbiNode id) const { return nodes_[id].kind; }
	const AbiNode* list(AbiNode id) const { return lists_.data() + nodes_[id].list_begin; }
	std::uint32_t list_size(AbiNode id) const { return nodes_[id].list_size; }
	AbiText text(AbiNode string) const { return texts_[nodes_[string].a]; }
	long long value(AbiNode literal) const;
	std::uint32_t size() const { return static_cast<std::uint32_t>(nodes_.size()); }

	// Strings and scopes.
	AbiNode string(const char* text, std::size_t length);
	AbiNode std_scope();
	// `default_argument`: the closures of a member function's default
	// argument (5.1.8), the parameter number from the last plus one.
	AbiNode function_scope(AbiNode encoding, std::uint32_t default_argument = 0);
	AbiNode raw_function_scope(AbiNode fragment);
	// A closure's <closure-prefix> (5.1.8): the member or variable whose
	// initializer holds the lambda, under `parent`.
	AbiNode data_member_prefix(AbiNode parent, AbiNode member);
	// `tags` need not be sorted or unique.
	AbiNode name(AbiNode parent, AbiNode unqualified, const AbiNode* tags = nullptr, std::size_t tag_count = 0,
		std::uint32_t discriminator = 0, bool internal_linkage = false);
	// Adds ABI tags to a name, a standard abbreviation or the template of a
	// template-id. A tagged abbreviation is a substitution candidate (5.1.2).
	AbiNode with_tags(AbiNode tagged, const AbiNode* tags, std::size_t tag_count);
	// A namespace's name with the GNU ABI tags of an inline namespace,
	// which are not encoded but are the tags of the names under it.
	AbiNode tagged_namespace(AbiNode parent, AbiNode unqualified, const AbiNode* tags, std::size_t tag_count);
	// Appends the ABI tags a node's encoding writes or its namespaces carry
	// (GCC's implicit tags of a function's result type or a variable's
	// type are those its name and signature lack); sort_tags orders a
	// collection alphabetically without duplicates.
	// `written_in_scopes`: within a function scope only the tags its
	// encoding writes (a local entity's own encoding's tags, as Clang
	// derives a local entity's implicit tags).
	void collect_tags(AbiNode node, std::vector<AbiNode>& tags, bool written_in_scopes = false);
	void sort_tags(std::vector<AbiNode>& tags) const;
	// `derived` (a function's encoding or a variable's name with implicit
	// tags) is `underived` without them: collect_tags walks an entity
	// nested in another's encoding underived, as GCC's and Clang's
	// computations of the other's implicit tags mangle it.
	void note_implicit_tags(AbiNode derived, AbiNode underived);
	AbiNode operator_name(AbiTerminalKind fixed_kind);
	// `inherited`: an inheriting constructor's base class type (5.1.4.3:
	// CI1 <type>, CI2 <type>).
	AbiNode ctor_dtor_name(AbiTerminalKind kind, AbiNode inherited = ABI_NONE);
	AbiNode conversion_name(AbiNode type);
	AbiNode literal_operator_name(AbiNode suffix);
	// `number` is the Itanium closure number plus one, 0 when omitted;
	// a variadic lambda's lambda-sig ends with its ellipsis (z).
	AbiNode closure_name(const AbiNode* signature, std::size_t count, std::uint32_t number, bool variadic = false);
	AbiNode unnamed_type_name(std::uint32_t number);
	AbiNode decomposition_name(const AbiNode* names, std::size_t count);
	AbiNode template_id(AbiNode templ, const AbiNode* args, std::size_t count);
	// The template-id of a class template's specialization with the
	// standard abbreviations for ::std names applied (5.1.10: Sa and Sb for
	// the templates allocator and basic_string, Ss, Si, So and Sd for the
	// char string and stream specializations), for producers that name
	// types by their declarations.
	AbiNode abbreviated_template_id(AbiNode templ, const AbiNode* args, std::size_t count);
	AbiNode std_abbreviation(AbiStandardSubstitutionKind kind);

	// Types.
	AbiNode builtin(AbiBuiltinTypeKind kind);
	AbiNode vendor_type(AbiNode source_name, const AbiNode* args, std::size_t count);
	AbiNode pointer(AbiNode type) { return unary(ABI_NODE_POINTER, type); }
	AbiNode lvalue_reference(AbiNode type) { return unary(ABI_NODE_LVALUE_REFERENCE, type); }
	AbiNode rvalue_reference(AbiNode type) { return unary(ABI_NODE_RVALUE_REFERENCE, type); }
	AbiNode pack_expansion(AbiNode type) { return unary(ABI_NODE_PACK_EXPANSION, type); }
	AbiNode complex_pair(AbiNode type) { return unary(ABI_NODE_COMPLEX, type); }
	// Adds cv-qualifiers; qualifying a cv-qualified type merges them into one
	// canonical set (5.1.5.1), so source order never creates distinct types.
	// The qualifiers of a function type are an indivisible part of it
	// (5.1.5.3): the result is the qualified function type itself.
	AbiNode qualified(AbiNode type, unsigned cv);
	// U <source-name> <type>: a vendor qualifier keeps its position in the
	// type's structure; some (such as _Atomic) construct a distinct type
	// that cv-qualifiers then apply to.
	AbiNode vendor_qualified(AbiNode type, AbiNode qualifier);
	AbiNode array(AbiNode element, AbiArrayBound bound_kind, long long bound_value, AbiNode bound_expression);
	AbiNode function_type(AbiNode result, const AbiNode* params, std::size_t count, bool variadic, unsigned qualifiers);
	// M <class type> <member type>. A member function type is distinct from
	// every other function type, and from those of other classes' members
	// (5.1.10): its class becomes part of its identity.
	AbiNode member_pointer(AbiNode class_type, AbiNode member_type);
	// T_, T <index-1> _; with a `level` (a parameter's depth counted from
	// the outermost template, nonzero) TL <level-1> _ <index as T's> (the
	// form GCC's ABI 19 writes for a template parameter of an enclosing
	// level that a partially substituted pack expansion keeps). `kind`
	// tells apart parameters that index and level alone do not (a pack,
	// a template template-parameter) as substitution candidates.
	AbiNode template_param(std::uint32_t index, bool substitutable, std::uint32_t level = 0, std::uint32_t kind = 0);
	AbiNode decltype_of(AbiNode expression, bool id_expression);

	// Template arguments.
	// `unsigned_value`: the value is printed unsigned whatever the type (an
	// enumeration's of an unsigned underlying type, 5.1.6.1); a builtin's
	// signedness is its own.
	AbiNode literal(AbiNode type, long long value, bool unsigned_value = false);
	// The pointer literal nullptr as an expression, LDnE (5.1.6.1); a
	// template argument that is a null pointer is its type's literal 0
	// (LPi0E, LDn0E: literal()).
	AbiNode nullptr_literal();
	// <template-param-decl> (the current ABI's 5.1.5.10, cxx-abi issue 47):
	// a type parameter's Ty, a non-type parameter's Tn <type> (`type` its
	// declared type in its template's own terms), a template parameter's
	// Tt <template-param-decl>* E (`head` its own parameters'
	// declarations); Tp before a pack's.
	AbiNode template_param_decl(
		AbiTemplateParamDeclKind kind, bool pack, AbiNode type, const AbiNode* head = nullptr, std::size_t count = 0);
	// <template-arg> ::= <template-param-decl> <template-arg>: an argument
	// with the declaration of the parameter it is for, which a name gives
	// when the argument alone would not identify the parameter (an
	// expanded pack's declaration is one per element: `count` of them).
	AbiNode declared_argument(const AbiNode* declarations, std::size_t count, AbiNode argument);
	// Tn <type> <template-arg>.
	AbiNode typed_argument(AbiNode parameter_type, AbiNode argument);
	AbiNode expression_argument(AbiNode expression);
	AbiNode argument_pack(const AbiNode* args, std::size_t count);
	AbiNode entity_argument(AbiNode entity, bool address_of);

	// Expressions.
	// A function parameter by its position and its level (Itanium's L,
	// the function parameter scopes between the reference and it), or
	// `this`.
	AbiNode function_param(std::uint32_t index, std::uint32_t level = 0);
	AbiNode this_param();
	// `prefix`: a prefix increment or decrement (pp_, mm_).
	AbiNode operation(std::uint16_t code, const AbiNode* operands, std::size_t count, bool prefix = false);
	// `parenthesized`: cp <base-unresolved-name> <expression>* E, a call
	// that would use argument-dependent lookup but for the parentheses
	// around its callee's name (5.1.6).
	AbiNode call(const AbiNode* callee_and_args, std::size_t count, bool parenthesized = false);
	AbiNode cast(std::uint16_t code, AbiNode type, AbiNode operand);
	AbiNode unresolved_name(AbiNode source_name, const AbiNode* args, std::size_t count);
	AbiNode vendor_expression(AbiNode source_name, const AbiNode* args, std::size_t count);
	AbiNode sizeof_type(AbiNode type);
	// `code` is st, at or ti.
	AbiNode type_operation(std::uint16_t code, AbiNode type);
	AbiNode scoped_member(AbiNode qualifier_type, AbiNode member, bool close_qualifier, const AbiNode* args = nullptr,
		std::size_t count = 0);
	// The qualifier of an <unresolved-name> (5.1.6): sr <unresolved-type>
	// (no `levels`), srN <unresolved-type> <unresolved-qualifier-level>+ E,
	// [gs] sr <unresolved-qualifier-level>+ E (no `unresolved_type`), or gs
	// alone (neither). A level is a <simple-id>, an unresolved_name of a
	// source name.
	AbiNode unresolved_prefix(AbiNode unresolved_type, const AbiNode* levels, std::size_t count, bool global);
	AbiNode member_access(std::uint16_t code, AbiNode object, AbiNode member, const AbiNode* args, std::size_t count);
	AbiNode entity_reference(AbiNode entity);
	// cv <type> _ <expression>* E, tl <type> <expression>* E, il <expression>* E
	// (and pi <expression>* E, a new-expression's parenthesized initializer).
	AbiNode expression_list(std::uint16_t code, AbiNode type, const AbiNode* expressions, std::size_t count);
	// [gs] nw|na <expression>* _ <type> (E | <initializer>): `code` nw or na,
	// `operands` the placement arguments, `initializer` a pi or il list (or
	// ABI_NONE); [gs] dl|da <expression>: `code` dl or da, `type` and
	// `initializer` ABI_NONE, `operands` the operand alone.
	AbiNode allocation(
		std::uint16_t code, bool global, AbiNode type, const AbiNode* operands, std::size_t count, AbiNode initializer);
	// A floating literal: its IEEE bits at its type's width.
	AbiNode floating_literal(AbiNode type, unsigned long long bits);

	// Entities.
	AbiNode function_encoding(
		AbiNode name, AbiNode result, const AbiNode* params, std::size_t count, bool variadic, unsigned qualifiers);
	AbiNode external_symbol(AbiNode mangled_name);

	// Telemetry.
	std::size_t intern_requests() const { return intern_requests_; }
	std::size_t intern_hits() const { return intern_hits_; }
	std::size_t bytes_reserved() const;

private:
	bool is_char(AbiNode type) const;
	bool is_std_name(AbiNode name, const char* spelling) const;
	bool is_char_argument_of(AbiNode type, AbiStandardSubstitutionKind abbreviation) const;
	AbiNode unary(AbiNodeKind kind, AbiNode operand);
	AbiNode intern_with(AbiNodeKind kind, AbiNode a, const AbiNode* items, std::size_t count);
	AbiNode intern(const AbiNodeData& data, const AbiNode* items, std::size_t count);
	std::uint32_t hash_of(const AbiNodeData& data, const AbiNode* items, std::size_t count) const;
	bool same(AbiNode id, const AbiNodeData& data, const AbiNode* items, std::size_t count) const;
	std::uint32_t text_hash(const char* text, std::size_t length) const;
	void grow_index();

	Arena arena_;
	Arena::Mark arena_start_;
	std::vector<AbiNodeData> nodes_;
	std::vector<std::uint32_t> hashes_;
	std::vector<AbiNode> lists_;
	std::vector<AbiText> texts_;
	// Node ids by hash position; 0 marks an empty position. At most half full.
	std::vector<AbiNode> index_;
	std::vector<AbiNode> scratch_;
	// collect_tags' walk: per node whether it or a component carries a tag
	// (0 not known, 1 none, 2 some), and the epoch of its last visit.
	AbiNode tag_node(AbiNode node);
	void tag_components(AbiNode node, AbiNode& a, AbiNode& b, std::uint32_t& count, bool& own) const;
	bool holds_tags(AbiNode node);
	std::uint32_t tag_set(AbiNode node, unsigned mode);
	void walk_tags(AbiNode node, std::vector<AbiNode>& tags, unsigned mode);
	std::vector<std::uint8_t> tag_memo_;
	std::vector<std::uint32_t> tag_marks_;
	std::vector<std::uint32_t> tag_written_marks_;
	std::uint32_t tag_epoch_ = 0;
	// Per mode (plain, scoped, written), a node's small tag set: 0 not
	// known, 1 none, 2 too many (walked), else 3 + its index in tag_sets_
	// (unique STRING nodes in tag_set_items_).
	struct TagSet
	{
		std::uint32_t first;
		std::uint32_t count;
	};
	std::vector<std::uint32_t> tag_set_memo_[3];
	std::vector<TagSet> tag_sets_;
	std::vector<AbiNode> tag_set_items_;
	std::vector<AbiNode> tag_scratch_;
	struct NodeHash
	{
		std::size_t operator()(AbiNode node) const { return node * 0x9E3779B1u; }
	};
	FlatMap<AbiNode, AbiNode, NodeHash> underived_;
	std::size_t intern_requests_;
	std::size_t intern_hits_;
};

} // namespace abi_mangle
