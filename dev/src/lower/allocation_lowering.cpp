#include "lower/function_lowering.h"

#include <algorithm>

#include "sema/aggregates.h"
#include "sema/conversions.h"
#include "sema/integral_arithmetic.h"

// New- and delete-expressions (5.3.4, 5.3.5) over explicit storage: the
// allocation function's call, then the object (or each element) initialized
// as the ordinary construction actions; an array of class objects whose
// destructor is not trivial (or whose class's usual operator delete[] takes
// the size) carries its element count in a cookie before its elements
// (Itanium 2.7, see array_cookie), which delete[] reads to destroy them
// last to first before the deallocation function frees the storage.
namespace lower {

namespace {

// The class of an array's elements (arrays of arrays included), or null.
const sema::ClassEntity* ElementClass(const sema::Type* type)
{
	while (type->kind == sema::TypeKind::Array)
		type = type->element;
	return sema::ClassOf(type);
}

const uint64_t kCookieBytes = 8;

bool SignedCount(const sema::Type* type)
{
	sema::IntegralKind kind;
	return sema::IntegralKindOf(type, kind) && kind.is_signed && kind.bits > 1;
}

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

// The reserved placement form, 18.6.1.3 `::operator new(std::size_t,
// void*)`, which returns its argument (a class's own function of that
// shape is no reserved form).
bool ReservedPlacement(const sema::FunctionEntity* function)
{
	const sema::Type* parameters = function->type->canonical;
	const sema::Scope* owner = function->owner;
	const bool global = !function->member && owner != nullptr && owner->kind == sema::ScopeKind::Namespace &&
		owner->entity != nullptr && static_cast<const sema::NamespaceEntity*>(owner->entity)->depth == 0;
	return global && parameters->param_count == 2 && parameters->params[1]->kind == sema::TypeKind::Pointer &&
		sema::IsVoid(parameters->params[1]->element) && parameters->params[1]->element->cv == 0;
}

} // namespace

RValue FunctionLowering::allocation(const syntax::NewExpr& expr)
{
	if (expr.count != nullptr)
		return array_allocation(expr);
	// 5.3.4: the allocation function called with the object's size (an int
	// constant converted to std::size_t, the course's form) and the
	// placement arguments; the object initialized at the address it
	// returns, which is the expression's value.
	const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(expr.facts->entity);
	const sema::Type* type = expr.facts->target;
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const uint64_t bytes = lowering_.size_of(type, expr.location);
	const RValue size = bytes <= 0x7fffffff
		? convert_op(lowir::ConvertOp::SExt, i64, integer(lowir::Type::scalar(lowir::TypeKind::I32), bytes))
		: integer(i64, bytes);
	const size_t base = arguments_.size();
	arguments_.push_back(size.value);
	const sema::Type* signature = function->type->canonical;
	for (uint32_t i = 0; i < expr.placement.size; ++i)
		push_argument(*expr.placement[i], i + 1 < signature->param_count ? signature->params[i + 1] : nullptr);
	// 5.3.4p18-p20: an initialization that may throw frees the storage by
	// the matching deallocation function, a placement one with the same
	// further arguments (kept from the allocation's).
	const sema::ClassEntity* entity = sema::ClassOf(type);
	const sema::FunctionEntity* deallocator = expr.deallocation;
	if (deallocator != nullptr && !initialization_may_throw(expr, entity))
		deallocator = nullptr;
	uint32_t placement_values = 0;
	if (deallocator != nullptr && expr.placement.size != 0)
	{
		placement_values = static_cast<uint32_t>(element_values_.size()) + 1;
		element_values_.insert(element_values_.end(), arguments_.begin() + base + 1, arguments_.end());
	}
	lowering_.demand(function, EntryVariant::Complete);
	const lowir::Value callee = lowir::Value::symbol(lowering_.function_symbol(function));
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	RValue object;
	object.value = emit_call(pointer, callee, base, lowir::kNoId, call_may_throw(function));
	object.type = pointer;
	const bool placement = ReservedPlacement(function);
	lowir::BlockId end = lowir::kNoId;
	if (tests_allocation(expr))
	{
		// (A tracked full-expression's region ends with the allocation.)
		if (tracked_.active && region_open_ && explicit_regions_ == 0)
			close_region();
		const lowir::BlockId initialize = new_block(lowir::BlockKind::NewInit);
		end = new_block(lowir::BlockKind::NewEnd);
		branch(compare(lowir::ComparePredicate::Ne, pointer, object, integer(pointer, 0)), initialize, end);
		place(initialize);
	}
	if (deallocator == nullptr || placement)
	{
		allocation_initializer(expr, type, entity, object);
		if (end != lowir::kNoId)
		{
			jump(end);
			place(end);
		}
		return object;
	}
	// The storage is one object of the alive chain while the object is
	// initialized, under the initializer's temporaries (g++ and clang
	// destroy them first); an aggregate's members are tracked (15.2p2).
	// A full-expression making temporaries whose region has not opened
	// opens it at its start, covering the allocation, and ends it here (the
	// course's form).
	const bool opened = full_expression_ && eager_pending_ && segment_temporaries_ && !region_open_ &&
		explicit_regions_ == 0 && !tracked_.active && end == lowir::kNoId;
	if (opened)
	{
		open_region(true);
		close_region();
	}
	const size_t freed = cleanups_.size();
	if (tracked_.active)
		tracked_.construction = true;
	cleanups_.push_back(Cleanup{ object.value, type, nullptr, lowir::kNoId, lowir::kNoId, lowir::kNoId, false, false,
		lowir::kNoId, false, placement_values });
	cleanups_.back().deallocator = deallocator;
	split_region();
	ElementCount* const saved_count = element_count_;
	const size_t saved_members = aggregate_members_;
	const bool saved_regions = member_regions_;
	element_count_ = nullptr;
	aggregate_members_ = entity != nullptr && expr.constructor == nullptr && sema::IsAggregateClass(entity)
		? cleanups_.size()
		: SIZE_MAX;
	member_regions_ = false;
	allocation_initializer(expr, type, entity, object);
	element_count_ = saved_count;
	aggregate_members_ = saved_members;
	member_regions_ = saved_regions;
	end_counted(freed);
	if (end != lowir::kNoId)
	{
		jump(end);
		place(end);
	}
	// The storage freed no more, the full-expression's region opens again
	// at once, landing on the objects under the initializer's temporaries
	// (their destruction inside it; the course's form).
	if (opened && full_expression_ && !region_open_ && explicit_regions_ == 0)
	{
		const lowir::BlockId entry = eh_contexts_.empty() ? unwind_entry(freed) : region_landing(freed);
		emit_marker(lowir::Opcode::EhTry, entry);
		region_open_ = true;
		eager_pending_ = false;
		region_depth_ = freed;
		region_entry_ = entry;
		++regions_opened_;
		open_serial_ = regions_opened_;
	}
	return object;
}

bool FunctionLowering::tests_allocation(const syntax::NewExpr& expr) const
{
	// 5.3.4p13: a non-throwing allocation function signals failure with
	// null, and then no initialization is done (the reserved placement
	// form returns its argument).
	const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(expr.facts->entity);
	const bool work = expr.initializer != nullptr || (expr.constructor != nullptr && !expr.constructor->trivial);
	return expr.count == nullptr && lowering_.function_nothrow(function) && !ReservedPlacement(function) && work;
}

bool FunctionLowering::initialization_may_throw(const syntax::NewExpr& expr, const sema::ClassEntity* entity)
{
	// The object's initialization may really throw (the hidden regions are
	// behavior-neutral): its constructor, the constructor's default
	// arguments, the initializer or an aggregate's omitted members (the
	// initialization helper allocation_initializer may call takes scalar
	// members only and cannot throw).
	const bool hidden = hidden_eh_;
	hidden_eh_ = false;
	const syntax::Initializer* init = expr.initializer;
	const sema::FunctionEntity* constructor = entity != nullptr ? expr.constructor : nullptr;
	bool throws = constructor != nullptr && !constructor->trivial && call_may_throw(constructor);
	if (!throws && constructor != nullptr && constructor->default_arguments != nullptr)
	{
		const uint32_t written = init != nullptr && init->kind == syntax::InitializerKind::Parenthesized
			? init->arguments.size
			: 0;
		for (uint32_t i = written; i < constructor->type->canonical->param_count && !throws; ++i)
			throws = constructor->default_arguments[i] != nullptr &&
				may_throw(lowering_.default_argument(constructor, i));
	}
	if (!throws)
		throws = initializer_may_throw(init);
	if (!throws && entity != nullptr && constructor == nullptr && sema::IsAggregateClass(entity))
		throws = omitted_may_throw(entity, 0);
	hidden_eh_ = hidden;
	return throws;
}

void FunctionLowering::allocation_initializer(
	const syntax::NewExpr& expr, const sema::Type* type, const sema::ClassEntity* entity, const RValue& object)
{
	// The allocated object's initialization (5.3.4p15). A temporary bound
	// to a reference in it ends with the full-expression (12.2p5), not a
	// local aggregate's (see local_object).
	const syntax::Initializer* init = expr.initializer;
	const sema::Entity* saved = aggregate_extension_;
	aggregate_extension_ = nullptr;
	if (entity != nullptr)
	{
		const syntax::BracedInitList* list = init != nullptr && init->kind == syntax::InitializerKind::Braced
			? static_cast<const syntax::BracedInitList*>(init->expression)
			: nullptr;
		uint32_t count = 0;
		if (list != nullptr && expr.constructor == nullptr && sema::IsAggregateClass(entity) && !list->elements.empty())
		{
			// A class aggregate by its initialization helper for the
			// members the list initializes, else member by member.
			uint32_t next = 0;
			if (helper_shape(entity, list, count))
				aggregate_helper_call(entity, count, object.value, list, next, expr.location);
			else
				structured_class(entity, type->cv, object.value, list, next, expr.location);
		}
		else if (list != nullptr && list->elements.empty() && expr.constructor == nullptr &&
			sema::IsAggregateClass(entity) && lowering_.empty_list_constructs(entity))
		{
			// 8.5.4p3: an aggregate is aggregate-initialized, each member
			// from an empty list (8.5.1p7), in place.
			const AddressPath path = AddressPath{ AddressPath::Root::Value, object.value, path_steps_.size() };
			aggregate_object(type, path, list, false, expr.location);
		}
		else if (list != nullptr && list->elements.empty())
			value_initialize(type, object.value, expr.constructor, EntryVariant::Complete, expr.location);
		else
		{
			allocation_copy_ = true;
			construct(type, object.value, init, expr.constructor, EntryVariant::Complete, expr.location);
			allocation_copy_ = false;
		}
	}
	else if (init != nullptr)
	{
		Place place = Place();
		place.storage = object.value;
		// 8.5p10, 5.3.4p15: `()` value-initializes (zero).
		if (init->kind == syntax::InitializerKind::Parenthesized && init->arguments.empty())
			store(lowering_.storage_type(type), zero_value(type), place);
		else
			initialize_storage(type, place, init, expr.location);
	}
	aggregate_extension_ = saved;
}

FunctionLowering::ArrayNewShape FunctionLowering::array_new_shape(const syntax::NewExpr& expr)
{
	const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(expr.facts->entity);
	ArrayNewShape shape = ArrayNewShape();
	shape.per_element = 1;
	shape.element = expr.facts->target;
	for (; shape.element->kind == sema::TypeKind::Array; shape.element = shape.element->element)
		shape.per_element *= shape.element->bound;
	shape.element_size = lowering_.size_of(shape.element, expr.location);
	shape.stride = shape.element_size * shape.per_element;
	shape.cookie = array_cookie(shape.element, (expr.facts->flags & sema::kSizedArrayDelete) != 0, function);
	// Itanium 2.7: the cookie is max(sizeof(size_t), alignof(T)) bytes, the
	// count in the size_t just before the elements.
	shape.cookie_bytes = shape.cookie ? std::max(kCookieBytes, lowering_.alignment_of(shape.element, expr.location))
									  : 0;
	shape.count = WithoutParentheses(expr.count);
	const sema::ExprFacts* facts = shape.count->facts;
	shape.constant = (facts->flags & sema::kFolded) != 0 || shape.count->kind == syntax::ExprKind::Literal;
	shape.value = shape.constant ? facts->value : 0;
	shape.count_type = facts->conversion != nullptr && facts->conversion->result != nullptr ? facts->conversion->result
																							: facts->type->unqualified;
	// A braced list's elements (the analysis's array type of them, see
	// Analyzer::array_new_list), in the first bound's elements.
	const syntax::Initializer* init = expr.initializer;
	const syntax::BracedInitList* list = init != nullptr && init->kind == syntax::InitializerKind::Braced
		? static_cast<const syntax::BracedInitList*>(init->expression)
		: nullptr;
	shape.listed = list != nullptr && list->facts != nullptr && list->facts->type->kind == sema::TypeKind::Array
		? list->facts->type->bound
		: 0;
	// The largest count whose size, cookie included, std::ptrdiff_t holds
	// (5.3.4p7's implementation-defined limit, as g++ and the course take
	// it).
	shape.largest = shape.stride != 0 ? (uint64_t(INT64_MAX) - shape.cookie_bytes) / shape.stride : uint64_t(INT64_MAX);
	return shape;
}

bool FunctionLowering::count_may_be_invalid(const ArrayNewShape& shape, uint64_t& limit) const
{
	// A signed count's negative values are invalid (huge once widened to
	// std::size_t; an __int128's compared whole, see array_allocation, as
	// are its values beyond std::size_t's range).
	const sema::Type* type = shape.constant ? shape.count->facts->type : shape.count_type;
	const bool is_signed = SignedCount(type);
	const uint64_t bits = lowering_.size_of(type, shape.count->location) * 8;
	limit = shape.largest;
	if (shape.constant)
		return (is_signed && bits <= 64 && static_cast<int64_t>(shape.value) < 0) || shape.value > limit ||
			shape.value < shape.listed;
	if (shape.listed != 0 || is_signed)
		return true;
	return bits >= 64 || ((uint64_t(1) << bits) - 1) > limit;
}

bool FunctionLowering::length_may_throw(const syntax::NewExpr& expr)
{
	// (A non-throwing allocation function's form yields null instead, see
	// array_allocation.)
	uint64_t limit = 0;
	const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(expr.facts->entity);
	return expr.count != nullptr && !(lowering_.function_nothrow(function) && !ReservedPlacement(function)) &&
		count_may_be_invalid(array_new_shape(expr), limit);
}

void FunctionLowering::invalid_length(const RValue* invalid)
{
	// 5.3.4p7: no storage is obtained; std::bad_array_new_length is thrown
	// by the C++ ABI's __cxa_throw_bad_array_new_length (the LowIR runtime
	// role bad_array_new_length), a throwing call (unwinding through the
	// enclosing cleanups and handlers) that does not return: the (dead)
	// code after the call continues as after a throw-expression's, or the
	// invalid path ends in `unreachable` (nothing it computed stays live
	// across the call), the region the call opened ending first. A
	// full-expression's pending region opens before the branch (not later,
	// at a call begun before it, where only the invalid path would see it).
	const lowir::Type void_type = lowir::Type::scalar(lowir::TypeKind::Void);
	lowir::BlockId valid = lowir::kNoId;
	if (invalid != nullptr)
	{
		before_throwing_call();
		const lowir::BlockId fail = new_block(lowir::BlockKind::ArrayNewLengthInvalid);
		valid = new_block(lowir::BlockKind::ArrayNewLengthValid);
		branch(*invalid, fail, valid);
		place(fail);
	}
	emit_call(void_type, lowir::Value::symbol(lowering_.runtime_function(RuntimeFunction::BadArrayNewLength)),
		arguments_.size(), lowir::kNoId, true);
	if (valid == lowir::kNoId)
		return;
	terminate(lowir::make_instruction(lowir::Opcode::Unreachable, void_type));
	place(valid);
}

RValue FunctionLowering::array_allocation(const syntax::NewExpr& expr)
{
	// 5.3.4p10, p12: the elements' size, a cookie's added (array_cookie),
	// as a std::size_t: a constant count's as an immediate, another's
	// computed from the count widened to std::size_t and kept for the
	// cookie. Then the elements default-initialized in order, a
	// constructor's exception destroying those constructed and freeing the
	// storage (5.3.4p18), or value-initialized (scalars zeroed byte by
	// byte).
	const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(expr.facts->entity);
	const sema::Type* type = expr.facts->target;
	const ArrayNewShape shape = array_new_shape(expr);
	const uint64_t per_element = shape.per_element;
	const sema::Type* element = shape.element;
	const uint64_t element_size = shape.element_size;
	const uint64_t stride = shape.stride;
	const sema::ClassEntity* entity = ElementClass(type);
	const bool cookie = shape.cookie;
	const uint64_t cookie_bytes = shape.cookie_bytes;
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const bool constant = shape.constant;
	const syntax::Initializer* init = expr.initializer;
	const syntax::BracedInitList* list = init != nullptr && init->kind == syntax::InitializerKind::Braced
		? static_cast<const syntax::BracedInitList*>(init->expression)
		: nullptr;
	const uint64_t listed = shape.listed;
	// 5.3.4p7: a negative count, one whose size (the cookie's included)
	// exceeds the limit (shape.largest), or one below the elements a braced
	// list initializes obtains no storage: the new-expression throws
	// std::bad_array_new_length before the allocation function is called
	// (invalid_length); with a non-throwing allocation function its value
	// is null, the function not called (CWG 1992, C++17 [expr.new]p8, as
	// g++ and clang give `new (std::nothrow) S[-1]`).
	uint64_t limit = 0;
	const bool checked = count_may_be_invalid(shape, limit);
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	const bool nothrow = lowering_.function_nothrow(function) && !ReservedPlacement(function);
	lowir::BlockId end = lowir::kNoId;
	Place value_slot = Place();
	if (nothrow && checked)
	{
		value_slot.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Call, pointer));
		end = new_block(lowir::BlockKind::NewEnd);
	}
	const auto null_length = [&](const RValue& invalid) {
		const lowir::BlockId fail = new_block(lowir::BlockKind::ArrayNewLengthInvalid);
		const lowir::BlockId valid = new_block(lowir::BlockKind::ArrayNewLengthValid);
		branch(invalid, fail, valid);
		place(fail);
		store(pointer, integer(pointer, 0), value_slot);
		jump(end);
		place(valid);
	};
	uint64_t count_value = 0;
	lowir::SlotId size_slot = lowir::kNoId;
	RValue size;
	if (constant)
	{
		// The size as a std::size_t immediate.
		count_value = shape.value;
		if (checked && nothrow)
			null_length(compare(lowir::ComparePredicate::Eq, i64, integer(i64, 0), integer(i64, 0)));
		else if (checked)
			invalid_length(nullptr);
		size = integer(i64, checked ? ~uint64_t(0) : count_value * stride + cookie_bytes);
	}
	else
	{
		// 5.3.4p10: computed in std::size_t, the count widened by its
		// promoted type's signedness (pa12's 400-trivial-class-array-new,
		// which computed a u32 count's size in 32 bits, is corrected: loop 55
		// C1).
		const sema::Type* count_type = shape.count_type;
		const RValue promoted = operand(*expr.count, ConversionContext::Operand);
		const unsigned bits = lowir::integer_bits(promoted.type.kind());
		const RValue count = bits == 64
			? promoted
			: convert_integer(promoted, count_type, i64, ConversionContext::Operand, false, false);
		// A count wider than std::size_t (an __int128's) is compared whole:
		// its values beyond std::size_t's range (or negative) are invalid.
		const bool wide = bits > 64;
		const lowir::Type compared = wide ? promoted.type : i64;
		const RValue& full = wide ? promoted : count;
		const auto bound = [&](uint64_t value) {
			return wide ? integer(compared, lowir::Literal{ value, 0 }) : integer(i64, value);
		};
		if (checked)
		{
			// Valid from the listed elements (or 0) to `limit`: one unsigned
			// comparison (a count below the listed ones wraps, a negative
			// one is huge once widened).
			const RValue over = listed != 0 ? binary(lowir::BinaryOp::Sub, compared, full, bound(listed)) : full;
			const RValue invalid = compare(lowir::ComparePredicate::Ugt, compared, over, bound(limit - listed));
			if (nothrow)
				null_length(invalid);
			else
				invalid_length(&invalid);
		}
		size = count;
		if (stride != 1)
			size = binary(lowir::BinaryOp::Mul, i64, size, integer(i64, stride));
		if (cookie_bytes != 0)
			size = binary(lowir::BinaryOp::Add, i64, size, integer(i64, cookie_bytes));
		Place slot = Place();
		size_slot = generated_slot(lowir::SlotKind::ArrayNewSize, i64);
		slot.storage = lowir::Value::slot(size_slot);
		store(i64, size, slot);
	}
	const size_t base = arguments_.size();
	arguments_.push_back(size.value);
	const sema::Type* signature = function->type->canonical;
	for (uint32_t i = 0; i < expr.placement.size; ++i)
		push_argument(*expr.placement[i], i + 1 < signature->param_count ? signature->params[i + 1] : nullptr);
	// A placement deallocation function takes the same further arguments
	// (5.3.4p20).
	uint32_t placement_values = 0;
	if (expr.deallocation != nullptr && expr.placement.size != 0)
	{
		placement_values = static_cast<uint32_t>(element_values_.size()) + 1;
		element_values_.insert(element_values_.end(), arguments_.begin() + base + 1, arguments_.end());
	}
	lowering_.demand(function, EntryVariant::Complete);
	RValue raw;
	raw.type = pointer;
	raw.value = emit_call(pointer, lowir::Value::symbol(lowering_.function_symbol(function)), base, lowir::kNoId,
		call_may_throw(function));
	// 5.3.4p13: a non-throwing allocation function's null is the value,
	// with no cookie and no initialization (the reserved placement form
	// returns its argument).
	if (nothrow)
	{
		if (end == lowir::kNoId)
			value_slot.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Call, pointer));
		store(pointer, raw, value_slot);
		const lowir::BlockId initialize = new_block(lowir::BlockKind::NewInit);
		if (end == lowir::kNoId)
			end = new_block(lowir::BlockKind::NewEnd);
		branch(compare(lowir::ComparePredicate::Ne, pointer, raw, integer(pointer, 0)), initialize, end);
		place(initialize);
	}
	RValue known_size;
	if (size_slot != lowir::kNoId)
	{
		Place slot = Place();
		slot.storage = lowir::Value::slot(size_slot);
		known_size = load(i64, slot);
	}
	RValue elements = raw;
	RValue count;
	if (cookie)
	{
		// The count just before the elements.
		elements = index(
			lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, raw.value, integer(i64, cookie_bytes));
		if (constant)
			count = constant_value(i64, count_value * per_element);
		else
		{
			const RValue element_bytes = binary(lowir::BinaryOp::Sub, i64, known_size, integer(i64, cookie_bytes));
			count = binary(lowir::BinaryOp::UDiv, i64, element_bytes, integer(i64, element_size));
		}
		Place header = Place();
		header.storage = cookie_bytes == kCookieBytes
			? raw.value
			: index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, raw.value,
				  integer(i64, cookie_bytes - kCookieBytes))
				  .value;
		store(i64, count, header);
	}
	else if (entity != nullptr && !constant)
		count = binary(lowir::BinaryOp::UDiv, i64, known_size, integer(i64, element_size));
	// `()` value-initializes the elements, a braced list those after the
	// ones it initializes (from empty lists, 8.5.1p7: the same here).
	const bool value_init = (init != nullptr && init->kind == syntax::InitializerKind::Parenthesized &&
								init->arguments.empty()) ||
		list != nullptr;
	const sema::FunctionEntity* constructor = expr.constructor;
	const bool constructs = entity != nullptr && constructor != nullptr && !constructor->trivial &&
		!does_nothing(constructor);
	const uint64_t listed_elements = listed * per_element;
	const bool trailing = !constant || count_value * per_element > listed_elements;
	size_t freed = SIZE_MAX;
	RValue rest = elements;
	if (listed != 0)
	{
		const bool hidden = hidden_eh_;
		hidden_eh_ = false;
		const bool rest_throws = trailing && constructs && call_may_throw(constructor);
		hidden_eh_ = hidden;
		freed = new_list_elements(expr, *list, elements, raw, size, placement_values, rest_throws);
		rest = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, elements.value,
			integer(i64, listed * stride));
	}
	// The value-initialized elements' bytes (the request less the cookie
	// and the listed elements, as the course computes them) and number.
	const uint64_t listed_bytes = listed * stride;
	const auto rest_bytes = [&]() {
		RValue bytes = constant ? integer(i64, count_value * stride + cookie_bytes) : known_size;
		if (cookie_bytes != 0)
			bytes = binary(lowir::BinaryOp::Sub, i64, bytes, integer(i64, cookie_bytes));
		return listed_bytes != 0 ? binary(lowir::BinaryOp::Sub, i64, bytes, integer(i64, listed_bytes)) : bytes;
	};
	const auto rest_count = [&]() {
		if (constant)
			return constant_value(i64, count_value * per_element - listed_elements);
		const RValue all = cookie || entity != nullptr
			? count
			: binary(lowir::BinaryOp::UDiv, i64, known_size, integer(i64, element_size));
		return listed_elements != 0 ? binary(lowir::BinaryOp::Sub, i64, all, integer(i64, listed_elements)) : all;
	};
	if (trailing && value_init &&
		(entity == nullptr || constructor == nullptr || constructor->implicit || constructor->defaulted))
	{
		// 8.5p7: scalars, and class elements without a user-provided
		// default constructor, are zero-initialized (first).
		zero_bytes(rest, rest_bytes());
	}
	if (trailing && value_init && lowering_.holds_data_member_pointer(element) &&
		(entity == nullptr || constructor == nullptr || constructor->implicit || constructor->defaulted))
	{
		// Zero-initialized pointers to data members are -1 (Itanium 2.3).
		null_member_pointer_elements(rest, element, rest_count(), expr.location);
	}
	// A zero-length array keeps its (empty) construction loop (the course's
	// form, as for a count known only at run time).
	if ((trailing || (constant && count_value == 0)) && constructs)
		construct_new_elements(element, rest, rest_count(), raw, size, constructor,
			freed != SIZE_MAX ? nullptr : expr.deallocation, expr.location, freed != SIZE_MAX ? 0 : placement_values);
	if (freed != SIZE_MAX)
		end_counted(freed);
	if (end == lowir::kNoId)
		return elements;
	store(pointer, elements, value_slot);
	jump(end);
	place(end);
	return load(pointer, value_slot);
}

size_t FunctionLowering::new_list_elements(const syntax::NewExpr& expr, const syntax::BracedInitList& list,
	const RValue& elements, const RValue& raw, const RValue& size, uint32_t placement, bool rest_throws)
{
	// 5.3.4p15: the elements a braced list initializes, an array of their
	// number made at the allocation by an aggregate's actions. While they
	// (or the value-initialized ones after them, `rest_throws`) may throw,
	// the storage is an object of the alive chain freed by the deallocation
	// function (5.3.4p18; a sized one given the allocation's size, kept in
	// element_values_ as a placement one's arguments are), and the elements
	// made are destroyed (15.2p2): counted while made, then as a whole.
	// Their initialization may throw from the list's clauses, or from an
	// omitted member's or element's initialization from an empty list
	// (8.5.1p7: its default constructor, an aggregate's members), as the
	// single object's (initialization_may_throw). Returns the chain's
	// position to drop once every element is made (SIZE_MAX: nothing was
	// pushed).
	const sema::Type* prefix = list.facts->type;
	const sema::ClassEntity* entity = ElementClass(prefix);
	const bool hidden = hidden_eh_;
	hidden_eh_ = false;
	const bool throws = may_throw(&list) || rest_throws || (entity != nullptr && omitted_may_throw(entity, 0));
	hidden_eh_ = hidden;
	size_t freed = SIZE_MAX;
	if (throws && expr.deallocation != nullptr)
	{
		uint32_t saved = placement;
		if (saved == 0)
		{
			element_values_.push_back(size.value);
			saved = static_cast<uint32_t>(element_values_.size());
		}
		freed = cleanups_.size();
		cleanups_.push_back(Cleanup{ raw.value, prefix, nullptr, lowir::kNoId, lowir::kNoId, lowir::kNoId, false, false,
			lowir::kNoId, false, saved });
		cleanups_.back().deallocator = expr.deallocation;
		split_region();
	}
	// A temporary bound to a reference in it ends with the full-expression
	// (12.2p5), as an allocated aggregate's (allocation_initializer).
	const sema::Entity* const saved_extension = aggregate_extension_;
	ElementCount* const saved_count = element_count_;
	const size_t saved_members = aggregate_members_;
	const bool saved_regions = member_regions_;
	aggregate_extension_ = nullptr;
	element_count_ = nullptr;
	aggregate_members_ = freed != SIZE_MAX ? cleanups_.size() : SIZE_MAX;
	member_regions_ = false;
	const AddressPath path = AddressPath{ AddressPath::Root::Value, elements.value, path_steps_.size() };
	aggregate_array(prefix, path, &list, expr.location);
	aggregate_extension_ = saved_extension;
	element_count_ = saved_count;
	aggregate_members_ = saved_members;
	member_regions_ = saved_regions;
	return freed;
}

bool FunctionLowering::array_cookie(const sema::Type* element, bool sized, const sema::FunctionEntity* allocation) const
{
	const sema::ClassEntity* entity = ElementClass(element);
	if (entity == nullptr)
		return false;
	// The reserved placement form: a namespace-scope operator new[] taking
	// (std::size_t, void*) (GCC's test too).
	if (allocation != nullptr && !allocation->member)
	{
		const sema::Type* signature = allocation->type->canonical;
		const sema::Type* second = signature->param_count == 2 ? signature->params[1] : nullptr;
		if (second != nullptr && !signature->variadic && second->kind == sema::TypeKind::Pointer && second->cv == 0 &&
			second->element->kind == sema::TypeKind::Fundamental && second->element->cv == 0 &&
			second->element->fundamental == FundamentalType::Void)
			return false;
	}
	return sized || (entity->destructor != nullptr && !entity->destructor->trivial);
}

RValue FunctionLowering::constant_value(lowir::Type type, uint64_t bits)
{
	return constant(type, bits);
}

void FunctionLowering::zero_bytes(const RValue& storage, const RValue& size)
{
	// 8.5p7 for scalar elements: a host object calls memset, as the host
	// compilers do at every optimization level (a byte loop over a large
	// array took several times as long); the course's form zeroes each
	// byte in a loop.
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	if (lowering_.host_objects())
	{
		const size_t base = arguments_.size();
		arguments_.push_back(storage.value);
		arguments_.push_back(integer(lowir::Type::scalar(lowir::TypeKind::I32), 0).value);
		arguments_.push_back(size.value);
		emit_call(lowir::Type::scalar(lowir::TypeKind::Ptr), lowir::Value::symbol(lowering_.memset_function()), base,
			lowir::kNoId, false);
		return;
	}
	Place offset_slot = Place();
	offset_slot.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::ZeroinitOffset, i64));
	store(i64, integer(i64, 0), offset_slot);
	const lowir::BlockId condition = new_block(lowir::BlockKind::ZeroinitCond);
	const lowir::BlockId body = new_block(lowir::BlockKind::ZeroinitBody);
	const lowir::BlockId end = new_block(lowir::BlockKind::ZeroinitEnd);
	jump(condition);
	place(condition);
	const RValue offset = load(i64, offset_slot);
	branch(compare(lowir::ComparePredicate::Ult, i64, offset, size), body, end);
	place(body);
	Place byte = Place();
	byte.storage = index(i8, ir_model::IPK_NONE, storage.value, offset).value;
	store(i8, integer(i8, 0), byte);
	store(i64, binary(lowir::BinaryOp::Add, i64, offset, integer(i64, 1)), offset_slot);
	jump(condition);
	place(end);
}

void FunctionLowering::null_member_pointer_elements(
	const RValue& elements, const sema::Type* element, const RValue& count, uint32_t location)
{
	// Each zeroed element's pointers to data members set to their null
	// value (-1, Itanium 2.3) in a loop over an index slot.
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const uint64_t size = lowering_.size_of(element, location);
	Place index_slot = Place();
	index_slot.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::NullMemberIndex, i64));
	store(i64, integer(i64, 0), index_slot);
	const lowir::BlockId condition = new_block(lowir::BlockKind::NullMemberCond);
	const lowir::BlockId body = new_block(lowir::BlockKind::NullMemberBody);
	const lowir::BlockId end = new_block(lowir::BlockKind::NullMemberEnd);
	jump(condition);
	place(condition);
	const RValue i = load(i64, index_slot);
	branch(compare(lowir::ComparePredicate::Ult, i64, i, count), body, end);
	place(body);
	const lowir::Value at =
		index(i8, ir_model::IPK_NONE, elements.value, binary(lowir::BinaryOp::Mul, i64, i, integer(i64, size))).value;
	null_data_member_pointers(element, at, 0, true);
	store(i64, binary(lowir::BinaryOp::Add, i64, i, integer(i64, 1)), index_slot);
	jump(condition);
	place(end);
}

void FunctionLowering::construct_new_elements(const sema::Type* element, const RValue& elements, const RValue& count,
	const RValue& raw, const RValue& size_argument, const sema::FunctionEntity* constructor,
	const sema::FunctionEntity* deallocator, uint32_t location, uint32_t placement)
{
	// Each element by its default constructor in a loop over an index
	// slot; a constructor that may throw runs in a region whose handler
	// destroys the elements before the index, last first, frees the
	// storage and unwinds.
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const uint64_t size = lowering_.size_of(element, location);
	Place index_slot = Place();
	index_slot.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::ArrayNewIndex, i64));
	store(i64, integer(i64, 0), index_slot);
	const lowir::BlockId condition = new_block(lowir::BlockKind::ArrayNewCtorCond);
	const lowir::BlockId body = new_block(lowir::BlockKind::ArrayNewCtorBody);
	const lowir::BlockId end = new_block(lowir::BlockKind::ArrayNewCtorEnd);
	// A default argument that may throw guards the elements too (see
	// element_temporaries).
	bool guarded = false;
	const bool temporaries = element_temporaries(constructor, &guarded);
	const lowir::BlockId cleanup = guarded ? new_block(lowir::BlockKind::ArrayNewCtorCleanup) : lowir::kNoId;
	const lowir::BlockId after = new_block(lowir::BlockKind::ArrayNewCtorCont);
	jump(condition);
	place(condition);
	const RValue current = load(i64, index_slot);
	branch(compare(lowir::ComparePredicate::Ult, i64, current, count), body, end);
	place(body);
	const lowir::Value object = index(
		i8, ir_model::IPK_NONE, elements.value, binary(lowir::BinaryOp::Mul, i64, current, integer(i64, size)))
									.value;
	// 5.3.4p18, 15.2p1: unwinding destroys the constructed elements and
	// frees the storage (the new-expression's cleanup) before the objects
	// alive around the new-expression: their region encloses the element's.
	// Outside a full-expression's segment and a construction, the region
	// opened here covers the element alone, as a call's (emit_call): one
	// left open would be pushed again by each iteration, and not at all
	// with no elements.
	EhContext attempt = EhContext();
	bool dispatched = false;
	const bool transient = guarded && !full_expression_ && !construction_ && !region_open_;
	if (guarded)
	{
		before_throwing_call();
		dispatched = landing_try(attempt);
		emit_marker(lowir::Opcode::EhTry, cleanup);
	}
	// The temporaries an element's construction makes end with it (12.2p4,
	// see begin_element_temporaries).
	const ElementScope scope = begin_element_temporaries(temporaries);
	construct(element, object, nullptr, constructor, EntryVariant::Complete, location);
	end_element_temporaries(scope);
	if (guarded)
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	if (transient)
		close_region();
	store(i64, binary(lowir::BinaryOp::Add, i64, current, integer(i64, 1)), index_slot);
	jump(condition);
	place(end);
	jump(after);
	if (guarded)
	{
		place(cleanup);
		if (dispatched)
			landing_dispatch(attempt);
		const RValue constructed = load(i64, index_slot);
		if (needs_destruction(element))
		{
			Place dtor_slot = Place();
			dtor_slot.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::ArrayDtorIndex, i64));
			store(i64, constructed, dtor_slot);
			const lowir::BlockId dtor_condition = new_block(lowir::BlockKind::ArrayDtorCond);
			const lowir::BlockId dtor_body = new_block(lowir::BlockKind::ArrayDtorBody);
			const lowir::BlockId dtor_end = new_block(lowir::BlockKind::ArrayDtorEnd);
			jump(dtor_condition);
			place(dtor_condition);
			const RValue left = load(i64, dtor_slot);
			branch(compare(lowir::ComparePredicate::Ne, i64, left, integer(i64, 0)), dtor_body, dtor_end);
			place(dtor_body);
			const RValue previous = binary(lowir::BinaryOp::Sub, i64, left, integer(i64, 1));
			store(i64, previous, dtor_slot);
			destroy(element,
				index(i8, ir_model::IPK_NONE, elements.value,
					binary(lowir::BinaryOp::Mul, i64, previous, integer(i64, size)))
					.value,
				location);
			jump(dtor_condition);
			place(dtor_end);
		}
		// 5.3.4p20: no placement deallocation function matching, none.
		if (deallocator != nullptr)
			free_storage(deallocator, raw.value, size_argument, location, placement);
		if (dispatched)
			landing_exit(attempt);
		else
			emit_resume();
	}
	place(after);
}

void FunctionLowering::free_storage(const sema::FunctionEntity* deallocator, lowir::Value raw, const RValue& size,
	uint32_t location, uint32_t placement)
{
	// 5.3.4p18-p20: the deallocation function matching the allocation
	// function, as the analysis found it (a sized one also with the size
	// the allocation function was given, 3.7.4.2p2; a placement one with
	// the allocation's placement arguments).
	if (deallocator == nullptr)
		throw LoweringError(location, "no deallocation function matches the allocation function");
	const size_t base = arguments_.size();
	arguments_.push_back(raw);
	const uint32_t count = deallocator->type->canonical->param_count;
	for (uint32_t i = 1; i < count; ++i)
		arguments_.push_back(placement != 0 ? element_values_[placement - 2 + i] : size.value);
	lowering_.demand(deallocator, EntryVariant::Complete);
	emit_call(lowir::Type::scalar(lowir::TypeKind::Void), lowir::Value::symbol(lowering_.function_symbol(deallocator)),
		base, lowir::kNoId, false);
}

void FunctionLowering::destroy_deleted(const sema::ClassEntity* entity, lowir::Value object)
{
	// A deleted object's destructor: a throwing one leaves by the enclosing
	// objects' region (15.2p1; the deleted object is none of them).
	const sema::FunctionEntity* destructor = entity->destructor;
	const bool throws = call_may_throw(destructor);
	begin_call(throws);
	const size_t base = arguments_.size();
	arguments_.push_back(object);
	lowering_.demand(destructor, EntryVariant::Complete);
	emit_call(lowir::Type::scalar(lowir::TypeKind::Void),
		lowir::Value::symbol(lowering_.entry_symbol(destructor, EntryVariant::Complete)), base, lowir::kNoId, throws);
	end_call(throws);
}

size_t FunctionLowering::deleted_storage(
	const sema::FunctionEntity* deallocator, lowir::Value storage, const sema::Type* object, lowir::Value block)
{
	// 5.3.5p7: the deallocation function is called even when the destructor
	// (or an element's) exits by an exception: while it runs the storage is
	// an object of the alive chain (as a new-expression's, see allocation),
	// freed after the elements left (15.2p2) by a sized function with the
	// object's size, or with an array's block size (kept in element_values_).
	uint32_t saved = 0;
	if (block.kind != lowir::ValueKind::None)
	{
		element_values_.push_back(block);
		saved = static_cast<uint32_t>(element_values_.size());
	}
	const size_t freed = cleanups_.size();
	cleanups_.push_back(Cleanup{
		storage, object, nullptr, lowir::kNoId, lowir::kNoId, lowir::kNoId, false, false, lowir::kNoId, false, saved });
	cleanups_.back().deallocator = deallocator;
	split_region();
	return freed;
}

RValue FunctionLowering::deletion(const syntax::DeleteExpr& expr)
{
	// 5.3.5: a null pointer deletes nothing (the test only for a class
	// object or an array with a cookie, the course's form); the object's
	// destructor (each element's, last first, their count read from the
	// cookie), then the deallocation function with the storage (a sized one
	// also with the object's size). A destructor that may throw runs with
	// the storage, and the elements left, alive objects of the chain (see
	// deleted_storage).
	const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(expr.facts->entity);
	// An array's elements are its innermost ones (their count in the
	// cookie).
	const sema::Type* object = expr.facts->target;
	while (expr.array && object->kind == sema::TypeKind::Array)
		object = object->element;
	// An incomplete class's object is freed alone (5.3.5p5).
	const bool incomplete = (expr.facts->flags & sema::kIncompleteDelete) != 0;
	const sema::ClassEntity* entity = incomplete ? nullptr : sema::ClassOf(object);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	const RValue value = operand(*expr.operand, ConversionContext::Operand);
	RValue none;
	none.type = lowir::Type::scalar(lowir::TypeKind::Void);
	none.value = lowir::Value::none();
	const bool cookie = expr.array && entity != nullptr &&
		array_cookie(object, (expr.facts->flags & sema::kSizedArrayDelete) != 0, nullptr);
	lowir::BlockId end = lowir::kNoId;
	if (entity != nullptr && (cookie || !expr.array))
	{
		const lowir::BlockId nonnull = new_block(
			expr.array ? lowir::BlockKind::ArrayDeleteNonnull : lowir::BlockKind::DeleteNonnull);
		end = new_block(expr.array ? lowir::BlockKind::ArrayDeleteEnd : lowir::BlockKind::DeleteEnd);
		branch(compare(lowir::ComparePredicate::Ne, pointer, value, integer(pointer, 0)), nonnull, end);
		place(nonnull);
	}
	if (!expr.array && entity != nullptr && entity->destructor != nullptr && entity->destructor->is_virtual)
	{
		virtual_deletion(entity, value);
		jump(end);
		place(end);
		return none;
	}
	lowir::Value storage = value.value;
	const uint64_t size = incomplete ? 0 : lowering_.size_of(object, expr.location);
	const bool sized = function->type->canonical->param_count == 2;
	RValue block = integer(i64, size);
	size_t freed = SIZE_MAX;
	if (cookie)
	{
		// The cookie before the elements (Itanium 2.7, as array_allocation
		// lays it out), the count in its last size_t.
		const uint64_t cookie_bytes = std::max(kCookieBytes, lowering_.alignment_of(object, expr.location));
		storage = index(i8, ir_model::IPK_NONE, value.value, integer(i64, 0 - cookie_bytes)).value;
		Place header = Place();
		header.storage = cookie_bytes == kCookieBytes
			? storage
			: index(i8, ir_model::IPK_NONE, value.value, integer(i64, 0 - kCookieBytes)).value;
		const RValue count = load(i64, header);
		// 12.5p5: a sized operator delete[] is given the block's size.
		if (sized)
		{
			const RValue element_bytes = binary(lowir::BinaryOp::Mul, i64, count, integer(i64, size));
			block = binary(lowir::BinaryOp::Add, i64, element_bytes, integer(i64, cookie_bytes));
		}
		if (needs_destruction(object))
		{
			Place index_slot = Place();
			index_slot.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::ArrayDeleteIndex, i64));
			store(i64, count, index_slot);
			if (destructor_may_throw(object))
			{
				freed = deleted_storage(function, storage, object, sized ? block.value : lowir::Value::none());
				cleanups_.push_back(Cleanup{ value.value, object, nullptr, lowir::kNoId, lowir::kNoId, lowir::kNoId,
					false, false, index_slot.storage.index });
				cleanups_.back().counted = true;
				split_region();
			}
			const lowir::BlockId condition = new_block(lowir::BlockKind::ArrayDeleteDtorCond);
			const lowir::BlockId body = new_block(lowir::BlockKind::ArrayDeleteDtorBody);
			const lowir::BlockId done = new_block(lowir::BlockKind::ArrayDeleteDtorEnd);
			jump(condition);
			place(condition);
			const RValue left = load(i64, index_slot);
			branch(compare(lowir::ComparePredicate::Ne, i64, left, integer(i64, 0)), body, done);
			place(body);
			const RValue previous = binary(lowir::BinaryOp::Sub, i64, left, integer(i64, 1));
			store(i64, previous, index_slot);
			destroy_deleted(entity,
				index(i8, ir_model::IPK_NONE, value.value,
					binary(lowir::BinaryOp::Mul, i64, previous, integer(i64, size)))
					.value);
			jump(condition);
			place(done);
		}
	}
	else if (entity != nullptr && needs_destruction(object))
	{
		if (destructor_may_throw(object))
			freed = deleted_storage(function, storage, object, lowir::Value::none());
		destroy_deleted(entity, value.value);
	}
	if (freed != SIZE_MAX)
		end_counted(freed);
	const size_t base = arguments_.size();
	arguments_.push_back(storage);
	if (sized)
		arguments_.push_back(block.value);
	lowering_.demand(function, EntryVariant::Complete);
	emit_call(lowir::Type::scalar(lowir::TypeKind::Void), lowir::Value::symbol(lowering_.function_symbol(function)),
		base, lowir::kNoId, call_may_throw(function));
	if (end != lowir::kNoId)
	{
		jump(end);
		place(end);
	}
	return none;
}

} // namespace lower
