#pragma once

#include <cstdint>

#include "lex/token_kind.h"
#include "sema/semantic_types.h"

struct Identifier;

namespace syntax {
struct Expr;
struct FunctionBody;
struct Initializer;
struct LambdaExpr;
struct MemInitializer;
struct ParameterClause;
}

// The entities of a translation unit's semantic graph (sema/semantic_graph.h):
// what declarations denote, with the facts the analysis records on them
// (layouts, constants, linkage, demand edges). Scopes and bindings refer to
// them; they refer to scopes, bindings and the syntax tree by pointer.
namespace sema {

struct Scope;
struct Binding;
struct DemandList;
struct ClassEntity;
struct FunctionEntity;
struct VirtualFacts;
struct VtableGroup;
struct VirtualBase;
struct ClassTemplateFacts;
struct EnumTemplateFacts;
struct VariableTemplateFacts;
struct TemplateHead;

enum class EntityKind : uint8_t
{
	Namespace,
	NamespaceAlias,
	Class,
	Enum,
	Typedef,
	Variable,
	Function,
	Enumerator,
	Parameter,
	TemplateParameter,
	// An alias template (14.5.7).
	AliasTemplate
};

// What a declaration denotes. Several declarations (bindings) can denote
// one entity: redeclarations, using-declarations, injected anonymous-union
// members.
struct Entity
{
	EntityKind kind;
	const Identifier* name;
	// The scope the entity is a member of.
	Scope* owner;
	// The entity's type: declared (and completed, for arrays), or the class
	// or enumeration type itself.
	const Type* type;
};

// Namespaces nominated by a using-directive, or a namespace's inline
// members; newest first. A directive's place in declaration order (see
// Binding::ordinal): a template definition's context holds only the
// namespace-scope directives before it (14.6.4.2).
struct NamespaceList
{
	Scope* scope;
	NamespaceList* next;
	uint32_t ordinal;
};

// The classes that declared a function a friend.
struct ClassList
{
	const ClassEntity* entity;
	ClassList* next;
};

struct NamespaceEntity : Entity
{
	Scope* scope;
	// Enclosing namespaces (0 for the global namespace).
	uint32_t depth;
	// The argument-dependent lookup that last associated it (3.4.2p2).
	uint32_t association_mark;
	bool is_inline;
	// Unnamed, or nested in an unnamed namespace: its members have internal
	// linkage (3.5p4).
	bool internal;
	// Inline member namespaces, searched with this one by qualified lookup
	// (7.3.1p8).
	NamespaceList* inline_members;
};

struct NamespaceAliasEntity : Entity
{
	NamespaceEntity* target;
};

struct BaseClass
{
	Entity* base;
	bool is_virtual;
	// The base-specifier's access (11.2p2: private for a class, public
	// for a struct, by default).
	Access access;
	// The base-specifier depends on a template parameter (14.6.2p3: its
	// scope is not searched by unqualified lookup, in the template or any
	// of its specializations).
	bool dependent;
	// The base subobject's offset in bytes, once the class is laid out.
	uint64_t offset;
	BaseClass* next;
};

// A non-static data member in declaration order (for layout).
struct DataMember
{
	Entity* variable;
	// A bit-field's width; `bit_field` distinguishes width 0.
	uint64_t width;
	bool bit_field;
	// The access its declaration gives it (standard-layout classes, 9p7).
	Access access;
	// [[no_unique_address]] (C++20's potentially-overlapping member, here
	// an extension): laid out as a base is, an empty one taking no space.
	bool overlapping;
	// GNU packed on the member itself (as a packed class packs each of its
	// members).
	bool packed;
	// The member's offset in bits, once the class is laid out.
	uint64_t offset;
	// Its position among the class's non-static data members.
	uint32_t ordinal;
	DataMember* next;
	// A bit-field's storage unit, once the class is laid out (Analyzer::
	// assign_bit_units): its first byte in the class and its size in bytes;
	// `unit_shared` when another unit or subobject overlaps it (it is only
	// read-modify-written, never stored blind or transferred whole);
	// `unit_split` the bytes of a packed field no single access holds, read
	// and written piecewise (0 otherwise).
	uint64_t unit_offset;
	uint8_t unit_bytes;
	uint8_t unit_split;
	bool unit_shared;
};

enum class FactState : uint8_t
{
	NotStarted,
	InProgress,
	Done
};

struct ClassEntity : Entity
{
	// The class-key of the first declaration, which the type's spelling uses.
	TokenKind key;
	// A local class: how many classes of its name its function declared
	// before it (the Itanium discriminator, 5.1.6; saturates); a closure
	// type: how many closure types of its parameter types its context
	// declared before it (the Itanium closure number plus one, 5.1.8).
	uint16_t local_discriminator;
	// The class or a base declares a conversion function (12.3.2): the
	// filter before collecting them for a user-defined conversion.
	bool has_conversion_functions;
	// Semantics mode: the declaration state of the implicit copy and move
	// members (Done at once for a class that never gets them), and the copy
	// and move kinds its user-declared ones have (bits by CopyKind, known
	// at completion).
	FactState copy_members;
	uint8_t declared_copy_kinds;
	// Whether it is a literal type (3.9p10), once established (see
	// Analyzer::literal_type): 0 unknown, 1 yes, 2 no, 3 being established.
	uint8_t literal_state;
	// Declared with the class-virt-specifier final (9p3: it is no base).
	bool is_final;
	// It has a virtual table pointer (Itanium 2.4's dynamic class): it is
	// polymorphic or has a virtual base; known once complete.
	bool dynamic;
	// An anonymous class some member of which has a brace-or-equal-
	// initializer (directly or as an anonymous member's): its member
	// object is a variant member with one (9.5p8); known once complete.
	bool members_initialized;
	// This definition has a dependent base in its hierarchy, including a
	// local class in a function template. Set while analyzing its bases.
	bool dependent_bases;
	// An unnamed class's presentation name (for a typedef-named or
	// declarator-named class this is also `name`).
	const char* label;
	// The member scope, once the class is being defined.
	Scope* scope;
	bool complete;
	bool anonymous;
	bool polymorphic;
	// No non-static data members but zero-width bit-fields, no virtual
	// functions or bases, no non-empty bases (the Itanium ABI's empty
	// class), known once complete.
	bool empty;
	bool user_declared_constructor;
	// 8.5.1p1 disqualifies it as an aggregate: a user-provided constructor,
	// a private or protected non-static data member or a member's
	// brace-or-equal-initializer (bases and virtual functions apart).
	bool not_aggregate;
	// Its declarations make it no POD for the purpose of layout (Itanium
	// ABI 1.1, C++03 9p4 as GCC and Clang read it for C++11): a
	// user-provided constructor, copy assignment operator or destructor, a
	// private or protected non-static data member, a member initializer or
	// a reference member. Bases, virtual functions and members of non-POD
	// class type are layout's to add.
	bool non_pod;
	// Layout facts: some empty class is a subobject (the class, a base or
	// a member's), and the non-virtual size (sizeof without the tail
	// padding a derived class may reuse; sizeof for a POD).
	bool contains_empty;
	uint64_t nvsize;
	BaseClass* bases;
	BaseClass* last_base;
	DataMember* members;
	DataMember* last_member;
	uint32_t base_count;
	uint32_t member_count;
	// The defined implicit default constructor's scope (12.1p6), if any.
	Scope* implicit_default_constructor;
	FactState layout;
	// Layout fact: some subobject is volatile or a union (the class itself,
	// a base or a member), so zeroing it takes separate stores.
	bool contains_volatile_or_union;
	// GNU packed: its members are laid out packed. The layout clears it
	// when a member of a non-POD class stays unpacked (as GCC does), so no
	// class containing this one packs it either.
	bool packed;
	// A const object of the class may be default-initialized (8.5p6, CWG
	// 253), once established (see check_const_default).
	bool const_default_constructible;
	// The aggregate initialization that last value-initialized the class
	// as a member without an initializer (see aggregate_element).
	uint32_t empty_list_mark;
	uint64_t size;
	uint64_t alignment;
	// Traversal mark of derivation queries (sema/conversions.h), and the
	// argument-dependent lookup that last associated the class (3.4.2p2).
	uint32_t derivation_visit;
	uint32_t association_mark;
	// Semantics mode: an unnamed class's stable presentation name
	// (`__local_typeN`, in definition order), the implicitly declared
	// default and copy constructors once needed (12.1p5, 12.8p7), and an
	// anonymous union's unnamed object (9.5p5).
	const char* local_label;
	FunctionEntity* default_constructor;
	Binding* storage;
	// The implicitly declared copy and move constructors and assignment
	// operators (12.8p7, p9, p18, p20), declared on first demand (see
	// Analyzer::implicit_copy_members; null: user-declared, or a move
	// defined as deleted, which overload resolution ignores, p11, p23 as
	// amended by CWG 1402).
	FunctionEntity* copy_constructor;
	FunctionEntity* move_constructor;
	FunctionEntity* copy_assignment;
	FunctionEntity* move_assignment;
	// The destructor (12.4): the user-declared one, or the implicitly
	// declared one the class's completion declares.
	FunctionEntity* destructor;
	// The classes whose friend declarations name it (11.3; the classes and
	// functions it befriends are in Analyzer::friendships_).
	ClassList* befriending;
	// The alignment its alignment-specifiers request (7.6.2; 0 for none),
	// and the maximum alignment #pragma pack gives its members and bases
	// (0 for none).
	uint32_t alignment_request;
	uint32_t pack;
	// Only GNU aligned attributes request it: one weaker than the class's
	// natural alignment is ignored (GCC) rather than ill-formed (7.6.2p5).
	bool alignment_request_gnu;
	// The base whose constructors a using-declaration inherits (12.9).
	ClassEntity* inherits_constructors;
	// Semantics mode, a polymorphic class once complete: its virtual
	// facts (null for another class).
	VirtualFacts* virtuals;
	// Templates: a class template, a member class of a templated
	// class, a specialization or a member class of one (null otherwise;
	// sema/template_records.h).
	ClassTemplateFacts* templ;
};

struct EnumEntity : Entity
{
	bool scoped;
	// The enum-base was given (7.2p5); otherwise the underlying type is
	// chosen from the enumerator values when the definition is complete.
	bool fixed;
	bool defined;
	// Some enumerator is negative: without a fixed underlying type the
	// values of the enumeration then include negative ones (7.2p7).
	bool negative;
	const char* label;
	const Type* underlying;
	// The enumerators' lookup scope (scoped enumerations); unscoped
	// enumerators are declared in the enclosing scope.
	Scope* scope;
	// A member enumeration of a templated class defined out of it, or of an
	// instance (null otherwise; sema/template_records.h).
	EnumTemplateFacts* templ;
};

enum class ConstantState : uint8_t
{
	// Not yet evaluated.
	Unknown,
	Evaluating,
	Value,
	None,
	// A constexpr floating object's value (5.19p2): the bits of the double
	// that holds it exactly.
	Floating,
	// A constexpr object of another literal type, or a reference
	// initialized with a constant expression: its value in
	// Analyzer::evaluated_value.
	Object
};

struct VariableEntity : Entity
{
	bool defined;
	bool is_extern;
	bool is_static;
	// A non-static data member.
	bool member;
	// A const integral or reference variable's constant value, or a
	// constexpr floating one's (5.19p2), evaluated at its declaration.
	ConstantState constant;
	bool is_constexpr;
	// Not const in a const object (7.1.1p10, 9.3.2p3).
	bool is_mutable;
	// A bit-field member (9.6).
	bool bit_field;
	// Namespace-scope linkage facts of the first declaration: internal
	// linkage (3.5p3-p4), C language linkage (7.5), thread storage
	// duration (3.7.2).
	bool internal_linkage;
	bool c_linkage;
	bool is_thread_local : 1;
	// C++17's inline variable (hosted compilations): external even when
	// const, one object however many units define it (a weak definition
	// in each, initialized once, as a template's static data member is).
	bool is_inline : 1;
	// GCC's __constinit: constant initialization, without const qualification.
	bool is_constinit : 1;
	// A non-static data member's brace-or-equal-initializer may throw (it
	// calls a function or constructor that is not non-throwing, 15.4p14).
	bool throwing_initializer : 1;
	// A class template instance's non-static data member whose
	// brace-or-equal-initializer is analyzed only when a constructor uses
	// it (14.7.1p1 as CWG 1396 reads it; see instantiate_member_initializer),
	// and whether a constructor's plan has used it.
	bool initializer_pending : 1;
	bool initializer_used : 1;
	// A non-static data member's brace-or-equal-initializer has been typed
	// (see Analyzer::member_default_initializer).
	bool initializer_typed : 1;
	// A static data member's in-class initializer evaluated again once the
	// outermost class is complete (see record_constant_initializer): until
	// then a constant expression reads its provisional value (see
	// ConstantInterpreter::provisional_value).
	bool constant_deferred : 1;
	// A GNU zero-length array member `T m[0]` (hosted compilations): its
	// type is a flexible array member's, but no element initializes it
	// (GCC and Clang; see Analyzer::aggregate_members).
	bool zero_length : 1;
	// A function's predefined __func__ (8.4.1p8), __FUNCTION__ or
	// __PRETTY_FUNCTION__: a PredefinedNameEntity.
	bool predefined_name : 1;
	// A structured binding (a BindingEntity, sema/binding_records.h).
	bool structured_binding : 1;
	// The alignment a member's alignment-specifiers request (7.6.2).
	uint32_t alignment_request;
	Constant value;
	// Semantics mode: the constructor its definition runs (a class object
	// default-, direct-, copy- or list-initialized by a constructor), or for
	// a non-static data member the one its brace-or-equal-initializer runs;
	// null when no constructor runs (aggregates, a class prvalue that
	// initializes the object itself, non-class types).
	FunctionEntity* constructor;
	// A data member's brace-or-equal-initializer: a non-static member's
	// (9.2p4), typed in the complete class (9.2p2); a static member's
	// in-class one (9.4.2p3), which its namespace-scope definition takes.
	const syntax::Initializer* member_initializer;
	// A non-static data member's layout record in its class.
	const DataMember* data_member;
	// A static data member of a templated class or of an instance (null
	// otherwise; sema/template_records.h).
	VariableTemplateFacts* templ;
};

// A function's predefined name variable (VariableEntity::predefined_name):
// the characters it holds, rendered once when the body first names it.
struct PredefinedNameEntity : VariableEntity
{
	const char* text;
};

// An enumerator, or a non-type template parameter's name that an
// instantiation binds to its argument (14.1p6; `enumeration` null): a
// constant of the argument's value, and for a pointer or reference argument
// the argument itself (`argument`, a TypeKind::Value designating an
// entity; null otherwise).
struct EnumeratorEntity : Entity
{
	EnumEntity* enumeration;
	Constant value;
	const Type* argument;
};

// A template parameter (14.1), with its position: the number of enclosing
// template parameter lists (depth) and its index in its own list. Every
// declaration's parameter at one position and of one kind (type or
// non-type, pack or not) shares the canonical positional entity, so
// equivalent template declarations (14.5.6.1) have equal canonical types.
// A non-type parameter has its declared type (`value_type`, which
// may depend on earlier parameters); its type (`type`) stands for its
// value as a dependent template argument. A dependent name `Q::name` whose
// qualifier Q depends on a template parameter is a placeholder of this
// kind too (`qualifier` non-null; its name is the member's), interned per
// (Q, name); so is a decltype-specifier of a template definition
// (`expression`, analyzed in `expression_scope` once substituted,
// 14.6.2.2), and a value-dependent template argument (`non_type`: its
// `expression` evaluated once substituted, 14.6.2.3).
struct TemplateParameterEntity : Entity
{
	bool template_template;
	// A non-type parameter (or its positional entity), or a value
	// placeholder.
	bool non_type;
	// A template parameter pack (14.5.3p1).
	bool pack;
	// A decltype or value placeholder: the depth of the template heads
	// around its expression (see Analyzer::unsubstituted_depth).
	uint8_t head_depth;
	uint16_t depth;
	uint16_t index;
	TemplateParameterEntity* canonical;
	// A non-type parameter's declared type; a value placeholder's pointer
	// or reference parameter type, which its argument converts to (PA18,
	// 14.3.2p5; null for another or an unknown one).
	const Type* value_type;
	const Type* qualifier;
	const syntax::Expr* expression;
	Scope* expression_scope;
	// A decltype or value placeholder: the template parameter packs its
	// expression names outside nested pack expansions (a Pack of their
	// canonical types; null for none), which an expansion of a type naming
	// it expands (14.5.3p5), and those its nested expansions name (each
	// element of an expansion of one binds every element's name, see
	// Analyzer::substituted_value); and the non-type parameter its
	// expression names when it is just that name (an array bound deducing
	// it, 14.8.2.5p8), canonically.
	const Type *packs, *nested_packs;
	const TemplateParameterEntity* named;
	// A template template-parameter: its own template parameter
	// list (a canonical one's, that of the first declaration of its
	// shape). A template template-parameter, or a dependent member
	// template-name `Q::template name` (`qualifier` non-null), as the
	// template of a dependent template-id: the class template that stands
	// for it (see Analyzer::stand_in_template), made on first use.
	const TemplateHead* template_head;
	ClassEntity* stand_in;
	// A value (or decltype) placeholder substituted with arguments some of
	// which are still dependent: the placeholder and the
	// substitution it waits to apply (see Analyzer::substitute_value).
	const struct PendingSubstitution* pending;
	// A keyed placeholder's structural key (sema/placeholder_words.h), its
	// word count first: the resolved structure of its operand, which the
	// lowering mangles (Itanium 5.1.6); null for an operand of its own.
	const uint64_t* key;
	// A placeholder's declaration ordinal (its expression's definition
	// context, 14.6.4.2), and the type of `this` where it was made (a
	// member function's trailing return type, 5.1.1p3; null elsewhere).
	uint32_t ordinal;
	// sizeof_packs: a placeholder whose operand (or a placeholder in it)
	// applies sizeof... to a parameter pack, nests an expansion of one, or
	// is not covered by its key: substituted for one element of an
	// expansion, it needs the whole pack too (see
	// Analyzer::substitute_value); friend_keyed: made for a friend
	// declaration in a class keyed in its namespace (see
	// Analyzer::namespace_friend_class_), whose scope gives no access;
	// fixed: a value placeholder whose operand names nothing dependent and
	// is not type-dependent, whose value (`fixed_value`, once evaluated for
	// the parameter type `fixed_target`) is the same in every
	// instantiation (see Analyzer::template_value).
	bool sizeof_packs, friend_keyed, fixed;
	const Type *fixed_value, *fixed_target;
	const Type* this_type;
	// A keyed placeholder: the positions (below 63) of the declarator
	// parameters its operand names, bit 63 set when they are known (see
	// Analyzer::note_named_parameter). An equivalent operand of another
	// template shares the first declaration's scope (14.5.6.1p5), whose
	// other parameters' types its substitution leaves alone.
	uint64_t named_parameters;
};

// TemplateParameterEntity::named_parameters: the positions are known.
const uint64_t kNamedParametersKnown = 1ull << 63;

// TemplateParameterEntity::head_depth: no template head around the
// expression; heads of more than one depth (or of one too deep to record).
const uint8_t kHeadsNone = 0xfe;
const uint8_t kHeadsSeveral = 0xff;

inline bool IsPlaceholder(const TemplateParameterEntity* parameter)
{
	return parameter->qualifier != nullptr || parameter->expression != nullptr;
}

// A dependent array/vector bound is an expression placeholder, or partial
// ordering's synthesized non-type parameter. Only a direct parameter name
// deduces a bound; a compound expression remains a non-deduced context.
inline const TemplateParameterEntity* DirectBoundParameter(const Type* bound)
{
	const TemplateParameterEntity* parameter = static_cast<const TemplateParameterEntity*>(bound->entity);
	return IsPlaceholder(parameter) ? parameter->named : parameter;
}

} // namespace sema
