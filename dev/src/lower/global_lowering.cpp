#include "lower/program_lowering.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <string>

#include "lex/identifier_table.h"
#include "lex/interchange_float.h"
#include "lex/number_literal.h"
#include "lower/function_lowering.h"
#include "lowir/literals.h"
#include "sema/aggregates.h"
#include "sema/analyzer.h"
#include "sema/conversions.h"
#include "sema/integral_arithmetic.h"
#include "sema/string_literals.h"

// Namespace-scope variables (3.6.2): constant initialization states the
// object's bytes as LowIR data (scalars, address constants, zero runs);
// anything else is zero-initialized and completed by the dynamic
// initialization function, in declaration order.
namespace lower {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

// The single expression of `= e`, `(e)` or `{e}`; null otherwise.
// A complex variable's initializing expression: its single one, or its
// list of the two parts (see sema::PartList).
const syntax::Expr* ComplexInitializer(const syntax::Initializer* initializer);

const syntax::Expr* SingleInitializer(const syntax::Initializer* initializer)
{
	switch (initializer->kind)
	{
	case syntax::InitializerKind::Assign:
		if (initializer->expression->kind != syntax::ExprKind::BracedInit)
			return initializer->expression;
		// fall through
	case syntax::InitializerKind::Braced:
	{
		const syntax::BracedInitList* list = static_cast<const syntax::BracedInitList*>(initializer->expression);
		return list->elements.size == 1 && list->elements[0]->kind != syntax::ExprKind::BracedInit ? list->elements[0]
																								   : nullptr;
	}
	case syntax::InitializerKind::Parenthesized:
		return initializer->arguments.size == 1 ? initializer->arguments[0] : nullptr;
	default:
		return nullptr;
	}
}

const syntax::Expr* ComplexInitializer(const syntax::Initializer* initializer)
{
	if (initializer->kind != syntax::InitializerKind::Parenthesized &&
		initializer->expression->kind == syntax::ExprKind::BracedInit &&
		static_cast<const syntax::BracedInitList*>(initializer->expression)->elements.size == 2)
		return initializer->expression;
	return SingleInitializer(initializer);
}

// A vector variable's initializing list: `= {lanes}`, or the list of
// `V{lanes}` or GCC's `(V){lanes}` of the variable's vector type.
const syntax::BracedInitList* VectorLaneList(const sema::Type* type, const syntax::Expr* expr)
{
	if (expr->kind == syntax::ExprKind::BracedInit)
		return static_cast<const syntax::BracedInitList*>(expr);
	expr = WithoutParentheses(expr);
	if (expr->kind != syntax::ExprKind::FunctionalCast || expr->facts == nullptr ||
		expr->facts->type->unqualified != type->unqualified)
		return nullptr;
	return static_cast<const syntax::FunctionalCastExpr*>(expr)->braced;
}

bool IsEmptyBraces(const syntax::Initializer* initializer)
{
	if (initializer->kind != syntax::InitializerKind::Braced &&
		(initializer->kind != syntax::InitializerKind::Assign ||
			initializer->expression->kind != syntax::ExprKind::BracedInit))
		return false;
	return static_cast<const syntax::BracedInitList*>(initializer->expression)->elements.empty();
}

bool IsIntegralLike(const sema::Type* type)
{
	return sema::IsIntegralType(type) || type->unqualified->kind == sema::TypeKind::Enum;
}

// An operand of integral (or enumeration) type.
bool IntegralOperand(const syntax::Expr& expr)
{
	const sema::ExprFacts* facts = WithoutParentheses(&expr)->facts;
	return facts != nullptr && facts->type != nullptr && IsIntegralLike(facts->type);
}

bool IsPointerLike(const sema::Type* type)
{
	type = type->unqualified;
	return type->kind == sema::TypeKind::Pointer ||
		(type->kind == sema::TypeKind::Fundamental && type->fundamental == FundamentalType::NullptrT);
}

// 5.6, 5.7 as the executed operation rounds: f32 once in float; f64 in
// x87 extended precision, then rounded to double, as the native back end
// evaluates it ([expr]p11 allows the extra precision), which can differ
// in the last place from one rounding.
template<typename T>
long double FloatingArithmetic(TokenKind op, long double left, long double right)
{
	const T a = static_cast<T>(left), b = static_cast<T>(right);
	T result;
	switch (op)
	{
	case TokenKind::OP_PLUS:
		result = a + b;
		break;
	case TokenKind::OP_MINUS:
		result = a - b;
		break;
	case TokenKind::OP_STAR:
		result = a * b;
		break;
	default:
		result = a / b;
		break;
	}
	return result;
}

lowir::Constant IntegerConstant(uint64_t bits)
{
	lowir::Constant constant;
	constant.kind = lowir::ValueKind::Integer;
	constant.float_kind = lowir::TypeKind::Void;
	constant.bits = lowir::integer_literal(static_cast<int64_t>(bits));
	return constant;
}

} // namespace

ProgramLowering::StaticOutcome ProgramLowering::static_global(lowir::SymbolId symbol,
	const sema::VariableEntity* variable, const syntax::Initializer* initializer, uint32_t location,
	lowir::SymbolMetadata metadata)
{
	// The object's global with its static initialization (3.6.2p2): its
	// constant, else zero bytes a dynamic initialization completes.
	const sema::Type* type = variable->type;
	lowir::Global global = lowir::Global();
	bool dynamic = false;
	bool table_only = false;
	// Constant-initialized from its evaluated value (3.6.2p2).
	bool evaluated = false;
	// The analysis's facts of the initialization: the value its constant
	// evaluation recorded (a constexpr, const integral or reference
	// variable's), or that it found none.
	const sema::ConstValue* recorded = variable->is_thread_local ? nullptr : analyzer_->evaluated_value(variable);
	const bool nonconstant = variable->constant == sema::ConstantState::None;
	StaticData parts;
	if (type->is_reference())
	{
		global.typed = true;
		global.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		// Bound by the initialization function unless constant-initialized.
		global.init = lowir::GlobalInit::Zero;
		dynamic = !define_reference(global, variable, initializer, location);
		const sema::ConstValue* value = dynamic ? initialization_value(variable, initializer) : nullptr;
		if (value != nullptr && value->kind == sema::ValueKind::Pointer &&
			constant_global(type, *value, global, location) && global.init == lowir::GlobalInit::Address)
		{
			dynamic = false;
			StaticValue target = StaticValue();
			target.kind = StaticValue::Kind::Address;
			target.symbol = global.address;
			target.offset = global.addend;
			reference_index_.set(variable, static_cast<uint32_t>(reference_targets_.size()));
			reference_targets_.push_back(target);
		}
		if (!dynamic && initializer != nullptr && read_through_storage(variable))
			demand_initializer_calls(SingleInitializer(initializer), 0);
	}
	else if (type->kind == sema::TypeKind::Array || type->unqualified->kind == sema::TypeKind::Class)
	{
		// A class object a constructor initializes (or default-initializes,
		// not trivially) is zero-initialized, then constructed by the
		// initialization function (3.6.2p2).
		StaticData data;
		const sema::FunctionEntity* constructor = variable->constructor;
		const bool constructed = constructor != nullptr && (!constructor->trivial || initializer != nullptr);
		// A list giving GNU's flexible array member elements: the object's
		// data are its list's, the elements after its other members (see
		// static_aggregate); a dynamic initialization would write them past
		// the object (Clang refuses that initializer too).
		const syntax::Expr* list = initializer != nullptr ? initializer->expression : nullptr;
		const bool flexible = list != nullptr && list->kind == syntax::ExprKind::BracedInit && list->facts != nullptr &&
			(list->facts->flags & sema::kFlexibleElements) != 0;
		evaluated = !flexible && recorded != nullptr && constant_data(type, *recorded, data, location);
		if (!evaluated)
			data.items.clear();
		// A constant initialization first (3.6.2p2), then a construction
		// that only stores table pointers and constants.
		const bool construction = !evaluated && constructed && !variable->is_thread_local && !flexible;
		if (construction && !nonconstant)
		{
			const sema::ConstValue* value = initialization_value(variable, initializer);
			evaluated = value != nullptr && constant_data(type, *value, data, location);
			if (!evaluated)
				data.items.clear();
		}
		table_only = construction && !evaluated && static_construction(type, constructor, initializer, data);
		if (flexible && !static_object(type, initializer, data))
			throw LoweringError(location, "a flexible array member's elements in a dynamic initialization");
		if (!evaluated && !table_only && !flexible &&
			(initializer == nullptr || constructed || !static_object(type, initializer, data)))
		{
			data.items.clear();
			const sema::ConstValue* value = (initializer != nullptr || constructed) && !nonconstant && !construction
				? initialization_value(variable, initializer)
				: nullptr;
			evaluated = value != nullptr && !variable->is_thread_local && constant_data(type, *value, data, location);
			if (!evaluated)
			{
				dynamic = initializer != nullptr || constructed;
				data.items.clear();
				append_zero(size_of(type, location), data);
			}
		}
		// Null pointers to data members are -1 (8.5p5, Itanium 2.3).
		null_member_pointers(type, data);
		global.typed = false;
		global.init = lowir::GlobalInit::Data;
		global.first_item = static_cast<uint32_t>(program_.data_items.size());
		global.item_count = static_cast<uint32_t>(data.items.size());
		program_.data_items.insert(program_.data_items.end(), data.items.begin(), data.items.end());
	}
	else if (type->unqualified->kind == sema::TypeKind::MemberPointer && initializer != nullptr)
	{
		// A member pointer's bits when its initializer is a constant
		// expression (3.6.2p2), else the initialization function's.
		StaticData data;
		const sema::ConstValue* value = recorded != nullptr ? recorded
			: nonconstant || variable->is_thread_local      ? nullptr
															: initialization_value(variable, initializer);
		if (value != nullptr && constant_data(type, *value, data, location))
		{
			global.init = lowir::GlobalInit::Data;
			global.first_item = static_cast<uint32_t>(program_.data_items.size());
			global.item_count = static_cast<uint32_t>(data.items.size());
			program_.data_items.insert(program_.data_items.end(), data.items.begin(), data.items.end());
		}
		else
		{
			global.typed = true;
			global.type = storage_type(type);
			global.init = lowir::GlobalInit::Zero;
			dynamic = true;
		}
		// Zero-initialized before its dynamic initialization: a null
		// pointer to data member is -1.
		if (global.init == lowir::GlobalInit::Zero && holds_data_member_pointer(type))
		{
			global.init = lowir::GlobalInit::Constant;
			global.constant = IntegerConstant(static_cast<uint64_t>(null_data_member()));
		}
	}
	else if (type->unqualified->kind == sema::TypeKind::Complex && initializer != nullptr &&
		((ComplexInitializer(initializer) != nullptr &&
			 static_complex(type, *ComplexInitializer(initializer), parts)) ||
			evaluated_data(variable, initializer, recorded, nonconstant, parts, location)))
	{
		// A complex value of constant parts, or of a constant expression
		// (3.6.2p2).
		global.init = lowir::GlobalInit::Data;
		global.first_item = static_cast<uint32_t>(program_.data_items.size());
		global.item_count = static_cast<uint32_t>(parts.items.size());
		program_.data_items.insert(program_.data_items.end(), parts.items.begin(), parts.items.end());
	}
	else if (type->unqualified->kind == sema::TypeKind::Vector && initializer != nullptr &&
		((initializer->kind != syntax::InitializerKind::Parenthesized &&
			 VectorLaneList(type, initializer->expression) != nullptr) ||
			evaluated_data(variable, initializer, recorded, nonconstant, parts, location)))
	{
		// A vector's lanes from its list, or of a constant expression
		// (3.6.2p2), else zero bytes the initialization function completes.
		StaticData data;
		const syntax::BracedInitList* lanes = initializer->kind != syntax::InitializerKind::Parenthesized
			? VectorLaneList(type, initializer->expression)
			: nullptr;
		bool placed = lanes == nullptr;
		if (lanes == nullptr)
			data.items.swap(parts.items);
		else
			placed = static_vector(type, *lanes, data) ||
				evaluated_data(variable, initializer, recorded, nonconstant, data, location);
		if (placed)
		{
			global.init = lowir::GlobalInit::Data;
			global.first_item = static_cast<uint32_t>(program_.data_items.size());
			global.item_count = static_cast<uint32_t>(data.items.size());
			program_.data_items.insert(program_.data_items.end(), data.items.begin(), data.items.end());
		}
		else
		{
			global.typed = true;
			global.type = storage_type(type);
			global.init = lowir::GlobalInit::Zero;
			dynamic = true;
		}
	}
	else if (holds_data_member_pointer(type) && (initializer == nullptr || IsEmptyBraces(initializer)))
	{
		// A zero-initialized pointer to data member: its null value -1
		// (8.5p5, Itanium 2.3).
		global.typed = true;
		global.type = storage_type(type);
		global.init = lowir::GlobalInit::Constant;
		global.constant = IntegerConstant(static_cast<uint64_t>(null_data_member()));
	}
	else
	{
		global.typed = true;
		global.type = storage_type(type);
		global.init = lowir::GlobalInit::Zero;
		StaticValue value = StaticValue();
		const syntax::Expr* single = initializer != nullptr ? SingleInitializer(initializer) : nullptr;
		if (initializer != nullptr && IsEmptyBraces(initializer))
		{
			// 8.5.4p3: `{}` value-initializes: zero.
			global.init = lowir::GlobalInit::Constant;
			global.constant = IntegerConstant(0);
		}
		else if (initializer == nullptr && variable->constant == sema::ConstantState::Value)
		{
			// A static data member's in-class initializer (9.4.2p3).
			global.init = lowir::GlobalInit::Constant;
			global.constant = IntegerConstant(variable->value.bits);
			global.constant.bits = IntegralLiteral(type, variable->value.bits);
		}
		else if (recorded != nullptr && constant_global(type, *recorded, global, location))
			;
		else if (single != nullptr && static_converted(*single, type->unqualified, value, nonconstant))
		{
			lowir::DataItem item;
			static_item(type, value, item);
			if (item.kind == lowir::DataItemKind::Address)
			{
				global.init = lowir::GlobalInit::Address;
				global.address = item.address;
				global.addend = item.amount;
			}
			else if (item.kind == lowir::DataItemKind::Scalar)
			{
				global.init = lowir::GlobalInit::Constant;
				global.constant = item.constant;
				// A binary16 or binary128 value: its encoding's integer type.
				if (value.kind == StaticValue::Kind::Soft)
					global.type = item.type;
			}
			else if (value.kind == StaticValue::Kind::Null)
			{
				// An initialized null pointer is the constant 0 (the course's
				// form; `zero` is the uninitialized object's).
				global.init = lowir::GlobalInit::Constant;
				global.constant = IntegerConstant(0);
			}
		}
		else if (initializer != nullptr)
		{
			// Probed unless known not constant: by the analysis, or by the
			// folding's evaluation of an integral initializer (the probe's).
			const bool proved = nonconstant || (single != nullptr && IntegralOperand(*single));
			const sema::ConstValue* value = variable->is_thread_local || proved
				? nullptr
				: initialization_value(variable, initializer);
			dynamic = value == nullptr || !constant_global(type, *value, global, location);
		}
		// The README's readonly storage: a const scalar that is neither
		// volatile nor thread-local, its bytes fixed before any code runs.
		if (!dynamic && (type->cv & sema::kConst) != 0 && (type->cv & sema::kVolatile) == 0 &&
			!variable->is_thread_local)
			metadata.storage = ir_model::GSM_READONLY;
	}
	// The object's alignment: its type's, an alignment-specifier's (7.6.2),
	// and in a host object at least 16 for an array of 16 bytes or more (the
	// psABI's 3.1.2 rule, which host compilers' code assumes of another
	// unit's arrays).
	uint64_t alignment = type->is_reference() ? 8 : alignment_of(type, location);
	if (variable->alignment_request > alignment)
		alignment = variable->alignment_request;
	const uint64_t size = type->is_reference() ? 8 : size_of(type, location);
	if (object_mode_ && type->kind == sema::TypeKind::Array && alignment < 16 && size >= 16)
		alignment = 16;
	state_storage_type(global, size, alignment);
	program_.declare_global(symbol, lowir::SymbolKind::GlobalDefinition, global);
	program_.symbols[symbol].metadata = metadata;
	return StaticOutcome{ dynamic, table_only, evaluated };
}

void ProgramLowering::state_storage_type(lowir::Global& global, uint64_t size, uint64_t alignment)
{
	// lowir.md "Structured Global Data Initializers": an over-aligned scalar
	// is a structured global of its one item, and a structured global states
	// its storage type exactly when its alignment exceeds the largest of its
	// items' other than `zero` (1 when all are), its span the object's size
	// rounded up to the alignment. A host object keeps its size unless it is
	// smaller than the alignment (a span is at least its alignment), and
	// states one only beyond the 16 its back end gives data of `zero` items
	// alone.
	if (global.init != lowir::GlobalInit::Data)
	{
		if (!global.typed || global.type.is_object() || alignment <= lowir::data_alignment(global.type))
			return;
		lowir::DataItem item = lowir::DataItem();
		item.type = global.type;
		item.address = lowir::kNoId;
		item.kind = global.init == lowir::GlobalInit::Address ? lowir::DataItemKind::Address
			: global.init == lowir::GlobalInit::Zero          ? lowir::DataItemKind::Zero
															  : lowir::DataItemKind::Scalar;
		item.constant = global.constant;
		item.address = global.init == lowir::GlobalInit::Address ? global.address : lowir::kNoId;
		item.amount = global.init == lowir::GlobalInit::Address ? global.addend
			: global.init == lowir::GlobalInit::Zero            ? static_cast<int64_t>(size)
																: 0;
		global.init = lowir::GlobalInit::Data;
		global.typed = false;
		global.first_item = static_cast<uint32_t>(program_.data_items.size());
		global.item_count = 1;
		program_.data_items.push_back(item);
	}
	uint64_t implied = 0;
	for (uint32_t i = 0; i < global.item_count; ++i)
	{
		const lowir::DataItem& item = program_.data_items[global.first_item + i];
		if (item.kind == lowir::DataItemKind::Scalar || item.kind == lowir::DataItemKind::Address)
			implied = std::max<uint64_t>(implied,
				lowir::data_alignment(
					item.kind == lowir::DataItemKind::Address ? lowir::Type::scalar(lowir::TypeKind::Ptr) : item.type));
	}
	if (alignment <= (implied != 0 ? implied : object_mode_ ? 16 : 1))
		return;
	unsigned log2 = 0;
	while ((uint64_t(1) << log2) < alignment)
		++log2;
	global.typed = true;
	global.type = lowir::Type::object(
		object_mode_ ? std::max(size, alignment) : (size + alignment - 1) / alignment * alignment, log2);
}

void ProgramLowering::define_global(lowir::SymbolId symbol, const sema::VariableEntity* variable,
	const syntax::Initializer* initializer, uint32_t location)
{
	const sema::Type* type = variable->type;
	static_scope_ = variable->templ != nullptr && variable->templ->scope != nullptr ? variable->templ->scope
																					: variable->owner;
	// 9.4.2p3: a static data member's definition takes its in-class
	// initializer.
	if (initializer == nullptr && !variable->member && variable->member_initializer != nullptr)
		initializer = variable->member_initializer;
	const TemporaryNames names = begin_temporary_names(variable, symbol);
	const StaticOutcome outcome = static_global(
		symbol, variable, initializer, location, variable_metadata(variable, symbol));
	end_temporary_names(names);
	const bool dynamic = outcome.dynamic;
	const bool table_only = outcome.table_only;
	const bool evaluated = outcome.evaluated;
	lowir::SymbolId guard = lowir::kNoId;
	lowir::SymbolId constructed = lowir::kNoId;
	// Object mode: its destruction registered with atexit where its
	// initialization completes (see set_object_mode); a thread-local
	// object's with the thread's exit list, by its per-thread
	// initialization (thread_local_initialization).
	const bool registered = object_mode_ && FunctionLowering::needs_destruction(type);
	// Object mode: a thread-local object whose destruction is registered
	// is initialized per thread, as a dynamically initialized one is: each
	// thread's object is destroyed at that thread's exit (3.6.3p1), and
	// only once some use initialized it (3.6.2p4).
	const bool per_thread = variable->is_thread_local && (dynamic || registered);
	if (per_thread)
		guard = thread_local_initialization(symbol, variable, initializer, location, dynamic);
	else if (variable->is_thread_local && tls_wrappers_.find(variable) != PointerMap::kMissing)
	{
		// Used through its wrapper before this definition, which initializes
		// it statically: the wrapper only gives the thread's address.
		const lowir::SymbolId wrapper = tls_wrappers_.find(variable);
		{
			FunctionLowering body(*this, wrapper, nullptr);
			body.lower_thread_local_wrapper(lowir::kNoId, symbol);
		}
		release_definition(wrapper);
		++counters_.functions;
	}
	else if (dynamic && program_.symbols[symbol].metadata.binding == ir_model::SBM_WEAK)
	{
		// 3.6.2p2: a class template specialization's static data member has
		// unordered initialization; every unit instantiating it defines it
		// and runs the initialization, which only the first performs. That
		// unit's finalizer destroys the object (in object mode, that unit
		// registers its destruction).
		if (FunctionLowering::needs_destruction(type) && !registered)
		{
			constructed = new_symbol(std::string(program_.spelling(program_.symbols[symbol].name)) + "_constructed");
			lowir::Global flag = lowir::Global();
			flag.typed = true;
			flag.type = lowir::Type::scalar(lowir::TypeKind::I8);
			flag.init = lowir::GlobalInit::Zero;
			program_.declare_global(constructed, lowir::SymbolKind::GlobalDefinition, flag);
			program_.symbols[constructed].metadata.binding = ir_model::SBM_INTERNAL;
			++counters_.globals;
		}
		// The initializer function, then the guard: both may declare
		// symbols, and a member call's object and arguments are evaluated
		// in an unspecified order.
		FunctionLowering& function = initializer_function();
		function.lower_unordered_initializer(
			symbol, unordered_guard(symbol, variable), constructed, variable, initializer, location, true, registered);
	}
	else if (dynamic)
	{
		if (FunctionLowering::object_class(type) != nullptr && !type->is_reference())
			initializer_function().construct_global(symbol, variable, initializer, location);
		else
			initializer_function().lower_dynamic_initializer(symbol, variable, initializer, location);
		if (registered)
			initializer_function().register_destruction(symbol, type, location);
	}
	else if (evaluated)
	{
		// The constructors of a constant-initialized class object are still
		// emitted (the course's form).
		if (variable->constructor != nullptr && !variable->constructor->trivial)
			demand(variable->constructor);
		demand_constructions(initializer);
	}
	else if (initializer == nullptr && variable->constructor != nullptr && !type->is_reference() && !table_only)
	{
		// Default-initialization by a trivial constructor: the course keeps
		// the (empty) initialization step for a class with members; a class
		// of internal linkage (3.5p4) defines the constructor's base entry
		// instead.
		const sema::ClassEntity* cls = FunctionLowering::object_class(type);
		const sema::Scope* owner = cls->owner;
		while (owner != nullptr && owner->kind == sema::ScopeKind::Class)
			owner = owner->entity->owner;
		const bool internal = owner != nullptr && owner->kind == sema::ScopeKind::Namespace &&
			owner->entity != nullptr && static_cast<const sema::NamespaceEntity*>(owner->entity)->internal;
		if (internal)
			demand_trivial_base(variable->constructor);
		else if (cls->members != nullptr)
			initializer_function();
	}
	// A constant initialization's evaluation runs the constructors its
	// initializer names without calling them: emitted with the variable
	// (the course's form; a dynamic one's calls demand them).
	if (!dynamic && !table_only)
	{
		for (uint32_t index = graph_->initializer_constructor_heads.find(initializer); index != PointerMap::kMissing;
			index = graph_->initializer_constructors[index].next)
			demand(graph_->initializer_constructors[index].constructor);
	}
	// A statically initialized object's destruction is registered at its
	// position in the initialization order (as its dynamic initialization
	// would be); a class template specialization's member's (a weak
	// definition in every unit instantiating it) by the one unit that sets
	// the guard they share (Itanium 2.8).
	// Object mode: an external thread-local variable initialized statically
	// still defines its initialization entry (empty), which the wrappers of
	// the units using it call.
	if (object_mode_ && variable->is_thread_local && !per_thread && !unit_local(variable))
	{
		const lowir::SymbolId entry = thread_local_init_entry(symbol, variable, true);
		{
			FunctionLowering body(*this, entry, nullptr);
			body.finish_initializer();
		}
		release_definition(entry);
		++counters_.functions;
		// It defines the wrapper (weak) the units using it define, as the
		// reference does; the unit's own uses address the object directly.
		if (tls_wrappers_.find(variable) == PointerMap::kMissing)
		{
			const lowir::SymbolId wrapper = thread_local_wrapper_symbol(symbol, variable, false);
			{
				FunctionLowering body(*this, wrapper, nullptr);
				body.lower_thread_local_wrapper(lowir::kNoId, symbol);
			}
			release_definition(wrapper);
			++counters_.functions;
		}
	}
	if (registered && !dynamic && !variable->is_thread_local &&
		program_.symbols[symbol].metadata.binding == ir_model::SBM_WEAK)
	{
		// (The initializer function, then the guard: see above.)
		FunctionLowering& function = initializer_function();
		function.lower_unordered_initializer(
			symbol, unordered_guard(symbol, variable), lowir::kNoId, variable, initializer, location, false, true);
	}
	else if (registered && !dynamic && !per_thread)
	{
		// At its position in the initialization order (a thread-local
		// object's by its per-thread initialization instead).
		initializer_function().register_destruction(symbol, type, location);
	}
	else if (FunctionLowering::needs_destruction(type) && !registered)
	{
		finalize(symbol, type, location, guard);
		finalizers_.back().constructed = constructed;
	}
	static_scope_ = nullptr;
}

lowir::SymbolId ProgramLowering::unordered_guard(lowir::SymbolId global, const sema::VariableEntity* variable)
{
	// Weak like its member, its object spelling the guard variable's
	// special name of the variable's (Itanium 5.1.4: _ZGV <name>, a
	// global-namespace variable's included).
	const lowir::Symbol& member = program_.symbols[global];
	const std::string spelling = special_object(abi_mangle::ABI_TARGET_GUARD_VARIABLE, variable_name(variable));
	const lowir::SymbolId guard = new_symbol("__cppgm_guard__" + std::string(program_.spelling(member.name)));
	lowir::Global flag = lowir::Global();
	flag.typed = true;
	flag.type = lowir::Type::scalar(lowir::TypeKind::I64);
	flag.init = lowir::GlobalInit::Zero;
	program_.declare_global(guard, lowir::SymbolKind::GlobalDefinition, flag);
	program_.symbols[guard].metadata.binding = ir_model::SBM_WEAK;
	program_.symbols[guard].metadata.object = intern(spelling.data(), static_cast<uint32_t>(spelling.size()));
	++counters_.globals;
	return guard;
}

lowir::SymbolId ProgramLowering::thread_local_initialization(lowir::SymbolId global,
	const sema::VariableEntity* variable, const syntax::Initializer* initializer, uint32_t location, bool dynamic)
{
	// 3.6.2p2 per thread (3.7.2): each thread initializes its object once,
	// before its first use through the object's wrapper (`_ZTW`), guarded
	// by a thread-local flag; the synthesized symbols form their own
	// `__cppgm_tls_*` family, apart from source names.
	// Object mode (Itanium 3.7, as g++ and Clang): the unit's ordered
	// variables are initialized together by its thread-local
	// initialization function (unit_thread_local_init), in definition
	// order before the first odr-use of any (3.6.2p4), their destruction
	// registered for the thread there (a statically initialized object's
	// too); an external one's `_ZTH` (see thread_local_init_entry), which
	// other units' wrappers call, runs that function. A class template's
	// member (unordered: weak in every unit instantiating it) has its own
	// `_ZTH` initialization; it and a GNU weak variable have the guard the
	// units share (Itanium 2.8), so a thread initializes it once.
	const std::string base = program_.spelling(program_.symbols[global].name);
	const bool unordered = object_mode_ && variable->templ != nullptr && variable->templ->pattern != nullptr &&
		!unit_local(variable);
	lowir::SymbolId guard = lowir::kNoId;
	if (object_mode_ && program_.symbols[global].metadata.binding == ir_model::SBM_WEAK)
		guard = unordered_guard(global, variable);
	else if (!object_mode_)
	{
		guard = new_symbol("__cppgm_tls_guard__" + base);
		lowir::Global flag = lowir::Global();
		flag.typed = true;
		flag.type = lowir::Type::scalar(lowir::TypeKind::I64);
		flag.init = lowir::GlobalInit::Zero;
		program_.declare_global(guard, lowir::SymbolKind::GlobalDefinition, flag);
		program_.symbols[guard].metadata.binding = ir_model::SBM_INTERNAL;
		++counters_.globals;
	}
	if (guard != lowir::kNoId)
		program_.symbols[guard].metadata.storage = ir_model::GSM_THREAD_LOCAL;
	lowir::SymbolId init = lowir::kNoId;
	if (unordered)
		init = thread_local_init_entry(global, variable, true);
	else
	{
		init = new_symbol("__cppgm_tls_init__" + base);
		lowir::Signature signature = lowir::Signature();
		signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
		signature.result = lowir::Type::scalar(lowir::TypeKind::Void);
		program_.declare_function(init, lowir::SymbolKind::FunctionDefinition, program_.add_signature(signature));
		program_.symbols[init].metadata.binding = ir_model::SBM_INTERNAL;
	}
	{
		FunctionLowering body(*this, init, nullptr);
		body.lower_thread_local_initializer(global, guard, variable, initializer, location, dynamic);
	}
	release_definition(init);
	++counters_.functions;
	// What the wrappers call: the unit's initialization for an ordered
	// variable in object mode; a GNU weak one's `_ZTH`, which the link
	// resolves to the selected definition's unit (its initializer, once).
	lowir::SymbolId entry = init;
	if (object_mode_ && !unordered)
	{
		thread_local_inits_.push_back(init);
		entry = unit_thread_local_init();
		if (!unit_local(variable))
		{
			const lowir::SymbolId external = thread_local_init_entry(global, variable, true);
			{
				FunctionLowering body(*this, external, nullptr);
				body.lower_forwarder(external, entry);
			}
			release_definition(external);
			++counters_.functions;
			if (program_.symbols[global].metadata.binding == ir_model::SBM_WEAK)
				entry = external;
		}
	}
	const lowir::SymbolId wrapper = thread_local_wrapper_symbol(global, variable);
	{
		FunctionLowering body(*this, wrapper, nullptr);
		body.lower_thread_local_wrapper(entry, global);
	}
	release_definition(wrapper);
	++counters_.functions;
	return guard;
}

lowir::SymbolId ProgramLowering::unit_thread_local_init()
{
	// Object mode: the unit's thread-local initialization (g++'s and
	// Clang's `__tls_init`), internal, with its per-thread guard; declared
	// by the first ordered variable, defined at the unit's end
	// (define_unit_thread_local_init).
	if (thread_local_init_ != lowir::kNoId)
		return thread_local_init_;
	thread_local_init_ = new_symbol("__cppgm_tls_init");
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	signature.result = lowir::Type::scalar(lowir::TypeKind::Void);
	program_.declare_function(
		thread_local_init_, lowir::SymbolKind::FunctionDefinition, program_.add_signature(signature));
	program_.symbols[thread_local_init_].metadata.binding = ir_model::SBM_INTERNAL;
	thread_local_guard_ = new_symbol("__cppgm_tls_guard");
	lowir::Global flag = lowir::Global();
	flag.typed = true;
	flag.type = lowir::Type::scalar(lowir::TypeKind::I8);
	flag.init = lowir::GlobalInit::Zero;
	program_.declare_global(thread_local_guard_, lowir::SymbolKind::GlobalDefinition, flag);
	program_.symbols[thread_local_guard_].metadata.binding = ir_model::SBM_INTERNAL;
	program_.symbols[thread_local_guard_].metadata.storage = ir_model::GSM_THREAD_LOCAL;
	++counters_.globals;
	return thread_local_init_;
}

void ProgramLowering::define_unit_thread_local_init()
{
	if (thread_local_init_ == lowir::kNoId)
		return;
	{
		FunctionLowering body(*this, thread_local_init_, nullptr);
		body.lower_thread_local_inits(thread_local_guard_, thread_local_inits_);
	}
	release_definition(thread_local_init_);
	++counters_.functions;
	thread_local_init_ = lowir::kNoId;
	thread_local_guard_ = lowir::kNoId;
	thread_local_inits_.clear();
}

lowir::SymbolId ProgramLowering::thread_local_init_entry(
	lowir::SymbolId global, const sema::VariableEntity* variable, bool define)
{
	// Object mode: an external thread-local variable's initialization entry
	// (Itanium's `_ZTH` <name>), defined by the unit defining the variable
	// (empty for a static initialization without destruction), weak for a
	// weak variable (a class template's member or a GNU weak one, which
	// every unit defining it defines), so a reference from another unit's
	// wrapper always resolves.
	const std::string object = special_object(abi_mangle::ABI_TARGET_TLS_INIT, thread_local_name(variable));
	const lowir::SymbolId symbol = external_symbol(
		object, "__cppgm_tls_init__" + std::string(program_.spelling(program_.symbols[global].name)));
	if (program_.symbols[symbol].kind != lowir::SymbolKind::Unresolved)
		return symbol;
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	signature.result = lowir::Type::scalar(lowir::TypeKind::Void);
	program_.declare_function(symbol,
		define ? lowir::SymbolKind::FunctionDefinition : lowir::SymbolKind::FunctionDeclaration,
		program_.add_signature(signature));
	lowir::SymbolMetadata& metadata = program_.symbols[symbol].metadata;
	// Another unit's entry is a weak reference (see lower_thread_local_wrapper).
	metadata.binding = !define || program_.symbols[global].metadata.binding == ir_model::SBM_WEAK
		? ir_model::SBM_WEAK
		: ir_model::SBM_STRONG;
	// A GNU weak variable's entry is replaceable as the variable is: the
	// selected definition's unit's runs its initializer (never this unit's
	// copy inlined into its wrapper).
	if (define && (program_.symbols[global].metadata.flags & lowir::SF_REPLACEABLE) != 0)
		metadata.flags |= lowir::SF_REPLACEABLE;
	if (!define)
		weak_references_.push_back(symbol);
	metadata.object = symbol_objects_[symbol];
	return symbol;
}

lowir::SymbolId ProgramLowering::external_thread_local_wrapper(
	lowir::SymbolId global, const sema::VariableEntity* variable)
{
	// Object mode: a thread-local variable another unit defines is reached
	// through its wrapper, which runs that unit's initialization for the
	// thread first (through its entry, `_ZTH`), then gives the address.
	const lowir::SymbolId wrapper = thread_local_wrapper_symbol(global, variable);
	const lowir::SymbolId init = thread_local_init_entry(global, variable, false);
	{
		FunctionLowering body(*this, wrapper, nullptr);
		body.lower_thread_local_wrapper(init, global);
	}
	release_definition(wrapper);
	++counters_.functions;
	return wrapper;
}

lowir::SymbolId ProgramLowering::thread_local_wrapper_symbol(
	lowir::SymbolId global, const sema::VariableEntity* variable, bool used)
{
	// A thread-local variable's wrapper (`_ZTW`), declared by its
	// definition or by a use before it; the definition gives its body.
	// Uses call one `used` (recorded for the variable).
	const uint32_t found = tls_wrappers_.find(variable);
	if (found != PointerMap::kMissing)
		return found;
	const std::string base = program_.spelling(program_.symbols[global].name);
	const lowir::SymbolId wrapper = new_symbol("__cppgm_tls_wrapper__" + base);
	lowir::Signature returning = lowir::Signature();
	returning.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	returning.result = lowir::Type::scalar(lowir::TypeKind::Ptr);
	program_.declare_function(wrapper, lowir::SymbolKind::FunctionDefinition, program_.add_signature(returning));
	// The wrapper is its variable's unit's own when the variable is (an
	// internal symbol's object spelling is claimed once in the program).
	lowir::SymbolMetadata& metadata = program_.symbols[wrapper].metadata;
	const std::string object = tls_wrapper_object(variable);
	metadata.binding = unit_local(variable) ? ir_model::SBM_INTERNAL : ir_model::SBM_WEAK;
	metadata.object = metadata.binding == ir_model::SBM_INTERNAL
		? local_object(object, false)
		: intern(object.data(), static_cast<uint32_t>(object.size()));
	metadata.tls_for = global;
	if (used)
		tls_wrappers_.set(variable, wrapper);
	return wrapper;
}

void ProgramLowering::finalize(lowir::SymbolId symbol, const sema::Type* type, uint32_t location, lowir::SymbolId guard)
{
	// 3.6.3p1: its destructor runs at program end, in reverse order of
	// construction; a thread-local object initialized on first use only
	// once its guard shows it was.
	Finalizer finalizer = finalizer_for(symbol, type, location);
	finalizer.guard = guard;
	finalizers_.push_back(finalizer);
}

Finalizer ProgramLowering::finalizer_for(lowir::SymbolId symbol, const sema::Type* type, uint32_t location)
{
	uint64_t count = 1;
	const sema::Type* element = type;
	while (element->kind == sema::TypeKind::Array)
	{
		count *= element->bound;
		element = element->element;
	}
	const sema::FunctionEntity* destructor = sema::ClassOf(element)->destructor;
	demand(destructor);
	Finalizer finalizer = Finalizer();
	finalizer.global = symbol;
	finalizer.destructor = function_symbol(destructor);
	finalizer.array = type->kind == sema::TypeKind::Array;
	finalizer.count = count;
	finalizer.element_size = size_of(element, location);
	finalizer.guard = lowir::kNoId;
	finalizer.constructed = lowir::kNoId;
	return finalizer;
}

bool ProgramLowering::same_string_object(
	lowir::SymbolId symbol, lowir::Type unit, const std::vector<uint64_t>& units) const
{
	// Whether a string literal object holds these code units of this type.
	const lowir::Global& global = program_.globals[program_.symbols[symbol].detail];
	if (global.item_count != units.size())
		return false;
	for (uint32_t i = 0; i < global.item_count; ++i)
	{
		const lowir::DataItem& item = program_.data_items[global.first_item + i];
		if (item.type != unit || item.constant.bits.low != units[i])
			return false;
	}
	return true;
}

lowir::SymbolId ProgramLowering::static_temporary(const sema::Type* type)
{
	// Named after the variable whose initialization makes it (see
	// begin_temporary_names): an object's std::initializer_list array, a
	// reference's (or a reference member's) temporary.
	const sema::VariableEntity* owner = temporary_names_.variable;
	const std::string spelling = owner == nullptr ? "__ref_tmp__" + std::to_string(++static_temporaries_)
		: !owner->type->is_reference() && type->kind == sema::TypeKind::Array
		? "__cppgm_initlist_backing_" + std::to_string(++initializer_list_backings_)
		: "__cppgm_ref_temporary__" + std::string(program_.spelling(program_.symbols[temporary_names_.symbol].name)) +
			"_" + std::to_string(temporary_names_.sequence);
	const lowir::SymbolId temporary = new_symbol(spelling);
	lowir::Global storage = lowir::Global();
	if (type->unqualified->kind == sema::TypeKind::Class || type->kind == sema::TypeKind::Array)
	{
		// A class object's (or an array's, 12.2p1) zero bytes (as their
		// globals').
		StaticData data;
		append_zero(size_of(type, 0), data);
		storage.init = lowir::GlobalInit::Data;
		storage.first_item = static_cast<uint32_t>(program_.data_items.size());
		storage.item_count = static_cast<uint32_t>(data.items.size());
		program_.data_items.insert(program_.data_items.end(), data.items.begin(), data.items.end());
	}
	else
	{
		storage.typed = true;
		storage.type = storage_type(type);
		storage.init = lowir::GlobalInit::Zero;
	}
	program_.declare_global(temporary, lowir::SymbolKind::GlobalDefinition, storage);
	program_.symbols[temporary].metadata.binding = ir_model::SBM_INTERNAL;
	name_temporary(temporary);
	++counters_.globals;
	if (temporary_owner_ != nullptr)
		owned_temporaries_.push_back(std::make_pair(temporary, lowir::kNoId));
	return temporary;
}

namespace {

// Itanium 5.1.4.5's <seq-id> of a variable's n-th temporary (from 0): none
// for the first, then 0, 1, ... in base 36 (upper-case letters).
std::string TemporarySequence(size_t n)
{
	std::string sequence;
	if (n == 0)
		return sequence;
	for (size_t value = n - 1;; value /= 36)
	{
		sequence.insert(sequence.begin(), "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[value % 36]);
		if (value < 36)
			break;
	}
	return sequence;
}

} // namespace

void ProgramLowering::name_temporary(lowir::SymbolId temporary)
{
	const sema::VariableEntity* owner = temporary_names_.variable;
	if (owner == nullptr)
		return;
	const size_t index = temporary_names_.sequence++;
	// `_ZGR <name> [<seq-id>] _` (5.1.4.5), the special name's <name> as its
	// guard variable's (a function-local static's <local-name>), unit-local
	// as clang's are: every unit reaches its temporary through the shared
	// variable.
	const bool local = owner->is_static && owner->owner != nullptr &&
		(owner->owner->kind == sema::ScopeKind::Block || owner->owner->kind == sema::ScopeKind::Function);
	if (owner->name == nullptr && graph_->decomposition(owner) == nullptr)
		return;
	const std::string guard = special_object(
		abi_mangle::ABI_TARGET_GUARD_VARIABLE, local ? local_static_name(owner) : variable_name(owner));
	const std::string object = "_ZGR" + guard.substr(4) + TemporarySequence(index) + "_";
	program_.symbols[temporary].metadata.object = local_object(object, false);
}

void ProgramLowering::begin_owned_temporaries(const sema::VariableEntity* variable)
{
	// Only a class template specialization's static member (a reference,
	// or an object whose aggregate reference members bind temporaries) in a
	// host object.
	const bool owned = host_objects_ && variable->templ != nullptr && variable->templ->pattern != nullptr;
	temporary_owner_ = owned ? variable : nullptr;
	owned_temporaries_.clear();
}

lowir::SymbolId ProgramLowering::temporary_registration_guard(lowir::SymbolId temporary)
{
	for (std::pair<lowir::SymbolId, lowir::SymbolId>& owned : owned_temporaries_)
	{
		if (owned.first != temporary)
			continue;
		if (owned.second == lowir::kNoId)
		{
			owned.second = new_symbol(
				"__cppgm_guard__" + std::string(program_.spelling(program_.symbols[temporary].name)));
			lowir::Global flag = lowir::Global();
			flag.typed = true;
			flag.type = lowir::Type::scalar(lowir::TypeKind::I64);
			flag.init = lowir::GlobalInit::Zero;
			program_.declare_global(owned.second, lowir::SymbolKind::GlobalDefinition, flag);
			program_.symbols[owned.second].metadata.binding = ir_model::SBM_INTERNAL;
			++counters_.globals;
		}
		return owned.second;
	}
	return lowir::kNoId;
}

void ProgramLowering::end_owned_temporaries()
{
	// Each temporary whose destruction is registered (named unit-local at
	// its making, see name_temporary): its registration guarded by GCC's
	// shared guard variable of the temporary, _ZGV <name> [<seq-id>] _
	// (Itanium 5.1.4.5's numbering, in the order the initialization makes
	// them).
	const std::string named = owned_temporaries_.empty()
		? std::string()
		: special_object(abi_mangle::ABI_TARGET_GUARD_VARIABLE, variable_name(temporary_owner_));
	for (size_t i = 0; i < owned_temporaries_.size(); ++i)
	{
		if (owned_temporaries_[i].second == lowir::kNoId)
			continue;
		const std::string guard = named + TemporarySequence(i) + "_";
		lowir::SymbolMetadata& flag = program_.symbols[owned_temporaries_[i].second].metadata;
		flag.binding = ir_model::SBM_WEAK;
		flag.object = intern(guard.data(), static_cast<uint32_t>(guard.size()));
	}
	temporary_owner_ = nullptr;
	owned_temporaries_.clear();
}

lowir::SymbolId ProgramLowering::constexpr_image(
	const sema::VariableEntity* variable, const syntax::Initializer* initializer)
{
	// The course's form of a constexpr automatic array initialized by a
	// braced list: its constant image in a readonly internal global (one
	// per distinct image) that the object is copied from, from its
	// evaluated value (else its static form); kNoId when there is none.
	const sema::Type* type = variable->type;
	const sema::ConstValue* recorded = analyzer_->evaluated_value(variable);
	StaticData data;
	if (recorded == nullptr || !constant_data(type, *recorded, data, 0))
	{
		data.items.clear();
		if (!static_object(type, initializer, data))
			return lowir::kNoId;
	}
	std::string key;
	for (const lowir::DataItem& item : data.items)
	{
		const uint64_t words[] = { static_cast<uint64_t>(item.kind), item.type.kind_bits, item.type.align_log2,
			item.type.bytes, static_cast<uint64_t>(item.constant.kind), static_cast<uint64_t>(item.constant.float_kind),
			item.constant.bits.low, item.constant.bits.high, item.address, static_cast<uint64_t>(item.amount) };
		key.append(reinterpret_cast<const char*>(words), sizeof words);
	}
	const std::unordered_map<std::string, lowir::SymbolId>::const_iterator found = constexpr_images_.find(key);
	if (found != constexpr_images_.end())
		return found->second;
	const lowir::SymbolId image = new_symbol("__constexpr_template__" + std::to_string(constexpr_images_.size() + 1));
	lowir::Global storage = lowir::Global();
	storage.init = lowir::GlobalInit::Data;
	storage.first_item = static_cast<uint32_t>(program_.data_items.size());
	storage.item_count = static_cast<uint32_t>(data.items.size());
	program_.data_items.insert(program_.data_items.end(), data.items.begin(), data.items.end());
	program_.declare_global(image, lowir::SymbolKind::GlobalDefinition, storage);
	program_.symbols[image].metadata.binding = ir_model::SBM_INTERNAL;
	program_.symbols[image].metadata.storage = ir_model::GSM_READONLY;
	++counters_.globals;
	constexpr_images_.emplace(key, image);
	return image;
}

lowir::SymbolId ProgramLowering::static_temporary_constructed(
	lowir::SymbolId temporary, const sema::Type* type, uint32_t location)
{
	const lowir::SymbolId flag = constructed_flag(temporary);
	finalize(temporary, type, location, lowir::kNoId);
	finalizers_.back().constructed = flag;
	finalizers_.back().reference_temporary = true;
	return flag;
}

lowir::SymbolId ProgramLowering::constructed_flag(lowir::SymbolId temporary)
{
	const lowir::SymbolId flag = new_symbol(
		std::string(program_.spelling(program_.symbols[temporary].name)) + "_constructed");
	lowir::Global storage = lowir::Global();
	storage.typed = true;
	storage.type = lowir::Type::scalar(lowir::TypeKind::I8);
	storage.init = lowir::GlobalInit::Zero;
	program_.declare_global(flag, lowir::SymbolKind::GlobalDefinition, storage);
	program_.symbols[flag].metadata.binding = ir_model::SBM_INTERNAL;
	++counters_.globals;
	return flag;
}

bool ProgramLowering::define_reference(lowir::Global& global, const sema::VariableEntity* variable,
	const syntax::Initializer* initializer, uint32_t location)
{
	// 3.6.2p2: a reference bound to an lvalue with static storage duration
	// or to a temporary is constant-initialized; its temporary (12.2p5)
	// lives as long as the reference, in an internal global of its own.
	const syntax::Expr* single = SingleInitializer(initializer);
	if (single == nullptr)
		return false;
	const sema::Conversion* conversion = single->facts->conversion;
	StaticValue target = StaticValue();
	if (conversion != nullptr && conversion->form == sema::ConversionForm::Reference && conversion->temporary)
	{
		// Only a constant temporary: another is made by the initialization
		// function (3.6.2p2).
		const sema::Type* object = conversion->result;
		StaticValue value = StaticValue();
		if (object->unqualified->kind == sema::TypeKind::Class ||
			!static_converted(*single, object->unqualified, value))
			return false;
		// The temporary its evaluated value designates is this one: a
		// constant read of the reference (or of another bound to it) names
		// the same storage (see constant_temporary).
		const sema::ConstValue* bound = analyzer_ != nullptr ? analyzer_->evaluated_value(variable) : nullptr;
		const sema::ConstValue* extended = bound != nullptr && bound->kind == sema::ValueKind::Pointer &&
				bound->root == sema::RootKind::Temporary && bound->address.offset == 0
			? static_cast<const sema::ConstValue*>(bound->address.entity)
			: nullptr;
		const uint32_t made = extended != nullptr ? constant_temporaries_.find(extended) : PointerMap::kMissing;
		lowir::SymbolId temporary = made;
		if (made == PointerMap::kMissing)
		{
			std::string spelling = "__ref_tmp__";
			spelling += variable->name->spelling;
			temporary = new_symbol(spelling);
			lowir::Global storage = lowir::Global();
			storage.typed = true;
			storage.type = storage_type(object);
			lowir::DataItem item;
			static_item(object, value, item);
			storage.init = item.kind == lowir::DataItemKind::Scalar ? lowir::GlobalInit::Constant
																	: lowir::GlobalInit::Zero;
			storage.constant = item.constant;
			program_.declare_global(temporary, lowir::SymbolKind::GlobalDefinition, storage);
			program_.symbols[temporary].metadata.binding = ir_model::SBM_INTERNAL;
			name_temporary(temporary);
			if (extended != nullptr)
				constant_temporaries_.set(extended, temporary);
		}
		target.kind = StaticValue::Kind::Address;
		target.symbol = temporary;
		target.offset = 0;
	}
	else
	{
		target.kind = StaticValue::Kind::Address;
		if (!static_binding(*single, variable->type, target))
			return false;
	}
	global.init = lowir::GlobalInit::Address;
	global.address = target.symbol;
	global.addend = target.offset;
	reference_index_.set(variable, static_cast<uint32_t>(reference_targets_.size()));
	reference_targets_.push_back(target);
	return true;
}

// ------------------------------------------------------------ aggregates

bool ProgramLowering::static_construction(const sema::Type* type, const sema::FunctionEntity* constructor,
	const syntax::Initializer* initializer, StaticData& out)
{
	// 3.6.2p3: an object of a polymorphic class whose construction only
	// stores its table pointers and constants is initialized statically, as
	// the course does (static_constructor): its table pointers, the
	// constants at their members, zero elsewhere.
	const sema::ClassEntity* entity = type->kind == sema::TypeKind::Array ? nullptr : sema::ClassOf(type);
	if (entity == nullptr || !entity->polymorphic || entity->virtuals == nullptr || VirtualBaseCount(entity) != 0)
		return false;
	if (initializer != nullptr && initializer->kind != syntax::InitializerKind::Parenthesized)
		return false;
	std::vector<StaticPlacement> placements;
	uint64_t cursor = 0;
	if (!static_constructor(constructor, initializer != nullptr ? initializer->arguments.data : nullptr,
			initializer != nullptr ? initializer->arguments.size : 0, 0, placements, cursor, 0) ||
		cursor > size_of(type, 0))
		return false;
	std::stable_sort(placements.begin(), placements.end(),
		[](const StaticPlacement& a, const StaticPlacement& b) { return a.offset < b.offset; });
	uint64_t position = 0;
	for (const StaticPlacement& placement : placements)
	{
		if (placement.offset < position)
			return false;
		append_zero(placement.offset - position, out);
		append_item(placement.item, out);
		position = placement.offset + placement.size;
	}
	append_zero(size_of(type, 0) - position, out);
	return true;
}

bool ProgramLowering::static_constructor(const sema::FunctionEntity* constructor, syntax::Expr* const* arguments,
	uint32_t count, uint64_t origin, std::vector<StaticPlacement>& out, uint64_t& cursor, uint32_t depth)
{
	// The course's walk of a constructor whose object is initialized
	// statically: its bases, each constructed so at the end of the ones
	// before it; its table pointers (a primary base's replaced); then its
	// members past them, each a constant (or a parameter, whose argument
	// is one), zero for none, or constructed so; its body empty. `cursor`
	// ends past what it initialized, relative to `origin`.
	if (constructor == nullptr || depth > 64 || constructor->delegation != nullptr)
		return false;
	if (!constructor->implicit && !constructor->defaulted)
	{
		const syntax::FunctionBody* body = constructor->body;
		if (body == nullptr || body->try_block || body->body == nullptr || !body->body->items.empty())
			return false;
	}
	const sema::Type* signature = constructor->type->canonical;
	if (count > signature->param_count || signature->variadic)
		return false;
	std::vector<const sema::Entity*> parameters;
	for (const sema::Binding* binding = constructor->scope != nullptr ? constructor->scope->bindings : nullptr;
		binding != nullptr; binding = binding->next)
	{
		if (binding->kind == sema::BindingKind::Parameter)
			parameters.push_back(binding->entity);
	}
	const sema::ClassEntity* cls = sema::ClassOf(constructor->owner->entity->type);
	// (Laid out: its bases' offsets known.)
	size_of(cls->type, 0);
	cursor = 0;
	const size_t first = out.size();
	bool tables = false;
	for (uint32_t i = 0; i < constructor->init_count; ++i)
	{
		const sema::SubobjectInit& init = constructor->inits[i];
		if (init.inner_count != 0 || init.forwards || (init.base == nullptr && init.member->bit_field))
			return false;
		if (init.base == nullptr && !tables && cls->dynamic)
		{
			if (!static_tables(cls, origin, out, first, cursor))
				return false;
			tables = true;
		}
		const sema::Type* type = init.base != nullptr
			? init.base->base->type
			: static_cast<const sema::VariableEntity*>(init.member->variable)->type;
		const uint64_t offset = init.base != nullptr
			? init.base->offset
			: member_offset(static_cast<const sema::VariableEntity*>(init.member->variable), 0);
		if (init.base != nullptr ? offset != cursor : offset < cursor)
			return false;
		uint64_t end = 0;
		if (!static_subobject(init, type, constructor, arguments, count, parameters, origin + offset, out, end, depth))
			return false;
		cursor = offset + (init.base != nullptr ? size_of(type, 0) : std::max(end, size_of(type, 0)));
	}
	if (!tables && cls->dynamic && !static_tables(cls, origin, out, first, cursor))
		return false;
	return true;
}

bool ProgramLowering::static_tables(
	const sema::ClassEntity* cls, uint64_t origin, std::vector<StaticPlacement>& out, size_t first, uint64_t& cursor)
{
	// A dynamic class's table pointers: before anything else, its own; past
	// its bases, theirs replaced (the primary base's at the object's start,
	// Itanium 2.4; every other at a base subobject's).
	std::vector<StaticPlacement> tables;
	place_tables(cls, origin, tables);
	if (cursor == 0)
		out.insert(out.end(), tables.begin(), tables.end());
	else
	{
		for (const StaticPlacement& table : tables)
		{
			size_t found = out.size();
			for (size_t i = first; i < out.size(); ++i)
			{
				if (out[i].offset == table.offset && out[i].item.kind == lowir::DataItemKind::Address)
					found = i;
			}
			if (found == out.size())
				return false;
			out[found].item = table.item;
		}
	}
	cursor = std::max<uint64_t>(cursor, 8);
	return true;
}

bool ProgramLowering::static_subobject(const sema::SubobjectInit& init, const sema::Type* type,
	const sema::FunctionEntity* constructor, syntax::Expr* const* arguments, uint32_t count,
	const std::vector<const sema::Entity*>& parameters, uint64_t at, std::vector<StaticPlacement>& out, uint64_t& end,
	uint32_t depth)
{
	// One base or member of static_constructor's object at `at`: a class
	// constructed so (an empty one trivially), else a scalar's constant
	// from its mem-initializer or default member initializer (a
	// parameter's from its argument), zero without one.
	end = 0;
	const sema::ClassEntity* sub = type->kind == sema::TypeKind::Array ? nullptr : sema::ClassOf(type);
	if (sub != nullptr)
	{
		if (init.constructor == nullptr || init.constructor->trivial)
			return (init.base == nullptr || sub->empty) && init.mem_initializer == nullptr;
		if (sub->key == TokenKind::KW_UNION || VirtualBaseCount(sub) != 0)
			return false;
		const syntax::MemInitializer* written = init.mem_initializer;
		if (written != nullptr && (written->braced || written->pack))
			return false;
		return static_constructor(init.constructor, written != nullptr ? written->arguments.data : nullptr,
			written != nullptr ? written->arguments.size : 0, at, out, end, depth + 1);
	}
	const sema::Type* element = type;
	while (element->kind == sema::TypeKind::Array)
		element = element->element;
	if (init.member == nullptr || type->is_reference() ||
		(element != type &&
			(init.mem_initializer != nullptr || init.default_initializer != nullptr ||
				sema::ClassOf(element) != nullptr)))
		return false;
	const syntax::Expr* single = nullptr;
	if (init.mem_initializer != nullptr)
	{
		const syntax::MemInitializer* written = init.mem_initializer;
		if (written->pack)
			return false;
		if (written->braced)
			single = written->list != nullptr && written->list->elements.size == 1 ? written->list->elements[0]
																				   : nullptr;
		else
			single = written->arguments.size == 1 ? written->arguments[0] : nullptr;
		if (single == nullptr)
			return written->braced && written->list != nullptr && written->list->elements.size == 0;
	}
	else if (init.default_initializer != nullptr)
	{
		single = SingleInitializer(init.default_initializer);
		if (single == nullptr)
			return false;
	}
	if (single == nullptr)
		return true;
	StaticValue value = StaticValue();
	if (!static_parameter_value(*single, type, constructor, arguments, count, parameters, value) &&
		!static_converted(*single, type->unqualified, value))
		return false;
	lowir::DataItem item;
	if (!static_item(type, value, item))
		return false;
	if (item.kind != lowir::DataItemKind::Zero)
		out.push_back(StaticPlacement{ at, size_of(type, 0), item });
	return true;
}

bool ProgramLowering::static_parameter_value(const syntax::Expr& expr, const sema::Type* type,
	const sema::FunctionEntity* constructor, syntax::Expr* const* arguments, uint32_t count,
	const std::vector<const sema::Entity*>& parameters, StaticValue& out)
{
	// A member initialized from a parameter itself: its argument's value
	// (or its default argument's) as the parameter's type, then as the
	// member's (integral types, or one type).
	const syntax::Expr* e = WithoutParentheses(&expr);
	const sema::ExprFacts* facts = e->facts;
	if (facts == nullptr || facts->entity == nullptr || e->kind != syntax::ExprKind::Id)
		return false;
	for (uint32_t i = 0; i < parameters.size(); ++i)
	{
		if (parameters[i] != facts->entity)
			continue;
		const sema::Type* parameter = constructor->type->canonical->params[i];
		const syntax::Expr* argument = i < count        ? arguments[i]
			: constructor->default_arguments != nullptr ? constructor->default_arguments[i]
														: nullptr;
		if (argument == nullptr || parameter->is_reference() || !static_converted(*argument, parameter, out))
			return false;
		parameter = parameter->unqualified;
		const sema::Type* target = type->unqualified;
		if (parameter == target)
			return true;
		sema::IntegralKind kind;
		if (out.kind != StaticValue::Kind::Integer || !IsIntegralLike(parameter) || !IsIntegralLike(target) ||
			!sema::IntegralKindOf(target, kind) || kind.bits > 64)
			return false;
		out.bits = sema::ConvertIntegral(out.bits, kind);
		out.high = kind.is_signed && static_cast<int64_t>(out.bits) < 0 ? ~uint64_t(0) : 0;
		return true;
	}
	return false;
}

bool ProgramLowering::static_object(const sema::Type* type, const syntax::Initializer* initializer, StaticData& out)
{
	if (initializer->kind == syntax::InitializerKind::Parenthesized)
		return false;
	const syntax::Expr* expr = initializer->expression;
	if (expr->kind != syntax::ExprKind::BracedInit)
		return static_string(type, *expr, out);
	return static_element(type, *expr, out);
}

bool ProgramLowering::static_element(const sema::Type* type, const syntax::Expr& element, StaticData& out)
{
	// One array element or class member from its initializer: a nested
	// braced list, a string literal for a character array, or a scalar.
	if (type->kind == sema::TypeKind::Array)
	{
		if (element.kind != syntax::ExprKind::BracedInit)
			return static_string(type, element, out);
		const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(element);
		// `{"..."}` for a character array itself (8.5.2p1); an array of
		// such arrays takes its first element from the literal (8.5.1p11).
		if (list.elements.size == 1 && list.elements[0]->kind != syntax::ExprKind::BracedInit &&
			WithoutParentheses(list.elements[0])->kind == syntax::ExprKind::Literal &&
			list.elements[0]->facts->type->kind == sema::TypeKind::Array &&
			type->element->kind != sema::TypeKind::Array)
			return static_string(type, *list.elements[0], out);
		uint32_t next = 0;
		return static_aggregate(type, &list, next, out);
	}
	if (type->unqualified->kind == sema::TypeKind::Class)
	{
		// A braced list initializes an aggregate member by member; a class
		// with constructors runs one (8.5.4p3), the dynamic initialization's.
		if (element.kind != syntax::ExprKind::BracedInit)
			return false;
		const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(element);
		if (!sema::IsAggregateClass(sema::ClassOf(type)) &&
			(element.facts == nullptr || !memberwise_constructor(element.facts->entity, list.elements.size)))
			return false;
		uint32_t next = 0;
		return static_aggregate(type, static_cast<const syntax::BracedInitList*>(&element), next, out);
	}
	if (type->unqualified->kind == sema::TypeKind::Complex)
		return static_complex(type, element, out);
	// A vector's lanes from its list; from another expression, the dynamic
	// initialization's.
	if (type->unqualified->kind == sema::TypeKind::Vector)
		return element.kind == syntax::ExprKind::BracedInit &&
			static_vector(type, static_cast<const syntax::BracedInitList&>(element), out);
	if (element.kind == syntax::ExprKind::BracedInit)
	{
		const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(element);
		if (list.elements.empty())
			return missing_element(type, false, out);
		return static_element(type, *list.elements[0], out);
	}
	// A pointer to member's value is the constant evaluation's.
	if (type->unqualified->kind == sema::TypeKind::MemberPointer)
		return false;
	if (type->is_reference())
	{
		// A reference member bound to an lvalue with static storage duration
		// holds its address (3.6.2p2).
		lowir::DataItem item = lowir::DataItem();
		item.kind = lowir::DataItemKind::Address;
		item.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		const sema::Conversion* conversion = element.facts != nullptr ? element.facts->conversion : nullptr;
		StaticValue target = StaticValue();
		if ((conversion != nullptr && conversion->temporary) || !static_binding(element, type, target))
			return false;
		item.address = target.symbol;
		item.amount = target.offset;
		append_item(item, out);
		return true;
	}
	StaticValue value = StaticValue();
	if (!static_converted(element, type->unqualified, value))
		return false;
	lowir::DataItem item;
	if (!static_item(type, value, item))
		return false;
	append_item(item, out);
	return true;
}

bool ProgramLowering::static_item(
	const sema::Type* type, bool member, const syntax::BracedInitList* list, uint32_t& next, StaticData& out)
{
	// A member or element from the list at `next`: a subaggregate its
	// element does not initialize by itself takes its elements from this
	// list (8.5.1p11); one without an element is value-initialized (p7).
	const syntax::Expr* element = list != nullptr && next < list->elements.size ? list->elements[next] : nullptr;
	if (element == nullptr)
		return missing_element(type, member, out);
	if (sema::IsSubaggregate(type) && element->kind != syntax::ExprKind::BracedInit &&
		!sema::InitializesSubaggregate(type, *element))
		return static_aggregate(type, list, next, out);
	++next;
	return static_element(type, *element, out);
}

bool ProgramLowering::static_aggregate(
	const sema::Type* type, const syntax::BracedInitList* list, uint32_t& next, StaticData& out)
{
	if (type->kind == sema::TypeKind::Array)
	{
		for (uint64_t i = 0; i < type->bound; ++i)
		{
			if (!static_item(type->element, false, list, next, out))
				return false;
		}
		return true;
	}
	// 8.5.1p2: members in order, padding as zero bytes; a union's one
	// member (p15, see sema::UnionListMember); a run of bit-fields as the
	// bytes of their units, each field's value truncated to its width (9.6).
	const sema::ClassEntity* entity = sema::ClassOf(type);
	const bool is_union = entity->key == TokenKind::KW_UNION;
	const sema::DataMember* selected = is_union ? sema::ListUnionMember(entity, list, next) : nullptr;
	const uint64_t size = size_of(type, 0);
	uint64_t position = 0;
	// The bytes of a flexible array member's elements (see below).
	uint64_t trailing = 0;
	// The bytes of the bit-field storage unit being filled, at `unit_at`.
	std::vector<uint8_t> bits;
	uint64_t unit_at = 0;
	for (const sema::DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(member->variable);
		if (selected != nullptr && member != selected)
			continue;
		if (member->bit_field)
		{
			if (variable->name == nullptr)
				continue;
			const syntax::Expr* element = list != nullptr && next < list->elements.size ? list->elements[next++]
																						: nullptr;
			if (element != nullptr && element->kind == syntax::ExprKind::BracedInit)
			{
				const syntax::BracedInitList* inner = static_cast<const syntax::BracedInitList*>(element);
				element = inner->elements.empty() ? nullptr : inner->elements[0];
			}
			StaticValue value = StaticValue();
			if (element != nullptr &&
				(!static_converted(*element, variable->type->unqualified, value) ||
					value.kind != StaticValue::Kind::Integer))
				return false;
			// A shared or split unit is written at run time.
			const uint64_t bit = member_bit_offset(variable, 0);
			if (member->unit_shared || member->unit_split != 0 || member->unit_bytes > 8)
				return false;
			if (bits.empty() || member->unit_offset != unit_at)
			{
				flush_bits(bits, position, out);
				if (member->unit_offset < position)
					return false;
				append_zero(member->unit_offset - position, out);
				position = unit_at = member->unit_offset;
				bits.assign(member->unit_bytes, 0);
			}
			for (uint64_t k = 0; k < member->width && k < 64; ++k)
			{
				const uint64_t at = (bit + k) / 8 - unit_at;
				if (at >= bits.size())
					return false;
				if (((value.bits >> k) & 1) != 0)
					bits[static_cast<size_t>(at)] |= static_cast<uint8_t>(1u << ((bit + k) % 8));
			}
			if (is_union)
				break;
			continue;
		}
		flush_bits(bits, position, out);
		const uint64_t offset = member_offset(variable, 0);
		if (offset > position)
			append_zero(offset - position, out);
		const syntax::Expr* element = list != nullptr && next < list->elements.size ? list->elements[next] : nullptr;
		if (element != nullptr && list->facts != nullptr && (list->facts->flags & sema::kFlexibleElements) != 0 &&
			member->next == nullptr && variable->type->kind == sema::TypeKind::Array && variable->type->bound == 0)
		{
			// GNU's flexible array member given elements (the analysis
			// allows them only here, see sema::kFlexibleElements): the array
			// its element types, its bound the elements given; the object
			// grows by their bytes (GCC's size, past any tail padding).
			const sema::Type* given = WithoutParentheses(element)->facts->type;
			if (given->kind != sema::TypeKind::Array)
				return false;
			++next;
			if (!static_element(given, *element, out))
				return false;
			trailing = size_of(given, 0);
			position = offset + trailing;
			continue;
		}
		if (!static_item(variable->type, true, list, next, out))
			return false;
		position = offset + size_of(variable->type, 0);
		if (is_union)
			break;
	}
	flush_bits(bits, position, out);
	if (size + trailing > position)
		append_zero(size + trailing - position, out);
	return true;
}

void ProgramLowering::flush_bits(std::vector<uint8_t>& bits, uint64_t& position, StaticData& out)
{
	// A bit-field storage unit's bytes as one unsigned item of its width.
	if (bits.empty())
		return;
	uint64_t value = 0;
	for (size_t i = bits.size(); i-- > 0;)
		value = value << 8 | bits[i];
	lowir::DataItem item = lowir::DataItem();
	item.kind = lowir::DataItemKind::Scalar;
	item.type = lowir::Type::scalar(bits.size() == 1 ? lowir::TypeKind::U8
			: bits.size() == 2                       ? lowir::TypeKind::U16
			: bits.size() == 4                       ? lowir::TypeKind::U32
													 : lowir::TypeKind::I64);
	item.constant = IntegerConstant(value);
	append_item(item, out);
	position += bits.size();
	bits.clear();
}

bool ProgramLowering::static_string(const sema::Type* type, const syntax::Expr& element, StaticData& out)
{
	// 8.5.2: a character array from a string literal: its code units and
	// null, the rest of the array zero.
	const syntax::Expr* literal = WithoutParentheses(&element);
	if (literal->kind != syntax::ExprKind::Literal || type->kind != sema::TypeKind::Array ||
		literal->facts->type->kind != sema::TypeKind::Array)
		return false;
	sema::StringLiteralFacts facts;
	std::vector<uint64_t> units;
	if (!sema::StringLiteralUnits(static_cast<const syntax::LiteralExpr*>(literal)->spelling, facts, units))
		return false;
	const lowir::Type unit = storage_type(type->element);
	for (uint64_t i = 0; i < type->bound; ++i)
	{
		lowir::DataItem item = lowir::DataItem();
		item.kind = lowir::DataItemKind::Scalar;
		item.type = unit;
		item.constant = IntegerConstant(i < units.size() ? units[i] : 0);
		append_item(item, out);
	}
	return true;
}

bool ProgramLowering::static_vector(const sema::Type* type, const syntax::BracedInitList& list, StaticData& out)
{
	// GNU's list-initialization of a vector: each element converted to the
	// lane type, the lanes after them zero; a bool vector's bit-packed
	// lanes are the dynamic initialization's.
	const sema::Type* vector = type->unqualified;
	const sema::Type* lane = vector->element->unqualified;
	if (lane->kind == sema::TypeKind::Fundamental && lane->fundamental == FundamentalType::Bool)
		return false;
	if (list.elements.size > vector->bound)
		return false;
	for (const syntax::Expr* element : list.elements)
	{
		StaticValue value = StaticValue();
		lowir::DataItem item;
		if (element->kind == syntax::ExprKind::BracedInit || !static_converted(*element, lane, value) ||
			!static_item(lane, value, item))
			return false;
		append_item(item, out);
	}
	append_zero((vector->bound - list.elements.size) * size_of(lane, 0), out);
	return true;
}

bool ProgramLowering::static_complex(const sema::Type* type, const syntax::Expr& expr, StaticData& out)
{
	// A complex value __builtin_complex or a list makes of two constant
	// parts, or a real constant (one in braces) makes with a zero
	// imaginary part: their bytes, real then imaginary (the constant
	// evaluation has no binary16 or binary128 values: their complex
	// values' static data are made here); any other, the dynamic
	// initialization's.
	const syntax::Expr* e = WithoutParentheses(&expr);
	syntax::Slice<syntax::Expr*> parts = syntax::Slice<syntax::Expr*>();
	if (e->kind == syntax::ExprKind::BracedInit)
		parts = static_cast<const syntax::BracedInitList*>(e)->elements;
	else if (e->kind == syntax::ExprKind::Call && e->facts != nullptr && (e->facts->flags & sema::kIntrinsic) != 0 &&
		static_cast<sema::Intrinsic>(e->facts->value & 0xff) == sema::Intrinsic::MakeComplex)
		parts = static_cast<const syntax::CallExpr*>(e)->arguments;
	else
		parts = syntax::Slice<syntax::Expr*>{ const_cast<syntax::Expr**>(&e), 1 };
	const sema::Type* part = type->unqualified->element;
	if (parts.size == 1 && parts[0]->kind != syntax::ExprKind::BracedInit && parts[0]->facts != nullptr &&
		parts[0]->facts->type != nullptr && !sema::IsComplex(parts[0]->facts->type))
	{
		StaticValue value = StaticValue();
		lowir::DataItem item;
		if (!static_converted(*parts[0], part->unqualified, value) || !static_item(part, value, item))
			return false;
		append_item(item, out);
		return missing_element(part, false, out);
	}
	if (parts.size != 2)
		return false;
	for (const syntax::Expr* argument : parts)
	{
		StaticValue value = StaticValue();
		lowir::DataItem item;
		if (!static_converted(*argument, part->unqualified, value) || !static_item(part, value, item))
			return false;
		append_item(item, out);
	}
	return true;
}

bool ProgramLowering::evaluated_data(const sema::VariableEntity* variable, const syntax::Initializer* initializer,
	const sema::ConstValue* recorded, bool nonconstant, StaticData& out, uint32_t location)
{
	// A complex or vector variable's evaluated value (a conversion of a real
	// constant, constant arithmetic, a constexpr call's result, a constexpr
	// variable's value), unless the analysis found it not constant.
	out.items.clear();
	const sema::ConstValue* value = recorded != nullptr ? recorded
		: nonconstant || variable->is_thread_local      ? nullptr
														: initialization_value(variable, initializer);
	return value != nullptr && constant_data(variable->type, *value, out, location);
}

bool ProgramLowering::memberwise_constructor(const sema::Entity* entity, uint32_t arguments)
{
	// The course's static initialization of a braced list whose constructor
	// is not constexpr (3.6.2p3: the result a dynamic one would have): one
	// that only copies each parameter, in order, into the member of its
	// position and type, a class without bases or virtual functions.
	if (entity == nullptr || entity->kind != sema::EntityKind::Function)
		return false;
	const sema::FunctionEntity* constructor = static_cast<const sema::FunctionEntity*>(entity);
	const syntax::FunctionBody* body = constructor->body;
	if (!constructor->constructor || constructor->is_constexpr || constructor->delegation != nullptr ||
		body == nullptr || body->try_block || body->body == nullptr || !body->body->items.empty() ||
		constructor->scope == nullptr)
		return false;
	const sema::ClassEntity* cls = static_cast<const sema::ClassEntity*>(constructor->owner->entity);
	const sema::Type* type = constructor->type->canonical;
	if (cls->bases != nullptr || cls->polymorphic || type->param_count != arguments ||
		constructor->init_count != arguments)
		return false;
	std::vector<const sema::Entity*> parameters;
	for (const sema::Binding* binding = constructor->scope->bindings; binding != nullptr; binding = binding->next)
	{
		if (binding->kind == sema::BindingKind::Parameter)
			parameters.push_back(binding->entity);
	}
	if (parameters.size() != arguments)
		return false;
	for (uint32_t i = 0; i < arguments; ++i)
	{
		const sema::SubobjectInit& init = constructor->inits[i];
		if (init.base != nullptr || init.member == nullptr || init.member->bit_field || init.inner != nullptr ||
			init.mem_initializer == nullptr || init.mem_initializer->list != nullptr ||
			init.mem_initializer->arguments.size != 1)
			return false;
		const syntax::Expr* argument = WithoutParentheses(init.mem_initializer->arguments[0]);
		if (argument->kind != syntax::ExprKind::Id || argument->facts == nullptr ||
			argument->facts->entity != parameters[i] ||
			type->params[i]->unqualified != init.member->variable->type->unqualified)
			return false;
	}
	return true;
}

bool ProgramLowering::zero_value_initialized(const sema::Type* type, uint32_t depth)
{
	// Whether value-initialization (8.5p7) of an object of the type only
	// zeroes it: no constructor that is not trivial runs (a class's own, or
	// an aggregate member's).
	while (type->kind == sema::TypeKind::Array)
		type = type->element;
	const sema::ClassEntity* cls = type->is_reference() ? nullptr : sema::ClassOf(type);
	if (cls == nullptr)
		return true;
	if (!sema::IsAggregateClass(cls))
		return cls->default_constructor != nullptr && cls->default_constructor->trivial;
	if (depth > 64)
		return false;
	for (const sema::DataMember* member = cls->members; member != nullptr; member = member->next)
	{
		if (member->variable->kind == sema::EntityKind::Variable && !member->variable->type->is_reference() &&
			!zero_value_initialized(member->variable->type, depth + 1))
			return false;
	}
	return true;
}

bool ProgramLowering::missing_element(const sema::Type* type, bool member, StaticData& out)
{
	// A value-initialized element (8.5.1p7): the course data spells an
	// arithmetic array element and a floating member as a scalar zero, an
	// aggregate by its members, and other class objects, pointers and
	// absent integral members as zero bytes; one a constructor initializes
	// is the dynamic initialization's.
	if (!zero_value_initialized(type, 0))
		return false;
	if (type->kind == sema::TypeKind::Array)
	{
		for (uint64_t i = 0; i < type->bound; ++i)
			missing_element(type->element, false, out);
		return true;
	}
	const sema::ClassEntity* cls = type->is_reference() ? nullptr : sema::ClassOf(type);
	if (cls != nullptr && sema::IsAggregateClass(cls))
	{
		uint32_t next = 0;
		return static_aggregate(type, nullptr, next, out);
	}
	if (((!member && (sema::IsArithmetic(type) || type->unqualified->kind == sema::TypeKind::Enum)) ||
			(member && sema::IsFloating(type))) &&
		SoftFloatOf(type) == SoftFloat::None)
	{
		lowir::DataItem item = lowir::DataItem();
		item.kind = lowir::DataItemKind::Scalar;
		item.type = storage_type(type);
		if (item.type.is_float())
		{
			item.constant.kind = lowir::ValueKind::Float;
			item.constant.float_kind = item.type.kind();
			item.constant.bits = lowir::float_literal(item.type.kind(), 0.0L);
		}
		else
			item.constant = IntegerConstant(0);
		append_item(item, out);
		return true;
	}
	append_zero(size_of(type, 0), out);
	return true;
}

void ProgramLowering::append_item(const lowir::DataItem& item, StaticData& out)
{
	if (item.kind == lowir::DataItemKind::Zero)
	{
		append_zero(static_cast<uint64_t>(item.amount), out);
		return;
	}
	out.items.push_back(item);
}

void ProgramLowering::append_zero(uint64_t bytes, StaticData& out)
{
	if (bytes == 0)
		return;
	if (!out.items.empty() && out.items.back().kind == lowir::DataItemKind::Zero)
	{
		out.items.back().amount += static_cast<int64_t>(bytes);
		return;
	}
	lowir::DataItem item = lowir::DataItem();
	item.kind = lowir::DataItemKind::Zero;
	item.amount = static_cast<int64_t>(bytes);
	out.items.push_back(item);
}

bool ProgramLowering::static_item(const sema::Type* type, const StaticValue& value, lowir::DataItem& item)
{
	item = lowir::DataItem();
	item.type = storage_type(type);
	switch (value.kind)
	{
	case StaticValue::Kind::Soft:
		// A binary16, bfloat16 or binary128 value's encoding as an integer of
		// its size.
		item.kind = lowir::DataItemKind::Scalar;
		item.type = lowir::Type::scalar(
			SoftFloatOf(type) == SoftFloat::Quad ? lowir::TypeKind::I128 : lowir::TypeKind::U16);
		item.constant = IntegerConstant(value.bits);
		item.constant.bits.high = value.high;
		return SoftFloatOf(type) != SoftFloat::None;
	case StaticValue::Kind::Address:
		item.kind = lowir::DataItemKind::Address;
		item.address = value.symbol;
		item.amount = value.offset;
		return true;
	case StaticValue::Kind::Null:
		item.kind = lowir::DataItemKind::Zero;
		item.amount = 8;
		return true;
	case StaticValue::Kind::Floating:
		item.kind = lowir::DataItemKind::Scalar;
		item.constant.kind = lowir::ValueKind::Float;
		item.constant.float_kind = item.type.kind();
		item.constant.bits = lowir::float_literal(item.type.kind(), value.floating);
		return item.type.is_float();
	case StaticValue::Kind::Integer:
		item.kind = lowir::DataItemKind::Scalar;
		item.constant = IntegerConstant(value.bits);
		if (item.type.kind() == lowir::TypeKind::I128)
			item.constant.bits.high = value.high;
		return item.type.is_integer() || item.type.is_pointer();
	}
	return false;
}

// ------------------------------------------------------ constant values

bool ProgramLowering::static_converted(
	const syntax::Expr& expr, const sema::Type* target, StaticValue& out, bool nonconstant)
{
	// The expression's value converted to the initialized type (the
	// conversion the analysis recorded, or a cast's), in value terms.
	// `nonconstant`: the expression is known not to be an integral
	// constant expression (not evaluated again).
	const sema::ExprFacts* facts = expr.facts;
	if (facts == nullptr || facts->type == nullptr)
		return false;
	// A captureless lambda converted to a pointer to function (5.1.2p6):
	// the address of the closure's invoker, statically initialized (the
	// course's form, as 3.6.2p3 permits: the conversion has no effect).
	const sema::FunctionEntity* converting = facts->conversion != nullptr ? facts->conversion->constructor : nullptr;
	if (converting != nullptr && converting->conversion &&
		WithoutParentheses(&expr)->kind == syntax::ExprKind::Lambda && IsPointerLike(target->unqualified))
	{
		const sema::ClosureFacts* closure = graph_->closure_facts(sema::ClassOf(converting->owner->entity->type));
		if (closure == nullptr || closure->conversion != converting || closure->invoker == nullptr)
			return false;
		demand(closure->invoker);
		out = StaticValue();
		out.kind = StaticValue::Kind::Address;
		out.symbol = function_symbol(closure->invoker);
		out.offset = 0;
		return true;
	}
	StaticValue value = StaticValue();
	if (!static_value(expr, value, nonconstant))
		return false;
	const sema::Type* from = facts->type->kind == sema::TypeKind::Array || facts->type->kind == sema::TypeKind::Function
		? nullptr
		: facts->type->unqualified;
	target = target->unqualified;
	if (SoftFloatOf(target) != SoftFloat::None)
		return SoftStaticValue(value, from, SoftFloatOf(target), out);
	if (value.kind == StaticValue::Kind::Soft)
		return from != nullptr && SoftFloatOf(from) != SoftFloat::None &&
			SoftStaticConverted(value, SoftFloatOf(from), target, out);
	if (sema::IsFloating(target))
	{
		if (value.kind == StaticValue::Kind::Integer && !IntegralStaticFloating(value, from, target, value))
			return false;
		if (value.kind != StaticValue::Kind::Floating)
			return false;
		// Rounded once to the target's precision (4.8, 4.9).
		const lowir::TypeKind kind = storage_type(target).kind();
		value.floating = lowir::float_value(kind, lowir::float_literal(kind, value.floating));
		out = value;
		return true;
	}
	if (IsIntegralLike(target))
	{
		sema::IntegralKind kind;
		if (!sema::IntegralKindOf(target, kind))
			return false;
		if (value.kind == StaticValue::Kind::Floating)
		{
			// 4.9p1: truncation toward zero; 4.12 for bool.
			if (kind.bits > 64)
			{
				// A 128-bit type's value outside the 64-bit ranges converts
				// at run time.
				const long double limit = 9223372036854775808.0L;
				const long double f = value.floating;
				if (!(f > -limit - 1 && f < 2 * limit) || (!kind.is_signed && f <= -1))
					return false;
				value.bits = f < limit ? static_cast<uint64_t>(static_cast<int64_t>(f)) : static_cast<uint64_t>(f);
				value.high = f < 0 && static_cast<int64_t>(value.bits) < 0 ? ~uint64_t(0) : 0;
				value.kind = StaticValue::Kind::Integer;
				out = value;
				return true;
			}
			if (kind.bits == 1)
				value.bits = value.floating != 0 ? 1 : 0;
			else if (kind.is_signed)
				value.bits = static_cast<uint64_t>(static_cast<int64_t>(value.floating));
			else
				value.bits = static_cast<uint64_t>(value.floating);
			value.kind = StaticValue::Kind::Integer;
		}
		else if (value.kind == StaticValue::Kind::Null)
		{
			value.kind = StaticValue::Kind::Integer;
			value.bits = 0;
			value.high = 0;
		}
		else if (value.kind == StaticValue::Kind::Address)
		{
			// A pointer to an integer wide enough (5.2.10p4): the address
			// constant itself; to bool, a non-null address.
			if (kind.bits == 1)
			{
				value.kind = StaticValue::Kind::Integer;
				value.bits = 1;
				value.high = 0;
			}
			else if (kind.bits < 64)
				return false;
			out = value;
			return true;
		}
		// 4.7: modulo 2^N; a 128-bit type keeps both halves.
		if (kind.bits <= 64)
		{
			value.bits = sema::ConvertIntegral(value.bits, kind);
			value.high = kind.is_signed && static_cast<int64_t>(value.bits) < 0 ? ~uint64_t(0) : 0;
		}
		out = value;
		return true;
	}
	if (IsPointerLike(target))
	{
		if (value.kind == StaticValue::Kind::Integer && value.bits == 0 && value.high == 0)
			value.kind = StaticValue::Kind::Null;
		if (value.kind == StaticValue::Kind::Floating)
			return false;
		// 4.10p3: an address constant of the base class subobject.
		const sema::Conversion* conversion = facts->conversion;
		if (value.kind == StaticValue::Kind::Address && conversion != nullptr &&
			conversion->second == sema::ValueConversion::DerivedToBase && from != nullptr &&
			from->kind == sema::TypeKind::Pointer &&
			!static_base_offset(sema::ClassOf(from->element), sema::ClassOf(target->element), value, expr.location))
			return false;
		out = value;
		return true;
	}
	return false;
}

bool ProgramLowering::static_value(const syntax::Expr& expr, StaticValue& out, bool nonconstant)
{
	const syntax::Expr* e = WithoutParentheses(&expr);
	const sema::ExprFacts* facts = e->facts;
	if (facts == nullptr || facts->type == nullptr)
		return false;
	const sema::Type* type = facts->type;
	// An array or function designator stands for its address (4.2, 4.3).
	if ((type->kind == sema::TypeKind::Array || type->kind == sema::TypeKind::Function) &&
		facts->category != sema::ValueCategory::PRValue)
	{
		out = StaticValue();
		out.kind = StaticValue::Kind::Address;
		return static_lvalue(*e, out);
	}
	const bool integral = IsIntegralLike(type);
	if (integral && !nonconstant)
	{
		// A 128-bit type's value beyond 64 bits is as constant as any
		// (3.6.2p2), so the whole value is the object's.
		sema::Constant constant;
		uint64_t high = 0;
		if (analyzer_->integral_constant(*e, static_scope_, constant, high))
		{
			out = StaticValue();
			out.kind = StaticValue::Kind::Integer;
			out.bits = constant.bits;
			out.high = high;
			return true;
		}
	}
	switch (e->kind)
	{
	case syntax::ExprKind::Literal:
	{
		const syntax::LiteralExpr* literal = static_cast<const syntax::LiteralExpr*>(e);
		if (!sema::IsFloating(type))
			return false;
		if (SoftFloatOf(type) != SoftFloat::None)
		{
			out = StaticValue();
			out.kind = StaticValue::Kind::Soft;
			return ScanInterchangeValue(literal->spelling.text,
				ClassifyNumber(literal->spelling.text, literal->spelling.length).suffix_begin,
				SoftFloatOf(type) == SoftFloat::Half         ? kBinary16
					: SoftFloatOf(type) == SoftFloat::BFloat ? kBFloat16
															 : kBinary128,
				out.bits, out.high);
		}
		const lowir::TypeKind kind = storage_type(type).kind();
		lowir::TypeKind scanned = kind;
		lowir::Literal bits;
		std::string scratch;
		if (!lowir::scan_float_literal(
				literal->spelling.text, literal->spelling.length, false, kind, scanned, bits, scratch))
			return false;
		out = StaticValue();
		out.kind = StaticValue::Kind::Floating;
		out.floating = lowir::float_value(kind, bits);
		return true;
	}
	case syntax::ExprKind::KeywordLiteral:
		if (static_cast<const syntax::KeywordLiteralExpr*>(e)->keyword != TokenKind::KW_NULLPTR)
			return false;
		out = StaticValue();
		out.kind = StaticValue::Kind::Null;
		return true;
	case syntax::ExprKind::Unary:
	{
		const syntax::UnaryExpr* unary = static_cast<const syntax::UnaryExpr*>(e);
		if (unary->op == TokenKind::OP_AMP)
		{
			out = StaticValue();
			out.kind = StaticValue::Kind::Address;
			return static_lvalue(*unary->operand, out);
		}
		if ((unary->op == TokenKind::OP_MINUS || unary->op == TokenKind::OP_PLUS) && sema::IsFloating(type))
		{
			if (!static_converted(*unary->operand, type, out))
				return false;
			if (out.kind == StaticValue::Kind::Soft)
			{
				// The sign bit of a binary16, bfloat16 or binary128 value.
				if (unary->op == TokenKind::OP_MINUS && SoftFloatOf(type) != SoftFloat::Quad)
					out.bits ^= 0x8000;
				else if (unary->op == TokenKind::OP_MINUS)
					out.high ^= uint64_t(1) << 63;
				return true;
			}
			if (out.kind != StaticValue::Kind::Floating)
				return false;
			if (unary->op == TokenKind::OP_MINUS)
				out.floating = -out.floating;
			return true;
		}
		if (unary->op == TokenKind::OP_PLUS && IsPointerLike(type))
			return static_converted(*unary->operand, type, out);
		return false;
	}
	case syntax::ExprKind::Binary:
	{
		const syntax::BinaryExpr* binary = static_cast<const syntax::BinaryExpr*>(e);
		const bool additive = binary->op == TokenKind::OP_PLUS || binary->op == TokenKind::OP_MINUS;
		if (additive && IsPointerLike(type))
		{
			// 5.7: an address constant plus or minus an integral constant,
			// scaled by the element size.
			const bool left_pointer = IsPointerLike(binary->left->facts->type) ||
				binary->left->facts->type->kind == sema::TypeKind::Array;
			const syntax::Expr& pointer = left_pointer ? *binary->left : *binary->right;
			const syntax::Expr& count = left_pointer ? *binary->right : *binary->left;
			StaticValue base = StaticValue();
			sema::Constant offset;
			if (!static_converted(pointer, type, base) || base.kind != StaticValue::Kind::Address ||
				!analyzer_->integral_constant(count, static_scope_, offset))
				return false;
			const int64_t scaled = static_cast<int64_t>(offset.bits) *
				static_cast<int64_t>(size_of(type->unqualified->element, e->location));
			out = base;
			out.offset += binary->op == TokenKind::OP_MINUS ? -scaled : scaled;
			return true;
		}
		if (!sema::IsFloating(type) ||
			(!additive && binary->op != TokenKind::OP_STAR && binary->op != TokenKind::OP_DIV))
			return false;
		StaticValue left = StaticValue(), right = StaticValue();
		if (!static_converted(*binary->left, type, left) || !static_converted(*binary->right, type, right))
			return false;
		// Binary16 and binary128 arithmetic as the run-time routines round.
		if (SoftFloatOf(type) != SoftFloat::None)
			return SoftStaticArithmetic(binary->op, SoftFloatOf(type), left, right, out);
		out = left;
		switch (storage_type(type).kind())
		{
		case lowir::TypeKind::F32:
			out.floating = FloatingArithmetic<float>(binary->op, left.floating, right.floating);
			break;
		case lowir::TypeKind::F64:
			out.floating = static_cast<double>(
				FloatingArithmetic<long double>(binary->op, left.floating, right.floating));
			break;
		default:
			out.floating = FloatingArithmetic<long double>(binary->op, left.floating, right.floating);
			break;
		}
		return true;
	}
	case syntax::ExprKind::Conditional:
	{
		// With a constant condition, an integral conditional's selected
		// operand is as far from constant as the conditional just proved.
		const syntax::ConditionalExpr* conditional = static_cast<const syntax::ConditionalExpr*>(e);
		sema::Constant condition;
		if (!analyzer_->integral_constant(*conditional->condition, static_scope_, condition))
			return false;
		return static_converted(
			condition.bits != 0 ? *conditional->then_expr : *conditional->else_expr, type, out, integral);
	}
	case syntax::ExprKind::Cast:
	{
		// An integral cast of an integral operand is as constant as it.
		const syntax::Expr& operand = *static_cast<const syntax::CastExpr*>(e)->operand;
		if (!static_converted(operand, type, out, integral && IntegralOperand(operand)))
			return false;
		// 5.2.9p11: the derived object containing the base subobject; a
		// C-style cast to a base casting away constness: the base subobject.
		if ((facts->flags & (sema::kDerivedCast | sema::kBaseCast)) == 0 || out.kind != StaticValue::Kind::Address)
			return true;
		const sema::ClassEntity* to = sema::ClassOf(type->unqualified->element);
		const sema::ClassEntity* from = sema::ClassOf(operand.facts->type->unqualified->element);
		if ((facts->flags & sema::kDerivedCast) != 0)
			out.offset -= static_cast<int64_t>(base_offset(to, from, e->location));
		else if (!static_base_offset(from, to, out, e->location))
			return false;
		return true;
	}
	case syntax::ExprKind::FunctionalCast:
	{
		const syntax::FunctionalCastExpr* cast = static_cast<const syntax::FunctionalCastExpr*>(e);
		if (cast->braced == nullptr && cast->arguments.size == 1)
			return static_converted(*cast->arguments[0], type, out, integral && IntegralOperand(*cast->arguments[0]));
		return false;
	}
	case syntax::ExprKind::Call:
	{
		// GCC's quiet NaN and infinity constants of the call's type.
		const syntax::CallExpr* call = static_cast<const syntax::CallExpr*>(e);
		if ((facts->flags & sema::kIntrinsic) != 0)
		{
			const sema::Intrinsic which = static_cast<sema::Intrinsic>(facts->value & 0xff);
			if ((which != sema::Intrinsic::Nan && which != sema::Intrinsic::Inf) || !sema::IsFloating(type))
				return false;
			out = StaticValue();
			out.kind = StaticValue::Kind::Floating;
			out.floating = which == sema::Intrinsic::Nan
				? sema::BuiltinNan(type->unqualified->fundamental, *call->arguments[0])
				: std::numeric_limits<long double>::infinity();
			return true;
		}
		// `T(e)` that names a type the parser did not classify.
		if ((facts->flags & sema::kDirectCall) != 0 || call->arguments.size != 1 || call->callee->facts != nullptr)
			return false;
		return static_converted(*call->arguments[0], type, out, integral && IntegralOperand(*call->arguments[0]));
	}
	default:
		return false;
	}
}

namespace {

// `base` is a base class of `derived` (or `derived` itself).
bool IsBaseClass(const sema::ClassEntity* derived, const sema::ClassEntity* base)
{
	if (derived == base)
		return true;
	for (uint32_t i = 0; i < sema::VirtualBaseCount(derived); ++i)
	{
		if (sema::VirtualBases(derived)[i].base->base == base)
			return true;
	}
	for (const sema::BaseClass* edge = derived->bases; edge != nullptr; edge = edge->next)
	{
		if (!edge->is_virtual && IsBaseClass(static_cast<const sema::ClassEntity*>(edge->base), base))
			return true;
	}
	return false;
}

} // namespace

bool ProgramLowering::static_base_offset(
	const sema::ClassEntity* from, const sema::ClassEntity* to, StaticValue& value, uint32_t location, bool member)
{
	// A virtual base's offset belongs to its complete object's layout: a
	// static address reaches one only in a complete object it proves --
	// its named variable (an array's element by the address's position),
	// or the data member of the converted class the expression names --
	// and only from a subobject of it; else the initialization is dynamic.
	const BaseRoute route = base_route(from, to, location);
	if (route.via == nullptr)
	{
		value.offset += static_cast<int64_t>(route.offset);
		return true;
	}
	const sema::ClassEntity* owner = member ? from : value.root;
	if (owner == nullptr || !IsBaseClass(owner, from))
		return false;
	const int64_t origin = member ? value.offset
		: value.stride != 0       ? value.offset - value.offset % static_cast<int64_t>(value.stride)
								  : 0;
	const BaseRoute placed = base_route(owner, to, location);
	value.offset = origin + static_cast<int64_t>(placed.offset) +
		(placed.via != nullptr ? static_cast<int64_t>(virtual_base_offset(owner, placed.via)) : 0);
	return true;
}

bool ProgramLowering::static_binding(const syntax::Expr& expr, const sema::Type* reference, StaticValue& out)
{
	// A reference bound directly to an lvalue with static storage duration;
	// to a base class, to the base subobject (8.5.3p5).
	if (!static_lvalue(expr, out))
		return false;
	const sema::Conversion* conversion = expr.facts->conversion;
	if (conversion != nullptr && conversion->second == sema::ValueConversion::DerivedToBase)
		return static_base_offset(sema::ClassOf(expr.facts->type), sema::ClassOf(reference->element), out,
			expr.location, WithoutParentheses(&expr)->kind == syntax::ExprKind::Member);
	return true;
}

bool ProgramLowering::static_lvalue(const syntax::Expr& expr, StaticValue& out)
{
	// The address of an lvalue designating an object with static storage
	// duration or a function (5.19p3), with the named variable whose object
	// holds it (StaticValue::root).
	lowir::SymbolId& symbol = out.symbol;
	int64_t& offset = out.offset;
	out.root = nullptr;
	out.stride = 0;
	const syntax::Expr* e = WithoutParentheses(&expr);
	const sema::ExprFacts* facts = e->facts;
	if (facts == nullptr)
		return false;
	switch (e->kind)
	{
	case syntax::ExprKind::Id:
	{
		const sema::Entity* entity = facts->entity;
		if (entity == nullptr)
			return false;
		if (entity->kind == sema::EntityKind::Function)
		{
			const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(entity);
			demand(function);
			symbol = function_symbol(function);
			offset = 0;
			return true;
		}
		if (entity->kind != sema::EntityKind::Variable)
			return false;
		// A host object's thread-local object neither: its address is each
		// thread's own (3.7.2), no address constant (5.19p3), and no
		// relocation states it as data. (The LowIR contract's `addr` of a
		// thread-local global in a global's initializer, PA11's fixture,
		// is the course runtime's to resolve.)
		const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(entity);
		if (variable->is_static && !variable->is_thread_local && !variable->type->is_reference() &&
			variable->owner != nullptr &&
			(variable->owner->kind == sema::ScopeKind::Block || variable->owner->kind == sema::ScopeKind::Function))
		{
			// A block-scope static is its function's global (6.7p4), made
			// where its declaration was lowered.
			const uint32_t local = local_static_index_.find(variable);
			if (local == PointerMap::kMissing)
				return false;
			symbol = local_statics_[local].symbol;
			offset = 0;
			return true;
		}
		if (variable->member || variable->owner == nullptr || variable->owner->kind != sema::ScopeKind::Namespace ||
			(variable->is_thread_local && host_objects_))
			return false;
		if (variable->type->is_reference())
		{
			const uint32_t index = reference_index_.find(variable);
			if (index == PointerMap::kMissing)
				return false;
			symbol = reference_targets_[index].symbol;
			offset = reference_targets_[index].offset;
			out.root = reference_targets_[index].root;
			out.stride = reference_targets_[index].stride;
			return true;
		}
		symbol = variable_symbol(variable);
		offset = 0;
		{
			const sema::Type* element = variable->type->canonical;
			while (element->kind == sema::TypeKind::Array)
				element = element->element->canonical;
			out.root = sema::ClassOf(element->unqualified);
			if (out.root != nullptr && variable->type->canonical->kind == sema::TypeKind::Array)
				out.stride = size_of(element, e->location);
		}
		return true;
	}
	case syntax::ExprKind::Literal:
		if (facts->type->kind != sema::TypeKind::Array)
			return false;
		symbol = string_literal(*static_cast<const syntax::LiteralExpr*>(e));
		offset = 0;
		return true;
	case syntax::ExprKind::Subscript:
	{
		const syntax::SubscriptExpr* subscript = static_cast<const syntax::SubscriptExpr*>(e);
		const bool commuted = (facts->flags & sema::kCommuted) != 0;
		const syntax::Expr& base = commuted ? *subscript->index : *subscript->base;
		const syntax::Expr& index = commuted ? *subscript->base : *subscript->index;
		StaticValue pointer = StaticValue();
		sema::Constant count;
		if (!static_value(base, pointer, false) || pointer.kind != StaticValue::Kind::Address ||
			!analyzer_->integral_constant(index, static_scope_, count))
			return false;
		symbol = pointer.symbol;
		offset = pointer.offset +
			static_cast<int64_t>(count.bits) * static_cast<int64_t>(size_of(facts->type, e->location));
		out.root = pointer.root;
		out.stride = pointer.stride;
		return true;
	}
	case syntax::ExprKind::Unary:
	{
		const syntax::UnaryExpr* unary = static_cast<const syntax::UnaryExpr*>(e);
		if (unary->op != TokenKind::OP_STAR)
			return false;
		StaticValue pointer = StaticValue();
		if (!static_value(*unary->operand, pointer, false) || pointer.kind != StaticValue::Kind::Address)
			return false;
		symbol = pointer.symbol;
		offset = pointer.offset;
		out.root = pointer.root;
		out.stride = pointer.stride;
		return true;
	}
	case syntax::ExprKind::Member:
	{
		const syntax::MemberExpr* member = static_cast<const syntax::MemberExpr*>(e);
		if (facts->entity == nullptr || facts->entity->kind != sema::EntityKind::Variable)
			return false;
		const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(facts->entity);
		if (!variable->member || variable->type->is_reference())
			return false;
		if (member->op == TokenKind::OP_ARROW)
		{
			StaticValue pointer = StaticValue();
			if (!static_value(*member->base, pointer, false) || pointer.kind != StaticValue::Kind::Address)
				return false;
			out = pointer;
		}
		else if (!static_lvalue(*member->base, out))
			return false;
		// As FunctionLowering::member: the object's subobject of the naming
		// class (11.2p6), then of the member's class (through a virtual base
		// only in a known complete object), and the member's offset there, an
		// anonymous member's own included (9.5p5).
		const sema::Type* object_type = WithoutParentheses(member->base)->facts->type;
		if (member->op == TokenKind::OP_ARROW)
			object_type = object_type->kind == sema::TypeKind::Array ? object_type->element
																	 : object_type->unqualified->element;
		const sema::ClassEntity* from = sema::ClassOf(object_type);
		// (An object expression naming a member proves that member's layout.)
		const bool member_object = member->op != TokenKind::OP_ARROW &&
			WithoutParentheses(member->base)->kind == syntax::ExprKind::Member;
		if (const sema::ClassEntity* naming = member_naming_class(*member))
		{
			if (naming != from && !static_base_offset(from, naming, out, e->location, member_object))
				return false;
			from = naming;
		}
		uint64_t anonymous = 0;
		const sema::ClassEntity* home = member_home(variable, anonymous, e->location);
		if (home != from &&
			!static_base_offset(from, home, out, e->location, member_object && from == sema::ClassOf(object_type)))
			return false;
		offset += static_cast<int64_t>(anonymous + member_offset(variable, e->location));
		return true;
	}
	case syntax::ExprKind::Cast:
	{
		if ((facts->flags & sema::kReferenceCast) == 0)
			return false;
		const syntax::Expr& operand = *static_cast<const syntax::CastExpr*>(e)->operand;
		if (!static_lvalue(operand, out))
			return false;
		// 5.2.9p2: a base class subobject, or the derived object holding it.
		const sema::ClassEntity* from = sema::ClassOf(operand.facts->type);
		const sema::ClassEntity* to = sema::ClassOf(facts->type);
		if ((facts->flags & sema::kBaseCast) != 0)
			return static_base_offset(
				from, to, out, e->location, WithoutParentheses(&operand)->kind == syntax::ExprKind::Member);
		else if ((facts->flags & sema::kDerivedCast) != 0)
			offset -= static_cast<int64_t>(base_offset(to, from, e->location));
		return true;
	}
	default:
		return false;
	}
}

// -------------------------------------------------------- string literals

lowir::SymbolId ProgramLowering::string_literal(const syntax::LiteralExpr& literal, const sema::Type* element)
{
	// 2.14.5p8, p11: an array of const code units with the terminating
	// null, static storage duration, read-only and internal; literals of
	// one value share one object in a unit (2.14.5p12 leaves it open, the
	// course shares them). A raw literal operator's argument is the
	// number's characters (2.14.8p3).
	if ((literal.facts->flags & sema::kUserLiteral) != 0 && literal.facts->value == sema::kLiteralCallRaw)
		return predefined_name(std::string(
			literal.spelling.text, ClassifyNumber(literal.spelling.text, literal.spelling.length).suffix_begin)
				.c_str());
	sema::StringLiteralFacts facts;
	std::vector<uint64_t>& units = string_units_;
	if (!sema::StringLiteralUnits(literal.spelling, facts, units))
		throw LoweringError(literal.location, "an invalid string literal");
	// A user-defined string literal's array is its literal operator's
	// argument (2.14.8p5): of its own code units.
	if (element == nullptr && (literal.facts->flags & sema::kUserLiteral) != 0)
		element = graph_->types.fundamental(facts.element);
	const lowir::Type unit = storage_type(element != nullptr ? element : literal.facts->type->element);
	return string_object(unit, units);
}

lowir::SymbolId ProgramLowering::predefined_name(const char* text)
{
	// A function's predefined __func__ (8.4.1p8), __FUNCTION__ or
	// __PRETTY_FUNCTION__: its name's characters and the terminating null,
	// shared with string literals of the same value.
	std::vector<uint64_t>& units = string_units_;
	units.clear();
	for (const char* c = text; *c != '\0'; ++c)
		units.push_back(static_cast<unsigned char>(*c));
	units.push_back(0);
	return string_object(lowir::Type::scalar(lowir::TypeKind::I8), units);
}

lowir::SymbolId ProgramLowering::predefined_variable(const sema::VariableEntity* variable)
{
	const uint32_t known = predefined_symbols_.find(variable);
	if (known != PointerMap::kMissing)
		return known;
	const lowir::SymbolId symbol = predefined_name(static_cast<const sema::PredefinedNameEntity*>(variable)->text);
	predefined_symbols_.set(variable, symbol);
	return symbol;
}

uint64_t ProgramLowering::source_line(uint32_t location) const
{
	return analyzer_ != nullptr ? analyzer_->source_line(location) : 0;
}

std::string ProgramLowering::source_file(uint32_t location) const
{
	return analyzer_ != nullptr ? analyzer_->source_file(location) : std::string();
}

lowir::SymbolId ProgramLowering::string_object(lowir::Type unit, const std::vector<uint64_t>& units)
{
	uint64_t mixed = 0xcbf29ce484222325ull ^ static_cast<uint64_t>(unit.kind());
	for (uint64_t value : units)
		mixed = (mixed ^ value) * 0x100000001b3ull;
	const uint32_t hash = static_cast<uint32_t>(mixed ^ (mixed >> 32));
	if (string_objects_.empty())
		string_objects_.assign(64, StringObject{ 0, 0 });
	size_t mask = string_objects_.size() - 1;
	size_t probe = hash & mask;
	for (; string_objects_[probe].symbol != 0; probe = (probe + 1) & mask)
	{
		const StringObject& entry = string_objects_[probe];
		if (entry.hash == hash && same_string_object(entry.symbol - 1, unit, units))
			return entry.symbol - 1;
	}
	const std::string spelling = "__strlit__" + std::to_string(++string_literals_);
	const lowir::SymbolId symbol = new_symbol(spelling);
	string_objects_[probe] = StringObject{ hash, symbol + 1 };
	if (++string_object_count_ * 2 > string_objects_.size())
	{
		// Doubled, each entry placed again by its hash.
		std::vector<StringObject> old(string_objects_.size() * 2, StringObject{ 0, 0 });
		old.swap(string_objects_);
		mask = string_objects_.size() - 1;
		for (const StringObject& entry : old)
		{
			if (entry.symbol == 0)
				continue;
			size_t slot = entry.hash & mask;
			while (string_objects_[slot].symbol != 0)
				slot = (slot + 1) & mask;
			string_objects_[slot] = entry;
		}
	}
	lowir::Global global = lowir::Global();
	global.typed = false;
	global.init = lowir::GlobalInit::Data;
	global.first_item = static_cast<uint32_t>(program_.data_items.size());
	global.item_count = static_cast<uint32_t>(units.size());
	for (uint64_t value : units)
	{
		lowir::DataItem item = lowir::DataItem();
		item.kind = lowir::DataItemKind::Scalar;
		item.type = unit;
		item.constant = IntegerConstant(value);
		program_.data_items.push_back(item);
	}
	program_.declare_global(symbol, lowir::SymbolKind::GlobalDefinition, global);
	program_.symbols[symbol].metadata.storage = ir_model::GSM_READONLY;
	program_.symbols[symbol].metadata.binding = ir_model::SBM_INTERNAL;
	++counters_.globals;
	return symbol;
}

void ProgramLowering::finalizer_function()
{
	// 3.6.3: one `role=fini` function destroys the objects of static storage
	// duration of all units, in reverse order of their construction.
	if (finalizers_.empty())
		return;
	const lowir::SymbolId symbol = new_symbol("__cppgm_fini");
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	signature.result = lowir::Type::scalar(lowir::TypeKind::Void);
	program_.declare_function(symbol, lowir::SymbolKind::FunctionDefinition, program_.add_signature(signature));
	program_.symbols[symbol].metadata.role = ir_model::SR_FINI;
	program_.symbols[symbol].metadata.binding = ir_model::SBM_INTERNAL;
	{
		FunctionLowering body(*this, symbol, nullptr);
		for (size_t i = finalizers_.size(); i-- > 0;)
			body.finalize_global(finalizers_[i]);
		body.finish_initializer();
	}
	finalizers_.clear();
	release_definition(symbol);
	++counters_.functions;
}

FunctionLowering& ProgramLowering::initializer_function()
{
	// 3.6.2: one `role=init` function runs the dynamic initializers of all
	// units, in command-line and declaration order.
	if (!init_)
	{
		init_symbol_ = new_symbol("__cppgm_init");
		lowir::Signature signature = lowir::Signature();
		signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
		signature.result = lowir::Type::scalar(lowir::TypeKind::Void);
		program_.declare_function(
			init_symbol_, lowir::SymbolKind::FunctionDefinition, program_.add_signature(signature));
		program_.symbols[init_symbol_].metadata.role = ir_model::SR_INIT;
		program_.symbols[init_symbol_].metadata.binding = ir_model::SBM_INTERNAL;
		init_.reset(new FunctionLowering(*this, init_symbol_, nullptr));
		++counters_.functions;
	}
	return *init_;
}

} // namespace lower
