#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "abi/itanium/abi_encoder.h"
#include "lowir/builder.h"
#include "lowir/text_writer.h"
#include "sema/semantic_unit.h"
#include "support/flat_map.h"
#include "support/pointer_map.h"
#include "support/telemetry.h"
#include "syntax/ast.h"

class SourcePositions;

namespace sema {
class Analyzer;
struct ConstValue;
}

// Procedural source-to-LowIR lowering: the analyzed translation units,
// in command-line order, become one typed lowir::Program built directly from
// their semantic facts. ProgramLowering owns what crosses translation units
// (external symbols by ABI name, declarations still pending a definition,
// the dynamic-initialization function); lower_unit() reads one unit's tree,
// semantic graph and layouts and releases nothing it does not own, so the
// caller may discard the unit afterwards. FunctionLowering
// (lower/function_lowering.h) lowers one body.
namespace lower {

class SourceInlining;

class DebugInfo;

// A construct outside the supported lowering slice, at a unit location.
class LoweringError : public std::runtime_error
{
public:
	LoweringError(uint32_t location, const char* message) : std::runtime_error(message), location_(location) {}

	uint32_t location() const { return location_; }

private:
	uint32_t location_;
};

// The Itanium terminal of an operator function, or of an operator of a
// dependent expression, by its token (5.1.3); PLUS and MINUS take their
// unary or binary code from the operand count (abi_terminal_for_operands).
abi_mangle::AbiTerminalKind OperatorTerminal(TokenKind op, bool array);

// The function whose body declares a class declared in a block (or a
// member class of one; a closure type's lambda's enclosing function), null
// for another class; and the outermost function whose body holds
// `function` (a local class's member function, a closure's operator), or
// `function`.
const sema::FunctionEntity* EnclosingFunction(const sema::Scope* scope);
const sema::FunctionEntity* OutermostFunction(const sema::FunctionEntity* function);

// Work counters (spec section 9): they observe the lowering, never drive it.
struct LoweringCounters
{
	telemetry::Counter units = 0;
	telemetry::Counter functions = 0;
	telemetry::Counter declarations = 0;
	telemetry::Counter globals = 0;
	telemetry::Counter instructions = 0;
	telemetry::Counter blocks = 0;
	telemetry::Counter slots = 0;
	telemetry::Counter mangled_names = 0;
	// Virtual tables defined; the units' overriding-search probes and
	// virtual-table slots built (their semantic graphs' counts).
	telemetry::Counter vtables = 0;
	telemetry::Counter override_steps = 0;
	telemetry::Counter virtual_slots = 0;
	// Template work (the units' semantic graphs' counts): specializations
	// declared; class and function definitions, default arguments and
	// static data member definitions instantiated; value-dependent
	// arguments evaluated and partial specializations tried.
	telemetry::Counter specializations = 0;
	telemetry::Counter instantiated_classes = 0;
	telemetry::Counter instantiated_functions = 0;
	telemetry::Counter instantiated_arguments = 0;
	telemetry::Counter instantiated_variables = 0;
	telemetry::Counter value_substitutions = 0;
	telemetry::Counter partial_candidates = 0;
	// Substitution failures and limited lookups.
	telemetry::Counter substitution_failures = 0;
	telemetry::Counter limited_lookups = 0;
	// Constant evaluation (PA16, the units' semantic graphs' counts,
	// the lowering's constant-initialization probes included):
	// evaluations and the interpreter's steps.
	telemetry::Counter constant_evaluations = 0;
	telemetry::Counter constant_steps = 0;
	telemetry::Counter constant_call_builds = 0;
	telemetry::Counter constant_call_reuses = 0;
	telemetry::Counter function_order_builds = 0;
	telemetry::Counter function_order_reuses = 0;
	// The base walks' steps: the units' semantic graphs' and the
	// lowering's base offset searches.
	telemetry::Counter base_steps = 0;
};

// A global's static initialization, as the constant evaluation of its
// initializer found it: data items (scalars, addresses, zero runs) in
// layout order, or a failure meaning dynamic initialization.
struct StaticData
{
	std::vector<lowir::DataItem> items;
};

void PreserveStaticOffsets(StaticData& data);

// A subobject null_fill visits: its offset in its class, type, whether it
// is a complete object (its virtual bases are its own), its end and the
// largest end of the class's pieces up to it.
struct NullPiece
{
	uint64_t offset;
	const sema::Type* type;
	bool complete;
	uint64_t end;
	uint64_t reach;
};

// A class's pieces in null_piece_store_.
struct NullSpan
{
	uint32_t first;
	uint32_t count;
};

// A data item of a statically initialized object at its byte offset.
struct StaticPlacement
{
	uint64_t offset;
	uint64_t size;
	lowir::DataItem item;
};

// Itanium 2.5.2: a virtual table's offset-to-top and type information
// precede its slots; an object's table pointer points past them.
const uint64_t kTableHeaderBytes = 16;

// A value constant initialization states (3.6.2p2, 5.19): an integer's
// bits, a floating value, an address constant or a null pointer.
struct StaticValue
{
	enum class Kind : uint8_t
	{
		Integer,
		Floating,
		Address,
		Null,
		// A binary16 or binary128 value: its encoding in `bits` and `high`.
		Soft
	};

	Kind kind;
	// An integer's value as 128-bit two's complement: `bits` the low half,
	// `high` its extension (a 128-bit type's value can need both).
	uint64_t bits;
	uint64_t high;
	long double floating;
	lowir::SymbolId symbol;
	int64_t offset;
	// An address in a named variable's object: the variable's class (its
	// element class for an array, `stride` the element's size, else 0),
	// whose complete layout places the virtual bases of the subobjects it
	// holds (static_base_offset); null for none known.
	const sema::ClassEntity* root;
	uint64_t stride;
};

class FunctionLowering;

// The C++ runtime's functions the lowering calls (lower/rtti_lowering.cpp).
enum class RuntimeFunction : uint8_t
{
	BadTypeid,
	BadCast,
	DynamicCast,
	AllocateException,
	Throw,
	Rethrow,
	BeginCatch,
	EndCatch,
	// __cxa_call_unexpected (host objects' dynamic-exception-
	// specifications).
	CallUnexpected,
	// __cxa_throw_bad_array_new_length (role bad_array_new_length: array
	// new-expressions with an invalid count, 5.3.4p7).
	BadArrayNewLength,
	Count
};

// GCC's binary16 and binary128 floating types (_Float16; _Float128 and
// __float128) and Clang's bfloat16 (__bf16), whose values no LowIR type
// holds (lower/soft_float_lowering.cpp).
enum class SoftFloat : uint8_t
{
	None,
	Half,
	Quad,
	BFloat
};

// The C runtime's (libgcc's) soft-float routines that GCC and Clang call
// for those types' operations, by their names' operation and modes (tf:
// binary128, hf: binary16, sf/df/xf: float/double/long double, si/di/ti:
// 32/64/128-bit integers, uns: unsigned).
enum class SoftFloatHelper : uint8_t
{
	AddTF,
	SubTF,
	MulTF,
	DivTF,
	NegTF,
	EqTF,
	NeTF,
	LtTF,
	LeTF,
	GtTF,
	GeTF,
	ExtendSFTF,
	ExtendDFTF,
	ExtendXFTF,
	TruncTFSF,
	TruncTFDF,
	TruncTFXF,
	FloatSITF,
	FloatUnsSITF,
	FloatDITF,
	FloatUnsDITF,
	FloatTITF,
	FloatUnsTITF,
	FixTFSI,
	FixUnsTFSI,
	FixTFDI,
	FixUnsTFDI,
	FixTFTI,
	FixUnsTFTI,
	ExtendHFSF,
	ExtendHFDF,
	ExtendHFXF,
	ExtendHFTF,
	TruncSFHF,
	TruncDFHF,
	TruncXFHF,
	TruncTFHF,
	FloatTIHF,
	FloatUnsTIHF,
	FixHFTI,
	FixUnsHFTI,
	TruncSFBF,
	TruncDFBF,
	TruncXFBF,
	TruncTFBF,
	Count
};

// The kind of a (cv-qualified) type's values; of a floating operand's
// LowIR value (an object for binary16 and binary128).
inline SoftFloat SoftFloatOf(const sema::Type* type)
{
	type = type->unqualified;
	if (type->kind != sema::TypeKind::Fundamental)
		return SoftFloat::None;
	return type->fundamental == FundamentalType::Float16 ? SoftFloat::Half
		: type->fundamental == FundamentalType::Float128 || type->fundamental == FundamentalType::GnuFloat128
		? SoftFloat::Quad
		: type->fundamental == FundamentalType::BFloat16 ? SoftFloat::BFloat
														 : SoftFloat::None;
}
SoftFloat SoftFloatOfValue(lowir::Type type);
// A value of a scalar type that LowIR holds as an object of its bytes
// (binary16, binary128, complex or vector): a value, not a class object.
bool ObjectValueType(const sema::Type* type);
// A static initializer's constant (an integer, a standard floating value,
// or a binary16 or binary128 one of type `from`) converted to the `target`
// format, rounded once (4.8, 4.9); a value beyond the format's range is
// the infinity the run-time conversion gives (3.6.2p3).
bool SoftStaticValue(const StaticValue& value, const sema::Type* from, SoftFloat target, StaticValue& out);
// A binary16 or binary128 constant (`source` its kind) converted to an
// integral or standard floating type (4.9, 4.12, 4.8; beyond a floating
// type's range its infinity); false where an integral conversion is
// undefined (converted at run time).
bool SoftStaticConverted(const StaticValue& value, SoftFloat source, const sema::Type* target, StaticValue& out);
// An integer constant of type `from` (a 128-bit one's whole value) as a
// value of the standard floating type `target`, rounded once (4.9p2).
bool IntegralStaticFloating(
	const StaticValue& value, const sema::Type* from, const sema::Type* target, StaticValue& out);
// `left op right` (+, -, *, /) of two constants of the binary16 or
// binary128 kind, rounded once (an overflow to infinity, as at run time);
// false for an infinite or NaN operand or a division by zero (run time).
bool SoftStaticArithmetic(
	TokenKind op, SoftFloat kind, const StaticValue& left, const StaticValue& right, StaticValue& out);

// The runtime's type information kinds (Itanium 2.9.5), whose vtables the
// objects refer to.
enum class TypeInfoKind : uint8_t
{
	Class,
	SingleInheritance,
	MultipleInheritance,
	Fundamental,
	Pointer,
	PointerToMember,
	Function,
	Array,
	Enum,
	Count
};

// Items of read-only run-time data (lower/vtable_lowering.cpp).
lowir::DataItem AddressItem(lowir::SymbolId symbol, int64_t addend);
// A table's entry: its target's address, null for none (an unused entry).
lowir::DataItem EntryItem(lowir::SymbolId target);
lowir::DataItem ScalarItem(lowir::TypeKind kind, uint64_t value);
// A table group's segment whose table pointer is the complete class's
// (segment 0, or sharing it through primary bases).
bool OnPrimaryChain(const sema::VtableGroup* group, uint32_t segment);
// The literal of an integral constant of `type` from its 64-bit value: a
// 128-bit type's high half extends it by the type's signedness (the
// analysis's constants, sema/integral_arithmetic.h).
lowir::Literal IntegralLiteral(const sema::Type* type, uint64_t bits);

// A namespace-scope object whose destructor runs at program end (3.6.3p1):
// its global, the destructor's symbol, and its elements (an array's, last
// to first). Program-wide, so it holds no unit's semantic records.
struct Finalizer
{
	lowir::SymbolId global;
	lowir::SymbolId destructor;
	bool array;
	uint64_t count;
	uint64_t element_size;
	// A thread-local object's initialization guard (kNoId for none).
	lowir::SymbolId guard;
	// A static reference's temporary made on some paths only: its i8 flag
	// set once constructed (kNoId for none).
	lowir::SymbolId constructed;
	// A static reference's temporary (12.2p5): an array's elements
	// addressed at constant offsets from one address, as the course
	// destroys a reference's temporaries.
	bool reference_temporary;
};

// The ABI entry a constructor or destructor symbol stands for (Itanium
// 5.1.4.3): the complete-object entry, the base-subobject entry, or a
// virtual destructor's deleting entry (the complete destructor, then the
// deallocation of the object, 12.4p12).
enum class EntryVariant : uint8_t
{
	Complete,
	Base,
	Deleting
};

// The linkage a type confers on the entities built from it (3.5), from
// the widest: shared across units; internal (3.5p4: an unnamed
// namespace's); none (3.5p8, 9.8: a local class).
enum class Locality : uint8_t
{
	External,
	Internal,
	Local
};

class ProgramLowering
{
public:
	explicit ProgramLowering(lowir::Program& program);
	~ProgramLowering();

	ProgramLowering(const ProgramLowering&) = delete;
	ProgramLowering& operator=(const ProgramLowering&) = delete;

	// Lowers one analyzed translation unit. Throws LoweringError.
	void lower_unit(const syntax::TranslationUnit& unit, sema::SemanticGraph& graph, sema::Analyzer& analyzer);
	// Declares what was referenced but never defined and ends the
	// initialization function; the program is complete afterwards.
	void finish();

	const LoweringCounters& counters() const { return counters_; }

	// Writes each function definition to `stream` as it completes and
	// releases its body (spec section 8: function-local representations are
	// reclaimed incrementally); null keeps every body (audit validation).
	void set_definition_stream(lowir::DefinitionSink* stream) { stream_ = stream; }
	// The LowIR view's debug locations (a stated debug-information level
	// other than -g0): each emitted function and instruction carries the
	// source location it comes from, `!dbg(file, line, column)`
	// (lowir.md "Debug Locations"). The native encoder maps them to PCs.
	void set_debug_locations(bool on) { debug_locations_ = on; }
	void set_debug_types(bool on) { debug_types_ = on; }
	bool debug_locations() const { return positions_ != nullptr; }
	// A unit location's debug location (absent without debug locations, or
	// for a file name that is not one LowIR token).
	lowir::DebugLocation debug_location(uint32_t location);
	void debug_local(lowir::SymbolId function, lowir::SlotId slot, const sema::VariableEntity* variable,
		uint32_t location, uint32_t lexical_scope);
	void debug_function(const sema::FunctionEntity* function);
	void order_completed_definitions();
	bool streaming() const { return stream_ != nullptr; }
	// Object mode (the toolchain's units): an object of static storage
	// duration has its destruction registered with atexit when its
	// initialization completes (3.6.3p1, p3; Itanium 3.3.6's scheme), so
	// it interleaves with function-local statics' and atexit's; the LowIR
	// contract's units destroy namespace-scope objects in a `role=fini`
	// function instead. Thread-local objects keep that function.
	void set_object_mode(bool on)
	{
		object_mode_ = on;
		program_.object_facts = on;
	}
	bool object_mode() const { return object_mode_; }
	// Host objects (object mode for the host toolchain, not the course's
	// `.obj` and runtime): a block-scope static's initialization is
	// thread-safe (6.7p4), through Itanium's one-time construction API.
	void set_host_objects(bool on) { host_objects_ = on; }
	bool host_objects() const { return host_objects_; }
	// Whether a function type has a class parameter its caller destroys
	// (unit-scoped: function type -> 0 or 1, see
	// FunctionLowering::caller_destroys).
	PointerMap& caller_destroyed_types() { return caller_destroyed_types_; }
	PointerMap& element_temporaries_memo() { return element_temporaries_; }
	// A pointer to data member is its member's offset, its null value -1
	// (Itanium 2.3, in course LowIR and host objects alike).
	static int64_t null_data_member() { return -1; }
	// Whether zero-initializing `type` writes a null pointer to data member;
	// the items of bytes [lo, hi) of a zero-initialized object of `type` at
	// `base` (its virtual bases' too when `complete`); a static object's
	// data with its zero bytes' null values.
	bool holds_data_member_pointer(const sema::Type* type);
	// lowir.md's readonly storage: whether a statically initialized object
	// of `type` (of static storage duration, not thread-local) is never
	// written; whether a class has a mutable or volatile member at any
	// depth.
	bool readonly_object(const sema::Type* type);
	bool changeable_class(const sema::ClassEntity* cls);
	void null_fill(const sema::Type* type, uint64_t base, uint64_t lo, uint64_t hi, bool complete, StaticData& out);
	NullSpan null_pieces(const sema::ClassEntity* cls, bool complete);
	void null_member_pointers(const sema::Type* type, StaticData& data);

	// ------------------------------------------ services for bodies
	lowir::Program& program() { return program_; }
	LoweringCounters& counters() { return counters_; }
	sema::SemanticGraph& graph() { return *graph_; }
	// A class's implicitly declared copy and move members, which the
	// analysis declares on first demand: read them after this.
	void implicit_copy_members(const sema::ClassEntity* entity);
	// Whether a function's exception-specification is non-throwing (its
	// noexcept-specification evaluated on this first need).
	bool function_nothrow(const sema::FunctionEntity* function);
	// The value the analysis's constant evaluation gave a variable's
	// initialization, or null (sema::Analyzer::evaluated_value).
	const sema::ConstValue* evaluated_value(const sema::VariableEntity* variable) const;
	bool read_through_storage(const sema::VariableEntity* variable) const;
	// Whether a condition converted by a conversion function has a value
	// known without evaluating it (sema::Analyzer::constant_conversion).
	bool constant_conversion(const syntax::Expr& condition, bool& value) const;
	// A template instance's function definition, instantiated on this
	// demand (false when its template has none).
	bool instantiate_definition(const sema::FunctionEntity* function);
	const sema::ClassEntity* call_naming_class(const syntax::CallExpr& call) const;
	const sema::ClassEntity* member_naming_class(const syntax::Expr& access) const;
	// A function's default argument for a call, instantiated on this demand
	// if it is a template's.
	const syntax::Expr* default_argument(const sema::FunctionEntity* function, uint32_t index);
	// The LowIR type holding a value of `type`: scalars and pointers by
	// kind, references as `ptr`, arrays and classes as `obj<size x align>`.
	lowir::Type storage_type(const sema::Type* type);
	uint64_t size_of(const sema::Type* type, uint32_t location);
	uint64_t alignment_of(const sema::Type* type, uint32_t location);
	// The byte offset of a non-static data member in its class; a
	// bit-field's offset in bits.
	uint64_t member_offset(const sema::VariableEntity* member, uint32_t location);
	uint64_t member_bit_offset(const sema::VariableEntity* member, uint32_t location);
	// A member of an anonymous union (or struct) member lies in that
	// unnamed object (9.5p5): the named class holding it (sema's
	// MemberClassOf) and the object's offset there, through nested ones;
	// and the member's offset in that class.
	const sema::ClassEntity* member_home(const sema::VariableEntity* member, uint64_t& offset, uint32_t location);
	uint64_t member_home_offset(const sema::VariableEntity* member, uint32_t location);
	// The same walk stopped at `within` when it passes that class (an
	// anonymous class's own special member functions address its members
	// from its object, as the Itanium ABI's `_ZN1XUt_C1Ev` does).
	const sema::ClassEntity* member_holder(
		const sema::VariableEntity* member, const sema::ClassEntity* within, uint64_t& offset, uint32_t location);
	// An anonymous class's object in the named class holding it (through
	// nested ones): that class and the offset; any other class is its own.
	const sema::ClassEntity* anonymous_home(const sema::ClassEntity* cls, uint64_t& offset, uint32_t location);
	// The byte offset of `base`'s subobject in a `derived` object (10p3:
	// the steps of the path of direct bases leading to it).
	uint64_t base_offset(const sema::ClassEntity* derived, const sema::ClassEntity* base, uint32_t location);
	bool base_path_offset(
		const sema::ClassEntity* derived, const sema::ClassEntity* base, uint64_t& offset, uint32_t location);
	bool base_path_search(
		const sema::ClassEntity* derived, const sema::ClassEntity* base, uint64_t& offset, uint32_t location);
	// The symbols of functions and namespace-scope variables: declared on
	// first reference, defined when their definition is lowered.
	lowir::SymbolId function_symbol(const sema::FunctionEntity* function);
	lowir::SymbolId inline_copy(const sema::FunctionEntity* function);
	lowir::SymbolId variable_symbol(const sema::VariableEntity* variable);
	// A constructor's or destructor's ABI entry (Itanium 5.1.4.3): the
	// complete entry is the function's symbol; a constructor's base entry
	// is a definition of its own (a destructor's base subobjects use the
	// complete entry, whose base-entry spelling is an alias).
	lowir::SymbolId entry_symbol(const sema::FunctionEntity* function, EntryVariant variant);
	// Polymorphic classes (lower/vtable_lowering.cpp): a class's virtual
	// table and type information objects, declared on first reference;
	// a table is defined by the unit defining its class's key function,
	// else weak by each unit storing its address (Itanium 5.2.3), and
	// defines the type information objects it refers to.
	lowir::SymbolId vtable_symbol(const sema::ClassEntity* entity);
	lowir::SymbolId typeinfo_symbol(const sema::ClassEntity* entity, bool derived = false);
	// The type information object of any type (cv-unqualified), and a
	// runtime function, declared on first reference (rtti_lowering.cpp).
	lowir::SymbolId type_information(const sema::Type* type);
	// The type information a throw or handler names (15.1, 15.3): the
	// runtime's declaration for a fundamental type or a pointer to one (the
	// course's form), else type_information's.
	lowir::SymbolId exception_type_information(const sema::Type* type);
	static bool RuntimeTypeInformation(const sema::Type* type);
	// A thrown type's unreferenced companions in the course's LowIR units:
	// its own (defined) type information and an exception-object-sized
	// zero object (rtti_lowering.cpp).
	void thrown_type(const sema::Type* type);
	void caught_type(const sema::Type* type);
	lowir::SymbolId defined_type_information(const sema::Type* type);
	lowir::SymbolId class_information(const sema::ClassEntity* entity, bool table = false);
	void class_table(const sema::ClassEntity* entity);
	lowir::SymbolId runtime_function(RuntimeFunction which);
	// Itanium 2.9.7's src2dst hint of a dynamic_cast from `base` to
	// `derived` (dynamic_type_lowering.cpp).
	int64_t dynamic_cast_hint(const sema::ClassEntity* base, const sema::ClassEntity* derived, uint32_t location);
	void define_vtable(const sema::ClassEntity* entity);
	// A table pointer's value for a segment of a class's table group
	// (sema::VtableGroup; 0: the primary table): the address point of the
	// segment's table where this unit defines the group, else within the
	// class's external table (Itanium 2.5.2's group layout); and each
	// segment's subobject offset in the class.
	struct TableAddress
	{
		lowir::SymbolId symbol;
		int64_t addend;
	};
	TableAddress table_address(const sema::ClassEntity* entity, uint32_t segment);
	const uint64_t* segment_offsets(const sema::ClassEntity* entity);
	// Virtual bases (lower/virtual_base_lowering.cpp): the key of a class's
	// virtual bases' places (vbase_rows: the index of its list in
	// vbase_row_store_, which later queries may move; shared with its
	// primary base when that has the same virtual bases in the same order),
	// a virtual base's position in VirtualBases (PointerMap::kMissing for a
	// class that is none of them), its vbase offset's position from the
	// address point (Itanium 2.5.2, itanium_vbase_offset_offset), a virtual
	// base's offset in a complete object; the route to a base subobject
	// (through the virtual base it is or lies in, if any, then a static
	// offset).
	uint32_t vbase_rows(const sema::ClassEntity* cls);
	uint32_t virtual_base_position(const sema::ClassEntity* cls, const sema::Entity* base);
	// A class's virtual bases in construction order (12.6.2p10), as its
	// base-specifiers for them (VirtualBases).
	const sema::BaseClass* const* virtual_base_order(const sema::ClassEntity* cls);
	int64_t vbase_row_offset(const sema::ClassEntity* cls, const sema::ClassEntity* base);
	uint64_t virtual_base_offset(const sema::ClassEntity* complete, const sema::ClassEntity* base);
	struct BaseRoute
	{
		const sema::ClassEntity* via;
		uint64_t offset;
	};
	BaseRoute base_route(const sema::ClassEntity* derived, const sema::ClassEntity* base, uint32_t location);
	// A static address's base subobject; through a virtual base only in a
	// complete object of known layout: the address's named variable
	// (StaticValue::root), or with `member`, the data member of class
	// `from` the converted expression names.
	bool static_base_offset(const sema::ClassEntity* from, const sema::ClassEntity* to, StaticValue& value,
		uint32_t location, bool member = false);
	// A class's VTT (Itanium 2.6): its symbol, a sub-VTT's size, and the
	// positions a base-object entry reads (a direct base's sub-VTT, a
	// segment's table pointer, a virtual base's VTT in the complete one).
	lowir::SymbolId vtt_symbol(const sema::ClassEntity* entity);
	uint32_t vtt_size(const sema::ClassEntity* cls);
	uint32_t vtt_base_index(const sema::ClassEntity* cls, const sema::BaseClass* direct);
	uint32_t vtt_segment_index(const sema::ClassEntity* cls, uint32_t segment);
	uint32_t vtt_virtual_index(const sema::ClassEntity* cls, const sema::ClassEntity* base);
	// A virtual call's `as` boundary: the called function's own (a
	// deleting entry's is its destructor's).
	lowir::SignatureId virtual_call_signature(const sema::FunctionEntity* function);
	// A string literal's read-only array (one per literal occurrence).
	// `element`: a user-defined literal's code unit type (null: the
	// literal's own array type's).
	lowir::SymbolId string_literal(const syntax::LiteralExpr& literal, const sema::Type* element = nullptr);
	lowir::SymbolId predefined_name(const char* text);
	// A function's predefined name variable's array (one lookup of its
	// text per variable).
	lowir::SymbolId predefined_variable(const sema::VariableEntity* variable);
	// The presumed line and file name of a location (the source location
	// builtins).
	uint64_t source_line(uint32_t location) const;
	std::string source_file(uint32_t location) const;
	lowir::SymbolId string_object(lowir::Type unit, const std::vector<uint64_t>& units);
	bool same_string_object(lowir::SymbolId symbol, lowir::Type unit, const std::vector<uint64_t>& units) const;
	// The call boundary of a function type (an indirect call's `as`),
	// `unwind=no` when the callee is declared non-throwing.
	lowir::SignatureId call_signature(const sema::Type* function, bool nothrow = false);
	// A call through a pointer to member function: the function type's
	// boundary with the object first.
	lowir::SignatureId member_call_signature(const sema::Type* member_pointer, bool nothrow = false);
	// Whether a call through this callee does not throw (see
	// sema::Analyzer::callee_nothrow).
	bool callee_nothrow(const syntax::Expr& callee);
	// The initialization helper of a class aggregate (the course's form for
	// an aggregate object at a computed address): internal, emitted once per
	// unit after the definitions that use it.
	lowir::SymbolId aggregate_helper(const sema::ClassEntity* entity, uint32_t count, unsigned cv = 0);
	// An internal zero-initialized global holding a temporary of `type`
	// bound to a reference of an object with static storage duration.
	lowir::SymbolId static_temporary(const sema::Type* type);
	// The static-storage variable (and its global) whose initialization is
	// lowered between these calls: the temporaries its initializer makes
	// (12.2p5) are named after it, in the order it makes them (a pre-order
	// walk of the initializer): Itanium 5.1.4.5's `_ZGR <name> [<seq-id>]
	// _` (clang's, unit-local) and the course's spelling,
	// `__cppgm_ref_temporary__<global>_<n>`, or for an object's
	// std::initializer_list array `__cppgm_initlist_backing_<n>`.
	struct TemporaryNames
	{
		const sema::VariableEntity* variable = nullptr;
		lowir::SymbolId symbol = lowir::kNoId;
		uint32_t sequence = 0;
	};
	TemporaryNames begin_temporary_names(const sema::VariableEntity* variable, lowir::SymbolId symbol)
	{
		const TemporaryNames saved = temporary_names_;
		temporary_names_ = TemporaryNames();
		temporary_names_.variable = variable;
		temporary_names_.symbol = symbol;
		return saved;
	}
	void end_temporary_names(const TemporaryNames& saved) { temporary_names_ = saved; }
	// The next temporary's Itanium name (see begin_temporary_names) as its
	// object spelling; none outside a variable's initialization.
	void name_temporary(lowir::SymbolId temporary);
	// Host objects: the temporaries a class template specialization's
	// static member's initialization makes between these calls (a
	// reference's, an aggregate's reference members'; another variable's
	// own none). Each whose destruction is registered (under
	// temporary_registration_guard) has its registration guarded by weak
	// `_ZGV <name> [<seq-id>] _` (GCC's protocol), so whichever unit
	// initializes the member, a temporary is destroyed once; the
	// temporaries stay the unit's own (as clang's units reach theirs).
	void begin_owned_temporaries(const sema::VariableEntity* variable);
	void end_owned_temporaries();
	// The guard of an owned temporary's destruction registration (an
	// internal flag until end_owned_temporaries names it); kNoId for
	// another temporary.
	lowir::SymbolId temporary_registration_guard(lowir::SymbolId temporary);
	lowir::SymbolId constexpr_image(const sema::VariableEntity* variable, const syntax::Initializer* initializer);
	// A function-local static object's global, defined on its first lowering
	// with its static initialization; `guard` its first-use flag when a
	// dynamic initialization remains (kNoId otherwise). The C library's
	// atexit, and a function destroying an object at program end.
	// `tokens`: the declaration, named by its token span, when its
	// initializer makes a temporary.
	lowir::SymbolId local_static(const sema::VariableEntity* variable, const sema::FunctionEntity* function,
		uint32_t ordinal, const syntax::Initializer* initializer, uint32_t location, lowir::SymbolId& guard,
		const syntax::InitDeclarator* tokens);
	// The global its function's lowering made for a block-scope static
	// (kNoId before), which a lambda's or a local class's body names.
	lowir::SymbolId local_static_symbol(const sema::VariableEntity* variable) const
	{
		const uint32_t found = local_static_index_.find(variable);
		return found == PointerMap::kMissing ? lowir::kNoId : local_statics_[found].symbol;
	}
	lowir::SymbolId atexit_function();
	// Itanium's one-time construction API (3.3.2), declared once each:
	// __cxa_guard_acquire(guard) -> int (nonzero: initialize now),
	// __cxa_guard_release(guard), __cxa_guard_abort(guard).
	enum class GuardCall : uint8_t
	{
		Acquire,
		Release,
		Abort
	};
	lowir::SymbolId guard_function(GuardCall call);
	// The C++ runtime's __cxa_thread_atexit (the thread's exit list, run
	// before atexit's; object mode's thread-local objects).
	lowir::SymbolId thread_atexit_function();
	// The C library's fegetround (__builtin_flt_rounds).
	lowir::SymbolId fegetround_function();
	// The runtime's trap (__builtin_trap): a call that neither returns nor
	// throws, which the native back end encodes as a trap instruction.
	lowir::SymbolId trap_function();
	// The runtime's memset (`object=cppgm_builtin_memset`, the C library's
	// in a host object), as the optimizer's fill loops declare it.
	lowir::SymbolId memset_function();
	// A soft-float routine, declared on first reference (its binary16 and
	// binary128 operands' classes stated in object mode).
	lowir::SymbolId soft_float_helper(SoftFloatHelper which);
	// Object mode: the psABI classes of a binary16 or binary128 value (an
	// SSE eightbyte; binary128's SSEUP upper half shares its register).
	uint8_t soft_float_classes(SoftFloat kind) const;
	// The module's DSO handle (`__dso_handle`, the C runtime's), which
	// __cxa_thread_atexit's third argument names.
	lowir::SymbolId dso_handle();
	lowir::SymbolId local_static_destructor(lowir::SymbolId global, const sema::Type* type, uint32_t location);
	// One destroying a local static's temporaries (each a finalizer, the
	// last first), named after the static.
	lowir::SymbolId local_static_destructor(lowir::SymbolId global, const std::vector<Finalizer>& temporaries);
	// A finalizer for a static object or temporary of `type` (not queued).
	Finalizer finalizer_for(lowir::SymbolId symbol, const sema::Type* type, uint32_t location);
	// A static reference's temporary's i8 flag, set once it is constructed.
	lowir::SymbolId constructed_flag(lowir::SymbolId temporary);
	// A static temporary of class type now constructed (12.2p5, 3.6.3p1):
	// its flag global set, its destruction registered at that point of
	// the construction order; returns the flag.
	lowir::SymbolId static_temporary_constructed(lowir::SymbolId temporary, const sema::Type* type, uint32_t location);
	// The unnamed object of a namespace- or block-scope anonymous union the
	// declaration declares (9.5p5), or null.
	static const sema::VariableEntity* anonymous_object(const syntax::Decl& decl);
	// The function a call or address odr-uses: an inline definition joins
	// the emission queue (an implicit special member's too, which a later
	// unit's lowering synthesizes).
	void demand(const sema::FunctionEntity* function, EntryVariant variant = EntryVariant::Complete);
	// A virtual table's reference of a virtual destructor: its complete
	// and deleting entries; an automatic object's destruction: all three.
	// A destructor's base entry as a definition of its own (a virtual
	// base's destruction, the course's form).
	void demand_base_entry(const sema::FunctionEntity* function);
	void demand_vtable_entries(const sema::FunctionEntity* destructor);
	void demand_destruction(const sema::FunctionEntity* destructor);
	// The call boundary of a class object by value (symbol_lowering.cpp):
	// as its bytes, or through an address the caller owns; a result as its
	// bytes, or through the caller's destination (an indirect result).
	bool passed_directly(const sema::ClassEntity* entity);
	bool copied_trivially(const sema::ClassEntity* entity);
	// The x86-64 psABI classification (3.2.3) of a parameter crossing a
	// boundary as its bytes (lowir::ObjectClassBits, `classes=`; 0 for
	// none): in object mode a class object's or member function pointer's
	// (the classes host objects' boundaries follow, once per class), else
	// the course's (course_classes).
	uint8_t object_classes(const sema::Type* type);
	// The same for a direct result (`result_classes=`).
	uint8_t result_classes(const sema::Type* type);
	// lowir.md "Object ABI Conventions": the classes a course boundary
	// states for a direct class object of at most 16 bytes, exactly when
	// some eightbyte is SSE or holds no member; `memory` for a parameter
	// with a misaligned member (a result with one is indirect), or with an
	// x87 or 16-byte vector member (a result with one states none).
	uint8_t course_classes(const sema::Type* type, bool result);
	// The course's classification of a class (course_classes): two bits
	// for each four of its first 16 bytes (0 none, 1 INTEGER, 2 SSE), and
	// CC_MEMORY or CC_UNMODELED; once per class.
	uint32_t course_class_quarters(const sema::ClassEntity* entity);
	void course_member_quarters(const sema::Type* type, uint64_t offset, uint32_t& quarters);
	// A complex value's classes (lower/complex_lowering.cpp): its parts'
	// as two fields', a complex long double's COMPLEX_X87.
	uint8_t complex_classes(const sema::Type* type);
	// libgcc's complex product (`divide`: quotient) routine for parts of
	// the floating type `part`, declared on first reference.
	lowir::SymbolId complex_helper(bool divide, const sema::Type* part);
	// A vector's (lower/vector_lowering.cpp), as GCC and Clang pass them.
	uint8_t vector_classes(const sema::Type* type);
	void classify_eightbytes(const sema::Type* type, uint64_t offset, uint8_t* classes);
	// A class result's convention, decided by its completed class alone
	// (`function`: the function type of the call or definition).
	bool returned_directly(const sema::ClassEntity* entity);
	bool indirect_result(const sema::Type* result, const sema::Type* function);
	bool indirect_result(const sema::FunctionEntity* function);
	const sema::FunctionEntity* closure_call(const sema::ClassEntity* entity) const;
	// Whether a class's destructor has no effect: an empty body (or an
	// implicit one) and subobjects whose destructors have none. Such a
	// destructor is emitted empty; an automatic object's call of it is
	// omitted (the definition kept as an object root). A polymorphic
	// class's destructor stores its tables, which only `contained` (as a
	// subobject of an object destroyed without effect) does not count.
	bool destructor_does_nothing(const sema::ClassEntity* entity, bool contained = false);
	void root_destructor(const sema::FunctionEntity* destructor);
	// Aggregate facts per class and unit (aggregate_lowering.cpp): every
	// class in an object of the type (recursively) is an aggregate, so no
	// constructor runs; and whether initializing an aggregate from an
	// empty list runs a constructor (8.5.1p7). A chain of classes deeper
	// than the limit not yet known counts as running one.
	bool runs_no_constructor(const sema::Type* type, uint32_t depth = 0);
	bool trivially_copied_elements(const sema::Type* type);
	bool empty_list_constructs(const sema::ClassEntity* entity, uint32_t depth = 0);
	// The internal helper a terminate guard calls with the escaping
	// exception (15.5.1): it begins a catch of it and calls std::terminate;
	// declared when first named, defined with the demanded definitions.
	lowir::SymbolId terminate_helper();
	// The runtime's std::terminate (role terminate), declared when first
	// named.
	lowir::SymbolId terminate_function();
	// Interned presentation spellings: "entry", "arg<N>".
	lowir::NameId entry_name() const { return entry_name_; }
	lowir::NameId this_name() const { return this_name_; }
	lowir::NameId ret_name() const { return ret_name_; }
	lowir::NameId argument_name(uint32_t index);
	// "other": an implicit copy or move function's parameter (the course's
	// spelling); "__slot": an unnamed variable's slot.
	lowir::NameId other_name() const { return other_name_; }
	// A parameter declaration's name; a pack element's (sema's
	// `<pack>__pack<i>`) as the course spells it: the pack's name for the
	// first element, `<pack>__pack<i+1>` for the others.
	lowir::NameId parameter_name(const Identifier* name);
	lowir::NameId unnamed_slot_name() const { return unnamed_slot_name_; }
	// Whether a slot's name is a range-for statement's implicit variable's
	// (6.5.4p1: __range, __begin, __end; __idx for an array's index).
	bool range_for_name(lowir::NameId name) const
	{
		return name == range_for_names_[0] || name == range_for_names_[1] || name == range_for_names_[2] ||
			name == range_for_names_[3];
	}
	// Claims a source name for a slot of the body whose slot scope is
	// `scope`; false if a slot of that body already has it (a shadowing
	// declaration). Each body takes its own scope when it starts, so a
	// body begun while another is being lowered (the unit's initializer,
	// a thread_local's) leaves the other's claims intact.
	bool claim_slot_name(lowir::NameId name, uint32_t scope);
	uint32_t new_slot_scope() { return ++slot_epoch_; }
	lowir::NameId intern(const char* text, uint32_t length) { return program_.names.intern(text, length); }
	// The recursion budget (support/stack_budget.h) from where the unit's
	// lowering began.
	std::uintptr_t stack_base() const { return stack_base_; }
	std::uintptr_t stack_budget() const { return stack_budget_; }

private:
	friend class FunctionLowering;

	// A class aggregate's initialization helper: its class, the number of
	// leading members it takes as parameters, its symbol, and the class's
	// previous helper (index + 1, 0 for none).
	struct AggregateHelper
	{
		const sema::ClassEntity* entity;
		uint32_t count;
		// The initialized object's cv-qualifiers (a const temporary's
		// helper is another, the course's form).
		unsigned cv;
		lowir::SymbolId symbol;
		uint32_t previous;
	};
	struct PendingDeclaration
	{
		lowir::SymbolId symbol;
		bool function;
		// A function's boundary, stated at the end of the unit that first
		// referenced it (from `entity`, that unit's) if it is still
		// undefined then.
		const sema::FunctionEntity* entity;
		EntryVariant variant;
		lowir::SignatureId signature;
		bool typed;
		lowir::Type type;
		lowir::SymbolMetadata metadata;
		// A global this unit defines (analysis complete): a streamed
		// function addresses it as defined.
		bool defined_here;
	};

	// Declarations and definitions of one unit.
	void declarations(const syntax::Slice<syntax::Decl*>& list);
	void declaration(const syntax::Decl& decl);
	void asm_declaration(const syntax::AsmDecl& decl);
	void name_deferred_members(const syntax::Decl& decl);
	void name_member_classes(const syntax::ClassSpecifier& specifier);
	void name_local_classes(const sema::FunctionEntity* function);
	void simple_declaration(const syntax::SimpleDecl& decl, bool explicit_specialization = false);
	void define_variable(
		const sema::VariableEntity* variable, const syntax::Initializer* initializer, uint32_t location);
	void define_function(const sema::FunctionEntity* function, uint32_t location);
	void emit_definition(const sema::FunctionEntity* function, EntryVariant variant, lowir::SymbolId symbol);
	void demand_entry(const sema::FunctionEntity* function, EntryVariant variant);
	void emit_demanded();
	void emit_aggregate_helper(const AggregateHelper& helper);
	// A covariant result's adjustment (10.3p7, Itanium 2.5.3) from the
	// class an overrider returns (`from`) to the class its slot's
	// introducing function returns: through the virtual base `via` by its
	// offset in the returned object's table (null for none), then by a
	// fixed `offset`; a null pointer stays null.
	struct ResultAdjustment
	{
		const sema::ClassEntity* from;
		const sema::ClassEntity* via;
		int64_t offset;
		bool reference;
	};
	// From the overrider's result class to the one `introducer` returns:
	// through `through` (a table's class) when the result class has that as
	// a base and it reaches the target by non-virtual bases.
	bool result_adjustment(const sema::FunctionEntity* overrider, const sema::FunctionEntity* introducer,
		ResultAdjustment& out, const sema::ClassEntity* through = nullptr);
	// result_adjustment's answers (unit-scoped: by overrider and
	// introducer).
	struct FunctionPair
	{
		const sema::FunctionEntity* first;
		const sema::FunctionEntity* second;
		bool operator==(const FunctionPair& other) const { return first == other.first && second == other.second; }
	};
	struct FunctionPairHash
	{
		size_t operator()(const FunctionPair& key) const;
	};
	struct KnownAdjustment
	{
		ResultAdjustment adjustment;
		bool needed;
	};
	FlatMap<FunctionPair, KnownAdjustment, FunctionPairHash> result_adjustments_;
	// Itanium table facts (lower/itanium_layout.cpp). A
	// class's rows before its tables' offsets to the top (Itanium 2.5.2,
	// Clang's VCallAndVBaseOffsetBuilder components, the nearest first):
	// along its primary chain from the farthest class, each class's vbase
	// offsets of the virtual bases not met yet, then, for a virtual primary
	// base (a level), its vcall offsets (Clang's AddVCallOffsets: its
	// non-virtual primary chain's functions, its own, its other non-virtual
	// dynamic bases', one per signature over the whole list, destructors
	// one). A table of the class as a virtual base adds the class's own
	// vcall offsets after them: its full rows (`full`, `count`, made on
	// first need), the others `first`, `prefix`. A row is a vbase offset
	// (`vbase`) or a vcall offset (its function, the subobject declaring it
	// and that subobject's offset in the level).
	struct ItaniumRow
	{
		const sema::ClassEntity* vbase;
		const sema::FunctionEntity* function;
		const sema::ClassEntity* holder;
		const sema::ClassEntity* level;
		uint64_t offset;
	};
	struct ItaniumRows
	{
		uint32_t first;
		uint32_t prefix;
		uint32_t full;
		uint32_t count;
	};
	const ItaniumRows& itanium_rows(const sema::ClassEntity* cls);
	const ItaniumRows& itanium_full_rows(const sema::ClassEntity* cls);
	void itanium_build_rows(const sema::ClassEntity* cls);
	void itanium_build_full(const sema::ClassEntity* cls);
	void itanium_vcall_walk(const sema::ClassEntity* owner, const sema::ClassEntity* level,
		const sema::ClassEntity* holder, uint64_t offset);
	int64_t itanium_vbase_offset_offset(const sema::ClassEntity* cls, const sema::ClassEntity* base);
	int64_t itanium_vcall_offset(const sema::ClassEntity* vbase, const sema::FunctionEntity* function);
	// A table's context: a group in its owner's own layout (`complete` the
	// owner `cls`, offset 0) or, `construction`, `cls`'s group under
	// construction at `offset` in `complete`; a segment's offset there.
	struct TableContext
	{
		const sema::ClassEntity* complete;
		const sema::ClassEntity* cls;
		uint64_t offset;
		bool construction;
	};
	int64_t context_at(const TableContext& context, uint32_t segment);
	// The rows of a segment's table: their count, and their values in a
	// context (in itanium_row_values_).
	uint32_t itanium_table_rows(const sema::VtableGroup* group, uint32_t segment);
	const int64_t* itanium_row_values(const TableContext& context, uint32_t segment);
	// A group's segment by its subobject's offset (the owner's own layout)
	// and class.
	uint32_t group_segment(const sema::ClassEntity* owner, uint64_t offset, const sema::ClassEntity* cls);
	const sema::ClassEntity* itanium_primary_base(const sema::ClassEntity* cls, bool& is_virtual);
	// A thunk's `this` adjustment (Itanium 5.1.4.2's <call-offset>): a
	// fixed `offset`, then, when `vcall` is nonzero, the
	// vcall offset at that position from the address point of the table
	// `this` then designates (a virtual thunk, `_ZTv`). `unused`: the entry
	// is null (its primary chain lost the virtual primary base it lies in,
	// see slot_anchor).
	struct ThisAdjustment
	{
		int64_t offset;
		int64_t vcall;
		bool unused;
	};
	// A thunk (Itanium 2.5.2, 5.1.4.2): a function passing its parameters
	// to `target`'s entry with `this` adjusted, its result adjusted when
	// `has_result` (a covariant thunk), defined with its function: by a
	// unit that defines `target` (after the unit's definitions), strong
	// when a strong definition of the function asked for it, else weak (an
	// internal one internal); declared by any other.
	struct Thunk
	{
		lowir::SymbolId symbol;
		lowir::SymbolId target;
		uint32_t this_index;
		ThisAdjustment adjustment;
		bool has_result;
		ResultAdjustment result;
		// A jump thunk (lowir::JumpThunk): streamed without a body.
		bool jump;
		lowir::SignatureId signature;
		bool internal;
		bool strong;
		// The entry it adjusts for (its special-member family).
		const sema::FunctionEntity* function;
		EntryVariant variant;
	};
	lowir::SymbolId thunk_symbol(const sema::FunctionEntity* function, EntryVariant variant, ThisAdjustment adjustment,
		uint8_t binding, const ResultAdjustment* result = nullptr, bool strong = false);
	// The virtual primary base whose subobject a slot's entry in a segment's
	// table is laid out for (null: the table's own subobject), and whether
	// the entry is unused (null); whether a class's own primary table
	// entry for a slot is a thunk.
	const sema::ClassEntity* slot_anchor(const TableContext& context, uint32_t segment, uint32_t slot, bool& unused);
	bool own_entry_thunk(const sema::ClassEntity* cls, uint32_t slot);
	// A construction table's entry: the base's own (as its own group has it:
	// adjustments in its own layout, its unused entries kept), but for a
	// subobject without a table there (a virtual primary base lying
	// elsewhere in the complete object), placed in the context; a
	// destructor's is null (no object is destroyed through one, as g++
	// lays it out).
	ThisAdjustment construction_adjustment(const TableContext& context, uint32_t segment, uint32_t slot);
	// The `this` adjustment of a slot of a segment's table (a table pointer
	// of its own in the context) to its entry's overrider's subobject,
	// from the subobject of the chain class declaring the
	// slot's function nearest the table's class (Clang's MethodInfo: a
	// virtual primary base's is where the context put it), and, when the
	// overrider is not within the virtual base that subobject is (or is
	// in), to that base, reading its vcall offset for the function
	// (Itanium 2.5.3, as Clang computes it; a covariant entry adjusts even
	// at the same offset, as GCC does).
	ThisAdjustment entry_adjustment(const TableContext& context, uint32_t segment, uint32_t slot);
	bool covariant_entry(const sema::FunctionEntity* function, sema::SlotKind kind,
		const sema::FunctionEntity* introducer, const sema::ClassEntity* through, ResultAdjustment& result);
	void emit_thunk(const Thunk& thunk);
	// The thunks the unit's table groups and construction tables need for a
	// virtual function, defined with the function (strong when it is; see
	// definition_thunks), from their entries indexed once per unit.
	void definition_thunks(const sema::FunctionEntity* function, uint8_t binding);
	void index_definition_thunks(const sema::ClassEntity* cls);
	void index_construction_thunks(
		const sema::ClassEntity* complete, const sema::ClassEntity* cls, uint64_t offset, bool top);
	void note_definition_thunk(const sema::FunctionEntity* function, sema::SlotKind kind, ThisAdjustment adjustment,
		const sema::FunctionEntity* introducer, const sema::ClassEntity* through);
	void push_definition_thunk(const sema::FunctionEntity* function, sema::SlotKind kind,
		const sema::ClassEntity* through, ThisAdjustment adjustment, const sema::FunctionEntity* introducer);
	// The segment whose table a segment's pointer holds under construction
	// (`cls` at `offset` in `complete`).
	uint32_t construction_owner(
		const sema::ClassEntity* complete, const sema::ClassEntity* cls, uint64_t offset, uint32_t segment);
	void define_views(const sema::ClassEntity* entity, uint8_t binding);
	void group_targets(const sema::ClassEntity* entity, uint8_t binding, std::vector<lowir::SymbolId>& targets);
	void group_items(const sema::ClassEntity* entity, const lowir::SymbolId* targets);
	void view_targets(
		const sema::ClassEntity* entity, uint32_t segment, uint8_t binding, std::vector<lowir::SymbolId>& targets);
	const lowir::SymbolId* view_items(
		const sema::ClassEntity* entity, uint32_t segment, const lowir::SymbolId* targets);
	void vtt_entries(const sema::ClassEntity* complete, const sema::ClassEntity* cls, uint64_t offset, bool top,
		std::vector<TableAddress>& out);
	uint64_t construction_segment_offset(
		const sema::ClassEntity* complete, const sema::ClassEntity* cls, uint64_t offset, uint32_t segment);
	TableAddress construction_address(
		const sema::ClassEntity* complete, const sema::ClassEntity* cls, uint64_t offset, uint32_t segment);
	// A segment's rows before its table's offset to the top (row_count),
	// emitted in a context (table_rows): its Itanium vcall and vbase
	// offsets.
	uint32_t row_count(const sema::VtableGroup* group, uint32_t segment);
	void table_rows(const TableContext& context, uint32_t segment);
	// A class and one of its bases: the key of a base's route and of a
	// dynamic_cast's hint.
	struct ClassPair
	{
		const sema::ClassEntity* cls;
		const sema::Entity* base;
		bool operator==(const ClassPair& other) const { return cls == other.cls && base == other.base; }
	};
	struct ClassPairHash
	{
		size_t operator()(const ClassPair& key) const;
	};
	// A virtual base's place in a class: its position in VirtualBases, its
	// row in the class's table, and its VTT's index in the class's VTT past
	// the class's sub-VTT (PointerMap::kMissing until vtt_virtual_index
	// first needs the class's); all kMissing for a class that is none of
	// its virtual bases.
	struct VirtualBasePlace
	{
		uint32_t position;
		uint32_t row;
		uint32_t vtt;
	};
	VirtualBasePlace virtual_base_place(const sema::ClassEntity* cls, const sema::Entity* base);
	// A wide class's non-virtual direct bases (one with virtual bases; see
	// base_path_search): the index of the first in nonvirtual_store_, a
	// list ended by null (later queries may move it); the first virtual
	// base holding a class through non-virtual bases (see base_route).
	uint32_t nonvirtual_bases(const sema::ClassEntity* cls);
	uint32_t virtual_holder(const sema::ClassEntity* derived, const sema::ClassEntity* base);
	// A class's VTT positions, entered at its first query: the index of its
	// segment 0's in vtt_segment_store_ (which later queries may move); the
	// table pointers' order (vtt_pointer_order, from vtt_pointer_walk over
	// the inheritance graph).
	uint32_t vtt_positions(const sema::ClassEntity* cls);
	uint32_t vtt_pointer_order(const sema::ClassEntity* cls);
	void vtt_pointer_walk(
		const sema::ClassEntity* complete, const sema::ClassEntity* cls, uint64_t offset, bool morally_virtual);
	lowir::SymbolId table_entry(const sema::FunctionEntity* function, sema::SlotKind kind, ThisAdjustment adjustment,
		uint8_t binding, const sema::FunctionEntity* introducer, const sema::ClassEntity* through, bool strong = false);
	void resolve_singleton_roles();
	bool emitted_at_definition(const sema::FunctionEntity* function) const;
	void release_definition(lowir::SymbolId symbol);
	void mark_source_inline(const sema::FunctionEntity* function, lowir::SymbolId symbol);
	void finish_source_inlining();
	lowir::SymbolId source_immediate_guard();
	// Special-member families (lower/definition_order.cpp): a definition's
	// memberships (`base_object`: its base entry's object name if known),
	// the identities they share, and the families' order among the
	// definitions.
	void note_family_members(
		const sema::FunctionEntity* function, EntryVariant variant, lowir::SymbolId symbol, lowir::NameId base_object);
	void note_thunk_family(const Thunk& thunk);
	uint32_t function_identity(
		const sema::FunctionEntity* function, EntryVariant variant, lowir::SymbolId symbol, lowir::NameId base_object);
	uint32_t class_identity(const sema::ClassEntity* entity);
	void order_special_member_families(std::vector<lowir::SymbolId>& definitions);
	// The forward boundaries of a definition's direct callees still
	// referenced only (for a sink that selects code).
	void state_forward_boundaries(lowir::SymbolId symbol);

	// Symbols (lower/symbol_lowering.cpp).
	lowir::SymbolId new_symbol(const std::string& spelling);
	lowir::SymbolId external_symbol(const std::string& object, const std::string& spelling);
	lowir::NameId local_object(const std::string& object, bool c_linkage);
	void qualified_spelling(const sema::Scope* scope, std::string& out);
	std::string function_spelling(const sema::FunctionEntity* function);
	std::string class_spelling(const sema::ClassEntity* entity);
	void append_class_label(const sema::ClassEntity* entity, std::string& out);
	void append_type_label(const sema::Type* type, std::string& out, size_t limit);
	void append_label_identity(const sema::ClassEntity* entity, std::string& out);
	// A closure type of a lambda in a function body is named by where the
	// lambda appears (the course's form): its prefix and label.
	const sema::ClosureFacts* located_closure(const sema::ClassEntity* entity) const;
	void closure_prefix(const sema::ClosureFacts& facts, std::string& out);
	void append_closure_label(const sema::ClassEntity* entity, const sema::ClosureFacts& facts, std::string& out);
	void owner_spelling(const sema::ClassEntity* entity, std::string& out);
	// Localities (memoized per unit): a type's is its components'; a class's
	// or enumeration's its scopes' and a specialization's arguments'.
	Locality type_locality(const sema::Type* type, bool nested = false);
	Locality entity_locality(const sema::Entity* entity);
	Locality argument_locality(const sema::FunctionEntity* function);
	// 3.5p4, p8-p10: an entity only its own unit can name, whose object
	// spelling another unit's different entity may share.
	bool unit_local(const sema::FunctionEntity* function);
	bool local_signature(const sema::FunctionEntity* function);
	bool shared_local(const sema::Entity* entity);
	bool unit_local(const sema::VariableEntity* variable);
	// Whether a type's type information name is its unit's own (g++'s `*`
	// prefix, compared by address at run time).
	bool private_type_name(const sema::Type* type);
	bool private_entity_name(const sema::Entity* entity);
	std::string function_object(const sema::FunctionEntity* function, EntryVariant variant);
	std::string variable_object(const sema::VariableEntity* variable);
	abi_mangle::AbiNode variable_name(const sema::VariableEntity* variable);
	abi_mangle::AbiNode variable_source_name(const sema::VariableEntity* variable);
	// A function-local static object's <local-name> (5.1.7).
	abi_mangle::AbiNode local_static_name(const sema::VariableEntity* variable);
	abi_mangle::AbiNode function_encoding(const sema::FunctionEntity* function, EntryVariant variant);
	abi_mangle::AbiNode declared_argument(
		const sema::TemplateParameterEntity* parameter, const sema::Type* given, abi_mangle::AbiNode argument);
	abi_mangle::AbiNode declaration_node(const sema::TemplateParameterEntity* parameter);
	bool declared_type_dependent(const sema::Type* type);
	abi_mangle::AbiNode head_declaration(const sema::TemplateHead* head, bool pack, uint32_t level);
	abi_mangle::AbiNode argument_head_declaration(const sema::TemplateHead* head, bool pack, uint32_t level);
	std::string tls_wrapper_object(const sema::VariableEntity* variable);
	// A thread-local variable's <name> as its wrapper and initialization
	// function name it; a special name (ABI_TARGET_ENTITY: the object's own
	// mangled name) of an object's <name>.
	abi_mangle::AbiNode thread_local_name(const sema::VariableEntity* variable);
	std::string special_object(abi_mangle::AbiTargetKind kind, abi_mangle::AbiNode name);
	// A thread-local variable's wrapper symbol, or PointerMap::kMissing.
	uint32_t thread_local_wrapper(const sema::VariableEntity* variable) const { return tls_wrappers_.find(variable); }
	lowir::SymbolId thread_local_wrapper_symbol(
		lowir::SymbolId global, const sema::VariableEntity* variable, bool used = true);
	// Returns the guard of the object's per-thread initialization (none
	// but the destruction's registration unless `dynamic`).
	lowir::SymbolId thread_local_initialization(lowir::SymbolId global, const sema::VariableEntity* variable,
		const syntax::Initializer* initializer, uint32_t location, bool dynamic);
	// Object mode: an external thread-local variable's initialization entry
	// (`_ZTH`), and the wrapper of one another unit defines.
	lowir::SymbolId thread_local_init_entry(lowir::SymbolId global, const sema::VariableEntity* variable, bool define);
	// Object mode: the unit's thread-local initialization function, which
	// runs its ordered thread-local variables' initializations once per
	// thread (3.6.2p4: before the first odr-use of any of them); declared
	// when first needed, defined at the unit's end.
	lowir::SymbolId unit_thread_local_init();
	void define_unit_thread_local_init();
	lowir::SymbolId external_thread_local_wrapper(lowir::SymbolId global, const sema::VariableEntity* variable);
	// The guard that the units instantiating a member share (Itanium 2.8).
	lowir::SymbolId unordered_guard(lowir::SymbolId global, const sema::VariableEntity* variable);
	abi_mangle::AbiNode abi_scope(const sema::Scope* scope);
	uint32_t push_abi_tags(const void* owner);
	abi_mangle::AbiNode tagged_name(abi_mangle::AbiNode scope, abi_mangle::AbiNode unqualified, const void* owner,
		uint32_t discriminator, bool internal_linkage);
	abi_mangle::AbiNode with_implicit_tags(abi_mangle::AbiNode entity, const sema::Type* type);
	abi_mangle::AbiNode class_name(const sema::ClassEntity* entity);
	abi_mangle::AbiNode class_name_node(const sema::ClassEntity* entity);
	abi_mangle::AbiNode name_template_id(const sema::Type* name, const sema::Type* const* arguments, uint32_t count);
	abi_mangle::AbiNode type_unqualified_name(const sema::Entity* entity);
	abi_mangle::AbiNode closure_context_scope(const sema::ClassEntity* closure, abi_mangle::AbiNode scope);
	bool numbered_closure(const sema::ClassEntity* closure);
	bool visible_type(const sema::Entity* entity);
	abi_mangle::AbiNode anonymous_name(const sema::Entity* type);
	uint32_t anonymous_ordinal(const sema::Entity* type, const sema::FunctionEntity* context);
	uint32_t local_discriminator(const sema::Entity* entity, uint32_t declared);
	uint32_t unnamed_ordinal(const sema::Entity* entity);
	abi_mangle::AbiNode abi_type(const sema::Type* type);
	abi_mangle::AbiNode abi_type_node(const sema::Type* type);
	abi_mangle::AbiNode abi_literal(const sema::Type* type, uint64_t bits);
	uint32_t own_level(const sema::TemplateParameterEntity* parameter) const;
	// A template signature's placeholders (lower/placeholder_mangling.cpp):
	// a keyed placeholder's operand as an <expression> read from its key
	// (5.1.6; ABI_NONE for one the key does not mangle), as the type or
	// template argument it stands for, and whether a type names one the
	// key does not mangle (the specialization's type stands in).
	abi_mangle::AbiNode placeholder_expression(const sema::TemplateParameterEntity* placeholder);
	abi_mangle::AbiNode unresolved_prefix(const sema::Type* qualifier);
	abi_mangle::AbiNode qualifier_levels(const sema::Type* qualifier);
	abi_mangle::AbiNode placeholder_type(const sema::TemplateParameterEntity* placeholder);
	bool unmangled_placeholder(const sema::Type* type);
	class PlaceholderReader;
	friend class PlaceholderReader;
	lowir::SymbolMetadata function_metadata(const sema::FunctionEntity* function, lowir::SymbolId symbol);
	lowir::SymbolMetadata variable_metadata(const sema::VariableEntity* variable, lowir::SymbolId symbol);
	lowir::NameId variable_section(const sema::VariableEntity* variable);
	uint32_t overload_ordinal(const sema::FunctionEntity* function);
	void grow_symbol_tables();
	lowir::SignatureId function_signature(
		const sema::FunctionEntity* function, bool named, EntryVariant variant = EntryVariant::Complete);
	// A base-object entry of a class with virtual bases: it takes the VTT
	// after `this` (Itanium 2.6: (this, VTT, arguments)), and reads its
	// virtual bases' offsets from the tables the VTT names.
	static bool HiddenBaseParameters(const sema::FunctionEntity* function, EntryVariant variant);
	lowir::NameId vtt_name();
	lowir::Parameter parameter_for(const sema::Type* type, lowir::NameId name);
	void push_result_parameter(const sema::Type* result, bool named, lowir::Signature& signature);
	void set_pending(lowir::SymbolId symbol, const PendingDeclaration& pending);
	void state_pending_signatures(size_t first);

	// Polymorphic classes (lower/vtable_lowering.cpp).
	void define_typeinfo(const sema::ClassEntity* entity, lowir::SymbolId symbol);
	lowir::SymbolId typeinfo_name(const sema::ClassEntity* entity);
	lowir::SymbolId rtti_vtable(TypeInfoKind kind);
	lowir::SymbolId type_name_object(const sema::Type* type, const std::string& label, bool internal);
	std::string type_object(const sema::Type* type, abi_mangle::AbiTargetKind kind);
	lowir::SymbolId pure_virtual_symbol(const sema::FunctionEntity* function);
	lowir::SignatureId argument_signature(const sema::FunctionEntity* function);
	lowir::SymbolId class_global(
		const sema::ClassEntity* entity, abi_mangle::AbiTargetKind kind, const char* prefix, bool internal = false);
	// A host object's type information describing a class incomplete in
	// its unit (or a pointer reaching one) is the unit's own, as GCC and
	// Clang emit it: another unit's object of the name may describe the
	// complete class.
	bool local_type_information(const sema::ClassEntity* entity);
	bool local_type_information(const sema::Type* type);
	std::string class_object(const sema::ClassEntity* entity, abi_mangle::AbiTargetKind kind);
	uint32_t data_item(const lowir::DataItem& item);
	void define_data(lowir::SymbolId symbol, uint32_t first, lowir::SymbolMetadata metadata);
	bool emits_vtable(const sema::ClassEntity* entity) const;
	static bool instantiated_elsewhere(const sema::ClassEntity* entity);
	uint32_t inheritance_flags(const sema::ClassEntity* entity);
	// dynamic_cast hints (dynamic_type_lowering.cpp): one walk of the base
	// paths from `derived` (null `base`: every base's facts, see
	// hint_facts_), and a base's hint from those facts.
	int64_t hint_walk(const sema::ClassEntity* base, const sema::ClassEntity* derived, uint32_t location);
	int64_t walked_hint(const sema::ClassEntity* base) const;

	// Globals (lower/global_lowering.cpp): an object's global and static
	// initialization (whether dynamic initialization remains; a default
	// initialization stating only a table pointer; an evaluated constant).
	struct StaticOutcome
	{
		bool dynamic;
		bool table_only;
		bool evaluated;
	};
	void define_global(lowir::SymbolId symbol, const sema::VariableEntity* variable,
		const syntax::Initializer* initializer, uint32_t location);
	StaticOutcome static_global(lowir::SymbolId symbol, const sema::VariableEntity* variable,
		const syntax::Initializer* initializer, uint32_t location, lowir::SymbolMetadata metadata);
	// The alignment of a global object of `size` bytes that its LowIR form
	// does not imply: see the definition.
	void state_storage_type(lowir::Global& global, uint64_t size, uint64_t alignment);
	bool define_reference(lowir::Global& global, const sema::VariableEntity* variable,
		const syntax::Initializer* initializer, uint32_t location);
	bool static_object(const sema::Type* type, const syntax::Initializer* initializer, StaticData& out);
	bool static_construction(const sema::Type* type, const sema::FunctionEntity* constructor,
		const syntax::Initializer* initializer, StaticData& out);
	bool static_constructor(const sema::FunctionEntity* constructor, syntax::Expr* const* arguments, uint32_t count,
		uint64_t origin, std::vector<StaticPlacement>& out, uint64_t& cursor, uint32_t depth);
	bool static_tables(const sema::ClassEntity* cls, uint64_t origin, std::vector<StaticPlacement>& out, size_t first,
		uint64_t& cursor);
	bool static_subobject(const sema::SubobjectInit& init, const sema::Type* type,
		const sema::FunctionEntity* constructor, syntax::Expr* const* arguments, uint32_t count,
		const std::vector<const sema::Entity*>& parameters, uint64_t at, std::vector<StaticPlacement>& out,
		uint64_t& end, uint32_t depth);
	bool static_parameter_value(const syntax::Expr& expr, const sema::Type* type,
		const sema::FunctionEntity* constructor, syntax::Expr* const* arguments, uint32_t count,
		const std::vector<const sema::Entity*>& parameters, StaticValue& out);
	bool static_element(const sema::Type* type, const syntax::Expr& element, StaticData& out);
	bool static_item(
		const sema::Type* type, bool member, const syntax::BracedInitList* list, uint32_t& next, StaticData& out);
	bool static_aggregate(const sema::Type* type, const syntax::BracedInitList* list, uint32_t& next, StaticData& out);
	void flush_bits(std::vector<uint8_t>& bits, uint64_t& position, StaticData& out);
	bool static_string(const sema::Type* type, const syntax::Expr& element, StaticData& out);
	bool static_vector(const sema::Type* type, const syntax::BracedInitList& list, StaticData& out);
	bool static_complex(const sema::Type* type, const syntax::Expr& expr, StaticData& out);
	bool evaluated_data(const sema::VariableEntity* variable, const syntax::Initializer* initializer,
		const sema::ConstValue* recorded, bool nonconstant, StaticData& out, uint32_t location);
	bool missing_element(const sema::Type* type, bool member, StaticData& out);
	bool zero_value_initialized(const sema::Type* type, uint32_t depth);
	static bool memberwise_constructor(const sema::Entity* entity, uint32_t arguments);
	bool static_converted(
		const syntax::Expr& expr, const sema::Type* target, StaticValue& out, bool nonconstant = false);
	bool static_value(const syntax::Expr& expr, StaticValue& out, bool nonconstant);
	bool static_lvalue(const syntax::Expr& expr, StaticValue& out);
	bool static_binding(const syntax::Expr& expr, const sema::Type* reference, StaticValue& out);
	bool static_item(const sema::Type* type, const StaticValue& value, lowir::DataItem& item);
	void append_item(const lowir::DataItem& item, StaticData& out);
	void append_zero(uint64_t bytes, StaticData& out);
	// Constant initialization from evaluated values (constant_lowering.cpp).
	const sema::ConstValue* initialization_value(
		const sema::VariableEntity* variable, const syntax::Initializer* initializer);
	bool constant_item(const sema::Type* type, const sema::ConstValue& value, lowir::DataItem& item, uint32_t location);
	uint32_t member_pointer_items(const sema::ConstValue& value, lowir::DataItem* items, uint32_t location);
	void demand_constructions(const syntax::Expr* expr, uint32_t depth);
	void demand_constructions(const syntax::Initializer* initializer);
	void demand_initializer_calls(const syntax::Expr* expr, uint32_t depth);
	// A complete object's table pointers (each segment's with its own).
	void place_tables(const sema::ClassEntity* cls, uint64_t offset, std::vector<StaticPlacement>& out);
	bool constant_placements(const sema::Type* type, const sema::ConstValue& value, uint64_t offset,
		std::vector<StaticPlacement>& out, std::vector<uint8_t>& bits, uint32_t location, uint32_t depth,
		bool table_placed);
	bool constant_data(const sema::Type* type, const sema::ConstValue& value, StaticData& out, uint32_t location);
	bool scalar_array_data(
		const sema::Type* type, const sema::ConstValue& value, StaticData& out, uint32_t location, uint32_t depth);
	bool constant_global(
		const sema::Type* type, const sema::ConstValue& value, lowir::Global& global, uint32_t location);
	lowir::SymbolId constant_temporary(const sema::ConstValue* temporary, uint32_t location);
	FunctionLowering& initializer_function();
	void finalizer_function();
	void finalize(lowir::SymbolId symbol, const sema::Type* type, uint32_t location, lowir::SymbolId guard);

	lowir::Program& program_;
	LoweringCounters counters_;
	// The unit being lowered.
	sema::SemanticGraph* graph_;
	sema::Analyzer* analyzer_;
	std::unique_ptr<DebugInfo> source_debug_;
	// Unit-scoped identities: entity -> symbol.
	PointerMap symbols_;
	// Functions emitted where odr-used, waiting (unit-scoped), and the
	// mark of those queued or defined (by entity, and by entity for a
	// constructor's base entry).
	struct Demanded
	{
		const sema::FunctionEntity* function;
		EntryVariant variant;
	};
	std::vector<Demanded> demanded_;
	// GNU always_inline functions' unit-local copies (inline_copy), and
	// those whose definitions wait.
	PointerMap inline_copies_;
	std::vector<std::pair<const sema::FunctionEntity*, lowir::SymbolId>> pending_copies_;
	// Destructor definitions whose writing waits for the unit's end.
	std::vector<lowir::SymbolId> held_definitions_;
	std::unique_ptr<SourceInlining> source_inlining_;
	PointerMap queued_;
	PointerMap queued_base_;
	PointerMap queued_deleting_;
	// Constructors' base entries and virtual destructors' deleting entries
	// (unit-scoped): entity -> symbol.
	PointerMap base_symbols_;
	PointerMap deleting_symbols_;
	// Polymorphic classes' virtual tables and classes' type information
	// objects and names (unit-scoped): class -> symbol; virtual calls'
	// boundaries: function -> signature.
	PointerMap vtables_;
	PointerMap typeinfos_;
	// base_path_offset's classes known not to lead to the base it seeks,
	// cleared per query.
	PointerMap base_misses_;
	PointerMap typeinfo_names_;
	PointerMap virtual_signatures_;
	// The runtime's type information vtables by kind (the class kinds'
	// program-wide singleton roles), its functions, and void's type
	// information (kNoId until referenced); other types' type information
	// (unit-scoped: canonical type -> symbol).
	lowir::SymbolId rtti_vtables_[static_cast<size_t>(TypeInfoKind::Count)] = { lowir::kNoId, lowir::kNoId,
		lowir::kNoId, lowir::kNoId, lowir::kNoId, lowir::kNoId, lowir::kNoId, lowir::kNoId, lowir::kNoId };
	lowir::SymbolId runtime_functions_[static_cast<size_t>(RuntimeFunction::Count)] = { lowir::kNoId, lowir::kNoId,
		lowir::kNoId, lowir::kNoId, lowir::kNoId, lowir::kNoId, lowir::kNoId, lowir::kNoId, lowir::kNoId,
		lowir::kNoId };
	lowir::SymbolId void_information_ = lowir::kNoId;
	PointerMap type_informations_;
	// The runtime's type information a throw or handler names (unit-scoped:
	// canonical type -> symbol, see exception_type_information).
	PointerMap exception_informations_;
	// The types thrown, and the runtime types' defined information (see
	// thrown_type; unit-scoped: canonical type -> 1, symbol).
	PointerMap thrown_types_;
	PointerMap defined_informations_;
	// itanium_rows' lists (unit-scoped: class -> index in
	// itanium_row_heads_; the rows in itanium_row_store_), the positions of
	// a class's vbase offsets ((class, base) -> index in its list) and of
	// the vcall offsets of its table as a virtual base (by the class and
	// the function's signature, destructors as nulls), and the walks'
	// scratch: the virtual bases met, the classes expanded, the signatures
	// met; the values' scratch; group_segment's segments (unit-scoped: by
	// owner, offset and class, a null class marking an owner entered).
	struct SignatureKey
	{
		const void* name;
		const void* signature;
		bool operator==(const SignatureKey& other) const { return name == other.name && signature == other.signature; }
	};
	struct SignatureHash
	{
		size_t operator()(const SignatureKey& key) const;
	};
	struct VcallKey
	{
		const sema::ClassEntity* cls;
		const void* name;
		const void* signature;
		bool operator==(const VcallKey& other) const
		{
			return cls == other.cls && name == other.name && signature == other.signature;
		}
	};
	struct VcallHash
	{
		size_t operator()(const VcallKey& key) const;
	};
	struct SegmentKey
	{
		const sema::ClassEntity* owner;
		uint64_t offset;
		const sema::ClassEntity* cls;
		bool operator==(const SegmentKey& other) const
		{
			return owner == other.owner && offset == other.offset && cls == other.cls;
		}
	};
	struct SegmentHash
	{
		size_t operator()(const SegmentKey& key) const;
	};
	PointerMap itanium_row_lists_;
	std::vector<ItaniumRows> itanium_row_heads_;
	std::vector<ItaniumRow> itanium_row_store_;
	FlatMap<ClassPair, uint32_t, ClassPairHash> itanium_vbase_positions_;
	FlatMap<VcallKey, uint32_t, VcallHash> itanium_vcall_positions_;
	std::vector<std::pair<const sema::ClassEntity*, bool>> itanium_chain_;
	uint32_t itanium_row_base_ = 0;
	FlatMap<SignatureKey, uint8_t, SignatureHash> itanium_signatures_;
	std::vector<int64_t> itanium_row_values_;
	FlatMap<SegmentKey, uint32_t, SegmentHash> group_segments_;
	// caller_destroyed_types' map; FunctionLowering::element_temporaries'
	// (unit-scoped).
	PointerMap caller_destroyed_types_;
	PointerMap element_temporaries_;
	// holds_data_member_pointer's classes (unit-scoped: 1 none, 2 some).
	PointerMap data_member_pointer_classes_;
	// changeable_class's classes (unit-scoped: 1 no, 2 yes).
	PointerMap changeable_classes_;
	// null_pieces' spans by (class, completeness) (unit-scoped).
	PointerMap null_piece_spans_;
	std::vector<NullSpan> null_spans_;
	std::vector<NullPiece> null_piece_store_;
	// inheritance_flags' scratch.
	std::vector<const sema::ClassEntity*> flag_stack_;
	PointerMap flag_marks_;
	// dynamic_cast hints (unit-scoped: (derived, base) -> hint), and the
	// class whose base paths the last walk followed: whether that walk took
	// every base's facts (a public path's count, whether one is through a
	// virtual base, its offset; by class, an index into hint_facts_) and
	// went past the walk's limit; the walk's stack.
	FlatMap<ClassPair, int64_t, ClassPairHash> cast_hints_;
	const sema::ClassEntity* hint_class_ = nullptr;
	bool hint_facts_taken_ = false;
	bool hint_overflow_ = false;
	struct HintFacts
	{
		uint32_t public_count;
		bool virtual_path;
		uint64_t offset;
	};
	PointerMap hint_index_;
	std::vector<HintFacts> hint_facts_;
	struct HintStep
	{
		const sema::ClassEntity* cls;
		uint64_t offset;
		bool public_path;
		bool virtual_path;
	};
	std::vector<HintStep> hint_stack_;
	lowir::SymbolId pure_virtual_ = lowir::kNoId;
	// Class aggregates' initialization helpers (unit-scoped): one per
	// class and number of leading members taken as parameters; those from
	// `pending_helper_` on wait for their definitions.
	std::vector<AggregateHelper> aggregate_helpers_;
	// Each class's latest helper (index + 1).
	PointerMap helper_index_;
	// Thread-local objects initialized per thread: variable -> wrapper.
	PointerMap tls_wrappers_;
	// Destructors without effect (class -> 1 no effect, 2 some; as a
	// subobject: contained_destructors_), and those whose omitted calls
	// keep them as object roots.
	PointerMap inert_destructors_;
	PointerMap contained_destructors_;
	PointerMap rooted_destructors_;
	// The function definitions in completion order, written or kept in
	// the program's order at its end (order_completed_definitions).
	std::vector<lowir::SymbolId> completed_;
	// Special-member families (lowir.md "Program Structure"), noted as
	// each definition is emitted: one class's copy forms before its move
	// forms (its constructors; its assignment operators), one constructor's
	// entries base before complete, one destructor's base, deleting,
	// complete (and so its thunks adjusting `this` alike). The completed
	// order puts each family's members in that order in the positions they
	// occupy (lower/definition_order.cpp).
	enum class FamilyKind : uint8_t
	{
		ConstructorForm,
		AssignmentForm,
		Entries
	};
	struct FamilyMember
	{
		lowir::SymbolId symbol;
		// The function's or class's identity (function_identity,
		// class_identity) and the thunks' `this` adjustment.
		uint32_t identity;
		FamilyKind kind;
		uint8_t rank;
		int64_t offset;
		int64_t vcall;
	};
	std::vector<FamilyMember> family_members_;
	// Unit-scoped: function or class -> identity (an object name across
	// units, or a unit-local entity's own number from kLocalIdentity).
	PointerMap identities_;
	uint32_t local_identities_;
	static const uint32_t kLocalIdentity = 0x80000000u;
	// Classes passed and returned as their bytes (class -> 1 yes, 2 no).
	PointerMap direct_classes_;
	// Object mode: their psABI classification (class -> object_classes);
	// otherwise the course's (class -> course_class_quarters).
	PointerMap object_classes_;
	// runs_no_constructor and empty_list_constructs (class -> 1 yes, 2 no).
	PointerMap constructor_free_;
	PointerMap empty_list_constructs_;
	// The terminate helper and the runtime functions it calls (kNoId until
	// needed), and whether its definition is still to be emitted.
	lowir::SymbolId terminate_helper_ = lowir::kNoId;
	lowir::SymbolId std_terminate_ = lowir::kNoId;
	bool terminate_pending_ = false;
	size_t pending_helper_ = 0;
	// The unit's thunks, waiting for their definitions from `pending_thunk_`
	// on (each a pending declaration until defined), and their positions
	// there by entry, adjustment and whether internal.
	std::vector<Thunk> thunks_;
	size_t pending_thunk_ = 0;
	struct ThunkKey
	{
		const sema::FunctionEntity* function;
		int64_t adjustment;
		int64_t vcall;
		EntryVariant variant;
		bool internal;
		// A covariant thunk's result adjustment (no `result_from`: none).
		const sema::ClassEntity* result_from;
		const sema::ClassEntity* result_via;
		int64_t result_offset;
		bool operator==(const ThunkKey& other) const
		{
			return function == other.function && adjustment == other.adjustment && vcall == other.vcall &&
				variant == other.variant && internal == other.internal && result_from == other.result_from &&
				result_via == other.result_via && result_offset == other.result_offset;
		}
	};
	struct ThunkHash
	{
		size_t operator()(const ThunkKey& key) const;
	};
	FlatMap<ThunkKey, uint32_t, ThunkHash> thunk_symbols_;
	// The table entries of a class's own virtual functions that may need
	// a thunk (unit-scoped: class -> 1 once indexed; function
	// -> its first entry in definition_thunks_, chained by `next`).
	struct DefinitionThunk
	{
		sema::SlotKind kind;
		const sema::ClassEntity* through;
		ThisAdjustment adjustment;
		const sema::FunctionEntity* introducer;
		uint32_t next;
		uint32_t previous;
	};
	struct DefinitionKey
	{
		const sema::FunctionEntity* function;
		const sema::FunctionEntity* introducer;
		int64_t offset;
		int64_t vcall;
		sema::SlotKind kind;
		const sema::ClassEntity* through;
		bool operator==(const DefinitionKey& other) const
		{
			return function == other.function && introducer == other.introducer && offset == other.offset &&
				vcall == other.vcall && kind == other.kind && through == other.through;
		}
	};
	struct DefinitionHash
	{
		size_t operator()(const DefinitionKey& key) const
		{
			return (reinterpret_cast<uintptr_t>(key.function) * 0x9E3779B97F4A7C15ull) ^
				(reinterpret_cast<uintptr_t>(key.introducer) * 31) ^
				static_cast<uint64_t>(key.offset) * 0xBF58476D1CE4E5B9ull ^ static_cast<uint64_t>(key.vcall) ^
				(static_cast<uint64_t>(key.kind) << 3) ^ reinterpret_cast<uintptr_t>(key.through) * 7;
		}
	};
	PointerMap definition_thunk_classes_;
	PointerMap definition_thunk_heads_;
	std::vector<DefinitionThunk> definition_thunks_;
	FlatMap<DefinitionKey, uint32_t, DefinitionHash> definition_thunk_seen_;
	PointerMap construction_sensitive_;
	// Table groups' segment symbols (unit-scoped: class -> index of its
	// first, the primary table's, in view_symbols_) and segment offsets
	// (class -> index of the first in segment_offsets_).
	PointerMap view_tables_;
	std::vector<lowir::SymbolId> view_symbols_;
	PointerMap offset_tables_;
	std::vector<uint64_t> segment_offsets_;
	// Each segment's address point in a class's table group that another
	// unit defines (unit-scoped: class -> index of segment 0's in
	// external_points_; see table_address).
	PointerMap external_groups_;
	std::vector<uint64_t> external_points_;
	// Virtual bases' lists (class -> index of its first in
	// vbase_row_store_, see vbase_rows), sub-VTT sizes, VTTs and construction tables (by
	// the complete class, the base under construction, its offset and the
	// segment; unit-scoped).
	PointerMap vbase_rows_;
	std::vector<const sema::ClassEntity*> vbase_row_store_;
	// Each virtual base's place in a class with virtual bases (unit-scoped:
	// entered with the class's row order, see vbase_rows), keyed by the
	// index of the class's rows (shared with the primary base when it has
	// the same virtual bases in the same order, and so the same places).
	struct PlaceKey
	{
		uint32_t rows;
		const sema::Entity* base;
		bool operator==(const PlaceKey& other) const { return rows == other.rows && base == other.base; }
	};
	struct PlaceHash
	{
		size_t operator()(const PlaceKey& key) const;
	};
	FlatMap<PlaceKey, VirtualBasePlace, PlaceHash> virtual_base_places_;
	// base_route's routes from classes with virtual bases (unit-scoped: a
	// complete class's layout is final).
	FlatMap<ClassPair, BaseRoute, ClassPairHash> base_routes_;
	// nonvirtual_bases' lists (unit-scoped: class -> index of its first
	// entry); virtual_holder's classes (unit-scoped: by the class's rows'
	// index and the class below, the virtual base's position; a null class
	// marks the rows entered) and its walk's stack.
	PointerMap nonvirtual_lists_;
	std::vector<const sema::BaseClass*> nonvirtual_store_;
	FlatMap<PlaceKey, uint32_t, PlaceHash> virtual_holders_;
	std::vector<const sema::ClassEntity*> holder_walk_;
	// Construction orders (class -> index of its first in
	// virtual_order_store_; unit-scoped) and virtual_base_order's scratch.
	PointerMap virtual_orders_;
	std::vector<const sema::BaseClass*> virtual_order_store_;
	std::vector<const sema::ClassEntity*> order_stack_;
	PointerMap order_positions_;
	std::vector<uint8_t> order_listed_;
	PointerMap vtt_sizes_;
	PointerMap vtts_;
	// VTT positions (unit-scoped, see vtt_positions): each direct base's
	// sub-VTT position, by its base-specifier; each segment's table
	// pointer's (class -> index of segment 0's in vtt_segment_store_, the
	// position past the bases' sub-VTTs).
	PointerMap vtt_base_positions_;
	PointerMap vtt_segments_;
	std::vector<uint32_t> vtt_segment_store_;
	// Host objects' VTT pointer orders (unit-scoped: class -> index of the
	// count in vtt_order_store_, the segments after it) and their walk's
	// scratch: the group's segments by subobject, the virtual bases met.
	struct SubobjectKey
	{
		uint64_t offset;
		const sema::ClassEntity* cls;
		bool operator==(const SubobjectKey& other) const { return offset == other.offset && cls == other.cls; }
	};
	struct SubobjectHash
	{
		size_t operator()(const SubobjectKey& key) const;
	};
	PointerMap vtt_orders_;
	std::vector<uint32_t> vtt_order_store_;
	FlatMap<SubobjectKey, uint32_t, SubobjectHash> vtt_subobjects_;
	PointerMap vtt_walked_;
	// Construction tables (unit-scoped): each base under construction (the
	// complete class, the base, its offset) -> the index in
	// construction_symbols_ of its group's segment 0, a symbol per segment
	// (kNoId until made).
	struct ConstructionKey
	{
		const sema::ClassEntity* complete;
		const sema::ClassEntity* cls;
		uint64_t offset;
		bool operator==(const ConstructionKey& other) const
		{
			return complete == other.complete && cls == other.cls && offset == other.offset;
		}
	};
	struct ConstructionHash
	{
		size_t operator()(const ConstructionKey& key) const;
	};
	FlatMap<ConstructionKey, uint32_t, ConstructionHash> construction_tables_;
	std::vector<lowir::SymbolId> construction_symbols_;
	// Host objects' construction groups: each segment's address point in
	// its group (beside construction_symbols_, which holds the group's
	// symbol for every segment); each context's segments' table owners
	// (context_owners_: the index of its first in context_owner_store_)
	// and their computation's scratch.
	std::vector<int64_t> construction_points_;
	FlatMap<ConstructionKey, uint32_t, ConstructionHash> context_owners_;
	std::vector<uint32_t> context_owner_store_;
	PointerMap owner_heads_;
	PointerMap owner_chain_;
	// Complete constructor and destructor entries defined, with the object
	// spelling of their base entry: an alias of the complete entry unless
	// the base entry is defined (program-wide, stated at finish()).
	struct EntryAlias
	{
		lowir::NameId object;
		lowir::SymbolId target;
	};
	std::vector<EntryAlias> entry_aliases_;
	// Symbols whose singleton role is secondary (an array allocation or
	// deallocation function, __builtin_abort): another symbol with the role
	// owns it (see resolve_singleton_roles). Indexed by SymbolId.
	std::vector<bool> secondary_roles_;
	// External symbols by ABI (object) name, across units; kLocalObject
	// marks a spelling an internal symbol claimed.
	static const lowir::SymbolId kLocalObject = lowir::kNoId - 1;
	std::vector<lowir::SymbolId> by_object_;
	// The next number to try for a taken spelling (see new_symbol), by the
	// spelling's name.
	std::vector<uint32_t> next_suffix_;
	// Declarations waiting for a definition that may still come, by symbol
	// (index + 1 into pending_, 0 for none).
	std::vector<uint32_t> pending_index_;
	std::vector<PendingDeclaration> pending_;
	// The ABI graph of the unit being lowered and its encoder; the nodes
	// of the unit's types and class names built so far (unit-scoped: type
	// or class -> node), so a type named again, or nested in a type of
	// shared components, is built once; the lists of the node constructions
	// under way, a stack (each takes the entries from its own base up).
	abi_mangle::AbiGraph abi_graph_;
	std::unique_ptr<abi_mangle::AbiEncoder> encoder_;
	PointerMap abi_types_;
	PointerMap class_names_;
	// Placeholders' expression nodes (unit-scoped: placeholder -> node), and
	// function template parameters' <template-param-decl> nodes (parameter
	// -> node, see declared_argument).
	PointerMap placeholder_nodes_;
	PointerMap parameter_declarations_;
	// Template arguments' heads as declarations (head -> index; see
	// argument_head_declaration): the pack bit and level each was encoded at.
	struct HeadDeclaration
	{
		uint32_t key;
		abi_mangle::AbiNode node;
	};
	PointerMap head_declaration_index_;
	std::vector<HeadDeclaration> head_declarations_;
	std::vector<abi_mangle::AbiNode> abi_scratch_;
	// with_implicit_tags' collections (a unit with GNU ABI tags only), and
	// the functions whose implicit tags are being derived with their
	// encodings without them (see abi_scope).
	std::vector<abi_mangle::AbiNode> abi_type_tags_;
	std::vector<abi_mangle::AbiNode> abi_used_tags_;
	struct EncodingUnderway
	{
		const sema::FunctionEntity* function;
		abi_mangle::AbiNode underived;
	};
	std::vector<EncodingUnderway> encodings_underway_;
	// While a function template specialization's signature is encoded:
	// the depth of its template's parameters counted from the outermost
	// template, which a pack expansion's pattern naming a bound pack gives
	// them (`abi_bound_pattern_`, set inside one; see abi_type_node).
	uint32_t abi_own_level_ = 0;
	bool abi_bound_pattern_ = false;
	// A node built since it was cleared encodes such a pattern.
	bool abi_level_dependent_ = false;
	// Localities (unit-scoped: type or entity -> Locality), and unnamed
	// member types' positions in their classes (entity -> ordinal).
	PointerMap localities_;
	PointerMap unnamed_ordinals_;
	// Object mode: declarations the object references weakly (another
	// unit's thread-local entries; see finish).
	std::vector<lowir::SymbolId> weak_references_;
	// `$_<n>` types' numbers (type -> n), their functions' next numbers
	// (function -> n) and the unit's count of them (see anonymous_ordinal).
	PointerMap anonymous_ordinals_;
	PointerMap anonymous_counts_;
	uint32_t anonymous_total_ = 0;
	// Local entities' discriminators numbered as mangled (entity -> count
	// of earlier ones) and their functions' counts by name (see
	// local_discriminator).
	struct LocalName
	{
		const sema::FunctionEntity* function;
		const Identifier* name;
		bool operator==(const LocalName& other) const { return function == other.function && name == other.name; }
	};
	struct LocalNameHash
	{
		size_t operator()(const LocalName& key) const
		{
			return (reinterpret_cast<uintptr_t>(key.function) * 0x9E3779B97F4A7C15ull) ^
				reinterpret_cast<uintptr_t>(key.name);
		}
	};
	PointerMap local_discriminators_;
	FlatMap<LocalName, uint32_t, LocalNameHash> local_name_counts_;
	// The unit's local classes by outermost function, in definition order
	// (function -> first index into SemanticGraph::local_classes, index ->
	// next; see name_local_classes), linked on first need.
	PointerMap local_class_heads_;
	std::vector<uint32_t> local_class_next_;
	bool local_classes_linked_ = false;
	// Local types shared across units (entity -> 0 or 1; see
	// shared_local): a nested closure's answer reads its enclosing one's.
	PointerMap shared_locals_;
	// The compact presentation identities of specializations labeled
	// inside other labels (unit-scoped: class -> ordinal).
	PointerMap label_identities_;
	// Each class scope's qualified spelling once made (unit-scoped: scope
	// -> index into class_spellings_; see qualified_spelling).
	PointerMap class_spelling_index_;
	std::vector<std::string> class_spellings_;
	// The dynamic-initialization function, created on first need, and the
	// objects the finalization function destroys (built at finish()).
	std::unique_ptr<FunctionLowering> init_;
	lowir::SymbolId init_symbol_;
	std::vector<Finalizer> finalizers_;
	uint32_t string_literals_;
	uint32_t static_temporaries_ = 0;
	TemporaryNames temporary_names_;
	uint32_t initializer_list_backings_ = 0;
	// begin_owned_temporaries' variable and its temporaries' (temporary,
	// registration guard or kNoId) pairs.
	const sema::VariableEntity* temporary_owner_ = nullptr;
	std::vector<std::pair<lowir::SymbolId, lowir::SymbolId>> owned_temporaries_;
	// The globals of temporaries constant references extend, by their
	// published values (unit-scoped: constant_temporary).
	PointerMap constant_temporaries_;
	// Function-local static objects' globals and guards (unit-scoped:
	// variable -> index), and atexit's declaration (program-wide).
	struct LocalStatic
	{
		lowir::SymbolId symbol;
		lowir::SymbolId guard;
	};
	PointerMap local_static_index_;
	std::vector<LocalStatic> local_statics_;
	lowir::SymbolId atexit_ = lowir::kNoId;
	lowir::SymbolId thread_atexit_ = lowir::kNoId;
	lowir::SymbolId fegetround_ = lowir::kNoId;
	lowir::SymbolId trap_ = lowir::kNoId;
	lowir::SymbolId memset_ = lowir::kNoId;
	std::vector<lowir::SymbolId> soft_float_helpers_;
	// complex_helper's symbols: products, then quotients, of float, double,
	// long double, binary16 and binary128 parts.
	std::vector<lowir::SymbolId> complex_helpers_;
	lowir::SymbolId dso_handle_ = lowir::kNoId;
	lowir::SymbolId guard_functions_[3] = { lowir::kNoId, lowir::kNoId, lowir::kNoId };
	// Object mode: the unit's thread-local initialization function, its
	// per-thread guard and the ordered variables' initializations it runs,
	// in definition order (see unit_thread_local_init).
	lowir::SymbolId thread_local_init_ = lowir::kNoId;
	lowir::SymbolId thread_local_guard_ = lowir::kNoId;
	std::vector<lowir::SymbolId> thread_local_inits_;
	// The constexpr automatic arrays' constant images (see constexpr_image),
	// one global per distinct image, keyed by its data items' bytes.
	std::unordered_map<std::string, lowir::SymbolId> constexpr_images_;
	// The unit's string literal objects by value: an open-addressed table
	// (a power of two long) of each object's hash of its element kind and
	// code units and its symbol + 1 (0: empty), compared with the object's
	// data items; and the code units of the literal being lowered.
	struct StringObject
	{
		uint32_t hash;
		uint32_t symbol;
	};
	std::vector<StringObject> string_objects_;
	uint32_t string_object_count_ = 0;
	std::vector<uint64_t> string_units_;
	// predefined_variable's arrays, by variable.
	PointerMap predefined_symbols_;
	// Inside an unbraced linkage-specification (7.5p7).
	bool linkage_extern_;
	// Where the initializer being evaluated statically was analyzed, and
	// the reference variables whose static targets are known (unit-scoped:
	// index into reference_targets_).
	sema::Scope* static_scope_;
	PointerMap reference_index_;
	std::vector<StaticValue> reference_targets_;
	lowir::NameId entry_name_;
	lowir::NameId this_name_;
	lowir::NameId ret_name_;
	lowir::NameId other_name_;
	lowir::NameId unnamed_slot_name_;
	lowir::NameId range_for_names_[4];
	// A base-object entry's VTT parameter (vtt_name).
	lowir::NameId vtt_name_ = 0;
	std::vector<lowir::NameId> argument_names_;
	// Per symbol, its object spelling (0 for none), computed once when the
	// symbol is created.
	std::vector<lowir::NameId> symbol_objects_;
	// Overload ordinals of the functions whose (scope, name) sets were
	// numbered (unit-scoped).
	PointerMap overload_ordinals_;
	// Indirect call boundaries by canonical function type (unit-scoped),
	// those of callees declared non-throwing apart.
	PointerMap call_signatures_;
	PointerMap nothrow_call_signatures_;
	// Slot names in use by the bodies being lowered: a mark per NameId set
	// to the claiming body's slot scope, so no body clears or sizes a table
	// of its own (the bodies that interleave with others claim no names).
	std::vector<uint32_t> slot_name_marks_;
	uint32_t slot_epoch_;
	lowir::DefinitionSink* stream_;
	bool debug_locations_ = false;
	bool debug_types_ = false;
	// The unit's source positions when it takes debug locations, and the
	// last file name looked up.
	const SourcePositions* positions_ = nullptr;
	std::string debug_file_;
	lowir::NameId debug_file_name_ = 0;
	bool object_mode_ = false;
	bool host_objects_ = false;
	std::uintptr_t stack_base_;
	std::uintptr_t stack_budget_;
};

} // namespace lower
