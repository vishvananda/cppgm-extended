#include <algorithm>

#include "lowir/builder.h"
#include "opt/passes.h"
#include "support/flat_map.h"

namespace opt {

using namespace lowir;

namespace {

// The object an address names: a slot or global plus a constant offset,
// or unknown (base kind None).
struct AddressFacts
{
	ValueKind base = ValueKind::None;
	uint32_t index = 0;
	int64_t offset = 0;
};

// A location: a slot or global and a constant offset when known, else the
// address value itself; and the access type.
struct Key
{
	ValueKind kind;
	uint32_t index;
	int64_t offset;
	uint64_t type;

	bool operator==(const Key& o) const
	{
		return kind == o.kind && index == o.index && offset == o.offset && type == o.type;
	}
};

struct KeyHash
{
	size_t operator()(const Key& k) const
	{
		return (static_cast<size_t>(k.index) * 0x9e3779b97f4a7c15ull) ^ (static_cast<size_t>(k.kind) << 3) ^
			static_cast<size_t>(k.type * 0x100000001b3ull) ^ static_cast<size_t>(k.offset * 0x2545f4914f6cdd1dull);
	}
};

struct Known
{
	Value value;
	uint32_t generation;
	bool typed; // from an IF_TYPED access
	Type type;
	AddressFacts at; // the address's facts (fixed during the walk)
	uint64_t bytes;  // storage_bytes(type)
};

// An EH region's start or end (a try or cleanup record pushed or popped):
// it writes no memory the program reads, and a handler block starts the
// walk knowing nothing.
bool region_marker(Opcode op)
{
	return op == Opcode::EhTry || op == Opcode::EhCleanup || op == Opcode::EhEnd;
}

// Two typed accesses (IF_TYPED) that name different objects unless their
// addresses agree: one of a pointer, the other of a non-pointer type wider
// than a byte (strict aliasing; a byte access may be a character type's).
bool typed_apart(bool typed_a, Type a, bool typed_b, Type b)
{
	auto wide = [](Type t) {
		const TypeKind k = t.kind();
		return k != TypeKind::I1 && k != TypeKind::I8 && k != TypeKind::U8 && k != TypeKind::Void && !t.is_object();
	};
	return typed_a && typed_b && a.is_pointer() != b.is_pointer() && wide(a) && wide(b);
}

// Bytes [start, end) of a slot, global or opaque pointer base known to
// read zero (zeroinit, zero literal stores).
struct ZeroRange
{
	AddressFacts base; // offset unused
	int64_t start, end;
};

// The other scalar type of an integer or pointer type's width with the
// same bits (signed and unsigned integers, a pointer and i64); the type
// itself when none.
Type same_width_twin(Type t)
{
	switch (t.kind())
	{
	case TypeKind::I8:
		return Type::scalar(TypeKind::U8);
	case TypeKind::U8:
		return Type::scalar(TypeKind::I8);
	case TypeKind::I16:
		return Type::scalar(TypeKind::U16);
	case TypeKind::U16:
		return Type::scalar(TypeKind::I16);
	case TypeKind::I32:
		return Type::scalar(TypeKind::U32);
	case TypeKind::U32:
		return Type::scalar(TypeKind::I32);
	case TypeKind::I64:
		return Type::scalar(TypeKind::Ptr);
	case TypeKind::Ptr:
		return Type::scalar(TypeKind::I64);
	default:
		return t;
	}
}

uint64_t type_key(Type t)
{
	uint64_t bits;
	__builtin_memcpy(&bits, &t, sizeof bits);
	return bits;
}

// Memory contents known at each point of a dominator-tree walk: the value a
// nonvolatile load read or a store wrote, by address and type, and the
// byte ranges known zero (an integer or pointer load there reads 0; a
// store of what the location holds goes). A block
// inherits its immediate dominator's facts only when that is its only
// predecessor; any write that may alias a fact ends it (distinct slots and
// globals, and disjoint constant ranges of one, do not alias; a slot whose
// address never escapes is not reached through an unknown pointer).
class MemoryForwarding
{
public:
	explicit MemoryForwarding(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	// The containers, kept across runs for their capacity (run() clears
	// or assigns each; diamond_join's marks are compared with an epoch the
	// pool keeps rising, so earlier runs' marks never match).
	struct Pool;
	void exchange(Pool& pool);
	bool run();

private:
	AddressFacts facts_of(Value address) const;
	bool may_alias(const AddressFacts& a, uint64_t a_bytes, const AddressFacts& b, uint64_t b_bytes) const;
	void invalidate(Value address, uint64_t bytes, const Instruction* writer = nullptr);
	void invalidate_all()
	{
		generation_ = ++clock_;
		live_.clear();
		zeros_.clear();
	}
	bool known_zero(Value address, uint64_t bytes) const;
	void note_zero(Value address, uint64_t bytes);
	void remember(Value address, Type type, Value value, bool typed);
	void process(BlockId block);
	InstId known_copy(BlockId block, InstId prev, InstId id);
	InstId field_copy(BlockId block, InstId prev, InstId id);
	void drop_unread_writes();
	bool may_read_slot(const Instruction& inst, uint32_t slot, uint64_t bytes) const;
	bool may_write_slot(const Instruction& inst, uint32_t slot, uint64_t bytes) const;
	bool overwrites_slot(const Instruction& inst, uint32_t slot, uint64_t bytes) const;
	bool slot_dead_from(BlockId start, uint32_t slot, uint64_t bytes) const;
	void sink_slot_stores();
	bool diamond_join(BlockId block);
	void replay_arms();
	void escapes();
	Value value_at_end(BlockId block, Value address, Type type, bool typed = false);
	Value value_at_end(BlockId block, const Key& key, const AddressFacts& want, Type type, bool typed);
	bool may_clobber(const Instruction& inst, const Key& key, const AddressFacts& want, Type type, bool typed) const;
	void phi_loads();
	bool phi_load(BlockId block, InstId prev, InstId id, InstId phi_id);
	bool pre_load(BlockId block, InstId prev, InstId id);
	bool chain_load(BlockId block, InstId prev, InstId id);
	Value append_before_jump(BlockId block, Instruction copy);
	void replace_with_phi(BlockId block, InstId prev, InstId id, const std::vector<Value>& pairs);
	void place_phi(BlockId block, TempId result, Type type, const std::vector<Value>& pairs);
	// phi_loads' per-block instruction lists, built on first use.
	// (Each block's (start, size) in list_arena_; size 0: not built.)
	std::pair<uint32_t, uint32_t> block_list(BlockId block);
	std::vector<std::pair<uint32_t, uint32_t>> lists_;
	std::vector<InstId> list_arena_;
	// phi_loads' per-block memory effects (kWrites* bits), built on first
	// use; a rewrite adds no writer, so they hold for the pass.
	uint8_t block_writes(BlockId block);
	std::vector<uint8_t> writes_;
	// phi_loads' per-block loop header above a block's chain of single
	// predecessors (chain_load; 0: not built, 1: none, else header + 2).
	BlockId chain_header(BlockId block);
	std::vector<uint32_t> headers_;
	Key key_of(Value address, Type type) const { return key_of(facts_of(address), address, type); }
	static Key key_of(const AddressFacts& f, Value address, Type type)
	{
		if (f.base != ValueKind::None)
			return Key{ f.base, f.index, f.offset, type_key(type) };
		return Key{ address.kind, address.index, 0, type_key(type) };
	}

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	std::vector<AddressFacts> address_; // per temp
	std::vector<uint8_t> escaped_;      // per slot: address leaves the analysis
	// A location's fact; generation 0 marks none (entries are reset in
	// place rather than erased). A location keeps its entry of facts_.
	struct Fact
	{
		Key key;
		Known known;
	};
	Known* find(const Key& key)
	{
		const uint32_t* at = index_.find(key);
		return at != nullptr ? &facts_[*at].known : nullptr;
	}
	FlatMap<Key, uint32_t, KeyHash> index_; // location -> its entry of facts_
	std::vector<Fact> facts_;
	std::vector<std::pair<uint32_t, Known>> undo_; // (entry, previous fact; generation 0: absent)
	std::vector<uint32_t> live_;                   // entries inserted in the current generation chain
	std::vector<ZeroRange> zeros_;                 // known zero bytes at the current point
	std::vector<ZeroRange> zeros_scratch_;         // invalidate's next zeros_
	// The walk's saved live_ and zeros_ (each frame's from its offsets on).
	std::vector<uint32_t> saved_live_;
	std::vector<ZeroRange> saved_zeros_;
	uint32_t generation_ = 1;
	uint32_t clock_ = 1;
	std::vector<Value> repl_;
	std::vector<BlockId> arms_;      // diamond_join's blocks between the dominator and the join
	std::vector<uint32_t> arm_mark_; // per block: arm_epoch_ when in arms_
	uint32_t arm_epoch_ = 0;
	bool changed_ = false;
	bool replaced_ = false; // some repl_ entry is set
};

struct MemoryForwarding::Pool
{
	std::vector<AddressFacts> address;
	std::vector<uint8_t> escaped;
	FlatMap<Key, uint32_t, KeyHash> index;
	std::vector<Fact> facts;
	std::vector<std::pair<uint32_t, Known>> undo;
	std::vector<uint32_t> live, saved_live, arm_mark;
	std::vector<ZeroRange> zeros, zeros_scratch, saved_zeros;
	std::vector<Value> repl;
	std::vector<BlockId> arms;
	uint32_t arm_epoch = 0;
	std::vector<std::pair<uint32_t, uint32_t>> lists;
	std::vector<InstId> list_arena;
	std::vector<uint8_t> writes;
	std::vector<uint32_t> headers;
};

void MemoryForwarding::exchange(Pool& pool)
{
	address_.swap(pool.address);
	escaped_.swap(pool.escaped);
	index_.swap(pool.index);
	facts_.swap(pool.facts);
	undo_.swap(pool.undo);
	live_.swap(pool.live);
	saved_live_.swap(pool.saved_live);
	arm_mark_.swap(pool.arm_mark);
	zeros_.swap(pool.zeros);
	zeros_scratch_.swap(pool.zeros_scratch);
	saved_zeros_.swap(pool.saved_zeros);
	repl_.swap(pool.repl);
	arms_.swap(pool.arms);
	std::swap(arm_epoch_, pool.arm_epoch);
	lists_.swap(pool.lists);
	list_arena_.swap(pool.list_arena);
	writes_.swap(pool.writes);
	headers_.swap(pool.headers);
}

// A slot or global, a constant offset into it; or, for an address not
// known to be one, the pointer temporary it steps from by a constant (an
// opaque base: `this` and its fields).
AddressFacts MemoryForwarding::facts_of(Value v) const
{
	AddressFacts f;
	if (v.is(ValueKind::Slot) || v.is(ValueKind::Symbol))
	{
		f.base = v.kind;
		f.index = v.index;
		return f;
	}
	if (v.is(ValueKind::Temp) && v.index < address_.size())
	{
		if (address_[v.index].base != ValueKind::None || !body_.temps[v.index].type.is_pointer())
			return address_[v.index];
		f.base = ValueKind::Temp;
		f.index = v.index;
	}
	return f;
}

bool MemoryForwarding::may_alias(const AddressFacts& a, uint64_t a_bytes, const AddressFacts& b, uint64_t b_bytes) const
{
	const bool ka = a.base != ValueKind::None, kb = b.base != ValueKind::None;
	if (ka && kb)
	{
		if (a.base == b.base && a.index == b.index)
			return a.offset < b.offset + static_cast<int64_t>(b_bytes) &&
				b.offset < a.offset + static_cast<int64_t>(a_bytes);
		// Distinct slots and globals are disjoint; a pointer base may
		// point anywhere but into a private slot.
		if (a.base != ValueKind::Temp && b.base != ValueKind::Temp)
			return false;
		if ((a.base == ValueKind::Slot && !escaped_[a.index]) || (b.base == ValueKind::Slot && !escaped_[b.index]))
			return false;
		return true;
	}
	// An unknown pointer reaches no private slot.
	if (ka && a.base == ValueKind::Slot && !escaped_[a.index])
		return false;
	if (kb && b.base == ValueKind::Slot && !escaped_[b.index])
		return false;
	return true;
}

// Facts a write may alias lapse in place (their previous state on the
// undo list); the others stay as they are. O(live facts).
void MemoryForwarding::invalidate(Value address, uint64_t bytes, const Instruction* writer)
{
	const AddressFacts w = facts_of(address);
	const bool typed = writer != nullptr && (writer->flags & IF_TYPED) != 0;
	size_t kept = 0;
	for (size_t k = 0; k < live_.size(); ++k)
	{
		Known& found = facts_[live_[k]].known;
		if (found.generation != generation_)
			continue;
		const AddressFacts& f = found.at;
		// (Typed apart: only the same base and offset may still be one
		// object, a union's members.)
		const bool apart = typed && typed_apart(true, writer->type, found.typed, found.type) &&
			!(w.base != ValueKind::None && w.base == f.base && w.index == f.index);
		if (!apart && may_alias(w, bytes, f, found.bytes))
		{
			undo_.push_back(std::make_pair(live_[k], found));
			found.generation = 0;
			continue;
		}
		live_[kept++] = live_[k];
	}
	live_.resize(kept);
	if (zeros_.empty())
		return;
	// A zero range keeps the bytes on either side of a write within its
	// own base; any other aliasing write ends it.
	std::vector<ZeroRange>& zeros = zeros_scratch_;
	zeros.clear();
	for (const ZeroRange& r : zeros_)
	{
		AddressFacts rf = r.base;
		rf.offset = r.start;
		if (!may_alias(w, bytes, rf, static_cast<uint64_t>(r.end - r.start)))
			zeros.push_back(r);
		else if (w.base == r.base.base && w.index == r.base.index && bytes < (static_cast<uint64_t>(1) << 32))
		{
			const int64_t end = w.offset + static_cast<int64_t>(bytes);
			if (r.start < w.offset)
				zeros.push_back(ZeroRange{ r.base, r.start, w.offset });
			if (end < r.end)
				zeros.push_back(ZeroRange{ r.base, end, r.end });
		}
	}
	zeros_.swap(zeros);
}

bool MemoryForwarding::known_zero(Value address, uint64_t bytes) const
{
	const AddressFacts f = facts_of(address);
	if (f.base == ValueKind::None)
		return false;
	for (const ZeroRange& r : zeros_)
		if (r.base.base == f.base && r.base.index == f.index && r.start <= f.offset &&
			f.offset + static_cast<int64_t>(bytes) <= r.end)
			return true;
	return false;
}

// The most zero ranges one point of the walk keeps (the oldest goes).
const size_t kMaxZeroRanges = 16;

void MemoryForwarding::note_zero(Value address, uint64_t bytes)
{
	AddressFacts f = facts_of(address);
	if (f.base == ValueKind::None || bytes == 0 || bytes >= (static_cast<uint64_t>(1) << 32))
		return;
	if (zeros_.size() >= kMaxZeroRanges)
		zeros_.erase(zeros_.begin());
	const int64_t start = f.offset;
	f.offset = 0;
	zeros_.push_back(ZeroRange{ f, start, start + static_cast<int64_t>(bytes) });
}

// The most facts one point of the walk keeps (each write rechecks them).
const size_t kMaxFacts = 64;

void MemoryForwarding::remember(Value address, Type type, Value value, bool typed)
{
	const AddressFacts f = facts_of(address);
	const Key key = key_of(f, address, type);
	const uint32_t* at = index_.find(key);
	Known* found = at != nullptr ? &facts_[*at].known : nullptr;
	const bool live = found != nullptr && found->generation == generation_;
	if (!live && live_.size() >= kMaxFacts)
		invalidate_all();
	const Known fact = { value, generation_, typed, type, f, storage_bytes(type) };
	uint32_t entry;
	if (found != nullptr)
	{
		entry = *at;
		undo_.push_back(std::make_pair(entry, *found));
		*found = fact;
	}
	else
	{
		entry = static_cast<uint32_t>(facts_.size());
		undo_.push_back(std::make_pair(entry, Known{ Value::none(), 0, false, type, AddressFacts(), 0 }));
		facts_.push_back(Fact{ key, fact });
		index_.insert(key, entry);
	}
	if (!live)
		live_.push_back(entry);
}

// Slots whose address reaches anything but loads, stores (as the address),
// index/copy chains, copyobj and zeroinit.
void MemoryForwarding::escapes()
{
	escaped_.assign(body_.slots.size(), 0);
	address_.assign(body_.temps.size(), AddressFacts());
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Addr && inst.result != kNoId)
			{
				address_[inst.result] = facts_of(inst.ops[0]);
				continue;
			}
			if (inst.op == Opcode::Copy && inst.type.is_pointer() && inst.result != kNoId)
			{
				address_[inst.result] = facts_of(inst.ops[0]);
				continue;
			}
			if (inst.op == Opcode::Index && inst.result != kNoId)
			{
				AddressFacts f = facts_of(inst.ops[0]);
				if (f.base != ValueKind::None && inst.ops[1].is(ValueKind::Integer))
				{
					f.offset += static_cast<int64_t>(body_.literals[inst.ops[1].index].low) *
						static_cast<int64_t>(storage_bytes(inst.type));
					address_[inst.result] = f;
					continue;
				}
				if (f.base == ValueKind::Slot)
					escaped_[f.index] = 1;
				continue;
			}
			// A phi may name an address defined later in layout: phis
			// are examined once every address is known.
			if (inst.op == Opcode::Phi)
				continue;
			uint32_t position = 0;
			for_each_operand(body_, inst, [&](const Value& v) {
				const uint32_t at = position++;
				const bool address_use = (inst.op == Opcode::Load && at == 0) ||
					(inst.op == Opcode::Store && at == 1) || inst.op == Opcode::CopyObject ||
					inst.op == Opcode::ZeroInit;
				if (address_use)
					return;
				const AddressFacts f = facts_of(v);
				if (f.base == ValueKind::Slot && (v.is(ValueKind::Temp) || inst.op != Opcode::Load))
					escaped_[f.index] = 1;
			});
		}
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
			i = body_.instructions[i].next)
			for_each_operand(body_, body_.instructions[i], [&](const Value& v) {
				const AddressFacts f = facts_of(v);
				if (f.base == ValueKind::Slot)
					escaped_[f.index] = 1;
			});
}

void MemoryForwarding::process(BlockId block)
{
	InstId prev = kNoId;
	for (InstId i = body_.blocks[block].first; i != kNoId;)
	{
		Instruction& inst = body_.instructions[i];
		const InstId next = inst.next;
		if (replaced_)
			for_each_operand(body_, inst, [&](Value& v) {
				while (v.is(ValueKind::Temp) && v.index < repl_.size() && !repl_[v.index].is(ValueKind::None))
					v = repl_[v.index];
			});
		if (inst.op == Opcode::Load && (inst.flags & IF_VOLATILE) == 0)
		{
			const Key key = key_of(inst.ops[0], inst.type);
			const Known* found = find(key);
			const bool integral = inst.type.is_pointer() ||
				(inst.type.is_integer() && inst.type.kind() != TypeKind::I128);
			if (found != nullptr && found->generation == generation_ && !found->value.is(ValueKind::None))
				repl_[inst.result] = found->value;
			else if (integral && known_zero(inst.ops[0], storage_bytes(inst.type)))
				repl_[inst.result] = integer_value(body_, Literal());
			else if (integral)
			{
				// A same-width access of the other integer (or pointer)
				// type read or wrote the same bits: the load becomes a copy
				// retyping that value.
				const Known* other = find(key_of(inst.ops[0], same_width_twin(inst.type)));
				if (other != nullptr && other->generation == generation_ && !other->value.is(ValueKind::None) &&
					!(other->value.is_literal() && (inst.type.is_pointer() || other->type.is_pointer())))
				{
					const Value address = inst.ops[0];
					inst.op = Opcode::Copy;
					inst.ops[0] = other->value;
					remember(address, inst.type, Value::temp(inst.result), (inst.flags & IF_TYPED) != 0);
					++c_.stats.loads_reused;
					changed_ = true;
					prev = i;
					i = next;
					continue;
				}
			}
			if (!repl_[inst.result].is(ValueKind::None))
			{
				unlink_after(body_, block, prev, i);
				++c_.stats.loads_reused;
				changed_ = true;
				replaced_ = true;
				i = next;
				continue;
			}
			remember(inst.ops[0], inst.type, Value::temp(inst.result), (inst.flags & IF_TYPED) != 0);
		}
		else if (inst.op == Opcode::Store)
		{
			const Value v = inst.ops[0];
			const uint64_t bytes = storage_bytes(inst.type);
			const bool zero = (inst.type.is_pointer() ||
								  (inst.type.is_integer() && inst.type.kind() != TypeKind::I128)) &&
				v.is(ValueKind::Integer) && body_.literals[v.index].low == 0 && body_.literals[v.index].high == 0;
			if ((inst.flags & IF_VOLATILE) == 0 && c_.level >= 2)
			{
				// O2+: a store of what the location already holds (O1
				// keeps them: its inlining fixtures pad with such stores).
				const Known* found = find(key_of(inst.ops[1], inst.type));
				if ((zero && known_zero(inst.ops[1], bytes)) ||
					(found != nullptr && found->generation == generation_ && found->value == v))
				{
					unlink_after(body_, block, prev, i);
					++c_.stats.stores_removed;
					changed_ = true;
					i = next;
					continue;
				}
			}
			// A volatile store writes its location too (remembering nothing:
			// each volatile access stays).
			invalidate(inst.ops[1], bytes, &inst);
			const bool exact = v.is_literal() ? !inst.type.is_float() || v.is(ValueKind::Float)
											  : v.is(ValueKind::Temp) && body_.temps[v.index].type == inst.type;
			if (exact && !v.is(ValueKind::Slot) && (inst.flags & IF_VOLATILE) == 0)
				remember(inst.ops[1], inst.type, v, (inst.flags & IF_TYPED) != 0);
			if (zero && (inst.flags & IF_VOLATILE) == 0)
				note_zero(inst.ops[1], bytes);
		}
		else if (inst.op == Opcode::Load || inst.op == Opcode::Exception || inst.op == Opcode::ExceptionSelector)
		{
			// A volatile load writes nothing; it is never reused.
		}
		else if (inst.op == Opcode::CopyObject)
		{
			InstId rewritten = known_copy(block, prev, i);
			if (rewritten == kNoId)
				rewritten = field_copy(block, prev, i);
			if (rewritten != kNoId)
			{
				i = rewritten; // a zeroinit and stores, or the stores alone, examined next
				continue;
			}
			invalidate(inst.ops[1], static_cast<uint64_t>(inst.immediate));
		}
		else if (inst.op == Opcode::ZeroInit)
		{
			invalidate(inst.ops[0], static_cast<uint64_t>(inst.immediate));
			note_zero(inst.ops[0], static_cast<uint64_t>(inst.immediate));
		}
		else if (may_write_memory(c_.program, inst) && !region_marker(inst.op))
			invalidate(Value::none(), ~static_cast<uint64_t>(0) >> 1); // private slots survive
		prev = i;
		i = next;
	}
}

// A copy whose source bytes are all known -- zero ranges and integer
// literal facts of its base -- becomes a zeroinit of the destination and
// stores of its nonzero eightbytes (each narrowed to its nonzero bytes; at
// most kKnownCopyStores): a temporary built from constants and copied out
// (push_back(T())) is built in place, and its own writes die once nothing
// reads it (drop_unread_writes).
const uint64_t kKnownCopyBytes = 256;
const size_t kKnownCopyStores = 8;
const size_t kWholeCopyStores = 4;

InstId MemoryForwarding::known_copy(BlockId block, InstId prev, InstId id)
{
	const Instruction copy = body_.instructions[id];
	const uint64_t bytes = static_cast<uint64_t>(copy.immediate);
	const AddressFacts src = facts_of(copy.ops[0]);
	if ((copy.flags & IF_VOLATILE) != 0 || bytes == 0 || bytes > kKnownCopyBytes || src.base == ValueKind::None)
		return kNoId;
	std::vector<uint8_t> covered(bytes, 0), image(bytes, 0);
	for (const ZeroRange& r : zeros_)
		if (r.base.base == src.base && r.base.index == src.index)
			for (int64_t b = std::max(r.start, src.offset); b < r.end && b < src.offset + static_cast<int64_t>(bytes);
				++b)
				covered[static_cast<size_t>(b - src.offset)] = 1;
	for (uint32_t entry : live_)
	{
		const Known* found = &facts_[entry].known;
		if (found->generation != generation_ || !found->value.is(ValueKind::Integer) ||
			!(found->type.is_pointer() || (found->type.is_integer() && found->type.kind() != TypeKind::I128)))
			continue;
		const AddressFacts at = found->at;
		const int64_t offset = at.offset - src.offset, size = static_cast<int64_t>(found->bytes);
		if (at.base != src.base || at.index != src.index || offset < 0 || offset + size > static_cast<int64_t>(bytes))
			continue;
		const uint64_t low = body_.literals[found->value.index].low;
		for (int64_t b = 0; b < size; ++b)
		{
			covered[static_cast<size_t>(offset + b)] = 1;
			image[static_cast<size_t>(offset + b)] = static_cast<uint8_t>(low >> (8 * b));
		}
	}
	if (std::find(covered.begin(), covered.end(), 0) != covered.end())
		return kNoId;
	// Per eightbyte: the narrowest aligned store holding its nonzero bytes
	// after a zeroinit; or, for a small object, every eightbyte stored
	// whole (no zeroinit) when that takes fewer instructions.
	std::vector<std::pair<uint64_t, uint64_t>> narrow, whole; // (offset, width)
	for (uint64_t c = 0; c < bytes; c += 8)
	{
		uint64_t lo = 8, hi = 0;
		for (uint64_t b = 0; b < 8 && c + b < bytes; ++b)
			if (image[c + b] != 0)
			{
				lo = std::min(lo, b);
				hi = b + 1;
			}
		for (uint64_t at = c, width = 8; at < c + 8 && at < bytes; width /= 2)
			if (at + width <= bytes)
			{
				whole.push_back(std::make_pair(at, width));
				at += width;
			}
		if (hi == 0)
			continue;
		uint64_t width = 1;
		while (lo / width != (hi - 1) / width)
			width *= 2;
		const uint64_t at = c + lo / width * width;
		if (at + width > bytes)
			return kNoId;
		narrow.push_back(std::make_pair(at, width));
	}
	const bool zeroing = !(whole.size() <= kWholeCopyStores && whole.size() < narrow.size() + (bytes + 15) / 16 + 1);
	if (zeroing && narrow.size() > kKnownCopyStores)
		return kNoId;
	const std::vector<std::pair<uint64_t, uint64_t>>& parts = zeroing ? narrow : whole;
	const Value dst = copy.ops[1];
	const AddressFacts dst_facts = facts_of(dst);
	Instruction& zero = body_.instructions[id];
	zero.op = Opcode::ZeroInit;
	zero.type = Type::scalar(TypeKind::Void);
	zero.ops[0] = dst;
	zero.ops[1] = Value::none();
	InstId after = id;
	for (const std::pair<uint64_t, uint64_t>& part : parts)
	{
		Value address = dst;
		if (part.first != 0)
		{
			Instruction index = make_instruction(Opcode::Index, Type::scalar(TypeKind::I8));
			index.sub = ir_model::IPK_FIELD;
			index.ops[0] = dst;
			index.ops[1] = integer_value(body_, integer_literal(static_cast<int64_t>(part.first)));
			index.location = copy.location;
			const InstId at = append_instruction(body_, index);
			const TempId t = append_temp(body_, Type::scalar(TypeKind::Ptr), at);
			body_.instructions[at].result = t;
			insert_after(body_, block, after, at);
			after = at;
			address_.resize(body_.temps.size(), AddressFacts());
			repl_.resize(body_.temps.size(), Value::none());
			if (dst_facts.base != ValueKind::None)
			{
				address_[t] = dst_facts;
				address_[t].offset += static_cast<int64_t>(part.first);
			}
			address = Value::temp(t);
		}
		uint64_t value = 0;
		for (uint64_t b = 0; b < part.second; ++b)
			value |= static_cast<uint64_t>(image[part.first + b]) << (8 * b);
		const TypeKind kind = part.second == 1 ? TypeKind::U8
			: part.second == 2                 ? TypeKind::U16
			: part.second == 4                 ? TypeKind::U32
											   : TypeKind::I64;
		Instruction store = make_instruction(Opcode::Store, Type::scalar(kind));
		store.ops[0] = integer_value(body_, integer_literal(static_cast<int64_t>(value)));
		store.ops[1] = address;
		store.location = copy.location;
		const InstId at = append_instruction(body_, store);
		insert_after(body_, block, after, at);
		after = at;
	}
	++c_.stats.loads_reused;
	changed_ = true;
	if (zeroing)
		return id;
	const InstId first = body_.instructions[id].next;
	unlink_after(body_, block, prev, id);
	return first;
}

// A small copy whose source the facts tile exactly -- each byte in one
// stored integer or pointer value, a temp or a literal, at most
// kWholeCopyStores of them -- becomes stores of those values: a pair
// built from a variable and copied out (push_back(make_pair(b, 0u))) no
// longer reloads the temporary whole right after its narrower stores,
// which store forwarding cannot serve.
InstId MemoryForwarding::field_copy(BlockId block, InstId prev, InstId id)
{
	const Instruction copy = body_.instructions[id];
	const uint64_t bytes = static_cast<uint64_t>(copy.immediate);
	const AddressFacts src = facts_of(copy.ops[0]);
	if ((copy.flags & IF_VOLATILE) != 0 || bytes == 0 || bytes > 8 * kWholeCopyStores || src.base == ValueKind::None)
		return kNoId;
	struct Part
	{
		uint64_t offset;
		Type type;
		Value value;
	};
	std::vector<Part> parts;
	for (uint32_t entry : live_)
	{
		const Known& found = facts_[entry].known;
		if (found.generation != generation_ ||
			!(found.value.is(ValueKind::Integer) || found.value.is(ValueKind::Temp)) ||
			!(found.type.is_pointer() || (found.type.is_integer() && found.type.kind() != TypeKind::I128)) ||
			(found.value.is(ValueKind::Temp) && !(body_.temps[found.value.index].type == found.type)))
			continue;
		const int64_t offset = found.at.offset - src.offset;
		if (found.at.base != src.base || found.at.index != src.index || offset < 0 ||
			offset + static_cast<int64_t>(found.bytes) > static_cast<int64_t>(bytes))
			continue;
		parts.push_back(Part{ static_cast<uint64_t>(offset), found.type, found.value });
	}
	std::sort(parts.begin(), parts.end(), [](const Part& a, const Part& b) { return a.offset < b.offset; });
	uint64_t next = 0;
	for (const Part& part : parts)
	{
		if (part.offset != next)
			return kNoId;
		next += storage_bytes(part.type);
	}
	if (next != bytes || parts.size() > kWholeCopyStores)
		return kNoId;
	const Value dst = copy.ops[1];
	const AddressFacts dst_facts = facts_of(dst);
	InstId after = id;
	for (const Part& part : parts)
	{
		Value address = dst;
		if (part.offset != 0)
		{
			Instruction index = make_instruction(Opcode::Index, Type::scalar(TypeKind::I8));
			index.sub = ir_model::IPK_FIELD;
			index.ops[0] = dst;
			index.ops[1] = integer_value(body_, integer_literal(static_cast<int64_t>(part.offset)));
			index.location = copy.location;
			const InstId at = append_instruction(body_, index);
			const TempId t = append_temp(body_, Type::scalar(TypeKind::Ptr), at);
			body_.instructions[at].result = t;
			insert_after(body_, block, after, at);
			after = at;
			address_.resize(body_.temps.size(), AddressFacts());
			repl_.resize(body_.temps.size(), Value::none());
			if (dst_facts.base != ValueKind::None)
			{
				address_[t] = dst_facts;
				address_[t].offset += static_cast<int64_t>(part.offset);
			}
			address = Value::temp(t);
		}
		Instruction store = make_instruction(Opcode::Store, part.type);
		store.ops[0] = part.value;
		store.ops[1] = address;
		store.location = copy.location;
		const InstId at = append_instruction(body_, store);
		insert_after(body_, block, after, at);
		after = at;
	}
	++c_.stats.loads_reused;
	changed_ = true;
	const InstId first = body_.instructions[id].next;
	unlink_after(body_, block, prev, id);
	return first;
}

// Writes to a private slot nothing reads (no load or copy from it; its
// address reaches nothing else) go.
void MemoryForwarding::drop_unread_writes()
{
	address_.resize(body_.temps.size(), AddressFacts());
	if (std::find(escaped_.begin(), escaped_.end(), 0) == escaped_.end())
		return; // no private slot
	std::vector<uint8_t> read(body_.slots.size(), 0);
	// (Once every private slot is read, no write goes.)
	size_t unread = static_cast<size_t>(std::count(escaped_.begin(), escaped_.end(), 0));
	for (size_t at = 0; at < body_.layout.size() && unread != 0; ++at)
		for (InstId i = body_.blocks[body_.layout[at]].first; i != kNoId && unread != 0; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Load || inst.op == Opcode::CopyObject)
			{
				const AddressFacts f = facts_of(inst.ops[0]);
				if (f.base == ValueKind::Slot && !read[f.index])
				{
					read[f.index] = 1;
					unread -= escaped_[f.index] == 0 ? 1 : 0;
				}
			}
		}
	if (unread == 0)
		return;
	for (BlockId b : body_.layout)
	{
		InstId prev = kNoId;
		for (InstId i = body_.blocks[b].first; i != kNoId;)
		{
			const Instruction& inst = body_.instructions[i];
			const InstId next = inst.next;
			const Value target = inst.op == Opcode::Store ? inst.ops[1]
				: inst.op == Opcode::CopyObject           ? inst.ops[1]
				: inst.op == Opcode::ZeroInit             ? inst.ops[0]
														  : Value::none();
			const AddressFacts f = target.is(ValueKind::None) ? AddressFacts() : facts_of(target);
			if (f.base == ValueKind::Slot && !escaped_[f.index] && !read[f.index] && (inst.flags & IF_VOLATILE) == 0)
			{
				unlink_after(body_, b, prev, i);
				++c_.stats.stores_removed;
				changed_ = true;
			}
			else
				prev = i;
			i = next;
		}
	}
}

// Whether `inst` may read slot `slot` (`bytes` long): a call; a load or
// copy from it, or from an address that may name it reading at most its
// bytes (an access is within one object: a wider one is not of the slot);
// any other use of its address.
bool MemoryForwarding::may_read_slot(const Instruction& inst, uint32_t slot, uint64_t bytes) const
{
	if (inst.op == Opcode::Call || is_eh_instruction(inst.op) || inst.op == Opcode::Throw || inst.op == Opcode::Resume)
		return true;
	// Forming an address reads nothing (its uses are what count).
	if (inst.op == Opcode::Addr || inst.op == Opcode::Index || (inst.op == Opcode::Copy && inst.type.is_pointer()))
		return false;
	AddressFacts own;
	own.base = ValueKind::Slot;
	own.index = slot;
	auto reads = [&](Value address, uint64_t size) {
		const AddressFacts f = facts_of(address);
		if (f.base == ValueKind::Slot && f.index == slot)
			return true;
		return f.base != ValueKind::Slot && f.base != ValueKind::Symbol && size <= bytes &&
			may_alias(f, size, own, bytes);
	};
	if (inst.op == Opcode::Load)
		return (inst.flags & IF_VOLATILE) != 0 || reads(inst.ops[0], storage_bytes(inst.type));
	if (inst.op == Opcode::CopyObject && reads(inst.ops[0], static_cast<uint64_t>(inst.immediate)))
		return true;
	bool named = false;
	uint32_t position = 0;
	for_each_operand(body_, inst, [&](const Value& v) {
		const uint32_t at = position++;
		const bool address_use = (inst.op == Opcode::Load && at == 0) || (inst.op == Opcode::Store && at == 1) ||
			(inst.op == Opcode::CopyObject && at <= 1) || (inst.op == Opcode::ZeroInit && at == 0);
		if (!address_use)
		{
			const AddressFacts f = facts_of(v);
			named = named || (f.base == ValueKind::Slot && f.index == slot);
		}
	});
	return named;
}

// Whether `inst` may write any of slot `slot` (`bytes` long): a store,
// copy or zeroing whose target may be in it.
bool MemoryForwarding::may_write_slot(const Instruction& inst, uint32_t slot, uint64_t bytes) const
{
	Value target = Value::none();
	uint64_t size = 0;
	if (inst.op == Opcode::Store)
	{
		target = inst.ops[1];
		size = storage_bytes(inst.type);
	}
	else if (inst.op == Opcode::ZeroInit || inst.op == Opcode::CopyObject)
	{
		target = inst.op == Opcode::ZeroInit ? inst.ops[0] : inst.ops[1];
		size = static_cast<uint64_t>(inst.immediate);
	}
	else
		return false;
	AddressFacts own;
	own.base = ValueKind::Slot;
	own.index = slot;
	return may_alias(facts_of(target), size, own, bytes);
}

// Whether `inst` writes all of slot `slot` (`bytes` long).
bool MemoryForwarding::overwrites_slot(const Instruction& inst, uint32_t slot, uint64_t bytes) const
{
	Value target = Value::none();
	uint64_t size = 0;
	if (inst.op == Opcode::Store)
	{
		target = inst.ops[1];
		size = storage_bytes(inst.type);
	}
	else if (inst.op == Opcode::ZeroInit || inst.op == Opcode::CopyObject)
	{
		target = inst.op == Opcode::ZeroInit ? inst.ops[0] : inst.ops[1];
		size = static_cast<uint64_t>(inst.immediate);
	}
	const AddressFacts f = target.is(ValueKind::None) ? AddressFacts() : facts_of(target);
	return f.base == ValueKind::Slot && f.index == slot && f.offset == 0 && size >= bytes;
}

// Whether slot `slot` holds nothing any path from the start of `start`
// reads: each overwrites it whole, or leaves the function, before an
// instruction may read it (at most kSinkBlocks blocks are walked).
const size_t kSinkBlocks = 64;

bool MemoryForwarding::slot_dead_from(BlockId start, uint32_t slot, uint64_t bytes) const
{
	std::vector<BlockId> work(1, start), seen(1, start);
	while (!work.empty())
	{
		const BlockId b = work.back();
		work.pop_back();
		if ((cfg_.flags[b] & BF_HANDLER) != 0)
			return false;
		bool overwritten = false;
		for (InstId i = body_.blocks[b].first; i != kNoId && !overwritten; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (may_read_slot(inst, slot, bytes))
				return false;
			overwritten = overwrites_slot(inst, slot, bytes);
		}
		if (overwritten)
			continue;
		for (uint32_t k = cfg_.succ_first[b]; k < cfg_.succ_first[b + 1]; ++k)
		{
			const BlockId s = cfg_.succs[k];
			if (std::find(seen.begin(), seen.end(), s) != seen.end())
				continue;
			if (seen.size() >= kSinkBlocks)
				return false;
			seen.push_back(s);
			work.push_back(s);
		}
	}
	return true;
}

// O2+: a store to a slot that nothing after it in its block may read,
// where the block branches two ways and the slot is dead along one of
// them (slot_dead_from), moves to the start of the other successor when
// that successor has no other predecessor. An inlined push_back's
// by-reference argument temporary is then stored only on the path that
// passes it to the growth call, not on every fast-path element.
void MemoryForwarding::sink_slot_stores()
{
	if (c_.level < 2)
		return;
	for (BlockId b : body_.layout)
	{
		const Instruction& term = body_.instructions[body_.blocks[b].last];
		if (term.op != Opcode::Branch || cfg_.succ_first[b + 1] - cfg_.succ_first[b] != 2 ||
			cfg_.succs[cfg_.succ_first[b]] == cfg_.succs[cfg_.succ_first[b] + 1])
			continue;
		// The last store to a slot in the block with nothing after it that
		// may read or write the slot (a later write through its address, a
		// copy into it, stays after it).
		InstId prev = kNoId, store = kNoId, store_prev = kNoId;
		uint32_t slot = 0;
		for (InstId i = body_.blocks[b].first; i != kNoId; prev = i, i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (store != kNoId &&
				(may_read_slot(inst, slot, storage_bytes(body_.slots[slot].type)) ||
					may_write_slot(inst, slot, storage_bytes(body_.slots[slot].type))))
				store = kNoId;
			if (inst.op == Opcode::Store && (inst.flags & IF_VOLATILE) == 0 && inst.ops[1].is(ValueKind::Slot))
			{
				store = i;
				store_prev = prev;
				slot = inst.ops[1].index;
			}
		}
		if (store == kNoId)
			continue;
		const uint64_t bytes = storage_bytes(body_.slots[slot].type);
		if (!overwrites_slot(body_.instructions[store], slot, bytes))
			continue;
		const BlockId s0 = cfg_.succs[cfg_.succ_first[b]], s1 = cfg_.succs[cfg_.succ_first[b] + 1];
		BlockId live = kNoId;
		if (slot_dead_from(s1, slot, bytes))
			live = s0;
		else if (slot_dead_from(s0, slot, bytes))
			live = s1;
		if (live == kNoId || cfg_.pred_count(live) != 1 || (cfg_.flags[live] & BF_HANDLER) != 0)
			continue;
		InstId after = kNoId;
		for (InstId i = body_.blocks[live].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
			i = body_.instructions[i].next)
			after = i;
		unlink_after(body_, b, store_prev, store);
		insert_after(body_, live, after, store);
		++c_.stats.stores_removed;
		changed_ = true;
	}
}

// Whether the facts at the end of `block`'s immediate dominator, less what
// the paths from it write, hold at its start: the blocks those paths pass
// (walking back from the predecessors to the dominator: the arms of a
// diamond or of a chain of tests, a slow path's chain) number at most
// kArmBlocks and none is a handler; each one's writes lapse the facts they
// may alias (replay_arms).
const size_t kArmBlocks = 32;

bool MemoryForwarding::diamond_join(BlockId block)
{
	const BlockId dom = cfg_.idom[block];
	if (cfg_.pred_count(block) == 0)
		return false;
	if (arm_mark_.size() != body_.blocks.size())
		arm_mark_.assign(body_.blocks.size(), 0);
	++arm_epoch_;
	arms_.clear();
	BlockId from = block;
	for (size_t walked = 0;; from = arms_[walked++])
	{
		if (cfg_.pred_count(from) == 0)
			return false;
		for (uint32_t k = cfg_.pred_first[from]; k < cfg_.pred_first[from + 1]; ++k)
		{
			const BlockId p = cfg_.preds[k];
			if (p == dom || arm_mark_[p] == arm_epoch_)
				continue;
			if (arms_.size() == kArmBlocks || (cfg_.flags[p] & BF_HANDLER) != 0)
				return false;
			arm_mark_[p] = arm_epoch_;
			arms_.push_back(p);
		}
		if (walked == arms_.size())
			return true;
	}
}

void MemoryForwarding::replay_arms()
{
	for (BlockId p : arms_)
		for (InstId i = body_.blocks[p].first; i != kNoId; i = body_.instructions[i].next)
		{
			if (live_.empty() && zeros_.empty())
				return; // nothing left to lapse
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Store)
				invalidate(inst.ops[1], storage_bytes(inst.type), &inst);
			else if (inst.op == Opcode::CopyObject)
				invalidate(inst.ops[1], static_cast<uint64_t>(inst.immediate));
			else if (inst.op == Opcode::ZeroInit)
				invalidate(inst.ops[0], static_cast<uint64_t>(inst.immediate));
			else if (inst.op != Opcode::Load && inst.op != Opcode::Exception && inst.op != Opcode::ExceptionSelector &&
				!region_marker(inst.op) && may_write_memory(c_.program, inst))
				invalidate(Value::none(), ~static_cast<uint64_t>(0) >> 1);
		}
}

bool MemoryForwarding::run()
{
	c_.ensure_cfg();
	if (cfg_.rpo.empty())
		return false;
	index_.clear(body_.instructions.size());
	facts_.clear();
	undo_.clear();
	live_.clear();
	zeros_.clear();
	saved_live_.clear();
	saved_zeros_.clear();
	escapes();
	repl_.assign(body_.temps.size(), Value::none());
	const DomChildren& tree = c_.ensure_dom();
	// (block, next child, undo mark, generation, saved live_ and zeros_)
	// per frame.
	struct Frame
	{
		BlockId block;
		uint32_t child;
		size_t undo;
		uint32_t generation;
		size_t live, zeros;
	};
	std::vector<Frame> stack;
	auto enter = [&](BlockId b) {
		stack.push_back(Frame{ b, 0, undo_.size(), generation_, saved_live_.size(), saved_zeros_.size() });
		saved_live_.insert(saved_live_.end(), live_.begin(), live_.end());
		saved_zeros_.insert(saved_zeros_.end(), zeros_.begin(), zeros_.end());
		// (Nothing known: a fresh generation is the same as inheriting.)
		const bool inherits = (!live_.empty() || !zeros_.empty()) && cfg_.idom[b] != kNoId &&
			(cfg_.flags[b] & BF_HANDLER) == 0 && diamond_join(b);
		if (!inherits)
		{
			generation_ = ++clock_;
			live_.clear();
			zeros_.clear();
		}
		else
			replay_arms();
		process(b);
	};
	enter(cfg_.rpo[0]);
	while (!stack.empty())
	{
		Frame& top = stack.back();
		if (tree.first[top.block] + top.child < tree.first[top.block + 1])
		{
			const BlockId child = tree.children[tree.first[top.block] + top.child++];
			enter(child);
			continue;
		}
		for (size_t u = undo_.size(); u > top.undo; --u)
		{
			const std::pair<uint32_t, Known>& e = undo_[u - 1];
			facts_[e.first].known = e.second; // generation 0: none
		}
		undo_.resize(top.undo);
		generation_ = top.generation;
		live_.assign(saved_live_.begin() + static_cast<std::ptrdiff_t>(top.live), saved_live_.end());
		zeros_.assign(saved_zeros_.begin() + static_cast<std::ptrdiff_t>(top.zeros), saved_zeros_.end());
		saved_live_.resize(top.live);
		saved_zeros_.resize(top.zeros);
		stack.pop_back();
	}
	// Uses in blocks the walk resolved later (phis on back edges).
	if (replaced_)
		for (BlockId b : body_.layout)
			for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
				for_each_operand(body_, body_.instructions[i], [&](Value& v) {
					while (v.is(ValueKind::Temp) && v.index < repl_.size() && !repl_[v.index].is(ValueKind::None))
						v = repl_[v.index];
				});
	drop_unread_writes();
	sink_slot_stores();
	phi_loads();
	return changed_;
}

// The value at `address` (type `type`) when control leaves `block`: the
// last load or store of that location before the end, searching back
// through single predecessors (at most kPhiLoadDepth blocks) and stopping
// at any write that may alias it.
const unsigned kPhiLoadDepth = 3;

// At most this many instructions are examined per question.
const unsigned kPhiLoadScan = 128;

// Whether `inst`, which may write memory (may_write_memory: the callers'
// cheap test first), may write the location `want` (key `key`, `type`,
// typed access `typed`): a store of another location that may alias it,
// an object copy or fill over it, or any other writer (a call) unless the
// location is a slot whose address never leaves the analysis.
bool MemoryForwarding::may_clobber(
	const Instruction& inst, const Key& key, const AddressFacts& want, Type type, bool typed) const
{
	if (inst.op == Opcode::Store)
	{
		if ((inst.flags & IF_VOLATILE) != 0 || key_of(inst.ops[1], inst.type) == key)
			return true;
		const AddressFacts f = facts_of(inst.ops[1]);
		const bool apart = typed_apart((inst.flags & IF_TYPED) != 0, inst.type, typed, type) &&
			!(f.base != ValueKind::None && f.base == want.base && f.index == want.index);
		return !apart && may_alias(f, storage_bytes(inst.type), want, storage_bytes(type));
	}
	if (inst.op == Opcode::CopyObject || inst.op == Opcode::ZeroInit)
	{
		const Value dst = inst.op == Opcode::CopyObject ? inst.ops[1] : inst.ops[0];
		return may_alias(facts_of(dst), static_cast<uint64_t>(inst.immediate), want, storage_bytes(type));
	}
	return !(want.base == ValueKind::Slot && !escaped_[want.index]);
}

Value MemoryForwarding::value_at_end(BlockId block, Value address, Type type, bool typed)
{
	return value_at_end(block, key_of(address, type), facts_of(address), type, typed);
}

Value MemoryForwarding::value_at_end(BlockId block, const Key& key, const AddressFacts& want, Type type, bool typed)
{
	unsigned budget = kPhiLoadScan;
	for (unsigned depth = 0; depth < kPhiLoadDepth && block != kNoId; ++depth)
	{
		const std::pair<uint32_t, uint32_t> list = block_list(block);
		for (size_t k = list.second; k-- > 0;)
		{
			if (budget-- == 0)
				return Value::none();
			const Instruction& inst = body_.instructions[list_arena_[list.first + k]];
			if (inst.op == Opcode::Load)
			{
				if ((inst.flags & IF_VOLATILE) == 0 && type_key(inst.type) == key.type &&
					key_of(inst.ops[0], inst.type) == key)
					return Value::temp(inst.result);
				continue;
			}
			if (!may_write_memory(c_.program, inst))
				continue;
			if (inst.op == Opcode::Store && (inst.flags & IF_VOLATILE) == 0 && type_key(inst.type) == key.type &&
				key_of(inst.ops[1], inst.type) == key)
			{
				const Value v = inst.ops[0];
				const bool exact = v.is_literal() ? !type.is_float() || v.is(ValueKind::Float)
												  : v.is(ValueKind::Temp) && body_.temps[v.index].type == type;
				return exact ? v : Value::none();
			}
			if (may_clobber(inst, key, want, type, typed))
				return Value::none();
		}
		if (cfg_.pred_count(block) == 1)
		{
			block = cfg_.preds[cfg_.pred_first[block]];
			continue;
		}
		// A join: its immediate dominator's value when nothing on the paths
		// between writes the location (a fast path's store, merged with a
		// slow path that stored elsewhere).
		if (!diamond_join(block))
			return Value::none();
		for (BlockId p : arms_)
			for (InstId i = body_.blocks[p].first; i != kNoId; i = body_.instructions[i].next)
			{
				if (budget-- == 0)
					return Value::none();
				const Instruction& inst = body_.instructions[i];
				if (may_write_memory(c_.program, inst) && may_clobber(inst, key, want, type, typed))
					return Value::none();
			}
		block = cfg_.idom[block];
	}
	return Value::none();
}

// A load through a phi of addresses (or a copy of one) in the phi's block
// reads, on each incoming edge, the value that edge's address held: a phi
// of those values when every predecessor knows its own.
void MemoryForwarding::phi_loads()
{
	c_.entry_block_of(forward_memory, !changed_);
	lists_.assign(body_.blocks.size(), std::make_pair(0u, 0u));
	list_arena_.clear();
	writes_.assign(body_.blocks.size(), 0);
	headers_.assign(body_.blocks.size(), 0);
	for (BlockId b : body_.layout)
	{
		InstId prev = kNoId;
		// Whether an instruction before the current one may write memory.
		bool written = false;
		for (InstId i = body_.blocks[b].first; i != kNoId;)
		{
			const InstId next = body_.instructions[i].next;
			const Instruction& load = body_.instructions[i];
			bool rewritten = false;
			if (load.op == Opcode::Load && (load.flags & IF_VOLATILE) == 0 &&
				(load.type.is_integer() ? load.type.kind() != TypeKind::I128
										: load.type.is_pointer() || load.type.kind() == TypeKind::F32 ||
							load.type.kind() == TypeKind::F64) &&
				(cfg_.flags[b] & BF_HANDLER) == 0 && !written)
			{
				Value address = load.ops[0];
				for (unsigned step = 0; step < 3 && address.is(ValueKind::Temp); ++step)
				{
					const InstId def = body_.temps[address.index].def;
					if (def == kNoId || body_.instructions[def].op != Opcode::Copy)
						break;
					address = body_.instructions[def].ops[0];
				}
				const InstId def = address.is(ValueKind::Temp) ? body_.temps[address.index].def : kNoId;
				const bool phi_address = def != kNoId && body_.instructions[def].op == Opcode::Phi &&
					c_.block_of.size() > def && c_.block_of[def] == b;
				rewritten = phi_address       ? phi_load(b, prev, i, def)
					: cfg_.pred_count(b) == 1 ? chain_header(b) != kNoId && chain_load(b, prev, i)
											  : pre_load(b, prev, i);
			}
			if (!rewritten)
			{
				prev = i;
				// (A rewritten load writes nothing.)
				const Instruction& inst = body_.instructions[i];
				written = written ||
					(inst.op != Opcode::Phi && inst.op != Opcode::Load &&
						(may_write_memory(c_.program, inst) || inst.op == Opcode::Store));
			}
			else
			{
				lists_[b].second = 0; // the block changed
				prev = next == kNoId ? kNoId : previous_in_block(body_, b, next);
			}
			i = next;
		}
	}
}

std::pair<uint32_t, uint32_t> MemoryForwarding::block_list(BlockId block)
{
	std::pair<uint32_t, uint32_t>& list = lists_[block];
	if (list.second == 0)
	{
		list.first = static_cast<uint32_t>(list_arena_.size());
		for (InstId i = body_.blocks[block].first; i != kNoId; i = body_.instructions[i].next)
			list_arena_.push_back(i);
		list.second = static_cast<uint32_t>(list_arena_.size()) - list.first;
	}
	return list;
}

bool MemoryForwarding::phi_load(BlockId block, InstId prev, InstId id, InstId phi_id)
{
	const Instruction phi = body_.instructions[phi_id];
	const Type type = body_.instructions[id].type;
	std::vector<Value> pairs;
	for (uint32_t k = 0; k < phi.extra_count; k += 2)
	{
		const BlockId pred = body_.operands[phi.extra_first + k].index;
		const Value v = value_at_end(
			pred, body_.operands[phi.extra_first + k + 1], type, (body_.instructions[id].flags & IF_TYPED) != 0);
		if (v.is(ValueKind::None))
			return false;
		pairs.push_back(Value::block(pred));
		pairs.push_back(v);
	}
	replace_with_phi(block, prev, id, pairs);
	return true;
}

// Partial redundancy of a load at a join (pre_load), before anything in
// its block writes memory, of an address available in every predecessor:
// when some predecessors know the location's value at their end and each
// of the others only jumps to the join, those others load it last (the
// load runs at most as often as before on every path) and a phi of the
// values replaces it. A loop header's load becomes the preheader's and the
// latch's store; a reload after a slow-path call stays on that path only.
// A join has at most this many predecessors.
const uint32_t kPreLoadPreds = 8;

// A load in a block with one predecessor (chain_load), before anything in
// its block writes memory: when the chain of single predecessors above it
// reaches a join without passing a write of the location (at most
// kChainBlocks blocks, kPhiLoadScan instructions), the load reads the
// location's value on entry to that join, and when every predecessor of
// the join knows that value at its end, a phi of theirs at the join
// replaces the load. No load is inserted: a loop header's body reads the
// preheader's value and the latch's reload after its call, through the
// phi, without a load of its own; a load that is not anticipated at the
// join stays.
const unsigned kChainBlocks = 3;

// block_writes' bits.
const uint8_t kWritesKnown = 1;  // built
const uint8_t kWritesStore = 2;  // a store, object copy or fill
const uint8_t kWritesOpaque = 4; // another writer (a call): everything but a private slot

uint8_t MemoryForwarding::block_writes(BlockId block)
{
	uint8_t& w = writes_[block];
	if (w == 0)
	{
		w = kWritesKnown;
		for (InstId i = body_.blocks[block].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Store || inst.op == Opcode::CopyObject || inst.op == Opcode::ZeroInit)
				w |= kWritesStore;
			else if (inst.op != Opcode::Load && inst.op != Opcode::Phi && may_write_memory(c_.program, inst))
				w |= kWritesOpaque;
		}
	}
	return w;
}

// The chain of single predecessors above `block` (at most kChainBlocks,
// no handler), the join last; its length, 0 when it reaches no join.
static unsigned chain_above(const Cfg& cfg, BlockId block, BlockId chain[kChainBlocks])
{
	unsigned length = 0;
	for (BlockId b = cfg.preds[cfg.pred_first[block]];; b = cfg.preds[cfg.pred_first[b]])
	{
		if (length == kChainBlocks || (cfg.flags[b] & BF_HANDLER) != 0 || b == block)
			return 0;
		chain[length++] = b;
		const uint32_t preds = cfg.pred_count(b);
		if (preds >= 2)
			return length;
		if (preds == 0)
			return 0;
	}
}

BlockId MemoryForwarding::chain_header(BlockId block)
{
	uint32_t& cached = headers_[block];
	if (cached == 0)
	{
		BlockId chain[kChainBlocks];
		const unsigned length = chain_above(cfg_, block, chain);
		BlockId join = length == 0 ? kNoId : chain[length - 1];
		if (join != kNoId && (cfg_.pred_count(join) > kPreLoadPreds || join == cfg_.rpo[0]))
			join = kNoId;
		// A loop header (a predecessor it dominates): the shape that pays.
		bool header = false;
		if (join != kNoId)
			for (uint32_t k = cfg_.pred_first[join]; k < cfg_.pred_first[join + 1] && !header; ++k)
				header = cfg_.dominates(join, cfg_.preds[k]);
		cached = header ? join + 2 : 1;
	}
	return cached == 1 ? kNoId : cached - 2;
}

bool MemoryForwarding::chain_load(BlockId block, InstId prev, InstId id)
{
	BlockId chain[kChainBlocks];
	const unsigned length = chain_above(cfg_, block, chain);
	const BlockId join = chain[length - 1]; // chain_header(block)
	const Instruction load = body_.instructions[id];
	const Value address = load.ops[0];
	const Type type = load.type;
	const bool typed = (load.flags & IF_TYPED) != 0;
	const AddressFacts want = facts_of(address);
	const bool private_slot = want.base == ValueKind::Slot && !escaped_[want.index];
	// The location must be the same one on every path into the join: its
	// base (the address itself when nothing is known of it) is defined
	// strictly above the join; or it is the join's own phi, which names
	// on each edge the object that edge brings (the predecessor then
	// answers for the location at its own offset from that object).
	InstId base_phi = kNoId;
	if (want.base != ValueKind::Slot && want.base != ValueKind::Symbol)
	{
		const Value base = want.base == ValueKind::Temp ? Value::temp(want.index) : address;
		if (base.is(ValueKind::Temp))
		{
			const InstId def = body_.temps[base.index].def;
			if (def != kNoId &&
				(def >= c_.block_of.size() || c_.block_of[def] == kNoId || !cfg_.dominates(c_.block_of[def], join)))
				return false;
			if (def != kNoId && c_.block_of[def] == join)
			{
				if (body_.instructions[def].op != Opcode::Phi || want.base != ValueKind::Temp)
					return false;
				base_phi = def;
			}
		}
	}
	// Nothing on the chain (the join included) writes the location.
	const Key key = key_of(want, address, type);
	unsigned budget = kPhiLoadScan;
	for (unsigned k = 0; k < length; ++k)
	{
		const uint8_t w = block_writes(chain[k]);
		if ((w & kWritesOpaque) != 0 && !private_slot)
			return false;
		if ((w & (kWritesStore | kWritesOpaque)) == 0)
			continue;
		for (InstId i = body_.blocks[chain[k]].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (budget-- == 0 || (may_write_memory(c_.program, inst) && may_clobber(inst, key, want, type, typed)))
				return false;
		}
	}
	// The forward edges first: a location unknown on entry to the loop
	// (most of them) is settled by one short scan, without the search
	// back through the body that a latch usually loses to a call.
	const uint32_t first = cfg_.pred_first[join], count = cfg_.pred_count(join);
	Value known[kPreLoadPreds];
	for (unsigned pass = 0; pass < 2; ++pass)
		for (uint32_t k = 0; k < count; ++k)
		{
			const BlockId pred = cfg_.preds[first + k];
			if (cfg_.dominates(join, pred) != (pass == 1))
				continue;
			if (base_phi == kNoId)
				known[k] = value_at_end(pred, address, type, typed);
			else
			{
				const Value in = phi_incoming(body_, body_.instructions[base_phi], pred);
				AddressFacts f = facts_of(in);
				if (f.base == ValueKind::None)
				{
					if (!in.is(ValueKind::Temp))
						return false;
					f.base = ValueKind::Temp;
					f.index = in.index;
				}
				f.offset += want.offset;
				known[k] = value_at_end(pred, key_of(f, in, type), f, type, typed);
			}
			if (known[k].is(ValueKind::None))
				return false;
		}
	std::vector<Value> pairs;
	for (uint32_t k = 0; k < count; ++k)
	{
		pairs.push_back(Value::block(cfg_.preds[first + k]));
		pairs.push_back(known[k]);
	}
	// The phi joins the load's block's chain at its head: it dominates
	// every use of the load.
	unlink_after(body_, block, prev, id);
	place_phi(join, load.result, type, pairs);
	lists_[join].second = 0;
	return true;
}

bool MemoryForwarding::pre_load(BlockId block, InstId prev, InstId id)
{
	const Instruction load = body_.instructions[id];
	const Value address = load.ops[0];
	const uint32_t preds = cfg_.pred_count(block);
	if (preds < 2 || preds > kPreLoadPreds || block == cfg_.rpo[0])
		return false;
	// The address: available at the join, or a constant step (index, addr)
	// from one, which a predecessor recomputes.
	auto available = [&](Value v) {
		if (!v.is(ValueKind::Temp))
			return v.is(ValueKind::Slot) || v.is(ValueKind::Symbol);
		const InstId def = body_.temps[v.index].def;
		return def == kNoId ||
			(def < c_.block_of.size() && c_.block_of[def] != kNoId && c_.block_of[def] != block &&
				cfg_.dominates(c_.block_of[def], block));
	};
	InstId step = kNoId;
	if (!available(address))
	{
		if (!address.is(ValueKind::Temp))
			return false;
		step = body_.temps[address.index].def;
		const Instruction& def = body_.instructions[step];
		if (!((def.op == Opcode::Index && def.ops[1].is(ValueKind::Integer)) || def.op == Opcode::Addr) ||
			!available(def.ops[0]))
			return false;
	}
	std::vector<Value> pairs;
	std::vector<BlockId> missing;
	for (uint32_t k = cfg_.pred_first[block]; k < cfg_.pred_first[block + 1]; ++k)
	{
		const BlockId pred = cfg_.preds[k];
		const Value v = value_at_end(pred, address, load.type, (load.flags & IF_TYPED) != 0);
		if (v.is(ValueKind::None))
		{
			const InstId last = body_.blocks[pred].last;
			if (cfg_.succ_count(pred) != 1 || (cfg_.flags[pred] & BF_HANDLER) != 0 || last == kNoId ||
				body_.instructions[last].op != Opcode::Jump)
				return false;
			missing.push_back(pred);
		}
		pairs.push_back(Value::block(pred));
		pairs.push_back(v);
	}
	if (missing.size() == preds)
		return false;
	// Past an instruction that may unwind the load may not have run on the
	// edges its copies go to: they state no `typed` (lowir.md "Instruction
	// Facts").
	bool speculates = false;
	for (InstId i = body_.blocks[block].first; i != id && i != kNoId && !speculates; i = body_.instructions[i].next)
		speculates = may_unwind(c_.program, body_.instructions[i]);
	for (size_t k = 1; k < pairs.size(); k += 2)
	{
		if (!pairs[k].is(ValueKind::None))
			continue;
		const BlockId pred = pairs[k - 1].index;
		Value at_address = address;
		if (step != kNoId)
			at_address = append_before_jump(pred, body_.instructions[step]);
		Instruction copy = load;
		copy.ops[0] = at_address;
		if (speculates)
			copy.flags &= static_cast<uint8_t>(~IF_TYPED);
		pairs[k] = append_before_jump(pred, copy);
	}
	replace_with_phi(block, prev, id, pairs);
	return true;
}

// A copy of `inst` (taken by value: appending moves the instructions)
// with a fresh result, before `block`'s final jump.
Value MemoryForwarding::append_before_jump(BlockId block, Instruction copy)
{
	const TempId original = copy.result;
	copy.next = kNoId;
	const InstId at = append_instruction(body_, copy);
	const TempId result = append_temp(body_, body_.temps[original].type, at);
	body_.instructions[at].result = result;
	// The same address as the original (escapes' facts).
	if (original < address_.size())
	{
		address_.resize(result + 1, AddressFacts());
		address_[result] = address_[original];
	}
	insert_after(body_, block, previous_in_block(body_, block, body_.blocks[block].last), at);
	if (c_.block_of.size() <= at)
		c_.block_of.resize(at + 1, kNoId);
	c_.block_of[at] = block;
	c_.invalidate_block_of();
	lists_[block].second = 0;
	return Value::temp(result);
}

void MemoryForwarding::replace_with_phi(BlockId block, InstId prev, InstId id, const std::vector<Value>& pairs)
{
	const Type type = body_.instructions[id].type;
	const TempId result = body_.instructions[id].result;
	unlink_after(body_, block, prev, id);
	place_phi(block, result, type, pairs);
}

// A phi of `pairs` defining `result` at the head of `block` (in place of
// the load that defined it, unlinked by the caller).
void MemoryForwarding::place_phi(BlockId block, TempId result, Type type, const std::vector<Value>& pairs)
{
	Instruction merged = make_instruction(Opcode::Phi, type);
	merged.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
	merged.extra_count = static_cast<uint32_t>(pairs.size());
	merged.result = result;
	const InstId m = append_instruction(body_, merged);
	body_.temps[result].def = m;
	insert_after(body_, block, kNoId, m);
	if (c_.block_of.size() <= m)
		c_.block_of.resize(m + 1, kNoId);
	c_.block_of[m] = block;
	c_.invalidate_block_of();
	++c_.stats.loads_reused;
	changed_ = true;
}

} // namespace

bool forward_memory(FunctionContext& context)
{
	static MemoryForwarding::Pool pool;
	MemoryForwarding pass(context);
	pass.exchange(pool);
	const bool changed = pass.run();
	pass.exchange(pool);
	// A reused value may be defined later in layout than the load it
	// replaced (it dominates it).
	if (changed)
		restore_definition_order(context);
	return changed;
}

} // namespace opt
