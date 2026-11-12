#include <vector>

#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

// The most places an address is formed again (blocks, and edges out of its
// own block).
const unsigned kSites = 3;
// The greatest byte distance an address expression's displacement takes.
const int64_t kDisplacement = int64_t(1) << 30;

// O2+, last: an address (an `index`) that its own block only reads memory
// through, and other blocks read, is formed again where those blocks read
// it. Its own block then needs it in no register: every access there takes
// the address expression itself (a hash table's probe reads the bucket's
// key through base + index * scale, and the bucket's address forms where
// the probe ends). The copies read the base and the offset, which reach
// every block the address did. No copy goes into a loop the address is
// formed outside of.
class AddressSinking
{
public:
	explicit AddressSinking(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}
	bool run();

private:
	struct Site
	{
		BlockId block; // where the copy goes; for an edge, the block the edge enters
		bool edge;     // on the edge from the address's block
		TempId copy;
	};
	struct Info
	{
		TempId temp = kNoId;
		uint32_t reads = 0; // loads and stores through it in its own block, and fields' addresses
		bool held = false;  // some use needs it where it is (or no copy can serve one)
		unsigned count = 0;
		Site sites[kSites];
	};

	bool candidate(const Instruction& inst) const;
	// The address `v` is (or names, as a copy of one in the address's own
	// block), or kNoId.
	TempId root(Value v) const
	{
		if (!v.is(ValueKind::Temp) || v.index >= info_of_.size())
			return kNoId;
		const TempId t = alias_[v.index] != kNoId ? alias_[v.index] : v.index;
		return info_of_[t] != kNoId ? t : kNoId;
	}
	Info& info(TempId address) { return infos_[info_of_[address]]; }
	BlockId home(TempId address) const { return block_of_[body_.temps[address].def]; }
	bool placeable(BlockId at, BlockId home) const;
	void outside(Info& info, BlockId at, bool edge, BlockId home);
	void note(TempId address, const Instruction& user, unsigned slot, BlockId at);
	Value copy_for(TempId address, BlockId at, bool edge);
	bool chosen(const Info& info) const { return !info.held && info.reads != 0 && info.count != 0; }
	InstId make_copy(TempId address, BlockId at, bool edge);

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	LoopForest forest_;
	std::vector<BlockId> block_of_;
	std::vector<uint32_t> info_of_; // per temp: its entry of infos_, or kNoId
	std::vector<TempId> alias_;     // per temp: the address it copies in that one's block, or kNoId
	std::vector<Info> infos_;
	std::vector<InstId> tail_; // per block: the last copy placed at its top
};

// An index whose address a memory operand can hold: a literal offset in
// reach, or a register offset at a scale of 1, 2, 4 or 8 on a base that is a
// value.
bool AddressSinking::candidate(const Instruction& inst) const
{
	if (inst.op != Opcode::Index || inst.result == kNoId)
		return false;
	const int64_t size = static_cast<int64_t>(storage_bytes(inst.type));
	const Value base = inst.ops[0], offset = inst.ops[1];
	if (offset.is(ValueKind::Integer))
	{
		const Literal k = body_.literals[offset.index];
		const int64_t count = static_cast<int64_t>(k.low);
		return (base.is(ValueKind::Temp) || base.is(ValueKind::Slot)) && size != 0 && count < kDisplacement / size &&
			count > -kDisplacement / size;
	}
	return base.is(ValueKind::Temp) && offset.is(ValueKind::Temp) && (size == 1 || size == 2 || size == 4 || size == 8);
}

// Whether a copy may go at the top of `at` (or on an edge into it): not in
// a handler, nor in a loop that does not hold `home`.
bool AddressSinking::placeable(BlockId at, BlockId home) const
{
	if (!cfg_.reachable(at) || (cfg_.flags[at] & BF_HANDLER) != 0 || forest_.dropped[at] != 0)
		return false;
	const uint32_t loop = forest_.innermost[at];
	return loop == kNoId || in_loop(forest_, loop, home);
}

// A use outside the address's block: served by a site that dominates it,
// or a new site (which serves the sites it dominates).
void AddressSinking::outside(Info& info, BlockId at, bool edge, BlockId home)
{
	if (info.held || !placeable(at, home))
	{
		info.held = true;
		return;
	}
	unsigned kept = 0;
	for (unsigned k = 0; k < info.count; ++k)
	{
		const Site& site = info.sites[k];
		if (edge ? site.edge && site.block == at : !site.edge && cfg_.dominates(site.block, at))
			return;
		if (edge || site.edge || !cfg_.dominates(at, site.block))
			info.sites[kept++] = site;
	}
	if (kept == kSites)
	{
		info.held = true;
		return;
	}
	info.sites[kept] = Site{ at, edge, kNoId };
	info.count = kept + 1;
}

// One read of `address` by operand `slot` of `user` in block `at` (for a
// phi, the predecessor the value comes from).
void AddressSinking::note(TempId address, const Instruction& user, unsigned slot, BlockId at)
{
	Info& info = this->info(address);
	const BlockId home = this->home(address);
	if (user.op == Opcode::Phi)
	{
		if (at != home)
		{
			outside(info, at, false, home);
			return;
		}
		// Read at the end of its own block: on the edge, when the block
		// branches two ways and the edge is not a loop's back edge (whose
		// copy would run as often as the block).
		const Instruction& term = body_.instructions[body_.blocks[home].last];
		const BlockId to = block_of_[static_cast<InstId>(&user - body_.instructions.data())];
		if (term.op != Opcode::Branch || term.ops[1].index == term.ops[2].index || cfg_.dominates(to, home))
			info.held = true;
		else
			outside(info, to, true, home);
		return;
	}
	if (at != home)
	{
		outside(info, at, false, home);
		return;
	}
	if ((user.op == Opcode::Load && slot == 0) || (user.op == Opcode::Store && slot == 1))
	{
		if ((user.flags & IF_VOLATILE) == 0 && !user.type.is_object())
			++info.reads;
		else
			info.held = true;
		return;
	}
	// A copy of it there is the address by another name: its readers count.
	if (user.op == Opcode::Copy && slot == 0 && user.result != kNoId && alias_[user.result] == address)
		return;
	// A field's address composes with it (and forms from its parts).
	if (user.op == Opcode::Index && slot == 0 && user.ops[1].is(ValueKind::Integer) && candidate(user))
		++info.reads;
	else
		info.held = true;
}

// A copy of `address` for a site (not yet in a block). Its base, when that
// is a chosen address of the same block, is that one's copy for the site,
// placed first.
InstId AddressSinking::make_copy(TempId address, BlockId at, bool edge)
{
	Instruction copy = body_.instructions[body_.temps[address].def];
	copy.next = kNoId;
	const TempId base = root(copy.ops[0]);
	if (base != kNoId && chosen(info(base)) && home(base) == home(address))
		copy.ops[0] = copy_for(base, at, edge);
	const InstId id = append_instruction(body_, copy);
	body_.instructions[id].result = append_temp(body_, body_.temps[address].type, id);
	return id;
}

// The copy of `address` that serves a use in `at` (on the edge into `at`).
Value AddressSinking::copy_for(TempId address, BlockId at, bool edge)
{
	for (unsigned k = 0; k < info(address).count; ++k)
	{
		const Site site = info(address).sites[k];
		if (edge ? !(site.edge && site.block == at) : site.edge || !cfg_.dominates(site.block, at))
			continue;
		if (site.copy != kNoId || edge)
			return site.copy == kNoId ? Value::temp(address) : Value::temp(site.copy);
		// At the block's top, after its phis and the copies before it (its
		// base's among them).
		const InstId id = make_copy(address, site.block, false);
		InstId after = tail_[site.block];
		if (after == kNoId)
			for (InstId i = body_.blocks[site.block].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
				i = body_.instructions[i].next)
				after = i;
		insert_after(body_, site.block, after, id);
		tail_[site.block] = id;
		info(address).sites[k].copy = body_.instructions[id].result;
		return Value::temp(body_.instructions[id].result);
	}
	return Value::temp(address);
}

bool AddressSinking::run()
{
	if (body_.layout.size() == 1)
		return false;
	c_.ensure_cfg();
	find_loops(body_, cfg_, forest_);
	if (forest_.partial)
		return false;
	map_instruction_blocks(body_, block_of_);
	info_of_.assign(body_.temps.size(), kNoId);
	infos_.clear();
	alias_.assign(body_.temps.size(), kNoId);
	for (BlockId b : cfg_.rpo)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (candidate(inst))
			{
				info_of_[inst.result] = static_cast<uint32_t>(infos_.size());
				infos_.push_back(Info());
				infos_.back().temp = inst.result;
			}
			else if (inst.op == Opcode::Copy && inst.result != kNoId && inst.type.is_pointer() &&
				root(inst.ops[0]) != kNoId && home(root(inst.ops[0])) == b)
				alias_[inst.result] = root(inst.ops[0]);
		}
	if (infos_.empty())
		return false;
	for (BlockId b : body_.layout)
	{
		const bool reachable = cfg_.reachable(b);
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			auto read = [&](Value v, unsigned slot, BlockId at) {
				const TempId address = root(v);
				if (address == kNoId)
					return;
				if (!reachable)
					info(address).held = true;
				else
					note(address, inst, slot, at);
			};
			for (unsigned k = 0; k < 3; ++k)
				read(inst.ops[k], k, b);
			for (uint32_t k = 0; k < inst.extra_count; ++k)
				if (inst.op != Opcode::Phi)
					read(body_.operands[inst.extra_first + k], 3 + k, b);
				else if (k % 2 == 1)
					read(body_.operands[inst.extra_first + k], 3 + k, body_.operands[inst.extra_first + k - 1].index);
		}
	}
	// A chosen field's address forms from its base at each of its sites:
	// they are the base's too (later addresses first: a field of a field).
	bool any = false;
	for (size_t k = infos_.size(); k-- > 0;)
	{
		if (!chosen(infos_[k]))
			continue;
		any = true;
		const InstId def = body_.temps[infos_[k].temp].def;
		const TempId base = root(body_.instructions[def].ops[0]);
		if (base == kNoId || home(base) != block_of_[def])
			continue;
		for (unsigned e = 0; e < infos_[k].count; ++e)
			outside(info(base), infos_[k].sites[e].block, infos_[k].sites[e].edge, block_of_[def]);
	}
	if (!any)
		return false;
	// The edges' blocks first (each holds its copies and a jump), then the
	// uses.
	tail_.assign(body_.blocks.size(), kNoId);
	struct Edge
	{
		BlockId from, to, block;
		InstId tail;
	};
	std::vector<Edge> edges;
	for (BlockId b : cfg_.rpo)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const TempId t = body_.instructions[i].result;
			if (t == kNoId || t >= info_of_.size() || info_of_[t] == kNoId || !chosen(infos_[info_of_[t]]))
				continue;
			Info& info = infos_[info_of_[t]];
			for (unsigned k = 0; k < info.count; ++k)
			{
				Site& site = info.sites[k];
				if (!site.edge)
					continue;
				size_t e = 0;
				while (e < edges.size() && !(edges[e].from == b && edges[e].to == site.block))
					++e;
				if (e == edges.size())
					edges.push_back(Edge{ b, site.block, kNoId, kNoId });
				const InstId id = make_copy(t, site.block, true);
				if (edges[e].block == kNoId)
					edges[e].block = append_block(body_, BlockKind::Split);
				insert_after(body_, edges[e].block, edges[e].tail, id);
				edges[e].tail = id;
				site.copy = body_.instructions[id].result;
			}
		}
	// (A copy's instruction may move the instructions: each operand is read
	// and written by its place.)
	for (BlockId b : cfg_.rpo)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			// (A copy took its operands as it was made.)
			if (i >= block_of_.size())
				continue;
			auto replaced = [&](Value v, BlockId at, bool phi) {
				const TempId address = root(v);
				if (address == kNoId || !chosen(info(address)))
					return v;
				if (phi && at == home(address))
					return copy_for(address, b, true);
				return at != home(address) ? copy_for(address, at, false) : v;
			};
			const bool phi = body_.instructions[i].op == Opcode::Phi;
			for (unsigned k = 0; k < 3; ++k)
			{
				const Value v = replaced(body_.instructions[i].ops[k], b, false);
				body_.instructions[i].ops[k] = v;
			}
			const uint32_t first = body_.instructions[i].extra_first, count = body_.instructions[i].extra_count;
			for (uint32_t k = phi ? 1 : 0; k < count; k += phi ? 2 : 1)
			{
				const BlockId at = phi ? body_.operands[first + k - 1].index : b;
				const Value v = replaced(body_.operands[first + k], at, phi);
				body_.operands[first + k] = v;
			}
		}
	for (const Edge& edge : edges)
	{
		Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
		jump.ops[0] = Value::block(edge.to);
		jump.location = body_.instructions[body_.blocks[edge.from].last].location;
		insert_after(body_, edge.block, edge.tail, append_instruction(body_, jump));
		Instruction& term = body_.instructions[body_.blocks[edge.from].last];
		for (unsigned k = 1; k < 3; ++k)
			if (term.ops[k].index == edge.to)
				term.ops[k] = Value::block(edge.block);
		rename_phi_incoming(body_, edge.to, edge.from, edge.block);
		std::vector<BlockId>& layout = body_.layout;
		size_t at = 0;
		while (at < layout.size() && layout[at] != edge.from)
			++at;
		layout.insert(layout.begin() + static_cast<std::ptrdiff_t>(at < layout.size() ? at + 1 : at), edge.block);
	}
	if (!edges.empty())
		c_.invalidate_cfg();
	c_.invalidate_block_of();
	c_.invalidate_uses();
	return true;
}

} // namespace

bool sink_addresses(FunctionContext& context)
{
	if (!context.costs.scaled_memory_operands)
		return false;
	// (Again for a copy that its own block reads memory through.)
	if (!AddressSinking(context).run())
		return false;
	AddressSinking(context).run();
	return true;
}

} // namespace opt
