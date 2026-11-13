#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

#include <algorithm>

namespace opt {

using namespace lowir;

namespace {

// The most accesses of a loop this pass reads, and the locations one loop
// moves.
const size_t kLoopAccesses = 256;
const size_t kLoopLocations = 4;
// Store motion: a location a loop stores on every pass lives in a slot for
// the loop, which promotion then keeps in a register: loaded in the preheader
// and stored on each exit. Otherwise (a loop with calls on some paths, or no
// location stored on every pass), a location the loop loads is read from such
// a slot instead: its stores stay, and write the slot too; a reload follows
// each call.
class StoreMotion
{
public:
	struct Access
	{
		InstId inst;
		Located at;   // the address the loop does not change, or its object
		Value object; // what the address steps from
		Type type;
		bool store, typed;
	};

	explicit StoreMotion(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();

private:
	struct Location
	{
		Located at;
		Type type;
		bool typed; // every store of it is
	};
	// The loop's calls when none runs on every pass (calls_ then empty when
	// it holds none).
	bool side_calls(uint32_t loop) const;
	bool copied_in(InstId load) const;
	void find_loaded_locations(uint32_t loop);
	void read_location(uint32_t loop, const Location& location);

	bool invariant(uint32_t loop, Value v) const;
	bool overlap(const Access& a, const Access& b) const;
	Located place(uint32_t loop, Value address, const Located& located) const;
	bool read_accesses(uint32_t loop);
	bool each_pass(uint32_t loop, const std::vector<BlockId>& exiting, const Access& location, bool stores) const;
	std::vector<BlockId> exiting_blocks(uint32_t loop) const;
	bool readable(uint32_t loop, const std::vector<BlockId>& exiting, const Access& location) const;
	void find_locations(uint32_t loop);
	Value address_of(const Location& location, BlockId block, InstId& tail);
	void move_location(uint32_t loop, const Location& location, const std::vector<BlockId>& exits);
	std::vector<BlockId> exit_blocks(uint32_t loop);
	void promote(uint32_t loop);

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	LoopForest forest_;
	std::vector<Access> accesses_;
	std::vector<InstId> calls_;
	std::vector<Location> locations_;
	bool loads_only_ = false; // locations_ came from find_loaded_locations
};

bool named(Value object)
{
	return object.is(ValueKind::Slot) || object.is(ValueKind::Symbol);
}

bool StoreMotion::invariant(uint32_t loop, Value v) const
{
	if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
		return true;
	const InstId def = body_.temps[v.index].def;
	return def != kNoId && c_.block_of[def] != kNoId && !in_loop(forest_, loop, c_.block_of[def]);
}

// Whether two accesses may touch one byte.
bool may_overlap(const StoreMotion::Access& a, const StoreMotion::Access& b)
{
	if (a.at.root == b.at.root)
		return !a.at.known || !b.at.known ||
			(a.at.offset < b.at.offset + static_cast<int64_t>(storage_bytes(b.type)) &&
				b.at.offset < a.at.offset + static_cast<int64_t>(storage_bytes(a.type)));
	if (!(a.object == b.object) && named(a.object) && named(b.object))
		return false;
	return !(a.typed && b.typed && typed_types_apart(a.type, b.type));
}

// may_overlap, but apart where one access is in storage the loop's function
// allocated and the other's pointer comes from before it.
bool StoreMotion::overlap(const Access& a, const Access& b) const
{
	return may_overlap(a, b) && !allocation_apart(c_.program, body_, a.object, b.object);
}

// Where an access stays through the loop: an address computed by steps of
// unknown size that the loop does not change is itself the place.
Located StoreMotion::place(uint32_t loop, Value address, const Located& located) const
{
	if (located.known)
		return located;
	Located out;
	for (unsigned step = 0; step < 8; ++step)
	{
		if (!address.is(ValueKind::Temp) || address.index < body_.parameter_count ||
			body_.temps[address.index].def == kNoId)
			break;
		const Instruction& def = body_.instructions[body_.temps[address.index].def];
		if (def.op != Opcode::Index || !def.ops[1].is(ValueKind::Integer) || invariant(loop, address))
			break;
		out.offset += static_cast<int64_t>(body_.literals[def.ops[1].index].low) *
			static_cast<int64_t>(storage_bytes(def.type));
		address = def.ops[0];
	}
	if (!invariant(loop, address))
		return located;
	out.root = address;
	return out;
}

// The loads and stores of the loop, none volatile, and false if anything
// else in it may touch memory.
bool StoreMotion::read_accesses(uint32_t loop)
{
	accesses_.clear();
	calls_.clear();
	for (BlockId b : forest_.loops[loop].blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Load || inst.op == Opcode::Store)
			{
				if ((inst.flags & IF_VOLATILE) != 0 || accesses_.size() == kLoopAccesses)
					return false;
				const Value address = inst.ops[inst.op == Opcode::Load ? 0 : 1];
				const Located located = locate(body_, address);
				accesses_.push_back(Access{ i, place(loop, address, located), located.root, inst.type,
					inst.op == Opcode::Store, (inst.flags & IF_TYPED) != 0 });
			}
			else if (inst.op == Opcode::Call && calls_.size() < kLoopAccesses)
				calls_.push_back(i);
			else if (!is_pure(inst) && inst.op != Opcode::Jump && inst.op != Opcode::Branch &&
				inst.op != Opcode::Switch)
				return false;
		}
	return true;
}

// Whether a store of the location (any access, unless `stores`) runs
// before every exit and on every pass: nothing then writes (or reads) it on
// a path that would not have.
bool StoreMotion::each_pass(
	uint32_t loop, const std::vector<BlockId>& exiting, const Access& location, bool stores) const
{
	for (const Access& a : accesses_)
	{
		if ((stores && !a.store) || !same_place(a.at, a.type, location.at, location.type))
			continue;
		const BlockId b = c_.block_of[a.inst];
		bool all = true;
		for (BlockId e : exiting)
			all = all && cfg_.dominates(b, e);
		for (BlockId latch : forest_.loops[loop].latches)
			all = all && cfg_.dominates(b, latch);
		if (all)
			return true;
	}
	return false;
}

// The loop's blocks with a successor outside it.
std::vector<BlockId> StoreMotion::exiting_blocks(uint32_t loop) const
{
	std::vector<BlockId> exiting;
	for (BlockId b : forest_.loops[loop].blocks)
		for (uint32_t k = cfg_.succ_first[b]; k < cfg_.succ_first[b + 1]; ++k)
			if (!in_loop(forest_, loop, cfg_.succs[k]))
			{
				exiting.push_back(b);
				break;
			}
	return exiting;
}

// Whether the preheader may read a location the loop loads: one in a slot
// or a defined global, one read before the loop (read_before_loop), or (in a loop
// without calls) one the loop reads or writes on every pass before it can
// leave.
bool StoreMotion::readable(uint32_t loop, const std::vector<BlockId>& exiting, const Access& location) const
{
	const Value root = location.at.root;
	if (root.is(ValueKind::Slot) ||
		(root.is(ValueKind::Symbol) && c_.program.symbols[root.index].kind == SymbolKind::GlobalDefinition &&
			location.at.offset >= 0))
		return true;
	return read_before_loop(body_, cfg_, forest_, loop, location.at, location.type) ||
		(calls_.empty() && each_pass(loop, exiting, location, false));
}

void StoreMotion::find_locations(uint32_t loop)
{
	locations_.clear();
	const std::vector<BlockId> exiting = exiting_blocks(loop);
	for (const Access& s : accesses_)
	{
		const TypeKind kind = s.type.kind();
		if (!s.store || !invariant(loop, s.at.root) || locations_.size() == kLoopLocations || s.type.is_object() ||
			s.type.is_vector() || kind == TypeKind::F80 || kind == TypeKind::I128)
			continue;
		// (A slot this pass made is promotion's already.)
		if (s.at.root.is(ValueKind::Slot) && body_.slots[s.at.root.index].kind == SlotKind::Demote)
			continue;
		bool seen = false;
		for (const Location& l : locations_)
			seen = seen || same_place(l.at, l.type, s.at, s.type);
		if (seen)
			continue;
		bool alone = true, typed = true;
		for (const Access& a : accesses_)
		{
			if (same_place(a.at, a.type, s.at, s.type))
				typed = typed && (a.typed || !a.store);
			else
				alone = alone && !overlap(a, s);
		}
		if (alone && each_pass(loop, exiting, s, true))
			locations_.push_back(Location{ s.at, s.type, typed });
	}
}

// Whether a load is one read_location made: its value goes straight into
// a slot of this pass.
bool StoreMotion::copied_in(InstId load) const
{
	const Instruction& inst = body_.instructions[load];
	if (inst.next == kNoId)
		return false;
	const Instruction& next = body_.instructions[inst.next];
	return next.op == Opcode::Store && next.ops[0] == Value::temp(inst.result) && next.ops[1].is(ValueKind::Slot) &&
		body_.slots[next.ops[1].index].kind == SlotKind::Demote;
}

bool StoreMotion::side_calls(uint32_t loop) const
{
	for (InstId call : calls_)
	{
		const BlockId b = c_.block_of[call];
		bool every = true;
		for (BlockId latch : forest_.loops[loop].latches)
			every = every && cfg_.dominates(b, latch);
		if (every)
			return false;
	}
	return true;
}

// Locations the loop loads, with a loop-invariant address no other store of
// the loop may overlap.
void StoreMotion::find_loaded_locations(uint32_t loop)
{
	locations_.clear();
	if (!side_calls(loop))
		return;
	const std::vector<BlockId> exiting = exiting_blocks(loop);
	for (const Access& l : accesses_)
	{
		const TypeKind kind = l.type.kind();
		if (l.store || !invariant(loop, l.at.root) || !l.at.known || locations_.size() == kLoopLocations ||
			l.type.is_object() || l.type.is_vector() || kind == TypeKind::F80 || kind == TypeKind::I128 ||
			(l.at.root.is(ValueKind::Slot) && body_.slots[l.at.root.index].kind == SlotKind::Demote))
			continue;
		bool seen = false;
		for (const Location& m : locations_)
			seen = seen || same_place(m.at, m.type, l.at, l.type);
		bool alone = !seen, typed = true, read = false;
		for (const Access& a : accesses_)
		{
			if (same_place(a.at, a.type, l.at, l.type))
			{
				typed = typed && a.typed;
				read = read || (!a.store && !copied_in(a.inst));
			}
			else
				alone = alone && !(a.store && overlap(a, l));
		}
		// (Without calls, a slot's elements stay: the lanes of a local
		// array an unrolled loop updates cost more registers than loads.)
		if (alone && read && readable(loop, exiting, l) && !(calls_.empty() && l.at.root.is(ValueKind::Slot)))
			locations_.push_back(Location{ l.at, l.type, typed });
	}
}

// A block on each edge out of the loop.
std::vector<BlockId> StoreMotion::exit_blocks(uint32_t loop)
{
	std::vector<BlockId> exits;
	const std::vector<BlockId> blocks = forest_.loops[loop].blocks;
	for (BlockId b : blocks)
	{
		std::vector<BlockId> targets;
		for_each_successor(body_, body_.instructions[body_.blocks[b].last], [&](BlockId& t) {
			if (!in_loop(forest_, loop, t) && std::find(targets.begin(), targets.end(), t) == targets.end())
				targets.push_back(t);
		});
		for (BlockId target : targets)
		{
			const BlockId exit = append_block(body_, BlockKind::LoopExit);
			Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
			jump.ops[0] = Value::block(target);
			insert_after(body_, exit, kNoId, append_instruction(body_, jump));
			auto at = std::find(body_.layout.begin(), body_.layout.end(), target);
			body_.layout.insert(at, exit);
			for_each_successor(body_, body_.instructions[body_.blocks[b].last], [&](BlockId& t) {
				if (t == target)
					t = exit;
			});
			rename_phi_incoming(body_, target, b, exit);
			exits.push_back(exit);
		}
	}
	return exits;
}

// The location's address, computed at the end of `block` where `tail` is the
// instruction before its terminator.
Value StoreMotion::address_of(const Location& location, BlockId block, InstId& tail)
{
	auto emit = [&](Instruction inst, Type type) {
		const InstId id = append_instruction(body_, inst);
		const TempId result = append_temp(body_, type, id);
		body_.instructions[id].result = result;
		insert_after(body_, block, tail, id);
		tail = id;
		return Value::temp(result);
	};
	const Type pointer = Type::scalar(TypeKind::Ptr);
	Value base = location.at.root;
	if (base.is(ValueKind::Slot))
	{
		Instruction addr = make_instruction(Opcode::Addr, pointer);
		addr.ops[0] = base;
		base = emit(addr, pointer);
	}
	if (location.at.offset == 0)
		return base;
	Instruction step = make_instruction(Opcode::Index, Type::scalar(TypeKind::I8));
	step.ops[0] = base;
	step.ops[1] = integer_value(body_, integer_literal(location.at.offset));
	return emit(step, pointer);
}

// The location's accesses become a slot's: a copy in at the preheader and a
// copy out on each exit.
void StoreMotion::move_location(uint32_t loop, const Location& location, const std::vector<BlockId>& exits)
{
	const SlotId slot = static_cast<SlotId>(body_.slots.size());
	body_.slots.push_back(Slot{ 0, SlotKind::Demote, 0, 0, location.type });
	for (const Access& a : accesses_)
		if (same_place(a.at, a.type, location.at, location.type))
		{
			Instruction& inst = body_.instructions[a.inst];
			inst.ops[a.store ? 1 : 0] = Value::slot(slot);
			inst.flags &= static_cast<uint8_t>(~IF_TYPED);
		}
	const BlockId pre = forest_.loops[loop].preheader;
	InstId tail = previous_in_block(body_, pre, body_.blocks[pre].last);
	Instruction load = make_instruction(Opcode::Load, location.type);
	load.ops[0] = address_of(location, pre, tail);
	const InstId loaded = append_instruction(body_, load);
	const TempId value = append_temp(body_, location.type, loaded);
	body_.instructions[loaded].result = value;
	insert_after(body_, pre, tail, loaded);
	Instruction entry = make_instruction(Opcode::Store, location.type);
	entry.ops[0] = Value::temp(value);
	entry.ops[1] = Value::slot(slot);
	insert_after(body_, pre, loaded, append_instruction(body_, entry));
	for (BlockId exit : exits)
	{
		tail = kNoId;
		Instruction out = make_instruction(Opcode::Load, location.type);
		out.ops[0] = Value::slot(slot);
		const InstId read = append_instruction(body_, out);
		const TempId held = append_temp(body_, location.type, read);
		body_.instructions[read].result = held;
		insert_after(body_, exit, tail, read);
		tail = read;
		Instruction store = make_instruction(Opcode::Store, location.type);
		store.ops[0] = Value::temp(held);
		store.ops[1] = address_of(location, exit, tail);
		if (location.typed)
			store.flags |= IF_TYPED;
		insert_after(body_, exit, tail, append_instruction(body_, store));
	}
	++c_.stats.stores_sunk;
}

// The location's loads read a slot: loaded in the preheader, written with
// each store's value, and loaded again after each call.
void StoreMotion::read_location(uint32_t loop, const Location& location)
{
	const SlotId slot = static_cast<SlotId>(body_.slots.size());
	body_.slots.push_back(Slot{ 0, SlotKind::Demote, 0, 0, location.type });
	auto copy_in = [&](BlockId block, InstId& tail) {
		Instruction load = make_instruction(Opcode::Load, location.type);
		load.ops[0] = address_of(location, block, tail);
		if (location.typed)
			load.flags |= IF_TYPED;
		const InstId loaded = append_instruction(body_, load);
		const TempId value = append_temp(body_, location.type, loaded);
		body_.instructions[loaded].result = value;
		insert_after(body_, block, tail, loaded);
		Instruction keep = make_instruction(Opcode::Store, location.type);
		keep.ops[0] = Value::temp(value);
		keep.ops[1] = Value::slot(slot);
		const InstId kept = append_instruction(body_, keep);
		insert_after(body_, block, loaded, kept);
		tail = kept;
	};
	for (const Access& a : accesses_)
	{
		if (!same_place(a.at, a.type, location.at, location.type))
			continue;
		if (!a.store)
		{
			body_.instructions[a.inst].ops[0] = Value::slot(slot);
			body_.instructions[a.inst].flags &= static_cast<uint8_t>(~IF_TYPED);
			continue;
		}
		Instruction keep = make_instruction(Opcode::Store, location.type);
		keep.ops[0] = body_.instructions[a.inst].ops[0];
		keep.ops[1] = Value::slot(slot);
		insert_after(body_, c_.block_of[a.inst], a.inst, append_instruction(body_, keep));
	}
	for (InstId call : calls_)
	{
		InstId tail = call;
		copy_in(c_.block_of[call], tail);
	}
	const BlockId pre = forest_.loops[loop].preheader;
	InstId tail = previous_in_block(body_, pre, body_.blocks[pre].last);
	copy_in(pre, tail);
	++c_.stats.loads_reused;
}

void StoreMotion::promote(uint32_t loop)
{
	if (loads_only_)
	{
		for (const Location& location : locations_)
			read_location(loop, location);
		return;
	}
	const std::vector<BlockId> exits = exit_blocks(loop);
	for (const Location& location : locations_)
		move_location(loop, location, exits);
}

bool StoreMotion::run()
{
	c_.ensure_cfg();
	find_loops(body_, cfg_, forest_);
	if (forest_.loops.empty())
		return false;
	c_.entry_block_of(promote_loop_memory);
	// Innermost loops first. Each round rewrites the loops that do not touch
	// one another. A change alters the graph: the loops are found again, and
	// the copies a loop leaves may move out of the loop around it.
	bool changed = false;
	std::vector<uint8_t> touched;
	for (unsigned round = 0; round < 16; ++round)
	{
		touched.assign(body_.blocks.size(), 0);
		bool one = false;
		for (uint32_t loop = static_cast<uint32_t>(forest_.loops.size()); loop-- > 0;)
		{
			// (A loop near one this round rewrote waits for the next round:
			// it may hold the new instructions, which block_of does not map.)
			if (loop_claimed(cfg_, forest_.loops[loop], touched) || !read_accesses(loop))
				continue;
			// (A location not stored on every pass, or a loop with calls:
			// its loads alone.) A loop with calls skips the first search:
			// the list must not keep the locations of the loop before.
			locations_.clear();
			loads_only_ = !calls_.empty();
			if (!loads_only_)
				find_locations(loop);
			if (locations_.empty())
			{
				loads_only_ = true;
				find_loaded_locations(loop);
			}
			if (locations_.empty() || !claim_loop(cfg_, forest_.loops[loop], touched))
				continue;
			if (forest_.loops[loop].preheader == kNoId)
				one = make_loop_preheader(body_, cfg_, forest_, loop) || one;
			else
			{
				promote(loop);
				one = true;
			}
		}
		if (!one)
			break;
		changed = true;
		c_.invalidate_cfg();
		c_.ensure_cfg();
		find_loops(body_, cfg_, forest_);
		map_instruction_blocks(body_, c_.block_of);
		c_.invalidate_block_of();
	}
	return changed;
}

} // namespace

bool promote_loop_memory(FunctionContext& context)
{
	StoreMotion pass(context);
	return pass.run();
}

} // namespace opt
