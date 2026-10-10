#include "native/link/runtime_support.h"

#include <cstring>

#include "native/runtime/runtime.h"

namespace native {

using lowir::BinaryOp;
using lowir::BlockId;
using lowir::ComparePredicate;
using lowir::SlotId;
using lowir::SymbolId;
using lowir::Value;

namespace {

struct SupportName
{
	const char* name;
	SupportMember member;
};

const SupportName kSupportNames[] = {
	{ "__cppgm_allocate", kAllocation },
	{ "__cppgm_release", kAllocation },
	{ "cppgm_builtin_operator_new", kAllocation },
	{ "cppgm_builtin_operator_new_array", kAllocation },
	{ "cppgm_builtin_operator_delete", kAllocation },
	{ "cppgm_builtin_operator_delete_array", kAllocation },
	{ "_Znwm", kAllocation },
	{ "_Znam", kAllocation },
	{ "_ZdlPv", kAllocation },
	{ "_ZdaPv", kAllocation },
	{ "_ZdlPvm", kAllocation },
	{ "_ZdaPvm", kAllocation },
	{ "__cppgm_operator_new", kAllocation },
	{ "malloc", kAllocation },
	{ "free", kAllocation },
	{ "calloc", kAllocation },
	{ "realloc", kAllocation },
	{ "atexit", kExitHandlers },
	{ "__cxa_atexit", kExitHandlers },
	{ "__cxa_thread_atexit", kExitHandlers },
	{ "__cxa_guard_acquire", kExitHandlers },
	{ "__cxa_guard_release", kExitHandlers },
	{ "__cxa_guard_abort", kExitHandlers },
	{ "__cxa_allocate_exception", kExceptions },
	{ "__cxa_free_exception", kExceptions },
	{ "__cxa_throw", kExceptions },
	{ "__cxa_begin_catch", kExceptions },
	{ "__cxa_end_catch", kExceptions },
	{ "__cxa_rethrow", kExceptions },
	{ "__cxa_get_exception_ptr", kExceptions },
	{ kEhCatchSymbol, kExceptions },
	{ "__dynamic_cast", kDynamicCast },
	{ "__cxa_bad_cast", kBadCast },
	{ "__cxa_bad_typeid", kBadCast },
	{ "__cxa_throw_bad_array_new_length", kBadCast },
	{ "__divti3", kWideDivision },
	{ "__udivti3", kWideDivision },
	{ "__modti3", kWideDivision },
	{ "__umodti3", kWideDivision },
	{ "__floattisf", kFloatConversion },
	{ "__floattidf", kFloatConversion },
	{ "__floattixf", kFloatConversion },
	{ "__floatuntisf", kFloatConversion },
	{ "__floatuntidf", kFloatConversion },
	{ "__floatuntixf", kFloatConversion },
	{ "__fixsfti", kFloatConversion },
	{ "__fixdfti", kFloatConversion },
	{ "__fixxfti", kFloatConversion },
	{ "__fixunssfti", kFloatConversion },
	{ "__fixunsdfti", kFloatConversion },
	{ "__fixunsxfti", kFloatConversion },
};

const char* const kAllocateNames[] = { "cppgm_builtin_operator_new", "_Znwm" };
const char* const kReleaseNames[] = { "cppgm_builtin_operator_delete", "_ZdlPv", "_ZdlPvm", "_ZdaPvm" };
// The array forms' defaults and the scalar forms they call (the course's
// names and the Itanium ones).
struct ArrayForm
{
	const char* array;
	const char* scalar;
	bool allocates;
};
const ArrayForm kArrayForms[] = {
	{ "cppgm_builtin_operator_new_array", "cppgm_builtin_operator_new", true },
	{ "_Znam", "_Znwm", true },
	{ "cppgm_builtin_operator_delete_array", "cppgm_builtin_operator_delete", false },
	{ "_ZdaPv", "_ZdlPv", false },
};

// Size classes: 16-byte steps up to kSmallLimit, powers of two from 2 KiB
// up to kMediumLimit; each class's free list is one 8-byte head.
const int64_t kSmallLimit = 1024;
const int64_t kMediumLimit = 131072;
const int64_t kChunkBytes = 1048576;
const int64_t kHeaderBytes = 16;
// Requests from here on (the user half of the address space) fail before
// the size rounding can wrap.
const int64_t kSizeLimit = int64_t(1) << 47;

// The exception header (Itanium 2.4.2's role, this runtime's layout).
const int64_t kExceptionHeader = 64;
enum : int64_t
{
	kNextCaught = 0,
	kNextInFlight = 8,
	kThrownType = 16,
	kDestructor = 24,
	kAdjusted = 32,
	kHandlers = 40,
	kSelector = 48
};

// Itanium 2.9.5: type_info {vptr, name}; __si_class_type_info adds the base;
// __vmi_class_type_info {flags, count} and {base, offset_flags} entries;
// __pbase_type_info {flags, pointee}; __pointer_to_member_type_info adds
// the class.
enum : int64_t
{
	kTypeName = 8,
	kSiBase = 16,
	kVmiCount = 20,
	kVmiBases = 24,
	kPointerFlags = 16,
	kPointee = 24,
	kMemberClass = 32
};

// The type_info class kinds, in runtime_library.cpp's vtable order.
enum TypeKindCode : int64_t
{
	kClassType = 1,
	kSiClassType,
	kVmiClassType,
	kFundamentalType,
	kPointerType,
	kMemberPointerType,
	kFunctionType,
	kArrayType,
	kEnumType
};

const char* const kVtableNames[] = {
	"_ZTVN10__cxxabiv117__class_type_infoE",
	"_ZTVN10__cxxabiv120__si_class_type_infoE",
	"_ZTVN10__cxxabiv121__vmi_class_type_infoE",
	"_ZTVN10__cxxabiv123__fundamental_type_infoE",
	"_ZTVN10__cxxabiv119__pointer_type_infoE",
	"_ZTVN10__cxxabiv129__pointer_to_member_type_infoE",
	"_ZTVN10__cxxabiv120__function_type_infoE",
	"_ZTVN10__cxxabiv117__array_type_infoE",
	"_ZTVN10__cxxabiv116__enum_type_infoE",
};

// Routines shared by the members, declared once per program.
class Support
{
public:
	explicit Support(RuntimeProgram& runtime) : runtime_(runtime) {}

	void allocation();
	void exit_handlers();
	void exceptions();
	void dynamic_cast_member();
	void bad_cast();
	void wide_division();
	void float_conversion();

private:
	SymbolId fn(const char* name, Kind result, std::initializer_list<Kind> parameters, bool define)
	{
		return runtime_.function(name, runtime_.signature(result, parameters), define);
	}
	SymbolId allocate() { return fn("__cppgm_allocate", Kind::Ptr, { Kind::I64 }, false); }
	SymbolId release() { return fn("__cppgm_release", Kind::Void, { Kind::Ptr }, false); }
	SymbolId terminate() { return fn("_ZSt9terminatev", Kind::Void, {}, false); }
	void size_class(RoutineBuilder& r, Value total, SlotId rounded, SlotId offset);
	void copy_words(RoutineBuilder& r, Value to, Value from, Value bytes);
	void c_allocation(SymbolId allocate_symbol, SymbolId release_symbol);
	void type_kind();
	void type_equal();
	void find_base();
	void upcast();
	void do_catch();
	void walk();
	void throw_runtime_error(const char* routine, const char* type, const char* name, const char* vtable,
		const char* base_type = nullptr, const char* base_name = nullptr);
	void decompose(RoutineBuilder& r, Kind kind, Value x, Value& significand, Value& top, Value& lowest);

	RuntimeProgram& runtime_;
};

// ------------------------------------------------------------ allocation

// The power-of-two class of a medium block: `rounded` (from 2 KiB) and the
// byte offset of its free list head.
void Support::size_class(RoutineBuilder& r, Value total, SlotId rounded, SlotId offset)
{
	r.write(Kind::I64, rounded, r.integer(2048));
	r.write(Kind::I64, offset, r.integer(0));
	const BlockId test = r.block();
	const BlockId step = r.block();
	const BlockId done = r.block();
	r.jump(test);
	r.enter(test);
	r.branch(r.compare(ComparePredicate::Ult, Kind::I64, r.read(Kind::I64, rounded), total), step, done);
	r.enter(step);
	r.write(Kind::I64, rounded, r.binary(BinaryOp::Shl, Kind::I64, r.read(Kind::I64, rounded), r.integer(1)));
	r.write(Kind::I64, offset, r.add(Kind::I64, r.read(Kind::I64, offset), r.integer(8)));
	r.jump(test);
	r.enter(done);
}

void Support::allocation()
{
	const SymbolId next = runtime_.global("__cppgm_heap_next", 8);
	const SymbolId end = runtime_.global("__cppgm_heap_end", 8);
	const SymbolId small = runtime_.global("__cppgm_free_small", 8 * (kSmallLimit / 16 + 1));
	const SymbolId medium = runtime_.global("__cppgm_free_medium", 8 * 8);
	const SymbolId mmap = fn("__cppgm_mmap", Kind::Ptr, { Kind::I64 }, false);
	const SymbolId munmap = fn("__cppgm_munmap", Kind::Void, { Kind::Ptr, Kind::I64 }, false);
	const SymbolId allocate_symbol = fn("__cppgm_allocate", Kind::Ptr, { Kind::I64 }, true);
	{
		RoutineBuilder r(runtime_.program(), allocate_symbol);
		const SlotId total = r.local(Kind::I64);
		const SlotId rounded = r.local(Kind::I64);
		const SlotId offset = r.local(Kind::I64);
		const SlotId list = r.local(Kind::Ptr);
		const BlockId empty = r.block(), small_block = r.block(), larger = r.block(), medium_block = r.block();
		const BlockId pop = r.block(), bump = r.block(), carve = r.block(), refill = r.block(), large = r.block();
		const BlockId exhausted = r.block(), mapped = r.block(), refilled = r.block(), possible = r.block();
		r.branch(r.compare(ComparePredicate::Uge, Kind::I64, r.param(0), r.integer(kSizeLimit)), exhausted, possible);
		r.enter(possible);
		Value size = r.binary(BinaryOp::And, Kind::I64, r.add(Kind::I64, r.param(0), r.integer(15)), r.integer(-16));
		r.write(Kind::I64, total, size);
		r.branch(r.equal(Kind::I64, size, r.integer(0)), empty, small_block);
		r.enter(empty);
		r.write(Kind::I64, total, r.integer(16));
		r.jump(small_block);
		// Small: the 16-byte class's list.
		r.enter(small_block);
		size = r.read(Kind::I64, total);
		const BlockId small_list = r.block();
		r.branch(r.compare(ComparePredicate::Ule, Kind::I64, size, r.integer(kSmallLimit)), small_list, larger);
		r.enter(small_list);
		const Value small_lists = r.address(small);
		r.write(Kind::Ptr, list, r.at(small_lists, r.binary(BinaryOp::UShr, Kind::I64, size, r.integer(1))));
		r.jump(pop);
		r.enter(larger);
		r.branch(r.compare(ComparePredicate::Ugt, Kind::I64, size, r.integer(kMediumLimit)), large, medium_block);
		r.enter(medium_block);
		size_class(r, size, rounded, offset);
		r.write(Kind::I64, total, r.read(Kind::I64, rounded));
		const Value medium_lists = r.address(medium);
		r.write(Kind::Ptr, list, r.at(medium_lists, r.read(Kind::I64, offset)));
		r.jump(pop);
		// A freed block of the class, else carved from the current mapping.
		r.enter(pop);
		const Value head = r.load(Kind::Ptr, r.read(Kind::Ptr, list));
		const BlockId reuse = r.block();
		r.branch(r.equal(Kind::Ptr, head, r.integer(0)), bump, reuse);
		r.enter(reuse);
		const Value rest = r.load(Kind::Ptr, head);
		r.store(Kind::Ptr, rest, r.read(Kind::Ptr, list));
		r.ret(Kind::Ptr, head);
		r.enter(bump);
		const Value start = r.load(Kind::Ptr, r.address(next));
		const Value limit = r.add(Kind::I64, r.read(Kind::I64, total), r.integer(kHeaderBytes));
		const Value after = r.at(start, limit);
		const BlockId fits = r.block();
		r.branch(r.equal(Kind::Ptr, start, r.integer(0)), refill, fits);
		r.enter(fits);
		r.branch(r.compare(ComparePredicate::Ugt, Kind::Ptr, after, r.load(Kind::Ptr, r.address(end))), refill, carve);
		r.enter(carve);
		r.store(Kind::Ptr, after, r.address(next));
		r.store(Kind::I64, r.read(Kind::I64, total), start);
		r.ret(Kind::Ptr, r.at(start, kHeaderBytes));
		r.enter(refill);
		const Value chunk = r.call(Kind::Ptr, mmap, { r.integer(kChunkBytes) });
		r.branch(r.equal(Kind::Ptr, chunk, r.integer(0)), exhausted, refilled);
		r.enter(refilled);
		r.store(Kind::Ptr, chunk, r.address(next));
		const Value chunk_end = r.at(chunk, kChunkBytes);
		r.store(Kind::Ptr, chunk_end, r.address(end));
		r.jump(bump);
		// Large: a mapping of its own.
		r.enter(large);
		const Value block = r.call(Kind::Ptr, mmap, { r.add(Kind::I64, size, r.integer(kHeaderBytes)) });
		r.branch(r.equal(Kind::Ptr, block, r.integer(0)), exhausted, mapped);
		r.enter(mapped);
		r.store(Kind::I64, size, block);
		r.ret(Kind::Ptr, r.at(block, kHeaderBytes));
		r.enter(exhausted);
		r.ret(Kind::Ptr, r.integer(0));
	}
	const SymbolId release_symbol = fn("__cppgm_release", Kind::Void, { Kind::Ptr }, true);
	{
		RoutineBuilder r(runtime_.program(), release_symbol);
		const SlotId rounded = r.local(Kind::I64);
		const SlotId offset = r.local(Kind::I64);
		const SlotId list = r.local(Kind::Ptr);
		const BlockId some = r.block(), small_block = r.block(), larger = r.block(), medium_block = r.block();
		const BlockId push = r.block(), large = r.block(), none = r.block();
		r.branch(r.equal(Kind::Ptr, r.param(0), r.integer(0)), none, some);
		r.enter(none);
		r.ret();
		r.enter(some);
		const Value header = r.at(r.param(0), -kHeaderBytes);
		const Value size = r.load(Kind::I64, header);
		r.branch(r.compare(ComparePredicate::Ule, Kind::I64, size, r.integer(kSmallLimit)), small_block, larger);
		r.enter(small_block);
		const Value small_lists = r.address(small);
		r.write(Kind::Ptr, list, r.at(small_lists, r.binary(BinaryOp::UShr, Kind::I64, size, r.integer(1))));
		r.jump(push);
		r.enter(larger);
		r.branch(r.compare(ComparePredicate::Ugt, Kind::I64, size, r.integer(kMediumLimit)), large, medium_block);
		r.enter(medium_block);
		size_class(r, size, rounded, offset);
		const Value medium_lists = r.address(medium);
		r.write(Kind::Ptr, list, r.at(medium_lists, r.read(Kind::I64, offset)));
		r.jump(push);
		r.enter(push);
		r.store(Kind::Ptr, r.load(Kind::Ptr, r.read(Kind::Ptr, list)), r.param(0));
		r.store(Kind::Ptr, r.param(0), r.read(Kind::Ptr, list));
		r.ret();
		r.enter(large);
		r.call(Kind::Void, munmap, { header, r.add(Kind::I64, size, r.integer(kHeaderBytes)) });
		r.ret();
	}
	// Operator new: exhaustion terminates (no std::bad_alloc).
	const SymbolId operator_new = fn("__cppgm_operator_new", Kind::Ptr, { Kind::I64 }, true);
	{
		RoutineBuilder r(runtime_.program(), operator_new);
		const BlockId failed = r.block(), done = r.block();
		const Value block = r.call(Kind::Ptr, allocate_symbol, { r.param(0) });
		r.branch(r.equal(Kind::Ptr, block, r.integer(0)), failed, done);
		r.enter(failed);
		r.call(Kind::Void, terminate(), {});
		r.unreachable();
		r.enter(done);
		r.ret(Kind::Ptr, block);
	}
	for (const char* name : kAllocateNames)
		runtime_.alias(name, operator_new);
	for (const char* name : kReleaseNames)
		runtime_.alias(name, release_symbol);
	// A default array form calls its scalar form (18.6.1.2p4, p13), which a
	// program may replace (17.6.4.6): a reference to the scalar name the
	// linker resolves, in place of the alias.
	for (const ArrayForm& form : kArrayForms)
	{
		const bool allocates = form.allocates;
		const Kind result = allocates ? Kind::Ptr : Kind::Void;
		const SymbolId scalar = fn(form.scalar, result, { allocates ? Kind::I64 : Kind::Ptr }, false);
		const SymbolId array = fn(form.array, result, { allocates ? Kind::I64 : Kind::Ptr }, true);
		RoutineBuilder r(runtime_.program(), array);
		const Value value = r.call(result, scalar, { r.param(0) });
		if (allocates)
			r.ret(Kind::Ptr, value);
		else
			r.ret();
	}
	runtime_.alias("malloc", allocate_symbol);
	runtime_.alias("free", release_symbol);
	c_allocation(allocate_symbol, release_symbol);
}

// Copies or clears `bytes` (a multiple of 8) 8 bytes at a time.
void Support::copy_words(RoutineBuilder& r, Value to, Value from, Value bytes)
{
	const SlotId at = r.local(Kind::I64);
	r.write(Kind::I64, at, r.integer(0));
	const BlockId test = r.block(), body = r.block(), done = r.block();
	r.jump(test);
	r.enter(test);
	r.branch(r.compare(ComparePredicate::Ult, Kind::I64, r.read(Kind::I64, at), bytes), body, done);
	r.enter(body);
	const Value offset = r.read(Kind::I64, at);
	const Value word = from.kind == lowir::ValueKind::None ? r.integer(0) : r.load(Kind::I64, r.at(from, offset));
	r.store(Kind::I64, word, r.at(to, offset));
	r.write(Kind::I64, at, r.add(Kind::I64, offset, r.integer(8)));
	r.jump(test);
	r.enter(done);
}

// calloc(count, size): zeroed (block sizes are multiples of 16; a large
// block is a fresh mapping, zero already), null on overflow;
// realloc(block, size): a block of at least `size` bytes with the old
// contents, the same block when it is large enough, a large block grown
// by remapping it (no copy).
void Support::c_allocation(SymbolId allocate_symbol, SymbolId release_symbol)
{
	{
		RoutineBuilder r(runtime_.program(), fn("calloc", Kind::Ptr, { Kind::I64, Kind::I64 }, true));
		const BlockId check = r.block(), overflow = r.block(), sized = r.block(), failed = r.block(), clear = r.block();
		const Value bytes = r.binary(BinaryOp::Mul, Kind::I64, r.param(0), r.param(1));
		r.branch(r.equal(Kind::I64, r.param(1), r.integer(0)), sized, check);
		r.enter(check);
		r.branch(
			r.equal(Kind::I64, r.binary(BinaryOp::UDiv, Kind::I64, bytes, r.param(1)), r.param(0)), sized, overflow);
		r.enter(overflow);
		r.ret(Kind::Ptr, r.integer(0));
		r.enter(sized);
		const Value block = r.call(Kind::Ptr, allocate_symbol, { bytes });
		r.branch(r.equal(Kind::Ptr, block, r.integer(0)), failed, clear);
		r.enter(failed);
		r.ret(Kind::Ptr, r.integer(0));
		r.enter(clear);
		const Value block_bytes = r.load(Kind::I64, block, -kHeaderBytes);
		const BlockId reused = r.block(), zeroed = r.block();
		r.branch(r.compare(ComparePredicate::Ugt, Kind::I64, block_bytes, r.integer(kMediumLimit)), zeroed, reused);
		r.enter(reused);
		copy_words(r, block, Value::none(), block_bytes);
		r.jump(zeroed);
		r.enter(zeroed);
		r.ret(Kind::Ptr, block);
	}
	{
		RoutineBuilder r(runtime_.program(), fn("realloc", Kind::Ptr, { Kind::Ptr, Kind::I64 }, true));
		const BlockId fresh = r.block(), existing = r.block(), grow = r.block(), keep = r.block(), failed = r.block();
		const BlockId copy = r.block();
		r.branch(r.equal(Kind::Ptr, r.param(0), r.integer(0)), fresh, existing);
		r.enter(fresh);
		r.ret(Kind::Ptr, r.call(Kind::Ptr, allocate_symbol, { r.param(1) }));
		r.enter(existing);
		const Value old_bytes = r.load(Kind::I64, r.param(0), -kHeaderBytes);
		r.branch(r.compare(ComparePredicate::Ule, Kind::I64, r.param(1), old_bytes), keep, grow);
		r.enter(keep);
		r.ret(Kind::Ptr, r.param(0));
		r.enter(grow);
		const SymbolId mremap = fn("__cppgm_mremap", Kind::Ptr, { Kind::Ptr, Kind::I64, Kind::I64 }, false);
		const BlockId sized = r.block(), remap = r.block(), move = r.block(), remapped = r.block();
		r.branch(r.compare(ComparePredicate::Ugt, Kind::I64, old_bytes, r.integer(kMediumLimit)), sized, move);
		r.enter(sized);
		r.branch(r.compare(ComparePredicate::Ult, Kind::I64, r.param(1), r.integer(kSizeLimit)), remap, move);
		r.enter(remap);
		const Value new_bytes = r.binary(
			BinaryOp::And, Kind::I64, r.add(Kind::I64, r.param(1), r.integer(15)), r.integer(-16));
		const Value header = r.at(r.param(0), -kHeaderBytes);
		const Value mapping = r.call(Kind::Ptr, mremap,
			{ header, r.add(Kind::I64, old_bytes, r.integer(kHeaderBytes)),
				r.add(Kind::I64, new_bytes, r.integer(kHeaderBytes)) });
		r.branch(r.equal(Kind::Ptr, mapping, r.integer(0)), failed, remapped);
		r.enter(remapped);
		r.store(Kind::I64, new_bytes, mapping);
		r.ret(Kind::Ptr, r.at(mapping, kHeaderBytes));
		r.enter(move);
		const Value block = r.call(Kind::Ptr, allocate_symbol, { r.param(1) });
		r.branch(r.equal(Kind::Ptr, block, r.integer(0)), failed, copy);
		r.enter(failed);
		r.ret(Kind::Ptr, r.integer(0));
		r.enter(copy);
		copy_words(r, block, r.param(0), old_bytes);
		r.call(Kind::Void, release_symbol, { r.param(0) });
		r.ret(Kind::Ptr, block);
	}
}

// --------------------------------------------------------- exit handlers

void Support::exit_handlers()
{
	// Itanium 3.3.2 guards (single-threaded): acquire answers whether the
	// object still needs initializing (byte 0 clear), release sets byte 0,
	// abort leaves it clear.
	{
		RoutineBuilder r(runtime_.program(), fn("__cxa_guard_acquire", Kind::I32, { Kind::Ptr }, true));
		const BlockId done = r.block(), pending = r.block();
		r.branch(r.load(Kind::I8, r.param(0)), done, pending);
		r.enter(done);
		r.ret(Kind::I32, r.integer(0));
		r.enter(pending);
		r.ret(Kind::I32, r.integer(1));
	}
	{
		RoutineBuilder r(runtime_.program(), fn("__cxa_guard_release", Kind::Void, { Kind::Ptr }, true));
		r.store(Kind::I8, r.integer(1), r.param(0));
		r.ret();
	}
	{
		RoutineBuilder r(runtime_.program(), fn("__cxa_guard_abort", Kind::Void, { Kind::Ptr }, true));
		r.ret();
	}
	// atexit's and __cxa_atexit's list, and __cxa_thread_atexit's (the one
	// thread's objects, which __cppgm_exit destroys first, 3.6.3p1).
	const SymbolId head = runtime_.global("__cppgm_atexit_head", 0);
	const SymbolId thread_head = runtime_.global("__cppgm_thread_atexit_head", 0);
	const SymbolId symbols[3] = { fn("atexit", Kind::I32, { Kind::Ptr }, true),
		fn("__cxa_atexit", Kind::I32, { Kind::Ptr, Kind::Ptr, Kind::Ptr }, true),
		fn("__cxa_thread_atexit", Kind::I32, { Kind::Ptr, Kind::Ptr, Kind::Ptr }, true) };
	for (unsigned k = 0; k < 3; ++k)
	{
		// {function, next, argument}, most recent first.
		const SymbolId list = k == 2 ? thread_head : head;
		RoutineBuilder r(runtime_.program(), symbols[k]);
		const Value node = r.call(Kind::Ptr, allocate(), { r.integer(24) });
		r.store(Kind::Ptr, r.param(0), node);
		const Value next = r.load(Kind::Ptr, r.address(list));
		r.store(Kind::Ptr, next, node, 8);
		r.store(Kind::Ptr, k == 0 ? r.integer(0) : r.param(1), node, 16);
		r.store(Kind::Ptr, node, r.address(list));
		r.ret(Kind::I32, r.integer(0));
	}
}

// ------------------------------------------------------ type information

// __cppgm_type_kind(type): its TypeKindCode by its vtable, 0 for others.
void Support::type_kind()
{
	if (runtime_.defined("__cppgm_type_kind"))
		return;
	const SymbolId symbol = fn("__cppgm_type_kind", Kind::I64, { Kind::Ptr }, true);
	RoutineBuilder r(runtime_.program(), symbol);
	const Value vptr = r.load(Kind::Ptr, r.param(0));
	for (unsigned k = 0; k < sizeof kVtableNames / sizeof kVtableNames[0]; ++k)
	{
		const BlockId found = r.block();
		const BlockId next = r.block();
		const Value expected = r.at(r.address(runtime_.global(kVtableNames[k], 0)), 16);
		r.branch(r.equal(Kind::Ptr, vptr, expected), found, next);
		r.enter(found);
		r.ret(Kind::I64, r.integer(static_cast<int64_t>(k) + 1));
		r.enter(next);
	}
	r.ret(Kind::I64, r.integer(0));
}

// __cppgm_type_equal(a, b): the same object, or equal names unless one
// starts with '*' (a type with internal linkage, compared by address).
void Support::type_equal()
{
	if (runtime_.defined("__cppgm_type_equal"))
		return;
	const SymbolId symbol = fn("__cppgm_type_equal", Kind::I64, { Kind::Ptr, Kind::Ptr }, true);
	RoutineBuilder r(runtime_.program(), symbol);
	const SlotId a = r.local(Kind::Ptr);
	const SlotId b = r.local(Kind::Ptr);
	const BlockId same = r.block(), differ = r.block(), names = r.block(), unique = r.block(), loop = r.block();
	const BlockId step = r.block(), check_b = r.block(), compare_names = r.block(), ended = r.block();
	r.branch(r.equal(Kind::Ptr, r.param(0), r.param(1)), same, names);
	r.enter(names);
	const Value name_a = r.load(Kind::Ptr, r.param(0), kTypeName);
	const Value name_b = r.load(Kind::Ptr, r.param(1), kTypeName);
	r.write(Kind::Ptr, a, name_a);
	r.write(Kind::Ptr, b, name_b);
	r.branch(r.equal(Kind::Ptr, name_a, name_b), same, unique);
	r.enter(unique);
	r.branch(r.equal(Kind::I8, r.load(Kind::I8, name_a), r.integer('*')), differ, check_b);
	r.enter(check_b);
	r.branch(r.equal(Kind::I8, r.load(Kind::I8, name_b), r.integer('*')), differ, loop);
	r.enter(loop);
	const Value x = r.load(Kind::I8, r.read(Kind::Ptr, a));
	const Value y = r.load(Kind::I8, r.read(Kind::Ptr, b));
	r.branch(r.equal(Kind::I8, x, y), compare_names, differ);
	r.enter(compare_names);
	r.branch(r.equal(Kind::I8, x, r.integer(0)), ended, step);
	r.enter(step);
	r.write(Kind::Ptr, a, r.at(r.read(Kind::Ptr, a), 1));
	r.write(Kind::Ptr, b, r.at(r.read(Kind::Ptr, b), 1));
	r.jump(loop);
	r.enter(ended);
	r.jump(same);
	r.enter(same);
	r.ret(Kind::I64, r.integer(1));
	r.enter(differ);
	r.ret(Kind::I64, r.integer(0));
}

// __cppgm_find_base(type, object, public, real): records every subobject
// of the search target's type in an object of `type` at `object`: the
// first address, whether it is reached through public bases only (by any
// path), and ambiguity (a second address). Virtual base offsets come from
// the object's vtable. Without a real object (`real` 0: a null pointer's
// conversion), subobjects get distinct stand-in addresses: offsets from
// `object`, a virtual base its type information's address shifted.
void Support::find_base()
{
	const SymbolId target = runtime_.global("__cppgm_search_target", 8);
	const SymbolId count = runtime_.global("__cppgm_search_count", 8);
	const SymbolId found = runtime_.global("__cppgm_search_address", 8);
	const SymbolId reached = runtime_.global("__cppgm_search_public", 8);
	const SymbolId ambiguous = runtime_.global("__cppgm_search_ambiguous", 8);
	const SymbolId kind = fn("__cppgm_type_kind", Kind::I64, { Kind::Ptr }, false);
	const SymbolId equal = fn("__cppgm_type_equal", Kind::I64, { Kind::Ptr, Kind::Ptr }, false);
	const SymbolId symbol = fn("__cppgm_find_base", Kind::Void, { Kind::Ptr, Kind::Ptr, Kind::I64, Kind::I64 }, true);
	RoutineBuilder r(runtime_.program(), symbol);
	const Value type = r.param(0), object = r.param(1), open = r.param(2), real = r.param(3);
	const BlockId match = r.block(), bases = r.block(), first = r.block(), again = r.block(), other = r.block();
	const BlockId same = r.block(), done = r.block();
	r.branch(r.call(Kind::I64, equal, { type, r.load(Kind::Ptr, r.address(target)) }), match, bases);
	r.enter(match);
	r.branch(r.equal(Kind::I64, r.load(Kind::I64, r.address(count)), r.integer(0)), first, again);
	r.enter(first);
	r.store(Kind::I64, r.integer(1), r.address(count));
	r.store(Kind::Ptr, object, r.address(found));
	r.store(Kind::I64, open, r.address(reached));
	r.ret();
	r.enter(again);
	r.branch(r.equal(Kind::Ptr, r.load(Kind::Ptr, r.address(found)), object), same, other);
	r.enter(other);
	r.store(Kind::I64, r.integer(1), r.address(ambiguous));
	r.ret();
	r.enter(same);
	const Value now_open = r.binary(BinaryOp::Or, Kind::I64, r.load(Kind::I64, r.address(reached)), open);
	r.store(Kind::I64, now_open, r.address(reached));
	r.ret();
	r.enter(bases);
	const Value code = r.call(Kind::I64, kind, { type });
	const BlockId single = r.block(), multiple = r.block(), not_single = r.block();
	r.branch(r.equal(Kind::I64, code, r.integer(kSiClassType)), single, not_single);
	r.enter(single);
	r.call(Kind::Void, symbol, { r.load(Kind::Ptr, type, kSiBase), object, open, real });
	r.ret();
	r.enter(not_single);
	r.branch(r.equal(Kind::I64, code, r.integer(kVmiClassType)), multiple, done);
	r.enter(multiple);
	const SlotId index = r.local(Kind::I64);
	const SlotId child = r.local(Kind::Ptr);
	r.write(Kind::I64, index, r.integer(0));
	const Value total = r.convert(lowir::ConvertOp::ZExt, Kind::I64, Kind::U32, r.load(Kind::U32, type, kVmiCount));
	const BlockId test = r.block(), body = r.block(), located = r.block(), present = r.block(),
				  virtual_base = r.block();
	const BlockId direct = r.block(), stand_in = r.block(), stand_in_virtual = r.block();
	r.jump(test);
	r.enter(test);
	r.branch(r.compare(ComparePredicate::Ult, Kind::I64, r.read(Kind::I64, index), total), body, done);
	r.enter(body);
	const Value bases_start = r.at(type, kVmiBases);
	const Value entry = r.at(bases_start, r.binary(BinaryOp::Mul, Kind::I64, r.read(Kind::I64, index), r.integer(16)));
	const Value flags = r.load(Kind::I64, entry, 8);
	const Value offset = r.binary(BinaryOp::Shr, Kind::I64, flags, r.integer(8));
	const Value is_virtual = r.binary(BinaryOp::And, Kind::I64, flags, r.integer(1));
	r.branch(real, present, stand_in);
	r.enter(stand_in);
	r.branch(is_virtual, stand_in_virtual, direct);
	r.enter(stand_in_virtual);
	r.write(Kind::Ptr, child, r.binary(BinaryOp::Shl, Kind::I64, r.load(Kind::Ptr, entry), r.integer(16)));
	r.jump(located);
	r.enter(present);
	r.branch(is_virtual, virtual_base, direct);
	r.enter(virtual_base);
	r.write(Kind::Ptr, child, r.at(object, r.load(Kind::I64, r.at(r.load(Kind::Ptr, object), offset))));
	r.jump(located);
	r.enter(direct);
	r.write(Kind::Ptr, child, r.at(object, offset));
	r.jump(located);
	r.enter(located);
	const Value inherited = r.binary(BinaryOp::And, Kind::I64, open,
		r.binary(BinaryOp::UShr, Kind::I64, r.binary(BinaryOp::And, Kind::I64, flags, r.integer(2)), r.integer(1)));
	r.call(Kind::Void, symbol, { r.load(Kind::Ptr, entry), r.read(Kind::Ptr, child), inherited, real });
	r.write(Kind::I64, index, r.add(Kind::I64, r.read(Kind::I64, index), r.integer(1)));
	r.jump(test);
	r.enter(done);
	r.ret();
}

// __cppgm_upcast(type, target, object_address): converts *object_address
// (an object of `type`) to its unambiguous public `target` base.
void Support::upcast()
{
	const SymbolId find = fn("__cppgm_find_base", Kind::Void, { Kind::Ptr, Kind::Ptr, Kind::I64, Kind::I64 }, false);
	const SymbolId symbol = fn("__cppgm_upcast", Kind::I64, { Kind::Ptr, Kind::Ptr, Kind::Ptr }, true);
	RoutineBuilder r(runtime_.program(), symbol);
	const SymbolId target = runtime_.global("__cppgm_search_target", 8);
	const SymbolId count = runtime_.global("__cppgm_search_count", 8);
	const SymbolId found = runtime_.global("__cppgm_search_address", 8);
	const SymbolId reached = runtime_.global("__cppgm_search_public", 8);
	const SymbolId ambiguous = runtime_.global("__cppgm_search_ambiguous", 8);
	r.store(Kind::Ptr, r.param(1), r.address(target));
	r.store(Kind::I64, r.integer(0), r.address(count));
	r.store(Kind::I64, r.integer(0), r.address(reached));
	r.store(Kind::I64, r.integer(0), r.address(ambiguous));
	const Value object = r.load(Kind::Ptr, r.param(2));
	const Value real = r.compare(ComparePredicate::Ne, Kind::Ptr, object, r.integer(0));
	r.call(Kind::Void, find,
		{ r.param(0), object, r.integer(1), r.convert(lowir::ConvertOp::ZExt, Kind::I64, Kind::I1, real) });
	const BlockId some = r.block(), unique = r.block(), open = r.block(), fail = r.block();
	r.branch(r.load(Kind::I64, r.address(count)), some, fail);
	r.enter(some);
	r.branch(r.load(Kind::I64, r.address(ambiguous)), fail, unique);
	r.enter(unique);
	r.branch(r.load(Kind::I64, r.address(reached)), open, fail);
	r.enter(open);
	// A null pointer stays null.
	const BlockId converted = r.block(), null = r.block();
	r.branch(real, converted, null);
	r.enter(converted);
	r.store(Kind::Ptr, r.load(Kind::Ptr, r.address(found)), r.param(2));
	r.jump(null);
	r.enter(null);
	r.ret(Kind::I64, r.integer(1));
	r.enter(fail);
	r.ret(Kind::I64, r.integer(0));
}

// __cppgm_do_catch(handler, thrown, object_address, outer): libsupc++'s
// __do_catch: whether a handler of type `handler` catches `thrown` with
// *object_address the (pointer-adjusted) exception, converting it: equal
// types; a class handler and an unambiguous public base (not below a
// pointer of two or more levels); a pointer or pointer-to-member handler
// and nullptr_t; matching pointers whose qualification conversion is
// valid (`outer` bit 0: every outer level const), a void pointee taking
// any object pointer at the first level.
void Support::do_catch()
{
	const SymbolId kind = fn("__cppgm_type_kind", Kind::I64, { Kind::Ptr }, false);
	const SymbolId equal = fn("__cppgm_type_equal", Kind::I64, { Kind::Ptr, Kind::Ptr }, false);
	const SymbolId up = fn("__cppgm_upcast", Kind::I64, { Kind::Ptr, Kind::Ptr, Kind::Ptr }, false);
	const SymbolId symbol = fn("__cppgm_do_catch", Kind::I64, { Kind::Ptr, Kind::Ptr, Kind::Ptr, Kind::I64 }, true);
	const SymbolId nullptr_type = runtime_.global("_ZTIDn", 0);
	const SymbolId void_type = runtime_.global("_ZTIv", 0);
	// The null data member pointer is -1, the offset representation's
	// (Itanium 2.3); the null member function pointer is two zero words.
	const SymbolId null_data_member = runtime_.words("__cppgm_null_data_member", { DataWord{ lowir::kNoId, -1 } });
	const SymbolId null_function_member = runtime_.words(
		"__cppgm_null_function_member", { DataWord{ lowir::kNoId, 0 }, DataWord{ lowir::kNoId, 0 } });
	RoutineBuilder r(runtime_.program(), symbol);
	const Value handler = r.param(0), thrown = r.param(1), object = r.param(2);
	const SlotId outer = r.local(Kind::I64);
	r.write(Kind::I64, outer, r.param(3));
	const BlockId yes = r.block(), no = r.block(), differ = r.block(), class_handler = r.block();
	const BlockId not_class = r.block(), shallow = r.block(), class_thrown = r.block(), pointerish = r.block();
	r.branch(r.call(Kind::I64, equal, { handler, thrown }), yes, differ);
	r.enter(differ);
	const Value handler_kind = r.call(Kind::I64, kind, { handler });
	r.branch(
		r.compare(ComparePredicate::Ule, Kind::I64, handler_kind, r.integer(kVmiClassType)), class_handler, not_class);
	r.enter(class_handler);
	const BlockId known = r.block();
	r.branch(r.equal(Kind::I64, handler_kind, r.integer(0)), no, known);
	r.enter(known);
	r.branch(r.compare(ComparePredicate::Uge, Kind::I64, r.read(Kind::I64, outer), r.integer(4)), no, shallow);
	r.enter(shallow);
	const Value thrown_kind = r.call(Kind::I64, kind, { thrown });
	const BlockId thrown_known = r.block();
	r.branch(r.compare(ComparePredicate::Ule, Kind::I64, thrown_kind, r.integer(kVmiClassType)), thrown_known, no);
	r.enter(thrown_known);
	r.branch(r.equal(Kind::I64, thrown_kind, r.integer(0)), no, class_thrown);
	r.enter(class_thrown);
	r.ret(Kind::I64, r.call(Kind::I64, up, { thrown, handler, object }));
	// Pointers and pointers to members.
	r.enter(not_class);
	const BlockId pointer_like = r.block();
	const Value is_pointer = r.equal(Kind::I64, handler_kind, r.integer(kPointerType));
	const Value is_member = r.equal(Kind::I64, handler_kind, r.integer(kMemberPointerType));
	r.branch(r.binary(BinaryOp::Or, Kind::I64, is_pointer, is_member), pointer_like, no);
	r.enter(pointer_like);
	const BlockId from_null = r.block(), not_null = r.block();
	r.branch(r.call(Kind::I64, equal, { thrown, r.address(nullptr_type) }), from_null, not_null);
	r.enter(from_null);
	{
		const BlockId to_pointer = r.block(), to_member = r.block(), to_function = r.block(), to_data = r.block();
		r.branch(is_pointer, to_pointer, to_member);
		r.enter(to_pointer);
		r.store(Kind::Ptr, r.integer(0), object);
		r.ret(Kind::I64, r.integer(1));
		r.enter(to_member);
		const Value pointee_kind = r.call(Kind::I64, kind, { r.load(Kind::Ptr, handler, kPointee) });
		r.branch(r.equal(Kind::I64, pointee_kind, r.integer(kFunctionType)), to_function, to_data);
		r.enter(to_function);
		r.store(Kind::Ptr, r.address(null_function_member), object);
		r.ret(Kind::I64, r.integer(1));
		r.enter(to_data);
		r.store(Kind::Ptr, r.address(null_data_member), object);
		r.ret(Kind::I64, r.integer(1));
	}
	r.enter(not_null);
	const BlockId same_kind = r.block(), all_const = r.block(), qualified = r.block();
	r.branch(r.equal(Kind::I64, r.call(Kind::I64, kind, { thrown }), handler_kind), same_kind, no);
	r.enter(same_kind);
	r.branch(r.binary(BinaryOp::And, Kind::I64, r.read(Kind::I64, outer), r.integer(1)), all_const, no);
	r.enter(all_const);
	const Value thrown_flags = r.convert(
		lowir::ConvertOp::ZExt, Kind::I64, Kind::U32, r.load(Kind::U32, thrown, kPointerFlags));
	const Value handler_flags = r.convert(
		lowir::ConvertOp::ZExt, Kind::I64, Kind::U32, r.load(Kind::U32, handler, kPointerFlags));
	const Value extra = r.binary(
		BinaryOp::And, Kind::I64, thrown_flags, r.binary(BinaryOp::Xor, Kind::I64, handler_flags, r.integer(-1)));
	r.branch(extra, no, qualified);
	r.enter(qualified);
	{
		const BlockId keep = r.block(), drop = r.block();
		r.branch(r.binary(BinaryOp::And, Kind::I64, handler_flags, r.integer(1)), keep, drop);
		r.enter(drop);
		r.write(Kind::I64, outer, r.binary(BinaryOp::And, Kind::I64, r.read(Kind::I64, outer), r.integer(-2)));
		r.jump(keep);
		r.enter(keep);
		const BlockId member_class = r.block(), to_pointee = r.block(), first_level = r.block();
		r.branch(is_member, member_class, pointerish);
		r.enter(member_class);
		r.branch(r.call(Kind::I64, equal,
					 { r.load(Kind::Ptr, handler, kMemberClass), r.load(Kind::Ptr, thrown, kMemberClass) }),
			to_pointee, no);
		r.enter(pointerish);
		r.branch(r.compare(ComparePredicate::Ult, Kind::I64, r.read(Kind::I64, outer), r.integer(2)), first_level,
			to_pointee);
		r.enter(first_level);
		const BlockId to_void = r.block();
		r.branch(r.call(Kind::I64, equal, { r.load(Kind::Ptr, handler, kPointee), r.address(void_type) }), to_void,
			to_pointee);
		r.enter(to_void);
		const Value pointee = r.call(Kind::I64, kind, { r.load(Kind::Ptr, thrown, kPointee) });
		r.branch(r.equal(Kind::I64, pointee, r.integer(kFunctionType)), no, yes);
		r.enter(to_pointee);
		r.ret(Kind::I64,
			r.call(Kind::I64, symbol,
				{ r.load(Kind::Ptr, handler, kPointee), r.load(Kind::Ptr, thrown, kPointee), object,
					r.add(Kind::I64, r.read(Kind::I64, outer), r.integer(2)) }));
	}
	r.enter(yes);
	r.ret(Kind::I64, r.integer(1));
	r.enter(no);
	r.ret(Kind::I64, r.integer(0));
}

// -------------------------------------------------------------- exceptions

void Support::exceptions()
{
	type_kind();
	type_equal();
	find_base();
	upcast();
	do_catch();
	const SymbolId payload = runtime_.global(kEhPayloadSymbol, 0);
	const SymbolId selector = runtime_.global(kEhSelectorSymbol, 0);
	const SymbolId caught = runtime_.global("__cppgm_eh_caught", 8);
	const SymbolId in_flight = runtime_.global("__cppgm_eh_in_flight", 8);
	const SymbolId dispatch = fn(kEhDispatchSymbol, Kind::Void, {}, false);
	const SymbolId kind = fn("__cppgm_type_kind", Kind::I64, { Kind::Ptr }, false);
	const SymbolId matches = fn("__cppgm_do_catch", Kind::I64, { Kind::Ptr, Kind::Ptr, Kind::Ptr, Kind::I64 }, false);
	{
		// __cxa_allocate_exception(size): a zeroed header, then the object.
		RoutineBuilder r(runtime_.program(), fn("__cxa_allocate_exception", Kind::Ptr, { Kind::I64 }, true));
		const Value block = r.call(
			Kind::Ptr, allocate(), { r.add(Kind::I64, r.param(0), r.integer(kExceptionHeader)) });
		for (int64_t field = 0; field < kExceptionHeader; field += 8)
			r.store(Kind::I64, r.integer(0), block, field);
		r.ret(Kind::Ptr, r.at(block, kExceptionHeader));
	}
	{
		RoutineBuilder r(runtime_.program(), fn("__cxa_free_exception", Kind::Void, { Kind::Ptr }, true));
		r.call(Kind::Void, release(), { r.at(r.param(0), -kExceptionHeader) });
		r.ret();
	}
	{
		// __cxa_throw(object, type, destructor): in flight, then dispatch.
		RoutineBuilder r(runtime_.program(), fn("__cxa_throw", Kind::Void, { Kind::Ptr, Kind::Ptr, Kind::Ptr }, true));
		const Value header = r.at(r.param(0), -kExceptionHeader);
		r.store(Kind::Ptr, r.param(1), header, kThrownType);
		r.store(Kind::Ptr, r.param(2), header, kDestructor);
		r.store(Kind::Ptr, r.param(0), header, kAdjusted);
		r.store(Kind::I64, r.integer(0), header, kHandlers);
		r.store(Kind::Ptr, r.load(Kind::Ptr, r.address(in_flight)), header, kNextInFlight);
		r.store(Kind::Ptr, header, r.address(in_flight));
		r.store(Kind::Ptr, r.param(0), r.address(payload));
		r.call(Kind::Void, dispatch, {});
		r.unreachable();
	}
	{
		// __cppgm_eh_catch(type, selector): one clause of the landing being
		// entered; the first clause matching sets the selector.
		RoutineBuilder r(runtime_.program(), fn(kEhCatchSymbol, Kind::Void, { Kind::Ptr, Kind::I64 }, true));
		const SlotId adjusted = r.local(Kind::Ptr);
		const BlockId open = r.block(), done = r.block(), typed = r.block(), matched = r.block(), test = r.block();
		const BlockId pointer = r.block();
		r.branch(r.load(Kind::I64, r.address(selector)), done, open);
		r.enter(open);
		const Value object = r.load(Kind::Ptr, r.address(payload));
		const Value header = r.at(object, -kExceptionHeader);
		r.write(Kind::Ptr, adjusted, object);
		r.branch(r.equal(Kind::Ptr, r.param(0), r.integer(0)), matched, typed);
		r.enter(typed);
		const Value thrown = r.load(Kind::Ptr, header, kThrownType);
		r.branch(r.equal(Kind::I64, r.call(Kind::I64, kind, { thrown }), r.integer(kPointerType)), pointer, test);
		r.enter(pointer);
		r.write(Kind::Ptr, adjusted, r.load(Kind::Ptr, object));
		r.jump(test);
		r.enter(test);
		r.branch(
			r.call(Kind::I64, matches, { r.param(0), thrown, r.slot_address(adjusted), r.integer(1) }), matched, done);
		r.enter(matched);
		r.store(Kind::Ptr, r.read(Kind::Ptr, adjusted), header, kAdjusted);
		r.store(Kind::I64, r.param(1), header, kSelector);
		r.store(Kind::I64, r.param(1), r.address(selector));
		r.ret();
		r.enter(done);
		r.ret();
	}
	{
		// __cxa_begin_catch(object): caught, off the in-flight list; the
		// enclosing in-flight exception's payload and selector come back for
		// the landing still running.
		RoutineBuilder r(runtime_.program(), fn("__cxa_begin_catch", Kind::Ptr, { Kind::Ptr }, true));
		const SlotId link = r.local(Kind::Ptr);
		const Value header = r.at(r.param(0), -kExceptionHeader);
		const BlockId top = r.block(), search = r.block(), test = r.block(), step = r.block(), unlink = r.block();
		const BlockId removed = r.block(), restore = r.block(), counted = r.block(), rethrown = r.block();
		const BlockId fresh = r.block(), stacked = r.block(), push = r.block();
		const Value first = r.load(Kind::Ptr, r.address(in_flight));
		r.branch(r.equal(Kind::Ptr, first, header), top, search);
		r.enter(top);
		const Value next_in_flight = r.load(Kind::Ptr, header, kNextInFlight);
		r.store(Kind::Ptr, next_in_flight, r.address(in_flight));
		r.jump(removed);
		r.enter(search);
		r.write(Kind::Ptr, link, first);
		r.jump(test);
		r.enter(test);
		const Value node = r.read(Kind::Ptr, link);
		const BlockId walk = r.block();
		r.branch(r.equal(Kind::Ptr, node, r.integer(0)), removed, walk);
		r.enter(walk);
		const Value successor = r.load(Kind::Ptr, node, kNextInFlight);
		r.branch(r.equal(Kind::Ptr, successor, header), unlink, step);
		r.enter(unlink);
		r.store(Kind::Ptr, r.load(Kind::Ptr, header, kNextInFlight), node, kNextInFlight);
		r.jump(removed);
		r.enter(step);
		r.write(Kind::Ptr, link, successor);
		r.jump(test);
		r.enter(removed);
		const Value enclosing = r.load(Kind::Ptr, r.address(in_flight));
		r.branch(r.equal(Kind::Ptr, enclosing, r.integer(0)), counted, restore);
		r.enter(restore);
		const Value enclosing_object = r.at(enclosing, kExceptionHeader);
		r.store(Kind::Ptr, enclosing_object, r.address(payload));
		const Value enclosing_selector = r.load(Kind::I64, enclosing, kSelector);
		r.store(Kind::I64, enclosing_selector, r.address(selector));
		r.jump(counted);
		r.enter(counted);
		const SlotId handlers = r.local(Kind::I64);
		const Value count = r.load(Kind::I64, header, kHandlers);
		r.branch(r.compare(ComparePredicate::Lt, Kind::I64, count, r.integer(0)), rethrown, fresh);
		r.enter(rethrown);
		r.write(Kind::I64, handlers, r.binary(BinaryOp::Sub, Kind::I64, r.integer(1), count));
		r.jump(stacked);
		r.enter(fresh);
		r.write(Kind::I64, handlers, r.add(Kind::I64, count, r.integer(1)));
		r.jump(stacked);
		r.enter(stacked);
		r.store(Kind::I64, r.read(Kind::I64, handlers), header, kHandlers);
		const BlockId done = r.block();
		r.branch(r.equal(Kind::Ptr, r.load(Kind::Ptr, r.address(caught)), header), done, push);
		r.enter(push);
		r.store(Kind::Ptr, r.load(Kind::Ptr, r.address(caught)), header, kNextCaught);
		r.store(Kind::Ptr, header, r.address(caught));
		r.jump(done);
		r.enter(done);
		r.ret(Kind::Ptr, r.load(Kind::Ptr, header, kAdjusted));
	}
	{
		// __cxa_end_catch(): the innermost caught exception loses a handler;
		// its last one destroys it unless it was rethrown.
		RoutineBuilder r(runtime_.program(), fn("__cxa_end_catch", Kind::Void, {}, true));
		const lowir::SignatureId destructor_signature = runtime_.signature(Kind::Void, { Kind::Ptr });
		const BlockId some = r.block(), none = r.block(), rethrown = r.block(), handled = r.block();
		const BlockId still_rethrown = r.block(), uncaught = r.block(), keep = r.block(), destroy = r.block();
		const BlockId call = r.block(), free_block = r.block();
		const Value header = r.load(Kind::Ptr, r.address(caught));
		r.branch(r.equal(Kind::Ptr, header, r.integer(0)), none, some);
		r.enter(none);
		r.ret();
		r.enter(some);
		const Value count = r.load(Kind::I64, header, kHandlers);
		r.branch(r.compare(ComparePredicate::Lt, Kind::I64, count, r.integer(0)), rethrown, handled);
		r.enter(rethrown);
		const Value up_count = r.add(Kind::I64, count, r.integer(1));
		r.store(Kind::I64, up_count, header, kHandlers);
		r.branch(r.equal(Kind::I64, up_count, r.integer(0)), uncaught, still_rethrown);
		r.enter(uncaught);
		const Value next_caught = r.load(Kind::Ptr, header, kNextCaught);
		r.store(Kind::Ptr, next_caught, r.address(caught));
		r.jump(still_rethrown);
		r.enter(still_rethrown);
		r.ret();
		r.enter(handled);
		const Value down_count = r.binary(BinaryOp::Sub, Kind::I64, count, r.integer(1));
		r.store(Kind::I64, down_count, header, kHandlers);
		r.branch(r.equal(Kind::I64, down_count, r.integer(0)), destroy, keep);
		r.enter(keep);
		r.ret();
		r.enter(destroy);
		const Value below = r.load(Kind::Ptr, header, kNextCaught);
		r.store(Kind::Ptr, below, r.address(caught));
		const Value destructor = r.load(Kind::Ptr, header, kDestructor);
		r.branch(r.equal(Kind::Ptr, destructor, r.integer(0)), free_block, call);
		r.enter(call);
		r.call_indirect(Kind::Void, destructor, destructor_signature, { r.at(header, kExceptionHeader) });
		r.jump(free_block);
		r.enter(free_block);
		r.call(Kind::Void, release(), { header });
		r.ret();
	}
	{
		// __cxa_rethrow(): the innermost caught exception in flight again.
		RoutineBuilder r(runtime_.program(), fn("__cxa_rethrow", Kind::Void, {}, true));
		const BlockId none = r.block(), some = r.block();
		const Value header = r.load(Kind::Ptr, r.address(caught));
		r.branch(r.equal(Kind::Ptr, header, r.integer(0)), none, some);
		r.enter(none);
		r.call(Kind::Void, terminate(), {});
		r.unreachable();
		r.enter(some);
		r.store(Kind::I64, r.binary(BinaryOp::Sub, Kind::I64, r.integer(0), r.load(Kind::I64, header, kHandlers)),
			header, kHandlers);
		// The clause that caught it no longer stands: a nested exception's
		// catch must not restore it for a landing of the rethrow.
		r.store(Kind::I64, r.integer(0), header, kSelector);
		r.store(Kind::Ptr, r.load(Kind::Ptr, r.address(in_flight)), header, kNextInFlight);
		r.store(Kind::Ptr, header, r.address(in_flight));
		const Value object = r.at(header, kExceptionHeader);
		r.store(Kind::Ptr, object, r.address(payload));
		r.call(Kind::Void, dispatch, {});
		r.unreachable();
	}
	{
		RoutineBuilder r(runtime_.program(), fn("__cxa_get_exception_ptr", Kind::Ptr, { Kind::Ptr }, true));
		r.ret(Kind::Ptr, r.load(Kind::Ptr, r.param(0), kAdjusted - kExceptionHeader));
	}
}

// ------------------------------------------------------------ dynamic cast

// __cppgm_cast_walk(type, object, public, derived, derived_public): visits
// every subobject of the most derived object: a subobject of the target
// type is a cross-cast candidate (recorded like __cppgm_find_base) and
// the `derived` of its bases; reaching the source subobject records
// whether it is a public base of the whole object, and its enclosing
// target subobject when the path from that is public.
void Support::walk()
{
	const SymbolId equal = fn("__cppgm_type_equal", Kind::I64, { Kind::Ptr, Kind::Ptr }, false);
	const SymbolId kind = fn("__cppgm_type_kind", Kind::I64, { Kind::Ptr }, false);
	const SymbolId target = runtime_.global("__cppgm_cast_target", 8);
	const SymbolId source = runtime_.global("__cppgm_cast_source", 8);
	const SymbolId source_type = runtime_.global("__cppgm_cast_source_type", 8);
	const SymbolId source_public = runtime_.global("__cppgm_cast_source_public", 8);
	const SymbolId down_count = runtime_.global("__cppgm_cast_down_count", 8);
	const SymbolId down_address = runtime_.global("__cppgm_cast_down_address", 8);
	const SymbolId cross_count = runtime_.global("__cppgm_cast_cross_count", 8);
	const SymbolId cross_address = runtime_.global("__cppgm_cast_cross_address", 8);
	const SymbolId cross_public = runtime_.global("__cppgm_cast_cross_public", 8);
	const SymbolId symbol = fn(
		"__cppgm_cast_walk", Kind::Void, { Kind::Ptr, Kind::Ptr, Kind::I64, Kind::Ptr, Kind::I64 }, true);
	RoutineBuilder r(runtime_.program(), symbol);
	const Value type = r.param(0), object = r.param(1), open = r.param(2);
	const SlotId derived = r.local(Kind::Ptr);
	const SlotId derived_open = r.local(Kind::I64);
	r.write(Kind::Ptr, derived, r.param(3));
	r.write(Kind::I64, derived_open, r.param(4));
	const BlockId is_target = r.block(), source_check = r.block(), first = r.block(), again = r.block();
	const BlockId other = r.block(), same = r.block();
	r.branch(r.call(Kind::I64, equal, { type, r.load(Kind::Ptr, r.address(target)) }), is_target, source_check);
	r.enter(is_target);
	r.write(Kind::Ptr, derived, object);
	r.write(Kind::I64, derived_open, r.integer(1));
	r.branch(r.equal(Kind::I64, r.load(Kind::I64, r.address(cross_count)), r.integer(0)), first, again);
	r.enter(first);
	r.store(Kind::I64, r.integer(1), r.address(cross_count));
	r.store(Kind::Ptr, object, r.address(cross_address));
	r.store(Kind::I64, open, r.address(cross_public));
	r.jump(source_check);
	r.enter(again);
	r.branch(r.equal(Kind::Ptr, r.load(Kind::Ptr, r.address(cross_address)), object), same, other);
	r.enter(other);
	r.store(Kind::I64, r.integer(2), r.address(cross_count));
	r.jump(source_check);
	r.enter(same);
	const Value cross_public_now = r.binary(BinaryOp::Or, Kind::I64, r.load(Kind::I64, r.address(cross_public)), open);
	r.store(Kind::I64, cross_public_now, r.address(cross_public));
	r.jump(source_check);
	r.enter(source_check);
	const BlockId at_source = r.block(), source_type_check = r.block(), bases = r.block();
	r.branch(r.equal(Kind::Ptr, object, r.load(Kind::Ptr, r.address(source))), source_type_check, bases);
	r.enter(source_type_check);
	r.branch(r.call(Kind::I64, equal, { type, r.load(Kind::Ptr, r.address(source_type)) }), at_source, bases);
	r.enter(at_source);
	const Value source_public_now = r.binary(
		BinaryOp::Or, Kind::I64, r.load(Kind::I64, r.address(source_public)), open);
	r.store(Kind::I64, source_public_now, r.address(source_public));
	{
		const BlockId inside = r.block(), record = r.block(), first_down = r.block(), next_down = r.block();
		const BlockId other_down = r.block();
		r.branch(r.equal(Kind::Ptr, r.read(Kind::Ptr, derived), r.integer(0)), bases, inside);
		r.enter(inside);
		r.branch(r.read(Kind::I64, derived_open), record, bases);
		r.enter(record);
		r.branch(r.equal(Kind::I64, r.load(Kind::I64, r.address(down_count)), r.integer(0)), first_down, next_down);
		r.enter(first_down);
		r.store(Kind::I64, r.integer(1), r.address(down_count));
		const Value derived_address = r.read(Kind::Ptr, derived);
		r.store(Kind::Ptr, derived_address, r.address(down_address));
		r.jump(bases);
		r.enter(next_down);
		const Value recorded = r.load(Kind::Ptr, r.address(down_address));
		r.branch(r.equal(Kind::Ptr, recorded, r.read(Kind::Ptr, derived)), bases, other_down);
		r.enter(other_down);
		r.store(Kind::I64, r.integer(2), r.address(down_count));
		r.jump(bases);
	}
	r.enter(bases);
	const Value code = r.call(Kind::I64, kind, { type });
	const BlockId single = r.block(), not_single = r.block(), multiple = r.block(), done = r.block();
	r.branch(r.equal(Kind::I64, code, r.integer(kSiClassType)), single, not_single);
	r.enter(single);
	r.call(Kind::Void, symbol,
		{ r.load(Kind::Ptr, type, kSiBase), object, open, r.read(Kind::Ptr, derived),
			r.read(Kind::I64, derived_open) });
	r.ret();
	r.enter(not_single);
	r.branch(r.equal(Kind::I64, code, r.integer(kVmiClassType)), multiple, done);
	r.enter(multiple);
	const SlotId index = r.local(Kind::I64);
	const SlotId child = r.local(Kind::Ptr);
	r.write(Kind::I64, index, r.integer(0));
	const Value total = r.convert(lowir::ConvertOp::ZExt, Kind::I64, Kind::U32, r.load(Kind::U32, type, kVmiCount));
	const BlockId test = r.block(), body = r.block(), virtual_base = r.block(), direct = r.block(), located = r.block();
	r.jump(test);
	r.enter(test);
	r.branch(r.compare(ComparePredicate::Ult, Kind::I64, r.read(Kind::I64, index), total), body, done);
	r.enter(body);
	const Value bases_start = r.at(type, kVmiBases);
	const Value entry = r.at(bases_start, r.binary(BinaryOp::Mul, Kind::I64, r.read(Kind::I64, index), r.integer(16)));
	const Value flags = r.load(Kind::I64, entry, 8);
	const Value offset = r.binary(BinaryOp::Shr, Kind::I64, flags, r.integer(8));
	r.branch(r.binary(BinaryOp::And, Kind::I64, flags, r.integer(1)), virtual_base, direct);
	r.enter(virtual_base);
	r.write(Kind::Ptr, child, r.at(object, r.load(Kind::I64, r.at(r.load(Kind::Ptr, object), offset))));
	r.jump(located);
	r.enter(direct);
	r.write(Kind::Ptr, child, r.at(object, offset));
	r.jump(located);
	r.enter(located);
	const Value base_public = r.binary(
		BinaryOp::UShr, Kind::I64, r.binary(BinaryOp::And, Kind::I64, flags, r.integer(2)), r.integer(1));
	r.call(Kind::Void, symbol,
		{ r.load(Kind::Ptr, entry), r.read(Kind::Ptr, child), r.binary(BinaryOp::And, Kind::I64, open, base_public),
			r.read(Kind::Ptr, derived),
			r.binary(BinaryOp::And, Kind::I64, r.read(Kind::I64, derived_open), base_public) });
	r.write(Kind::I64, index, r.add(Kind::I64, r.read(Kind::I64, index), r.integer(1)));
	r.jump(test);
	r.enter(done);
	r.ret();
}

// __dynamic_cast(source, source_type, target_type, hint) (5.2.7p8):
// the unique target subobject the source subobject is a public base of,
// else, when the source is a public base of the most derived object, its
// unambiguous public target base; else null. The hint (Itanium 2.9.7) is
// not needed by the walk.
void Support::dynamic_cast_member()
{
	type_kind();
	type_equal();
	walk();
	const SymbolId walk_symbol = fn(
		"__cppgm_cast_walk", Kind::Void, { Kind::Ptr, Kind::Ptr, Kind::I64, Kind::Ptr, Kind::I64 }, false);
	const SymbolId symbol = fn("__dynamic_cast", Kind::Ptr, { Kind::Ptr, Kind::Ptr, Kind::Ptr, Kind::I64 }, true);
	RoutineBuilder r(runtime_.program(), symbol);
	const char* const state[] = { "__cppgm_cast_source_public", "__cppgm_cast_down_count", "__cppgm_cast_cross_count",
		"__cppgm_cast_cross_public" };
	const BlockId null = r.block(), cast = r.block();
	r.branch(r.equal(Kind::Ptr, r.param(0), r.integer(0)), null, cast);
	r.enter(null);
	r.ret(Kind::Ptr, r.integer(0));
	r.enter(cast);
	for (const char* name : state)
		r.store(Kind::I64, r.integer(0), r.address(runtime_.global(name, 8)));
	r.store(Kind::Ptr, r.param(0), r.address(runtime_.global("__cppgm_cast_source", 8)));
	r.store(Kind::Ptr, r.param(1), r.address(runtime_.global("__cppgm_cast_source_type", 8)));
	r.store(Kind::Ptr, r.param(2), r.address(runtime_.global("__cppgm_cast_target", 8)));
	const Value vtable = r.load(Kind::Ptr, r.param(0));
	const Value whole = r.at(r.param(0), r.load(Kind::I64, vtable, -16));
	r.call(Kind::Void, walk_symbol, { r.load(Kind::Ptr, vtable, -8), whole, r.integer(1), r.integer(0), r.integer(0) });
	const BlockId down = r.block(), not_down = r.block(), source_open = r.block(), unique = r.block();
	const BlockId open = r.block(), fail = r.block();
	r.branch(
		r.equal(Kind::I64, r.load(Kind::I64, r.address(runtime_.global("__cppgm_cast_down_count", 8))), r.integer(1)),
		down, not_down);
	r.enter(down);
	r.ret(Kind::Ptr, r.load(Kind::Ptr, r.address(runtime_.global("__cppgm_cast_down_address", 8))));
	r.enter(not_down);
	r.branch(r.load(Kind::I64, r.address(runtime_.global("__cppgm_cast_source_public", 8))), source_open, fail);
	r.enter(source_open);
	r.branch(
		r.equal(Kind::I64, r.load(Kind::I64, r.address(runtime_.global("__cppgm_cast_cross_count", 8))), r.integer(1)),
		unique, fail);
	r.enter(unique);
	r.branch(r.load(Kind::I64, r.address(runtime_.global("__cppgm_cast_cross_public", 8))), open, fail);
	r.enter(open);
	r.ret(Kind::Ptr, r.load(Kind::Ptr, r.address(runtime_.global("__cppgm_cast_cross_address", 8))));
	r.enter(fail);
	r.ret(Kind::Ptr, r.integer(0));
}

// ---------------------------------------------------------------- bad cast

// A thrower of a std::exception-derived object with only a vtable pointer:
// its type information (an __si_class_type_info over std::exception's, or
// over that of the class `base_name` derived from std::exception, whose
// type information is `base_type`) and vtable (offset to top and type
// information; no virtual calls are made on it).
void Support::throw_runtime_error(const char* routine, const char* type, const char* name, const char* vtable,
	const char* base_type, const char* base_name)
{
	const SymbolId exception_name = runtime_.string("_ZTSSt9exception", "St9exception");
	SymbolId base = runtime_.program().symbol_for(runtime_.name("_ZTISt9exception"));
	if (runtime_.program().symbols[base].kind == lowir::SymbolKind::Unresolved)
		runtime_.words(
			"_ZTISt9exception", { DataWord{ runtime_.global(kVtableNames[0], 0), 16 }, DataWord{ exception_name, 0 } });
	if (base_type != nullptr)
	{
		const SymbolId exception = base;
		base = runtime_.program().symbol_for(runtime_.name(base_type));
		if (runtime_.program().symbols[base].kind == lowir::SymbolKind::Unresolved)
			runtime_.words(base_type,
				{ DataWord{ runtime_.global(kVtableNames[1], 0), 16 },
					DataWord{ runtime_.string((std::string("_ZTS") + base_name).c_str(), base_name), 0 },
					DataWord{ exception, 0 } });
	}
	const SymbolId type_name = runtime_.string((std::string("_ZTS") + name).c_str(), name);
	const SymbolId information = runtime_.words(
		type, { DataWord{ runtime_.global(kVtableNames[1], 0), 16 }, DataWord{ type_name, 0 }, DataWord{ base, 0 } });
	const SymbolId table = runtime_.words(vtable,
		{ DataWord{ lowir::kNoId, 0 }, DataWord{ information, 0 }, DataWord{ lowir::kNoId, 0 },
			DataWord{ lowir::kNoId, 0 } });
	RoutineBuilder r(runtime_.program(), fn(routine, Kind::Void, {}, true));
	const Value object = r.call(
		Kind::Ptr, fn("__cxa_allocate_exception", Kind::Ptr, { Kind::I64 }, false), { r.integer(8) });
	r.store(Kind::Ptr, r.at(r.address(table), 16), object);
	r.call(Kind::Void, fn("__cxa_throw", Kind::Void, { Kind::Ptr, Kind::Ptr, Kind::Ptr }, false),
		{ object, r.address(information), r.integer(0) });
	r.unreachable();
}

void Support::bad_cast()
{
	throw_runtime_error("__cxa_bad_cast", "_ZTISt8bad_cast", "St8bad_cast", "_ZTVSt8bad_cast");
	throw_runtime_error("__cxa_bad_typeid", "_ZTISt10bad_typeid", "St10bad_typeid", "_ZTVSt10bad_typeid");
	// An array new-expression's invalid count (5.3.4p7).
	throw_runtime_error("__cxa_throw_bad_array_new_length", "_ZTISt20bad_array_new_length", "St20bad_array_new_length",
		"_ZTVSt20bad_array_new_length", "_ZTISt9bad_alloc", "St9bad_alloc");
}

// ----------------------------------------------------------- wide division

void Support::wide_division()
{
	using lowir::ConvertOp;
	// __cppgm_udivmod128(n, d, remainder): the unsigned quotient, or the
	// remainder when `remainder` is nonzero.
	const SymbolId core = fn("__cppgm_udivmod128", Kind::I128, { Kind::I128, Kind::I128, Kind::I64 }, true);
	{
		RoutineBuilder r(runtime_.program(), core);
		const Value n = r.param(0), d = r.param(1), remainder = r.param(2);
		const BlockId narrow = r.block(), wide = r.block(), narrow_remainder = r.block(), narrow_quotient = r.block();
		const Value n_high = r.binary(BinaryOp::UShr, Kind::I128, n, r.integer(64));
		const Value high = r.binary(
			BinaryOp::Or, Kind::I128, n_high, r.binary(BinaryOp::UShr, Kind::I128, d, r.integer(64)));
		r.branch(r.equal(Kind::I128, high, r.integer(0)), narrow, wide);
		r.enter(narrow);
		const Value n64 = r.convert(ConvertOp::Trunc, Kind::I64, Kind::I128, n);
		const Value d64 = r.convert(ConvertOp::Trunc, Kind::I64, Kind::I128, d);
		r.branch(remainder, narrow_remainder, narrow_quotient);
		r.enter(narrow_remainder);
		r.ret(Kind::I128,
			r.convert(ConvertOp::ZExt, Kind::I128, Kind::I64, r.binary(BinaryOp::UMod, Kind::I64, n64, d64)));
		r.enter(narrow_quotient);
		r.ret(Kind::I128,
			r.convert(ConvertOp::ZExt, Kind::I128, Kind::I64, r.binary(BinaryOp::UDiv, Kind::I64, n64, d64)));
		r.enter(wide);
		const SlotId quotient = r.local(Kind::I128);
		const SlotId rest = r.local(Kind::I128);
		const SlotId bit = r.local(Kind::I64);
		r.write(Kind::I128, quotient, r.integer(0));
		r.write(Kind::I128, rest, r.integer(0));
		r.write(Kind::I64, bit, r.integer(127));
		const BlockId test = r.block(), body = r.block(), subtract = r.block(), next = r.block(), done = r.block();
		const BlockId give_remainder = r.block(), give_quotient = r.block();
		r.jump(test);
		r.enter(test);
		r.branch(r.compare(ComparePredicate::Lt, Kind::I64, r.read(Kind::I64, bit), r.integer(0)), done, body);
		r.enter(body);
		const Value i = r.read(Kind::I64, bit);
		const Value incoming = r.binary(
			BinaryOp::And, Kind::I128, r.binary(BinaryOp::UShr, Kind::I128, n, i), r.integer(1));
		const Value shifted = r.binary(BinaryOp::Or, Kind::I128,
			r.binary(BinaryOp::Shl, Kind::I128, r.read(Kind::I128, rest), r.integer(1)), incoming);
		r.write(Kind::I128, rest, shifted);
		r.branch(r.compare(ComparePredicate::Uge, Kind::I128, shifted, d), subtract, next);
		r.enter(subtract);
		r.write(Kind::I128, rest, r.binary(BinaryOp::Sub, Kind::I128, shifted, d));
		const Value so_far = r.read(Kind::I128, quotient);
		r.write(Kind::I128, quotient,
			r.binary(BinaryOp::Or, Kind::I128, so_far, r.binary(BinaryOp::Shl, Kind::I128, r.integer(1), i)));
		r.jump(next);
		r.enter(next);
		r.write(Kind::I64, bit, r.binary(BinaryOp::Sub, Kind::I64, i, r.integer(1)));
		r.jump(test);
		r.enter(done);
		r.branch(remainder, give_remainder, give_quotient);
		r.enter(give_remainder);
		r.ret(Kind::I128, r.read(Kind::I128, rest));
		r.enter(give_quotient);
		r.ret(Kind::I128, r.read(Kind::I128, quotient));
	}
	const char* const names[4] = { "__divti3", "__udivti3", "__modti3", "__umodti3" };
	for (unsigned k = 0; k < 4; ++k)
	{
		const bool is_signed = k == 0 || k == 2;
		const bool remainder = k >= 2;
		RoutineBuilder r(runtime_.program(), fn(names[k], Kind::I128, { Kind::I128, Kind::I128 }, true));
		if (!is_signed)
		{
			r.ret(Kind::I128, r.call(Kind::I128, core, { r.param(0), r.param(1), r.integer(remainder ? 1 : 0) }));
			continue;
		}
		// The magnitudes divided; the quotient negative when the signs
		// differ, the remainder with the dividend's sign.
		SlotId magnitude[2];
		Value negative[2];
		for (unsigned operand = 0; operand < 2; ++operand)
		{
			magnitude[operand] = r.local(Kind::I128);
			negative[operand] = r.compare(ComparePredicate::Lt, Kind::I128, r.param(operand), r.integer(0));
			const BlockId flip = r.block(), keep = r.block(), joined = r.block();
			r.branch(negative[operand], flip, keep);
			r.enter(flip);
			r.write(
				Kind::I128, magnitude[operand], r.binary(BinaryOp::Sub, Kind::I128, r.integer(0), r.param(operand)));
			r.jump(joined);
			r.enter(keep);
			r.write(Kind::I128, magnitude[operand], r.param(operand));
			r.jump(joined);
			r.enter(joined);
		}
		const Value result = r.call(Kind::I128, core,
			{ r.read(Kind::I128, magnitude[0]), r.read(Kind::I128, magnitude[1]), r.integer(remainder ? 1 : 0) });
		const Value flips = remainder ? negative[0] : r.binary(BinaryOp::Xor, Kind::I64, negative[0], negative[1]);
		const BlockId negate = r.block(), plain = r.block();
		r.branch(flips, negate, plain);
		r.enter(negate);
		r.ret(Kind::I128, r.binary(BinaryOp::Sub, Kind::I128, r.integer(0), result));
		r.enter(plain);
		r.ret(Kind::I128, result);
	}
}

// ------------------------------------------------- floating conversions

// A finite f64 or f80 `x` of 1 or more as `significand` * 2^`lowest`, the
// significand's leading bit at 2^`top`.
void Support::decompose(RoutineBuilder& r, Kind kind, Value x, Value& significand, Value& top, Value& lowest)
{
	using lowir::ConvertOp;
	const SlotId image = r.local(kind);
	r.write(kind, image, x);
	const Value address = r.slot_address(image);
	if (kind == Kind::F64)
	{
		const Value bits = r.load(Kind::I64, address);
		const Value biased = r.binary(
			BinaryOp::And, Kind::I64, r.binary(BinaryOp::UShr, Kind::I64, bits, r.integer(52)), r.integer(0x7ff));
		significand = r.binary(BinaryOp::Or, Kind::I64,
			r.binary(BinaryOp::And, Kind::I64, bits, r.integer((int64_t(1) << 52) - 1)), r.integer(int64_t(1) << 52));
		top = r.binary(BinaryOp::Sub, Kind::I64, biased, r.integer(1023));
		lowest = r.binary(BinaryOp::Sub, Kind::I64, top, r.integer(52));
		return;
	}
	// f80: the explicit 64-bit significand, then sign and exponent.
	significand = r.load(Kind::I64, address);
	const Value biased = r.binary(BinaryOp::And, Kind::I64,
		r.convert(ConvertOp::ZExt, Kind::I64, Kind::U16, r.load(Kind::U16, address, 8)), r.integer(0x7fff));
	top = r.binary(BinaryOp::Sub, Kind::I64, biased, r.integer(16383));
	lowest = r.binary(BinaryOp::Sub, Kind::I64, top, r.integer(63));
}

void Support::float_conversion()
{
	using lowir::ConvertOp;
	const Kind floats[3] = { Kind::F32, Kind::F64, Kind::F80 };
	// Unsigned to floating: a value below 2^64 converts directly. Above, an
	// f80 is the exact sum of its halves, rounded once; an f32 or f64
	// converts the value shifted right to 64 bits with any bit shifted out
	// ORed into the lowest (below the rounding position: round to nearest
	// even still sees it), then scales back exactly (an f32 may overflow to
	// infinity, the rounded value).
	SymbolId unsigned_to[3];
	for (unsigned k = 0; k < 3; ++k)
	{
		const Kind f = floats[k];
		unsigned_to[k] = fn(kFloatConversionSymbols[3 + k], f, { Kind::I128 }, true);
		RoutineBuilder r(runtime_.program(), unsigned_to[k]);
		const Value u = r.param(0);
		const Value low = r.convert(ConvertOp::Trunc, Kind::I64, Kind::I128, u);
		const Value high = r.convert(
			ConvertOp::Trunc, Kind::I64, Kind::I128, r.binary(BinaryOp::UShr, Kind::I128, u, r.integer(64)));
		const BlockId narrow = r.block(), wide = r.block();
		r.branch(r.equal(Kind::I64, high, r.integer(0)), narrow, wide);
		r.enter(narrow);
		r.ret(f, r.convert(ConvertOp::UIToFP, f, Kind::I64, low));
		r.enter(wide);
		if (f == Kind::F80)
		{
			const Value scaled = r.binary(BinaryOp::Mul, f, r.convert(ConvertOp::UIToFP, f, Kind::I64, high),
				r.floating(f, 18446744073709551616.0L));
			r.ret(f, r.add(f, scaled, r.convert(ConvertOp::UIToFP, f, Kind::I64, low)));
			continue;
		}
		// The bit length of the high half (1-64).
		const SlotId length = r.local(Kind::I64);
		const SlotId rest = r.local(Kind::I64);
		r.write(Kind::I64, length, r.integer(0));
		r.write(Kind::I64, rest, high);
		const BlockId test = r.block(), step = r.block(), done = r.block();
		r.jump(test);
		r.enter(test);
		r.branch(r.equal(Kind::I64, r.read(Kind::I64, rest), r.integer(0)), done, step);
		r.enter(step);
		r.write(Kind::I64, rest, r.binary(BinaryOp::UShr, Kind::I64, r.read(Kind::I64, rest), r.integer(1)));
		r.write(Kind::I64, length, r.add(Kind::I64, r.read(Kind::I64, length), r.integer(1)));
		r.jump(test);
		r.enter(done);
		const Value s = r.read(Kind::I64, length);
		const Value shifted = r.convert(
			ConvertOp::Trunc, Kind::I64, Kind::I128, r.binary(BinaryOp::UShr, Kind::I128, u, s));
		const Value lost = r.binary(
			BinaryOp::Shl, Kind::I128, u, r.binary(BinaryOp::Sub, Kind::I64, r.integer(128), s));
		const Value sticky = r.compare(ComparePredicate::Ne, Kind::I128, lost, r.integer(0));
		const Value rounded = r.convert(
			ConvertOp::UIToFP, f, Kind::I64, r.binary(BinaryOp::Or, Kind::I64, shifted, sticky));
		const Value scale = r.convert(ConvertOp::UIToFP, f, Kind::I64,
			r.binary(BinaryOp::Shl, Kind::I64, r.integer(1), r.binary(BinaryOp::Sub, Kind::I64, s, r.integer(1))));
		r.ret(f, r.binary(BinaryOp::Mul, f, r.binary(BinaryOp::Mul, f, rounded, scale), r.floating(f, 2.0L)));
	}
	// Signed to floating: a value of 64 bits directly, else the magnitude's
	// conversion negated (rounding to nearest is symmetric).
	for (unsigned k = 0; k < 3; ++k)
	{
		const Kind f = floats[k];
		RoutineBuilder r(runtime_.program(), fn(kFloatConversionSymbols[k], f, { Kind::I128 }, true));
		const Value v = r.param(0);
		const Value low = r.convert(ConvertOp::Trunc, Kind::I64, Kind::I128, v);
		const BlockId narrow = r.block(), wide = r.block(), negative = r.block(), positive = r.block();
		r.branch(r.equal(Kind::I128, r.convert(ConvertOp::SExt, Kind::I128, Kind::I64, low), v), narrow, wide);
		r.enter(narrow);
		r.ret(f, r.convert(ConvertOp::SIToFP, f, Kind::I64, low));
		r.enter(wide);
		r.branch(r.compare(ComparePredicate::Lt, Kind::I128, v, r.integer(0)), negative, positive);
		r.enter(negative);
		const Value magnitude = r.call(f, unsigned_to[k], { r.binary(BinaryOp::Sub, Kind::I128, r.integer(0), v) });
		r.ret(f, r.binary(BinaryOp::Sub, f, r.floating(f, 0.0L), magnitude));
		r.enter(positive);
		r.ret(f, r.call(f, unsigned_to[k], { v }));
	}
	// Floating to unsigned (4.9p1: truncation; undefined out of range):
	// below 2^64 directly, above the significand shifted into place;
	// negative values give 0, 2^128 or more (or NaN) all ones.
	SymbolId to_unsigned[3];
	for (unsigned k = 1; k < 3; ++k)
	{
		const Kind f = floats[k];
		to_unsigned[k] = fn(kFloatConversionSymbols[9 + k], Kind::I128, { f }, true);
		RoutineBuilder r(runtime_.program(), to_unsigned[k]);
		const Value x = r.param(0);
		const BlockId small = r.block(), large = r.block(), direct = r.block(), zero = r.block();
		const BlockId shift = r.block(), saturate = r.block();
		r.branch(r.compare(ComparePredicate::Lt, f, x, r.floating(f, 18446744073709551616.0L)), small, large);
		r.enter(small);
		r.branch(r.compare(ComparePredicate::Gt, f, x, r.floating(f, -1.0L)), direct, zero);
		r.enter(direct);
		r.ret(Kind::I128,
			r.convert(ConvertOp::ZExt, Kind::I128, Kind::I64, r.convert(ConvertOp::FPToUI, Kind::I64, f, x)));
		r.enter(zero);
		r.ret(Kind::I128, r.integer(0));
		r.enter(large);
		Value significand, top, lowest;
		decompose(r, f, x, significand, top, lowest);
		r.branch(r.compare(ComparePredicate::Lt, Kind::I64, top, r.integer(128)), shift, saturate);
		r.enter(shift);
		r.ret(Kind::I128,
			r.binary(
				BinaryOp::Shl, Kind::I128, r.convert(ConvertOp::ZExt, Kind::I128, Kind::I64, significand), lowest));
		r.enter(saturate);
		r.ret(Kind::I128, r.integer(-1));
	}
	// Floating to signed: within 64 bits directly, else as libgcc2's
	// __fixdfti: the unsigned conversion of the magnitude, negated for a
	// negative value (out of range, undefined, wraps as there).
	SymbolId to_signed[3];
	for (unsigned k = 1; k < 3; ++k)
	{
		const Kind f = floats[k];
		to_signed[k] = fn(kFloatConversionSymbols[6 + k], Kind::I128, { f }, true);
		RoutineBuilder r(runtime_.program(), to_signed[k]);
		const Value x = r.param(0);
		const BlockId below = r.block(), small = r.block(), large = r.block(), negative = r.block(),
					  positive = r.block();
		r.branch(r.compare(ComparePredicate::Lt, f, x, r.floating(f, 9223372036854775808.0L)), below, large);
		r.enter(below);
		r.branch(r.compare(ComparePredicate::Ge, f, x, r.floating(f, -9223372036854775808.0L)), small, large);
		r.enter(small);
		r.ret(Kind::I128,
			r.convert(ConvertOp::SExt, Kind::I128, Kind::I64, r.convert(ConvertOp::FPToSI, Kind::I64, f, x)));
		r.enter(large);
		r.branch(r.compare(ComparePredicate::Lt, f, x, r.floating(f, 0.0L)), negative, positive);
		r.enter(negative);
		const Value magnitude = r.call(
			Kind::I128, to_unsigned[k], { r.binary(BinaryOp::Sub, f, r.floating(f, 0.0L), x) });
		r.ret(Kind::I128, r.binary(BinaryOp::Sub, Kind::I128, r.integer(0), magnitude));
		r.enter(positive);
		r.ret(Kind::I128, r.call(Kind::I128, to_unsigned[k], { x }));
	}
	// An f32 widens to f64 exactly.
	for (unsigned is_unsigned = 0; is_unsigned < 2; ++is_unsigned)
	{
		RoutineBuilder r(
			runtime_.program(), fn(kFloatConversionSymbols[6 + 3 * is_unsigned], Kind::I128, { Kind::F32 }, true));
		const Value wide = r.convert(ConvertOp::FPExt, Kind::F64, Kind::F32, r.param(0));
		r.ret(Kind::I128, r.call(Kind::I128, is_unsigned ? to_unsigned[1] : to_signed[1], { wide }));
	}
}

} // namespace

SupportMember support_member_of(const char* name)
{
	for (const SupportName& support : kSupportNames)
	{
		if (std::strcmp(support.name, name) == 0)
			return support.member;
	}
	return kSupportMembers;
}

void build_support_member(RuntimeProgram& program, SupportMember member)
{
	Support support(program);
	switch (member)
	{
	case kAllocation:
		support.allocation();
		break;
	case kExitHandlers:
		support.exit_handlers();
		break;
	case kExceptions:
		support.exceptions();
		break;
	case kDynamicCast:
		support.dynamic_cast_member();
		break;
	case kBadCast:
		support.bad_cast();
		break;
	case kWideDivision:
		support.wide_division();
		break;
	case kFloatConversion:
		support.float_conversion();
		break;
	default:
		break;
	}
}

} // namespace native
