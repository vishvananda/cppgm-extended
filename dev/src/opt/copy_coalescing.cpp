#include <algorithm>

#include "lowir/builder.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

// An address as a root (parameter, slot, global) and a constant byte
// offset, through literal `index` steps and pointer copies.
struct Rooted
{
	Value root;
	int64_t offset = 0;
};

bool rooted(const FunctionBody& body, Value v, Rooted& out)
{
	out.offset = 0;
	for (unsigned step = 0; step < 8; ++step)
	{
		if (v.is(ValueKind::Slot) || v.is(ValueKind::Symbol) ||
			(v.is(ValueKind::Temp) && v.index < body.parameter_count))
		{
			out.root = v;
			return true;
		}
		if (!v.is(ValueKind::Temp) || body.temps[v.index].def == kNoId)
			return false;
		const Instruction& def = body.instructions[body.temps[v.index].def];
		if (def.op == Opcode::Addr || (def.op == Opcode::Copy && def.type.is_pointer()))
			v = def.ops[0];
		else if (def.op == Opcode::Index && def.ops[1].is(ValueKind::Integer))
		{
			out.offset += static_cast<int64_t>(body.literals[def.ops[1].index].low) *
				static_cast<int64_t>(storage_bytes(def.type));
			v = def.ops[0];
		}
		else
			return false;
	}
	return false;
}

struct Pair
{
	InstId load;
	InstId store;
	Rooted from;
	Rooted to;
	Value source;      // the load's address operand
	Value destination; // the store's address operand
	uint64_t bytes;
};

class CopyCoalescing
{
public:
	explicit CopyCoalescing(FunctionContext& context) : c_(context), body_(context.body)
	{
		result_bytes_ = exclusive_result_bytes(c_.program, c_.symbol);
	}

	bool run();

private:
	// Whether two roots denote storage that cannot overlap: distinct slots
	// or globals, a slot whose address never escapes and anything else,
	// or two parameters both promised noalias.
	bool disjoint(Value a, Value b) const;
	bool disjoint(const Rooted& a, const Rooted& b, uint64_t bytes) const
	{
		return disjoint(a.root, b.root) ||
			exclusive_result_apart(result_bytes_, a.root, a.offset, bytes, b.root, body_.parameter_count) ||
			exclusive_result_apart(result_bytes_, b.root, b.offset, bytes, a.root, body_.parameter_count);
	}
	void flush(BlockId block, std::vector<Pair>& run);
	bool forward_copies(BlockId block);
	bool zero_fill_slots();
	bool constant_slots();
	bool uniform_slots();

	FunctionContext& c_;
	FunctionBody& body_;
	uint64_t result_bytes_ = 0;
	std::vector<uint32_t> uses_;
	std::vector<uint32_t> slot_uses_; // per slot: instructions naming it (directly or by addr)
	std::vector<uint8_t> escaped_;    // per slot: its address reaches more than accesses and copies
	std::vector<uint8_t> dead_;       // per instruction: a merged pair or removable fill
	uint32_t merges_ = 0;
	bool changed_ = false;
};

bool CopyCoalescing::disjoint(Value a, Value b) const
{
	if (a == b)
		return false;
	const bool named_a = a.is(ValueKind::Slot) || a.is(ValueKind::Symbol);
	const bool named_b = b.is(ValueKind::Slot) || b.is(ValueKind::Symbol);
	if (named_a && named_b)
		return true;
	// No pointer reaches a slot whose address never escapes.
	if ((a.is(ValueKind::Slot) && a.index < escaped_.size() && !escaped_[a.index]) ||
		(b.is(ValueKind::Slot) && b.index < escaped_.size() && !escaped_[b.index]))
		return true;
	if (a.is(ValueKind::Temp) && b.is(ValueKind::Temp))
	{
		const Signature& sig =
			c_.program.signatures[c_.program.functions[c_.program.symbols[c_.symbol].detail].signature];
		const Parameter* params = c_.program.parameters_of(sig);
		return a.index < sig.parameter_count && b.index < sig.parameter_count &&
			params[a.index].alias == ir_model::PALM_NOALIAS && params[b.index].alias == ir_model::PALM_NOALIAS;
	}
	return false;
}

// Two or more adjacent pairs become one copyobj at the last store.
void CopyCoalescing::flush(BlockId block, std::vector<Pair>& run)
{
	// A loaded value used beyond its store may not be used before the last
	// store (it is read again after the copy).
	for (size_t k = 0; k < run.size() && run.size() >= 2; ++k)
	{
		const TempId v = body_.instructions[run[k].load].result;
		if (uses_[v] == 1)
			continue;
		for (InstId i = body_.instructions[run[k].load].next; i != kNoId && i != run.back().store;
			i = body_.instructions[i].next)
		{
			bool used = false;
			for_each_operand(body_, body_.instructions[i],
				[&](const Value& x) { used = used || (x.is(ValueKind::Temp) && x.index == v); });
			if (used && i != run[k].store)
			{
				run.clear();
				return;
			}
		}
	}
	if (run.size() >= 2)
	{
		uint64_t bytes = 0;
		for (const Pair& p : run)
			bytes += p.bytes;
		Instruction copy = make_instruction(Opcode::CopyObject, Type::scalar(TypeKind::Void));
		copy.ops[0] = run[0].source;
		copy.ops[1] = run[0].destination;
		copy.immediate = static_cast<int64_t>(bytes);
		copy.sub = static_cast<uint8_t>(__builtin_ctzll(run[0].bytes));
		copy.location = body_.instructions[run.back().store].location;
		const InstId id = append_instruction(body_, copy);
		insert_after(body_, block, run.back().store, id);
		// The pairs leave in one sweep of the block (run()); a loaded value
		// used beyond its store is read again after the copy, from where
		// it was stored.
		InstId after = id;
		for (const Pair& p : run)
		{
			dead_[p.store] = 1;
			if (uses_[body_.instructions[p.load].result] == 1)
			{
				dead_[p.load] = 1;
				continue;
			}
			Instruction reread = body_.instructions[p.load];
			reread.ops[0] = p.destination;
			// An access of the destination: `typed` as its store was.
			reread.flags = static_cast<uint8_t>(
				(reread.flags & ~IF_TYPED) | (body_.instructions[p.store].flags & IF_TYPED));
			const InstId at = append_instruction(body_, reread);
			body_.temps[reread.result].def = at;
			insert_after(body_, block, after, at);
			after = at;
			dead_.resize(body_.instructions.size(), 0);
			dead_[p.load] = 1;
		}
		++merges_;
		changed_ = true;
		++c_.stats.stores_removed;
	}
	run.clear();
}

// A copy into a whole slot that only a later copy of the whole slot reads
// (nothing else names the slot): the later copy reads the first one's
// source instead, when that is an object value or storage nothing in
// between writes (disjoint from the later destination), and the first
// copy goes; so does one a return of the slot follows, the return naming
// the source slot. Returns whether the block changed.
bool CopyCoalescing::forward_copies(BlockId block)
{
	bool changed = false;
	InstId first_prev = kNoId, first = kNoId, prev = kNoId;
	uint32_t slot = kNoId;
	bool written = false; // a write since the first copy
	for (InstId i = body_.blocks[block].first; i != kNoId; prev = i, i = body_.instructions[i].next)
	{
		Instruction& inst = body_.instructions[i];
		Rooted from, to;

		// A whole scalar copy immediately read at its own width can use that
		// read's SSA value directly. The store preserves every private slot
		// byte; ordinary promotion may then remove the temporary's address.
		if (inst.op == Opcode::CopyObject && !(inst.flags & IF_VOLATILE) && inst.next != kNoId &&
			rooted(body_, inst.ops[1], to) && to.root.is(ValueKind::Slot) && to.offset == 0 && !escaped_[to.root.index])
		{
			const Type type = body_.slots[to.root.index].type;
			const unsigned bits = type.is_pointer() ? 64 : integer_bits(type.kind());
			const InstId next = inst.next;
			const Instruction& read = body_.instructions[next];
			const bool source = !inst.ops[0].is(ValueKind::Temp) || body_.temps[inst.ops[0].index].type.is_pointer();
			if (source && bits >= 8 && bits <= 64 && bits == 8 * storage_bytes(type) &&
				inst.immediate == static_cast<int64_t>(storage_bytes(type)) && read.op == Opcode::Load &&
				read.type == type && read.ops[0] == to.root && !(read.flags & IF_VOLATILE))
			{
				const TempId value = read.result;
				const uint32_t location = inst.location;
				Instruction load = make_instruction(Opcode::Load, type);
				load.ops[0] = inst.ops[0];
				load.result = value;
				load.location = location;
				load.next = next;
				Instruction store = make_instruction(Opcode::Store, type);
				store.ops[0] = Value::temp(value);
				store.ops[1] = to.root;
				store.location = location;
				store.next = read.next;
				body_.instructions[i] = load;
				body_.instructions[next] = store;
				body_.temps[value].def = i;
				first = kNoId;
				changed = true;
				continue;
			}
		}
		if (inst.op == Opcode::CopyObject && first != kNoId && rooted(body_, inst.ops[0], from) &&
			from.root == Value::slot(slot) && from.offset == 0 &&
			inst.immediate == body_.instructions[first].immediate && slot_uses_[slot] == 2)
		{
			const Value source = body_.instructions[first].ops[0];
			Rooted origin;
			const bool value = source.is(ValueKind::Temp) && body_.temps[source.index].type.is_object();
			// A destination slot no pointer reaches needs no root for the
			// source (a variable element of an array).
			const bool private_to = rooted(body_, inst.ops[1], to) && to.root.is(ValueKind::Slot) &&
				to.root.index < escaped_.size() && !escaped_[to.root.index];
			if (value ||
				(!written &&
					(private_to ||
						(rooted(body_, source, origin) && rooted(body_, inst.ops[1], to) &&
							disjoint(origin, to, static_cast<uint64_t>(inst.immediate))))))
			{
				inst.ops[0] = source;
				unlink_after(body_, block, first_prev, first);
				if (prev == first)
					prev = first_prev;
				++c_.stats.stores_removed;
				changed = true;
				first = kNoId;
				continue;
			}
		}
		// A return of the slot the copy just filled from another slot
		// returns that slot (nothing reads the copy after the return).
		Rooted origin;
		if (inst.op == Opcode::Return && first != kNoId && !written && inst.ops[0] == Value::slot(slot) &&
			rooted(body_, body_.instructions[first].ops[0], origin) && origin.root.is(ValueKind::Slot) &&
			origin.offset == 0 &&
			storage_bytes(body_.slots[origin.root.index].type) ==
				static_cast<uint64_t>(body_.instructions[first].immediate))
		{
			inst.ops[0] = origin.root;
			unlink_after(body_, block, first_prev, first);
			++c_.stats.stores_removed;
			changed = true;
			first = kNoId;
			continue;
		}
		if (inst.op == Opcode::CopyObject && rooted(body_, inst.ops[1], to) && to.root.is(ValueKind::Slot) &&
			to.offset == 0 && static_cast<uint64_t>(inst.immediate) == storage_bytes(body_.slots[to.root.index].type))
		{
			first_prev = prev;
			first = i;
			slot = to.root.index;
			written = false;
			continue;
		}
		if (first != kNoId &&
			(inst.op == Opcode::Store || inst.op == Opcode::CopyObject || inst.op == Opcode::ZeroInit ||
				may_write_memory(c_.program, inst)))
			written = true;
	}
	return changed;
}

// An object slot (at most 64 bytes) written only by stores of zero and
// zeroing, whose address reaches nothing but its accesses and copies from
// it: in each block its first zero store zeroes the whole slot and the
// block's later ones go. The bytes no store wrote (padding) read zero
// instead of indeterminate, and a wide read of the slot forwards from
// whole stores instead of stalling on byte ones.
bool CopyCoalescing::zero_fill_slots()
{
	const size_t n = body_.slots.size();
	std::vector<uint8_t> ok(n, 1);
	for (size_t s = 0; s < n; ++s)
		ok[s] = body_.slots[s].type.is_object() && storage_bytes(body_.slots[s].type) <= 64;
	auto zero_literal = [&](Value v) {
		return v.is(ValueKind::Integer) && body_.literals[v.index].low == 0 && body_.literals[v.index].high == 0;
	};
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Addr ||
				((inst.op == Opcode::Index || inst.op == Opcode::Copy) && inst.result != kNoId &&
					uses_[inst.result] == 0))
				continue;
			unsigned position = 0;
			for_each_operand(body_, inst, [&](const Value& v) {
				const unsigned at = position++;
				Rooted r;
				if (!rooted(body_, v, r) || !r.root.is(ValueKind::Slot))
					return;
				const uint32_t slot = r.root.index;
				const bool address_of_load = inst.op == Opcode::Load && at == 0 && !(inst.flags & IF_VOLATILE);
				const bool zero_store = inst.op == Opcode::Store && at == 1 && zero_literal(inst.ops[0]) &&
					!(inst.flags & IF_VOLATILE) && (inst.type.is_integer() || inst.type.is_pointer());
				const bool copy_source = inst.op == Opcode::CopyObject && at == 0;
				const bool zeroing = inst.op == Opcode::ZeroInit && at == 0;
				const bool returned = inst.op == Opcode::Return && v.is(ValueKind::Slot);
				const bool step = inst.op == Opcode::Index && at == 0; // a field address, checked at its uses
				if (!(address_of_load || zero_store || copy_source || zeroing || returned || step))
					ok[slot] = 0;
			});
		}
	bool changed = false;
	for (BlockId b : body_.layout)
	{
		std::vector<uint8_t> done(n, 0);
		InstId prev = kNoId;
		for (InstId i = body_.blocks[b].first; i != kNoId;)
		{
			Instruction& inst = body_.instructions[i];
			const InstId next = inst.next;
			Rooted r;
			if (inst.op == Opcode::Store && rooted(body_, inst.ops[1], r) && r.root.is(ValueKind::Slot) &&
				ok[r.root.index])
			{
				const uint32_t slot = r.root.index;
				if (done[slot])
				{
					unlink_after(body_, b, prev, i);
					++c_.stats.stores_removed;
					changed = true;
					i = next;
					continue;
				}
				done[slot] = 1;
				inst.op = Opcode::ZeroInit;
				inst.type = Type::scalar(TypeKind::Void);
				inst.ops[0] = r.root;
				inst.ops[1] = Value::none();
				inst.immediate = static_cast<int64_t>(storage_bytes(body_.slots[slot].type));
				inst.sub = static_cast<uint8_t>(body_.slots[slot].type.align_log2);
				changed = true;
			}
			prev = i;
			i = next;
		}
	}
	return changed;
}

// A slot filled whole by one copy from a global of constant contents and
// otherwise only read -- its address (through index steps, constant or
// not, and pointer copies) reaching nothing but loads and copy sources --
// reads the global instead, and the copy goes (a local `constexpr` table
// such as std::to_string's digit pairs, copied in on each call). A read
// before the copy saw indeterminate bytes, which the global's may be.
bool CopyCoalescing::constant_slots()
{
	const uint32_t slots = static_cast<uint32_t>(body_.slots.size());
	if (slots == 0)
		return false;
	std::vector<InstId> fill(slots, kNoId);
	std::vector<uint8_t> bad(slots, 0);
	std::vector<SlotId> derived(body_.temps.size(), kNoId); // per temp: the slot its address names
	auto slot_of = [&](const Value& v) -> SlotId {
		if (v.is(ValueKind::Slot))
			return v.index;
		return v.is(ValueKind::Temp) && v.index < derived.size() ? derived[v.index] : kNoId;
	};
	// Address steps first (layout order defines before uses, phis aside:
	// a phi of an address marks its slot below).
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.result == kNoId)
				continue;
			if ((inst.op == Opcode::Addr && inst.ops[0].is(ValueKind::Slot)) ||
				((inst.op == Opcode::Index || (inst.op == Opcode::Copy && inst.type.is_pointer())) &&
					slot_of(inst.ops[0]) != kNoId))
				derived[inst.result] = inst.op == Opcode::Addr ? inst.ops[0].index : slot_of(inst.ops[0]);
		}
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			const bool step = inst.result != kNoId && derived[inst.result] != kNoId;
			uint32_t position = 0;
			for_each_operand(body_, inst, [&](const Value& v) {
				const uint32_t at = position++;
				const SlotId slot = slot_of(v);
				if (slot == kNoId)
					return;
				if (step && at == 0)
					return; // the address step itself
				if ((inst.op == Opcode::Load && at == 0 && (inst.flags & IF_VOLATILE) == 0) ||
					(inst.op == Opcode::CopyObject && at == 0))
					return;
				Rooted r;
				if (inst.op == Opcode::CopyObject && at == 1 && fill[slot] == kNoId &&
					inst.ops[0].is(ValueKind::Symbol) && rooted(body_, v, r) && r.offset == 0 &&
					static_cast<uint64_t>(inst.immediate) == storage_bytes(body_.slots[slot].type) &&
					constant_contents(c_.program, inst.ops[0].index, c_.unwritten))
				{
					fill[slot] = i;
					return;
				}
				bad[slot] = 1;
			});
		}
	bool changed = false;
	for (SlotId slot = 0; slot < slots; ++slot)
	{
		if (bad[slot] || fill[slot] == kNoId)
			continue;
		const Value global = body_.instructions[fill[slot]].ops[0];
		for (BlockId b : body_.layout)
		{
			InstId prev = kNoId;
			for (InstId i = body_.blocks[b].first; i != kNoId;)
			{
				Instruction& inst = body_.instructions[i];
				const InstId next = inst.next;
				if (i == fill[slot])
				{
					unlink_after(body_, b, prev, i);
					i = next;
					continue;
				}
				for_each_operand(body_, inst, [&](Value& v) {
					if (v == Value::slot(slot))
						v = global;
				});
				prev = i;
				i = next;
			}
		}
		++c_.stats.stores_removed;
		changed = true;
	}
	return changed;
}

// A whole entry fill of a nonescaping object determines ordinary integer
// loads through its derived addresses, including variable indices.
// No other writes, address observations, copy sources or EH entry are allowed.
bool CopyCoalescing::uniform_slots()
{
	if (body_.slots.empty() || body_.layout.empty())
		return false;
	const BlockId entry = body_.layout.front();
	struct Fill
	{
		SlotId slot;
		InstId inst;
		uint8_t byte;
	};
	std::vector<Fill> fills;
	for (InstId i = body_.blocks[entry].first; i != kNoId; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		if (is_eh_instruction(inst.op))
			return false;
		if (inst.op != Opcode::Call || inst.extra_count != 3 || !inst.ops[0].is(ValueKind::Symbol) ||
			!c_.program.memset_function(inst.ops[0].index) || (inst.result != kNoId && uses_[inst.result] != 0))
			continue;
		const Value* args = &body_.operands[inst.extra_first];
		Rooted at;
		if (!rooted(body_, args[0], at) || !at.root.is(ValueKind::Slot) || at.offset != 0 ||
			!args[1].is(ValueKind::Integer) || !args[2].is(ValueKind::Integer))
			continue;
		const Literal& n = body_.literals[args[2].index];
		if (!body_.slots[at.root.index].type.is_object() || n.high != 0 || n.low == 0 ||
			n.low != storage_bytes(body_.slots[at.root.index].type))
			continue;
		fills.push_back(Fill{ at.root.index, i, static_cast<uint8_t>(body_.literals[args[1].index].low) });
	}
	if (fills.empty())
		return false;
	std::vector<InstId> fill(body_.slots.size(), kNoId);
	std::vector<uint8_t> byte(body_.slots.size(), 0), bad(body_.slots.size(), 0), seen(body_.slots.size(), 0);
	for (const Fill& f : fills)
	{
		if (fill[f.slot] != kNoId)
			bad[f.slot] = 1;
		fill[f.slot] = f.inst;
		byte[f.slot] = f.byte;
	}
	std::vector<SlotId> derived(body_.temps.size(), kNoId);
	auto slot_of = [&](Value v) -> SlotId {
		if (v.is(ValueKind::Slot))
			return v.index;
		return v.is(ValueKind::Temp) ? derived[v.index] : kNoId;
	};
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.result == kNoId)
				continue;
			if ((inst.op == Opcode::Addr && inst.ops[0].is(ValueKind::Slot)) || inst.op == Opcode::Index ||
				(inst.op == Opcode::Copy && inst.type.is_pointer()))
				derived[inst.result] = slot_of(inst.ops[0]);
		}
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			const SlotId target = inst.op == Opcode::Call && inst.extra_count == 3
				? slot_of(body_.operands[inst.extra_first])
				: kNoId;
			if (target != kNoId && fill[target] == i)
				seen[target] = 1;
			unsigned position = 0;
			for_each_operand(body_, inst, [&](Value v) {
				const unsigned at = position++;
				const SlotId slot = slot_of(v);
				if (slot == kNoId || fill[slot] == kNoId)
					return;
				if (inst.result != kNoId && derived[inst.result] != kNoId && at == 0)
					return;
				if (i == fill[slot] && at == 3)
					return;
				if (inst.op == Opcode::Load && at == 0 && !(inst.flags & IF_VOLATILE) && inst.type.is_integer() &&
					storage_bytes(inst.type) <= 8 && (b != entry || seen[slot]))
					return;
				bad[slot] = 1;
			});
		}
	bool any = false;
	for (const Fill& f : fills)
		any = any || !bad[f.slot];
	if (!any)
		return false;
	// Reuse the instruction mask; all eligible fills share one replacement walk.
	dead_.assign(body_.instructions.size(), 0);
	for (const Fill& f : fills)
		if (!bad[f.slot])
			dead_[f.inst] = 1;
	bool changed = false;
	FunctionBuilder builder(c_.program, c_.symbol);
	for (BlockId b : body_.layout)
	{
		InstId prev = kNoId;
		for (InstId i = body_.blocks[b].first; i != kNoId;)
		{
			Instruction& inst = body_.instructions[i];
			const InstId next = inst.next;
			if (inst.op == Opcode::Load && !(inst.flags & IF_VOLATILE))
			{
				const SlotId slot = slot_of(inst.ops[0]);
				if (slot != kNoId && fill[slot] != kNoId && !bad[slot])
				{
					uint64_t bits = static_cast<uint64_t>(byte[slot]) * 0x0101010101010101ull;
					const uint64_t width = storage_bytes(inst.type);
					if (width < 8)
						bits &= (uint64_t(1) << (width * 8)) - 1;
					inst.op = Opcode::Copy;
					inst.ops[0] = builder.integer(static_cast<int64_t>(bits));
					inst.ops[1] = inst.ops[2] = Value::none();
					inst.flags = inst.sub = 0;
					inst.extra_count = inst.extra_first = 0;
					inst.immediate = 0;
					++c_.stats.loads_reused;
					changed = true;
				}
			}
			if (dead_[i])
			{
				unlink_after(body_, b, prev, i);
				++c_.stats.stores_removed;
				changed = true;
			}
			else
				prev = i;
			i = next;
		}
	}
	return changed;
}

// Adjacent nonvolatile scalar load/store pairs copying a contiguous range
// of one size between storage that cannot overlap, each loaded value used
// only by its store, merge into one copyobj; up to four loads may come
// before their stores. Then copies through a temporary slot forward
// (forward_copies). One block scan each.
bool CopyCoalescing::run()
{
	uses_ = c_.entry_uses(coalesce_copies);
	escaped_ = escaped_slots(body_);
	dead_.assign(body_.instructions.size(), 0);
	std::vector<Pair> run, group; // (emptied per block, per load group)
	std::vector<InstId> loads;
	for (BlockId b : body_.layout)
	{
		run.clear();
		const uint32_t before = merges_;
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (is_pure(inst) && inst.op != Opcode::Phi)
				continue; // address arithmetic between the pairs
			if (inst.op != Opcode::Load)
			{
				flush(b, run);
				continue;
			}
			// Loads (at most four, address arithmetic between) whose next
			// effects are the stores of their values, in their order.
			loads.assign(1, i);
			InstId next = inst.next;
			for (;;)
			{
				while (next != kNoId && is_pure(body_.instructions[next]) && body_.instructions[next].op != Opcode::Phi)
					next = body_.instructions[next].next;
				if (next == kNoId || loads.size() == 4 || body_.instructions[next].op != Opcode::Load)
					break;
				loads.push_back(next);
				next = body_.instructions[next].next;
			}
			group.clear();
			for (size_t k = 0; k < loads.size(); ++k)
			{
				while (k > 0 && next != kNoId && is_pure(body_.instructions[next]) &&
					body_.instructions[next].op != Opcode::Phi)
					next = body_.instructions[next].next;
				const Instruction& load = body_.instructions[loads[k]];
				const Instruction* store = next != kNoId ? &body_.instructions[next] : nullptr;
				Pair pair;
				const bool ok = store != nullptr && store->op == Opcode::Store && (load.flags & IF_VOLATILE) == 0 &&
					(store->flags & IF_VOLATILE) == 0 && store->type == load.type &&
					(load.type.is_integer() || load.type.is_pointer()) && store->ops[0].is(ValueKind::Temp) &&
					store->ops[0].index == load.result && uses_[load.result] >= 1 &&
					rooted(body_, load.ops[0], pair.from) && rooted(body_, store->ops[1], pair.to) &&
					disjoint(pair.from, pair.to, storage_bytes(load.type));
				if (!ok)
					break;
				pair.load = loads[k];
				pair.store = next;
				pair.source = load.ops[0];
				pair.destination = store->ops[1];
				pair.bytes = storage_bytes(load.type);
				group.push_back(pair);
				next = body_.instructions[next].next;
			}
			if (group.size() != loads.size())
			{
				flush(b, run);
				continue;
			}
			// Every load in a group precedes its stores. Separate copies
			// must not delay a later load past an earlier write to its
			// source (for example, the saved values in a swap). At most
			// four loads bound this dependence check.
			bool safe = true;
			for (size_t later = 1; later < group.size() && safe; ++later)
				for (size_t earlier = 0; earlier < later; ++earlier)
					if (!disjoint(
							group[earlier].to, group[later].from, std::max(group[earlier].bytes, group[later].bytes)))
					{
						safe = false;
						break;
					}
			if (!safe)
			{
				flush(b, run);
				continue;
			}
			for (const Pair& pair : group)
			{
				if (!run.empty())
				{
					const Pair& last = run.back();
					const bool extends = last.bytes == pair.bytes && last.from.root == pair.from.root &&
						last.to.root == pair.to.root &&
						pair.from.offset == last.from.offset + static_cast<int64_t>(last.bytes) &&
						pair.to.offset == last.to.offset + static_cast<int64_t>(last.bytes) &&
						pair.from.offset - run[0].from.offset == pair.to.offset - run[0].to.offset;
					if (!extends)
						flush(b, run);
				}
				run.push_back(pair);
			}
			i = group.back().store; // continue after the last store
		}
		flush(b, run);
		if (merges_ == before)
			continue;
		InstId prev = kNoId;
		for (InstId i = body_.blocks[b].first; i != kNoId;)
		{
			const InstId next = body_.instructions[i].next;
			if (i < dead_.size() && dead_[i])
				unlink_after(body_, b, prev, i);
			else
				prev = i;
			i = next;
		}
	}
	// Slot uses (an addr of a slot counts at its users: the copies name
	// the slot through it).
	// Pure instructions left unused (the merged pairs' addresses) do not
	// count.
	if (merges_ != 0)
		count_uses(body_, uses_);
	slot_uses_.assign(body_.slots.size(), 0);
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if ((inst.op == Opcode::Addr && inst.ops[0].is(ValueKind::Slot)) ||
				(is_pure(inst) && inst.result != kNoId && uses_[inst.result] == 0))
				continue;
			for_each_operand(body_, inst, [&](const Value& v) {
				Rooted r;
				if (rooted(body_, v, r) && r.root.is(ValueKind::Slot))
					++slot_uses_[r.root.index];
			});
		}
	bool forwarded = false;
	for (BlockId b : body_.layout)
		forwarded = forward_copies(b) || forwarded;
	changed_ = forwarded || changed_;
	if (c_.level >= 2)
	{
		if (forwarded)
			count_uses(body_, uses_); // (else as counted above)
		changed_ = zero_fill_slots() || changed_;
		changed_ = constant_slots() || changed_;
		changed_ = uniform_slots() || changed_;
	}
	return changed_;
}

} // namespace

bool coalesce_copies(FunctionContext& context)
{
	CopyCoalescing pass(context);
	return pass.run();
}

// A slot every copied return copies whole into the indirect result (a named
// return value) becomes that result: its address is the result pointer and
// those copies go. Only when the slot's address reaches loads, stores,
// copies, zeroing and calls' own indirect results (through field
// addresses), and the result is used only in blocks that return, none of
// them using the slot after the result's first use. The result's storage
// is the caller's: no other pointer the function holds reaches it.
bool name_returned_slot(FunctionContext& context)
{
	FunctionBody& body = context.body;
	const Program& program = context.program;
	const Function& function = program.functions[program.symbols[context.symbol].detail];
	const Signature& sig = program.signatures[function.signature];
	const Parameter* params = program.parameters_of(sig);
	uint32_t ret = kNoId;
	for (uint32_t p = 0; p < sig.parameter_count && p < body.parameter_count; ++p)
		if (params[p].pass == ir_model::PPM_INDIRECT_RESULT)
			ret = p;
	if (ret == kNoId)
		return false;
	const Value result = Value::temp(ret);
	// The slot each `copyobj $s, %ret; return` copies (the first found).
	SlotId slot = kNoId;
	std::vector<InstId> copies;
	auto slot_of = [&](Value v) -> SlotId {
		if (v.is(ValueKind::Slot))
			return v.index;
		if (!v.is(ValueKind::Temp) || v.index < body.parameter_count || body.temps[v.index].def == kNoId)
			return kNoId;
		const Instruction& d = body.instructions[body.temps[v.index].def];
		return d.op == Opcode::Addr && d.ops[0].is(ValueKind::Slot) ? d.ops[0].index : kNoId;
	};
	for (BlockId b : body.layout)
	{
		const InstId last = body.blocks[b].last;
		if (last == kNoId || body.instructions[last].op != Opcode::Return)
			continue;
		const InstId prev = previous_in_block(body, b, last);
		if (prev == kNoId)
			continue;
		const Instruction& copy = body.instructions[prev];
		if (copy.op != Opcode::CopyObject || !(copy.ops[1] == result))
			continue;
		const SlotId s = slot_of(copy.ops[0]);
		if (s == kNoId || storage_bytes(body.slots[s].type) != static_cast<uint64_t>(copy.immediate) ||
			(slot != kNoId && s != slot))
			continue;
		slot = s;
		copies.push_back(prev);
	}
	if (slot == kNoId)
		return false;
	// The slot's addresses (addr results and field addresses from them) and
	// the result's, and where each is used.
	std::vector<uint8_t> of_slot(body.temps.size(), 0), of_result(body.temps.size(), 0);
	of_result[ret] = 1;
	std::vector<BlockId> block_of(body.instructions.size(), kNoId);
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			block_of[i] = b;
	// Address temps in definition order (addr and index chains).
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if (inst.result == kNoId)
				continue;
			if (inst.op == Opcode::Addr && inst.ops[0].is(ValueKind::Slot) && inst.ops[0].index == slot)
				of_slot[inst.result] = 1;
			else if ((inst.op == Opcode::Index || (inst.op == Opcode::Copy && inst.type.is_pointer())) &&
				inst.ops[0].is(ValueKind::Temp))
			{
				of_slot[inst.result] = of_slot[inst.ops[0].index];
				of_result[inst.result] = of_result[inst.ops[0].index];
			}
		}
	auto is_slot = [&](const Value& v) {
		return (v.is(ValueKind::Slot) && v.index == slot) || (v.is(ValueKind::Temp) && of_slot[v.index]);
	};
	auto is_result = [&](const Value& v) { return v.is(ValueKind::Temp) && of_result[v.index]; };
	// Per block: position of the first result use and the last slot use.
	std::vector<uint32_t> first_result(body.blocks.size(), ~0u), last_slot(body.blocks.size(), 0);
	std::vector<uint8_t> slot_used(body.blocks.size(), 0);
	for (BlockId b : body.layout)
	{
		uint32_t position = 0;
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next, ++position)
		{
			const Instruction& inst = body.instructions[i];
			const bool copy = std::find(copies.begin(), copies.end(), i) != copies.end();
			bool ok = true, uses_slot = false, uses_result = false;
			const Signature* callee = inst.op == Opcode::Call ? program.call_boundary(inst) : nullptr;
			const Parameter* cparams = callee != nullptr ? program.parameters_of(*callee) : nullptr;
			uint32_t at = 0;
			for_each_operand(body, inst, [&](const Value& v) {
				const uint32_t k = at++;
				const bool s = is_slot(v), r = is_result(v);
				if (!s && !r)
					return;
				uses_slot = uses_slot || s;
				uses_result = uses_result || r;
				// Where an address may go: a memory operation's address, a
				// field address, a call's indirect result.
				bool address = false;
				switch (inst.op)
				{
				case Opcode::Load:
					address = k == 0;
					break;
				case Opcode::Store:
					address = k == 1;
					break;
				case Opcode::CopyObject:
					address = k == 0 || k == 1;
					break;
				case Opcode::ZeroInit:
					address = k == 0;
					break;
				case Opcode::Index:
					address = k == 0;
					break;
				case Opcode::Copy:
					address = inst.type.is_pointer();
					break;
				case Opcode::Addr:
					address = true;
					break;
				case Opcode::Call:
					address = k >= 3 && cparams != nullptr && k - 3 < callee->parameter_count &&
						cparams[k - 3].pass == ir_model::PPM_INDIRECT_RESULT;
					break;
				default:
					break;
				}
				ok = ok && address;
			});
			if (!ok)
				return false;
			if (uses_result && !copy && first_result[b] == ~0u)
				first_result[b] = position;
			if (uses_slot && !copy)
			{
				slot_used[b] = 1;
				last_slot[b] = position;
			}
		}
	}
	for (BlockId b : body.layout)
	{
		if (first_result[b] == ~0u)
			continue;
		const InstId last = body.blocks[b].last;
		if (last == kNoId || body.instructions[last].op != Opcode::Return ||
			(slot_used[b] && last_slot[b] > first_result[b]))
			return false;
	}
	// The slot's address is the result; its copies into the result go.
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first, prev = kNoId; i != kNoId;)
		{
			const InstId next = body.instructions[i].next;
			if (std::find(copies.begin(), copies.end(), i) != copies.end())
			{
				unlink_after(body, b, prev, i);
				i = next;
				continue;
			}
			Instruction& inst = body.instructions[i];
			if (!(inst.op == Opcode::Addr && inst.ops[0].is(ValueKind::Slot)))
				for_each_operand(body, inst, [&](Value& v) {
					if (v.is(ValueKind::Slot) && v.index == slot)
						v = result;
				});
			if (inst.op == Opcode::Addr && inst.ops[0].is(ValueKind::Slot) && inst.ops[0].index == slot)
			{
				inst.op = Opcode::Copy;
				inst.ops[0] = result;
			}
			prev = i;
			i = next;
		}
	return true;
}

} // namespace opt
