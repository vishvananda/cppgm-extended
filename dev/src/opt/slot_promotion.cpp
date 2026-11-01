#include "lowir/builder.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

bool phi_type(Type type)
{
	switch (type.kind())
	{
	case TypeKind::I1:
	case TypeKind::I8:
	case TypeKind::U8:
	case TypeKind::I16:
	case TypeKind::U16:
	case TypeKind::I32:
	case TypeKind::U32:
	case TypeKind::I64:
	case TypeKind::F32:
	case TypeKind::F64:
	case TypeKind::V128:
	case TypeKind::Ptr:
		return true;
	default:
		return false;
	}
}

unsigned value_bits(Type type)
{
	return type.is_pointer() ? 64 : integer_bits(type.kind());
}

// The type a store's value operand has (a literal takes the store's).
Type stored_type(const FunctionBody& body, const Instruction& store)
{
	return store.ops[0].is(ValueKind::Temp) ? body.temps[store.ops[0].index].type : store.type;
}

// A store may write an integer value of another width than its type: a
// wider one keeps its low bits, a narrower one is written as its register
// holds it (extended by its own signedness). Promotion makes the
// conversion explicit.
bool storable(const FunctionBody& body, const Instruction& store)
{
	const Type from = stored_type(body, store);
	if (from == store.type)
		return true;
	return value_bits(from) != 0 && value_bits(store.type) != 0;
}

bool signed_kind(Type type)
{
	switch (type.kind())
	{
	case TypeKind::I8:
	case TypeKind::I16:
	case TypeKind::I32:
	case TypeKind::I64:
	case TypeKind::I128:
		return true;
	default:
		return false;
	}
}

// Signed and unsigned integer types of one width.
bool integer_twins(Type a, Type b)
{
	return a.is_integer() && b.is_integer() && a.kind() != b.kind() && a.kind() != TypeKind::I1 &&
		b.kind() != TypeKind::I1 && integer_bits(a.kind()) == integer_bits(b.kind());
}

class SlotPromotion
{
public:
	explicit SlotPromotion(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();
	// The members' vectors, kept across runs for their capacity (each run
	// assigns or empties every one before reading it).
	struct Pool
	{
		std::vector<uint32_t> candidate, df_first, df, succ_slot, phi_first;
		std::vector<SlotId> slots;
		std::vector<std::pair<uint32_t, InstId>> phis;
		std::vector<Value> current, repl;
		std::vector<std::pair<uint32_t, Value>> undo;
		std::vector<uint8_t> remove, accessed;
	};
	void exchange(Pool& pool)
	{
		candidate_.swap(pool.candidate);
		df_first_.swap(pool.df_first);
		df_.swap(pool.df);
		succ_slot_.swap(pool.succ_slot);
		phi_first_.swap(pool.phi_first);
		slots_.swap(pool.slots);
		phis_.swap(pool.phis);
		current_.swap(pool.current);
		repl_.swap(pool.repl);
		undo_.swap(pool.undo);
		remove_.swap(pool.remove);
		accessed_.swap(pool.accessed);
	}
	// The run promoted only slots nothing accesses (the body is as it was).
	bool kept_body() const { return kept_body_; }

private:
	void classify();
	void exclude_handler_state(std::vector<uint8_t>& ok);
	bool handler_liveness(const std::vector<uint8_t>& reached, const std::vector<uint32_t>& relevant, uint32_t count,
		std::vector<uint8_t>& live);
	void dominance_frontiers();
	void place_phis();
	void rename();
	void rename_block(BlockId block);
	void finish();

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	// Per slot: dense candidate index or kNoId.
	std::vector<uint32_t> candidate_;
	std::vector<SlotId> slots_;           // candidate -> slot
	std::vector<uint32_t> df_first_, df_; // dominance frontiers (CSR)
	std::vector<uint32_t> succ_slot_;     // per cfg succs entry: its predecessor slot in the target
	// Phis placed: (candidate, instruction) per block (CSR, in placement
	// order).
	std::vector<uint32_t> phi_first_;
	std::vector<std::pair<uint32_t, InstId>> phis_;
	std::vector<Value> current_; // per candidate: the value now
	std::vector<std::pair<uint32_t, Value>> undo_;
	std::vector<Value> repl_;                  // per temp: a promoted load's value
	std::vector<uint8_t> remove_;              // per instruction
	uint64_t phi_inputs_ = 0, phi_budget_ = 0; // place_phis' work budget
	std::vector<uint8_t> accessed_;            // per slot: some instruction names it
	bool kept_body_ = false;
	// Whether any candidate still promoted is accessed.
	bool any_accessed() const
	{
		for (SlotId s : slots_)
			if (candidate_[s] != kNoId && accessed_[s])
				return true;
		return false;
	}
};

// Handler entry (opener blocks' handler edges, opener/eh_end depth over
// ordinary edges): the renaming gives a handler block the value each of
// its openers' blocks ends with (one value: a candidate needing a phi in
// a handler block stays a slot, place_phis). That is the value at every
// throw point of each region when the slot is not stored while a region
// is active, nor after an opener in that opener's block (where an eh_end
// may already have popped it), nor in handler code (an outer cleanup
// that an inner one jumps to included), and no handler block is also
// reached from the entry over ordinary edges (the walk below would give
// it that path's region depth). A slot read in code reachable from a
// handler (its code, and what follows a catch) and live at a handler's
// entry is promoted only under those conditions; others keep their
// memory. O(blocks + edges + instructions), plus the bounded liveness.
void SlotPromotion::exclude_handler_state(std::vector<uint8_t>& ok)
{
	const uint32_t n = cfg_.block_count;
	bool any = false;
	for (BlockId b : body_.layout)
		any = any || (cfg_.flags[b] & BF_HANDLER) != 0;
	if (!any)
		return;
	// Blocks a handler reaches (ordinary and handler edges).
	std::vector<uint8_t> reached(n, 0);
	std::vector<BlockId> work;
	bool fallback = false;
	for (BlockId b : body_.layout)
		if ((cfg_.flags[b] & BF_HANDLER) != 0)
		{
			reached[b] = 1;
			work.push_back(b);
		}
	while (!work.empty())
	{
		const BlockId b = work.back();
		work.pop_back();
		auto visit = [&](BlockId s) {
			if (!reached[s] && (cfg_.flags[s] & BF_PLACED) != 0)
			{
				reached[s] = 1;
				work.push_back(s);
			}
		};
		for (uint32_t k = cfg_.succ_first[b]; k < cfg_.succ_first[b + 1]; ++k)
			visit(cfg_.succs[k]);
		for (uint32_t k = cfg_.handler_first[b]; k < cfg_.handler_first[b + 1]; ++k)
			visit(cfg_.handlers[k]);
	}
	// Region depth at each normal block's entry (blocks the entry reaches
	// over ordinary edges); stores the handler entry cannot see.
	const uint32_t slots = static_cast<uint32_t>(body_.slots.size());
	std::vector<uint8_t> unseen(slots, 0);
	std::vector<uint32_t> depth(n, kNoId);
	depth[body_.layout[0]] = 0;
	work.assign(1, body_.layout[0]);
	while (!work.empty() && !fallback)
	{
		const BlockId b = work.back();
		work.pop_back();
		uint32_t d = depth[b];
		bool opened = false;
		for (InstId i = body_.blocks[b].first; i != kNoId && !fallback; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::EhTry || inst.op == Opcode::EhCleanup)
			{
				++d;
				opened = true;
			}
			else if (inst.op == Opcode::EhEnd)
			{
				if (d == 0)
					fallback = true;
				else
					--d;
			}
			else if (inst.op == Opcode::Store && inst.ops[1].is(ValueKind::Slot) && (d != 0 || opened))
				unseen[inst.ops[1].index] = 1;
		}
		for (uint32_t k = cfg_.succ_first[b]; k < cfg_.succ_first[b + 1]; ++k)
		{
			const BlockId s = cfg_.succs[k];
			if (depth[s] == kNoId)
			{
				depth[s] = d;
				work.push_back(s);
			}
			else if (depth[s] != d)
				fallback = true;
		}
	}
	for (BlockId b : body_.layout)
		fallback = fallback || ((cfg_.flags[b] & BF_HANDLER) != 0 && depth[b] != kNoId);
	// Handler code: blocks no ordinary path from the entry reaches.
	std::vector<uint8_t> loaded(slots, 0);
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Store && inst.ops[1].is(ValueKind::Slot) && depth[b] == kNoId)
				unseen[inst.ops[1].index] = 1;
			if (inst.op == Opcode::Load && inst.ops[0].is(ValueKind::Slot) && reached[b])
				loaded[inst.ops[0].index] = 1;
		}
	// The slots at stake: read where a handler reaches and stored where
	// its entry cannot see. Each is excluded when live at a handler's
	// entry (a handler that stores it before any read does not care).
	std::vector<uint32_t> relevant(slots, kNoId);
	std::vector<SlotId> at_stake;
	for (SlotId s = 0; s < slots; ++s)
		if (ok[s] && loaded[s] && (unseen[s] || fallback))
		{
			relevant[s] = static_cast<uint32_t>(at_stake.size());
			at_stake.push_back(s);
		}
	if (at_stake.empty())
		return;
	std::vector<uint8_t> live;
	if (!handler_liveness(reached, relevant, static_cast<uint32_t>(at_stake.size()), live))
		live.assign(at_stake.size(), 1);
	for (uint32_t r = 0; r < at_stake.size(); ++r)
		if (live[r])
			ok[at_stake[r]] = 0;
}

// Liveness of the slots at stake (relevant: slot -> dense index) at the
// entries of handler blocks, over the blocks handlers reach (ordinary and
// handler edges): a backward worklist of bit sets, each bit set at most
// once. False (unknown: every slot live) past the work budget.
bool SlotPromotion::handler_liveness(const std::vector<uint8_t>& reached, const std::vector<uint32_t>& relevant,
	uint32_t count, std::vector<uint8_t>& live)
{
	const uint64_t kBudgetBits = 1u << 22;
	const uint32_t words = (count + 63) / 64;
	std::vector<uint32_t> dense(cfg_.block_count, kNoId);
	std::vector<BlockId> blocks;
	for (BlockId b : body_.layout)
		if (reached[b])
		{
			dense[b] = static_cast<uint32_t>(blocks.size());
			blocks.push_back(b);
		}
	if (static_cast<uint64_t>(blocks.size()) * words * 64 > kBudgetBits)
		return false;
	const size_t n = blocks.size();
	std::vector<uint64_t> exposed(n * words, 0), killed(n * words, 0), in(n * words, 0);
	for (size_t d = 0; d < n; ++d)
		for (InstId i = body_.blocks[blocks[d]].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			const bool load = inst.op == Opcode::Load && inst.ops[0].is(ValueKind::Slot);
			const bool store = inst.op == Opcode::Store && inst.ops[1].is(ValueKind::Slot);
			if (!load && !store)
				continue;
			const uint32_t r = relevant[inst.ops[load ? 0 : 1].index];
			if (r == kNoId)
				continue;
			const uint64_t bit = 1ull << (r % 64);
			uint64_t& k = killed[d * words + r / 64];
			if (load && (k & bit) == 0)
				exposed[d * words + r / 64] |= bit;
			if (store)
				k |= bit;
		}
	// Predecessors within the reached blocks (ordinary and handler edges).
	std::vector<std::vector<uint32_t>> preds(n);
	for (size_t d = 0; d < n; ++d)
	{
		const BlockId b = blocks[d];
		for (uint32_t k = cfg_.succ_first[b]; k < cfg_.succ_first[b + 1]; ++k)
			if (dense[cfg_.succs[k]] != kNoId)
				preds[dense[cfg_.succs[k]]].push_back(static_cast<uint32_t>(d));
		for (uint32_t k = cfg_.handler_first[b]; k < cfg_.handler_first[b + 1]; ++k)
			if (dense[cfg_.handlers[k]] != kNoId)
				preds[dense[cfg_.handlers[k]]].push_back(static_cast<uint32_t>(d));
	}
	std::vector<uint32_t> work;
	std::vector<uint8_t> queued(n, 1);
	for (size_t d = n; d-- > 0;)
	{
		work.push_back(static_cast<uint32_t>(d));
		for (uint32_t w = 0; w < words; ++w)
			in[d * words + w] = exposed[d * words + w];
	}
	while (!work.empty())
	{
		const uint32_t d = work.back();
		work.pop_back();
		queued[d] = 0;
		// in(p) |= in(d) - killed(p) for each predecessor p.
		for (uint32_t p : preds[d])
		{
			bool grew = false;
			for (uint32_t w = 0; w < words; ++w)
			{
				const uint64_t add = in[d * words + w] & ~killed[p * words + w] & ~in[p * words + w];
				if (add != 0)
				{
					in[p * words + w] |= add;
					grew = true;
				}
			}
			if (grew && !queued[p])
			{
				queued[p] = 1;
				work.push_back(p);
			}
		}
	}
	live.assign(count, 0);
	for (size_t d = 0; d < n; ++d)
		if ((cfg_.flags[blocks[d]] & BF_HANDLER) != 0)
			for (uint32_t r = 0; r < count; ++r)
				if ((in[d * words + r / 64] >> (r % 64)) & 1)
					live[r] = 1;
	return true;
}

// A slot is promoted when every use is a nonvolatile load or store of its
// own scalar (phi-representable) type, or of the other integer type of its
// width (`int r; r += size;` reads r as unsigned: a retyping copy), naming
// it directly, and the state a handler entry sees is the renaming's
// (exclude_handler_state).
void SlotPromotion::classify()
{
	const uint32_t slots = static_cast<uint32_t>(body_.slots.size());
	static std::vector<uint8_t> ok; // (kept for its capacity)
	ok.assign(slots, 1);
	accessed_.assign(slots, 0);
	for (SlotId s = 0; s < slots; ++s)
		ok[s] = phi_type(body_.slots[s].type) ? 1 : 0;
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			uint32_t position = 0;
			for_each_operand(body_, inst, [&](const Value& v) {
				const uint32_t at = position++;
				if (!v.is(ValueKind::Slot))
					return;
				accessed_[v.index] = 1;
				const Type type = body_.slots[v.index].type;
				const bool typed = inst.type == type || integer_twins(inst.type, type);
				const bool load = inst.op == Opcode::Load && at == 0 && (inst.flags & IF_VOLATILE) == 0 && typed;
				const bool store = inst.op == Opcode::Store && at == 1 && (inst.flags & IF_VOLATILE) == 0 && typed &&
					!inst.ops[0].is(ValueKind::Slot) && storable(body_, inst);
				if (!load && !store)
					ok[v.index] = 0;
			});
		}
	// (It excludes only slots some instruction loads.)
	bool accessed = false;
	for (SlotId s = 0; s < slots && !accessed; ++s)
		accessed = ok[s] && accessed_[s];
	if (accessed)
		exclude_handler_state(ok);
	candidate_.assign(slots, kNoId);
	slots_.clear();
	for (SlotId s = 0; s < slots; ++s)
		if (ok[s])
		{
			candidate_[s] = static_cast<uint32_t>(slots_.size());
			slots_.push_back(s);
		}
}

// Cytron et al.'s frontiers from the dominator tree, over ordinary edges.
void SlotPromotion::dominance_frontiers()
{
	const uint32_t n = cfg_.block_count;
	// (block, frontier block) in discovery order, a block's repeat of the
	// last one it found dropped; then by block.
	static std::vector<std::pair<BlockId, BlockId>> found; // (kept for its capacity)
	static std::vector<BlockId> last;
	found.clear();
	last.assign(n, kNoId);
	for (BlockId b : cfg_.rpo)
	{
		if (cfg_.pred_count(b) < 2)
			continue;
		for (uint32_t k = cfg_.pred_first[b]; k < cfg_.pred_first[b + 1]; ++k)
		{
			BlockId runner = cfg_.preds[k];
			if (!cfg_.reachable(runner))
				continue;
			while (runner != kNoId && runner != cfg_.idom[b])
			{
				if (last[runner] != b)
				{
					last[runner] = b;
					found.push_back(std::make_pair(runner, b));
				}
				runner = cfg_.idom[runner];
			}
		}
	}
	df_first_.assign(n + 1, 0);
	for (const std::pair<BlockId, BlockId>& f : found)
		++df_first_[f.first + 1];
	for (BlockId b = 0; b < n; ++b)
		df_first_[b + 1] += df_first_[b];
	df_.assign(found.size(), 0);
	std::vector<uint32_t>& fill = last; // (now each block's next position)
	fill.assign(df_first_.begin(), df_first_.end() - 1);
	for (const std::pair<BlockId, BlockId>& f : found)
		df_[fill[f.first]++] = f.second;
}

// Phis at the iterated frontier of each candidate's stores; a candidate
// needing a phi in a handler block stays a slot.
void SlotPromotion::place_phis()
{
	const uint32_t n = cfg_.block_count;
	const uint32_t count = static_cast<uint32_t>(slots_.size());
	// The budget counts the body's other instructions (phis placed by
	// earlier runs do not raise it).
	phi_inputs_ = 0;
	phi_budget_ = 4096;
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			phi_budget_ += body_.instructions[i].op == Opcode::Phi ? 0 : 4;
	// Each candidate's store blocks in rpo order, a repeat of the last one
	// dropped: (candidate, block) pairs, then CSR by candidate.
	// (The vectors below persist for their capacity: each is assigned or
	// emptied before use; promotion never runs inside itself.)
	static std::vector<std::pair<uint32_t, BlockId>> stores;
	stores.clear();
	// Semi-pruned placement: a candidate read in no block before that
	// block stores it is never live into a block, and needs no phi.
	static std::vector<uint8_t> exposed;
	static std::vector<uint32_t> stored_in; // also the last store block
	exposed.assign(count, 0);
	stored_in.assign(count, kNoId);
	for (BlockId b : cfg_.rpo)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Load && inst.ops[0].is(ValueKind::Slot) && candidate_[inst.ops[0].index] != kNoId &&
				stored_in[candidate_[inst.ops[0].index]] != b)
				exposed[candidate_[inst.ops[0].index]] = 1;
			if (inst.op == Opcode::Store && inst.ops[1].is(ValueKind::Slot) && candidate_[inst.ops[1].index] != kNoId)
			{
				const uint32_t c = candidate_[inst.ops[1].index];
				if (stored_in[c] != b)
					stores.push_back(std::make_pair(c, b));
				stored_in[c] = b;
			}
		}
	static std::vector<uint32_t> def_first;
	def_first.assign(count + 1, 0);
	for (const std::pair<uint32_t, BlockId>& d : stores)
		++def_first[d.first + 1];
	for (uint32_t c = 0; c < count; ++c)
		def_first[c + 1] += def_first[c];
	static std::vector<BlockId> defs;
	defs.assign(stores.size(), 0);
	{
		static std::vector<uint32_t> fill;
		fill.assign(def_first.begin(), def_first.end() - 1);
		for (const std::pair<uint32_t, BlockId>& d : stores)
			defs[fill[d.first]++] = d.second;
	}
	// (block, candidate, phi) in placement order.
	static std::vector<std::pair<BlockId, std::pair<uint32_t, InstId>>> placements;
	static std::vector<uint32_t> placed, queued;
	static std::vector<BlockId> work;
	static std::vector<std::pair<BlockId, uint32_t>> wanted;
	placements.clear();
	placed.assign(n, kNoId);
	queued.assign(n, kNoId);
	work.clear();
	wanted.clear();
	for (uint32_t c = 0; c < count; ++c)
	{
		if (!exposed[c])
			continue;
		wanted.clear();
		work.assign(defs.begin() + def_first[c], defs.begin() + def_first[c + 1]);
		for (BlockId b : work)
			queued[b] = c;
		bool rejected = false;
		while (!work.empty() && !rejected)
		{
			const BlockId b = work.back();
			work.pop_back();
			for (uint32_t k = df_first_[b]; k < df_first_[b + 1]; ++k)
			{
				const BlockId f = df_[k];
				if (placed[f] == c)
					continue;
				if ((cfg_.flags[f] & BF_HANDLER) != 0)
				{
					rejected = true;
					break;
				}
				placed[f] = c;
				wanted.push_back(std::make_pair(f, c));
				if (queued[f] != c)
				{
					queued[f] = c;
					work.push_back(f);
				}
			}
		}
		// The work budget: the phis' inputs over the whole body stay within
		// a multiple of its size (a deep loop nest whose counters each get
		// a dead phi at every enclosing header would place depth^2); past
		// it the remaining candidates keep their memory.
		uint64_t inputs = 0;
		for (const std::pair<BlockId, uint32_t>& w : wanted)
			inputs += cfg_.pred_count(w.first);
		if (rejected || phi_inputs_ + inputs > phi_budget_)
		{
			candidate_[slots_[c]] = kNoId;
			continue;
		}
		phi_inputs_ += inputs;
		const Type type = body_.slots[slots_[c]].type;
		for (const std::pair<BlockId, uint32_t>& w : wanted)
		{
			// One (pred, value) pair per ordinary predecessor, values set
			// by the renaming.
			std::vector<Value> pairs;
			for (uint32_t k = cfg_.pred_first[w.first]; k < cfg_.pred_first[w.first + 1]; ++k)
			{
				pairs.push_back(Value::block(cfg_.preds[k]));
				pairs.push_back(Value::none());
			}
			Instruction phi = make_instruction(Opcode::Phi, type);
			phi.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
			phi.extra_count = static_cast<uint32_t>(pairs.size());
			const InstId id = append_instruction(body_, phi);
			body_.instructions[id].result = append_temp(body_, type, id);
			placements.push_back(std::make_pair(w.first, std::make_pair(c, id)));
			++c_.stats.phis_inserted;
		}
	}
	phi_first_.assign(n + 1, 0);
	for (const std::pair<BlockId, std::pair<uint32_t, InstId>>& p : placements)
		++phi_first_[p.first + 1];
	for (BlockId b = 0; b < n; ++b)
		phi_first_[b + 1] += phi_first_[b];
	phis_.resize(placements.size());
	static std::vector<uint32_t> fill; // (kept for its capacity)
	fill.assign(phi_first_.begin(), phi_first_.end() - 1);
	for (const std::pair<BlockId, std::pair<uint32_t, InstId>>& p : placements)
		phis_[fill[p.first]++] = p.second;
}

void SlotPromotion::rename_block(BlockId block)
{
	for (uint32_t q = phi_first_[block]; q < phi_first_[block + 1]; ++q)
	{
		const std::pair<uint32_t, InstId>& p = phis_[q];
		if (candidate_[slots_[p.first]] == kNoId)
			continue;
		undo_.push_back(std::make_pair(p.first, current_[p.first]));
		current_[p.first] = Value::temp(body_.instructions[p.second].result);
	}
	InstId prev = kNoId;
	for (InstId i = body_.blocks[block].first; i != kNoId; i = body_.instructions[i].next)
	{
		const Instruction inst = body_.instructions[i];
		if (inst.op == Opcode::Load && inst.ops[0].is(ValueKind::Slot) && candidate_[inst.ops[0].index] != kNoId)
		{
			const uint32_t c = candidate_[inst.ops[0].index];
			if (current_[c].is(ValueKind::None))
			{
				// Read before any store: an indeterminate value, zero.
				undo_.push_back(std::make_pair(c, current_[c]));
				Literal zero = { 0, 0 };
				current_[c] = inst.type.is_float() ? literal_value(body_, ValueKind::Float, inst.type.kind(), zero)
												   : integer_value(body_, zero);
			}
			repl_[inst.result] = current_[c];
			if (inst.type != body_.slots[inst.ops[0].index].type && current_[c].is(ValueKind::Temp))
			{
				// A twin-typed read: a copy retyping the value.
				Instruction retype = make_instruction(Opcode::Copy, inst.type);
				retype.ops[0] = current_[c];
				retype.location = inst.location;
				const InstId id = append_instruction(body_, retype);
				body_.instructions[id].result = append_temp(body_, inst.type, id);
				insert_after(body_, block, prev, id);
				remove_.resize(body_.instructions.size(), 0);
				repl_.resize(body_.temps.size(), Value::none());
				repl_[inst.result] = Value::temp(body_.instructions[id].result);
				prev = id;
			}
			remove_[i] = 1;
		}
		else if (inst.op == Opcode::Store && inst.ops[1].is(ValueKind::Slot) && candidate_[inst.ops[1].index] != kNoId)
		{
			const uint32_t c = candidate_[inst.ops[1].index];
			undo_.push_back(std::make_pair(c, current_[c]));
			current_[c] = inst.ops[0];
			const Type from = stored_type(body_, inst);
			const Type held = body_.slots[inst.ops[1].index].type;
			if (from != held)
			{
				// The value the slot holds: a retype, a truncation or an
				// extension.
				const unsigned bits_from = value_bits(from), bits_to = value_bits(held);
				const bool same = bits_from == bits_to;
				Instruction convert = make_instruction(same ? Opcode::Copy : Opcode::Convert, held);
				convert.ops[0] = inst.ops[0];
				convert.location = inst.location;
				if (!same)
				{
					convert.sub = static_cast<uint8_t>(bits_from > bits_to ? ConvertOp::Trunc
							: signed_kind(from)                            ? ConvertOp::SExt
																		   : ConvertOp::ZExt);
					set_convert_source_type(convert, from);
				}
				const InstId id = append_instruction(body_, convert);
				body_.instructions[id].result = append_temp(body_, held, id);
				insert_after(body_, block, prev, id);
				remove_.resize(body_.instructions.size(), 0);
				current_[c] = Value::temp(body_.instructions[id].result);
				prev = id;
			}
			remove_[i] = 1;
		}
		prev = i;
	}
	for (uint32_t k = cfg_.succ_first[block]; k < cfg_.succ_first[block + 1]; ++k)
	{
		const BlockId s = cfg_.succs[k];
		// A placed phi lists its block's predecessors in order: this
		// edge's pair is at the edge's predecessor slot.
		const uint32_t pair = 2 * (succ_slot_[k] - cfg_.pred_first[s]);
		for (uint32_t q = phi_first_[s]; q < phi_first_[s + 1]; ++q)
		{
			const std::pair<uint32_t, InstId>& p = phis_[q];
			if (candidate_[slots_[p.first]] == kNoId)
				continue;
			Instruction& phi = body_.instructions[p.second];
			for (uint32_t j = pair; j == pair && j < phi.extra_count; ++j)
				if (body_.operands[phi.extra_first + j].index == block)
				{
					Value v = current_[p.first];
					if (v.is(ValueKind::None))
					{
						Literal zero = { 0, 0 };
						v = phi.type.is_float() ? literal_value(body_, ValueKind::Float, phi.type.kind(), zero)
												: integer_value(body_, zero);
					}
					const Type phi_type = phi.type;
					v = phi_input(body_, phi_type, v);
					body_.operands[body_.instructions[p.second].extra_first + j + 1] = v;
				}
		}
	}
}

void SlotPromotion::rename()
{
	// build_cfg fills predecessors by source block, then successor order.
	succ_slot_.assign(cfg_.succs.size(), 0);
	static std::vector<uint32_t> fill; // (kept for its capacity)
	fill.assign(cfg_.pred_first.begin(), cfg_.pred_first.end() - 1);
	for (BlockId b = 0; b < cfg_.block_count; ++b)
		for (uint32_t k = cfg_.succ_first[b]; k < cfg_.succ_first[b + 1]; ++k)
			succ_slot_[k] = fill[cfg_.succs[k]]++;
	current_.assign(slots_.size(), Value::none());
	repl_.assign(body_.temps.size(), Value::none());
	remove_.assign(body_.instructions.size(), 0);
	undo_.clear();
	const DomChildren& tree_ = c_.ensure_dom();
	static std::vector<std::pair<BlockId, uint32_t>> stack; // (kept for their capacity)
	static std::vector<size_t> marks;
	stack.clear();
	marks.clear();
	const BlockId entry = cfg_.rpo[0];
	stack.push_back(std::make_pair(entry, 0u));
	marks.push_back(undo_.size());
	rename_block(entry);
	while (!stack.empty())
	{
		const BlockId b = stack.back().first;
		const uint32_t k = stack.back().second++;
		if (tree_.first[b] + k < tree_.first[b + 1])
		{
			const BlockId child = tree_.children[tree_.first[b] + k];
			stack.push_back(std::make_pair(child, 0u));
			marks.push_back(undo_.size());
			rename_block(child);
			continue;
		}
		for (size_t u = undo_.size(); u > marks.back(); --u)
			current_[undo_[u - 1].first] = undo_[u - 1].second;
		undo_.resize(marks.back());
		marks.pop_back();
		stack.pop_back();
	}
}

void SlotPromotion::finish()
{
	// Placed phis go at the start of their blocks; an input from a
	// predecessor the renaming never reached is zero.
	for (BlockId b = 0; b + 1 < phi_first_.size(); ++b)
		for (uint32_t q = phi_first_[b]; q < phi_first_[b + 1]; ++q)
			if (candidate_[slots_[phis_[q].first]] != kNoId)
			{
				const std::pair<uint32_t, InstId>& p = phis_[q];
				insert_after(body_, b, kNoId, p.second);
				const Instruction& phi = body_.instructions[p.second];
				for (uint32_t j = 1; j < phi.extra_count; j += 2)
					if (body_.operands[phi.extra_first + j].is(ValueKind::None))
					{
						Literal zero = { 0, 0 };
						body_.operands[phi.extra_first + j] = phi.type.is_float()
							? literal_value(body_, ValueKind::Float, phi.type.kind(), zero)
							: integer_value(body_, zero);
					}
			}
	auto resolve = [&](Value v) {
		while (v.is(ValueKind::Temp) && v.index < repl_.size() && !repl_[v.index].is(ValueKind::None))
			v = repl_[v.index];
		return v;
	};
	for (BlockId b : body_.layout)
	{
		InstId prev = kNoId;
		for (InstId i = body_.blocks[b].first; i != kNoId;)
		{
			const InstId next = body_.instructions[i].next;
			if (i < remove_.size() && remove_[i])
			{
				unlink_after(body_, b, prev, i);
				i = next;
				continue;
			}
			for_each_operand(body_, body_.instructions[i], [&](Value& v) { v = resolve(v); });
			prev = i;
			i = next;
		}
	}
}

bool SlotPromotion::run()
{
	if (body_.slots.empty())
		return false;
	c_.ensure_cfg();
	if (cfg_.rpo.empty())
		return false;
	classify();
	if (slots_.empty())
		return false;
	// Candidates nothing loads or stores (slots promoted before, which
	// stay in the slot table) are promoted with nothing to place or
	// rename: the run reports them, but the body is as it was.
	if (!any_accessed())
	{
		c_.stats.slots_promoted += slots_.size();
		kept_body_ = true;
		return true;
	}
	dominance_frontiers();
	place_phis();
	// Every candidate over the phi budget: nothing to rename (the body is
	// as it was; reporting a change would buy another scalar round).
	uint32_t kept = 0;
	for (uint32_t c = 0; c < slots_.size(); ++c)
		kept += candidate_[slots_[c]] != kNoId ? 1 : 0;
	if (kept == 0)
		return false;
	// (Only unaccessed candidates kept: place_phis placed nothing.)
	if (!any_accessed())
	{
		c_.stats.slots_promoted += kept;
		kept_body_ = true;
		return true;
	}
	rename();
	finish();
	c_.stats.slots_promoted += kept;
	return true;
}

} // namespace

bool promote_slots(FunctionContext& context)
{
	static SlotPromotion::Pool pool;
	SlotPromotion promotion(context);
	promotion.exchange(pool);
	const bool ran = promotion.run();
	promotion.exchange(pool);
	if (!ran)
		return false;
	// A run the driver made that left the body as it was: the version
	// still names the body, which may be known to be in order.
	const bool settled = promotion.kept_body() && context.running == promote_slots;
	if (settled && context.ordered_version == context.version)
	{
		context.kept_body = promote_slots;
		return true;
	}
	// A promoted value may now be used before its definition in layout
	// (input whose layout is not in dominance order: a jump into a loop).
	if (!restore_definition_order(context) && promotion.kept_body())
	{
		context.kept_body = promote_slots;
		if (settled)
			context.ordered_version = context.version;
	}
	return true;
}

} // namespace opt
