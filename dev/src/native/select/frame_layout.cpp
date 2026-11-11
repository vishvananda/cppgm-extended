#include <algorithm>
#include <utility>
#include <vector>

#include "native/mir/type_kinds.h"
#include "native/select/block_placement.h"
#include "native/select/frame_layout.h"

namespace native {

namespace {

bool frame_operand(const Operand& op)
{
	return op.kind == OperandKind::Mem && op.symbol == lowir::kNoId && (op.reg == kFrame || op.index == kFrame) &&
		op.disp <= 0;
}

// The bytes an instruction's memory operand covers (at least its first).
uint32_t access_bytes(const Inst& inst)
{
	if ((inst.op == Op::CopyBytes || inst.op == Op::ZeroBytes) && !(inst.flags & MF_COPY_DYNAMIC))
		return inst.aux;
	if (inst.op == Op::Lea || inst.type == lowir::TypeKind::Void)
		return 1;
	// An x87 extended value is ten bytes in memory (fld/fstp tbyte), not a
	// register's eight: a store into its last two bytes is read by it.
	if (inst.type == lowir::TypeKind::F80)
		return 10;
	// An i128 floating move transfers a whole XMM register (SSE/SSEUP),
	// including the upper eight bytes of a vector or binary128 value.
	if (inst.op == Op::FMov && (inst.type == lowir::TypeKind::I128 || inst.type == lowir::TypeKind::V128))
		return 16;
	return register_bytes(inst.type);
}

uint32_t align_up(uint32_t value, uint32_t align)
{
	return (value + align - 1) & ~(align - 1);
}

// A block's successors (branch labels, jump table entries).
void successors(const Function& f, uint32_t b, std::vector<uint32_t>& out)
{
	out.clear();
	const Block& block = f.blocks[b];
	for (uint32_t i = block.first; i < block.first + block.count; ++i)
	{
		const Inst& inst = f.insts[i];
		if ((inst.op == Op::Jcc || inst.op == Op::Jmp) && inst.ops[0].kind == OperandKind::Label)
			out.push_back(inst.ops[0].symbol);
		else if (inst.op == Op::WideCmpBranch)
		{
			out.push_back(inst.ops[2].symbol);
			out.push_back(inst.aux);
		}
		else if (inst.op == Op::JumpTable)
			for (uint32_t k = inst.aux; k < inst.aux + inst.aux2; ++k)
				out.push_back(f.jump_targets[k]);
	}
}

} // namespace

FrameObjectIndex::FrameObjectIndex(const std::vector<FrameObject>& objects)
	: objects_(objects), by_offset_(objects.size())
{
	for (uint32_t k = 0; k < by_offset_.size(); ++k)
		by_offset_[k] = k;
	std::sort(by_offset_.begin(), by_offset_.end(),
		[&](uint32_t x, uint32_t y) { return objects_[x].offset < objects_[y].offset; });
}

uint32_t FrameObjectIndex::find(int32_t disp) const
{
	uint32_t lo = 0, hi = static_cast<uint32_t>(by_offset_.size());
	while (lo < hi)
	{
		const uint32_t mid = (lo + hi) / 2;
		if (objects_[by_offset_[mid]].offset <= disp)
			lo = mid + 1;
		else
			hi = mid;
	}
	if (lo == 0)
		return lowir::kNoId;
	const FrameObject& o = objects_[by_offset_[lo - 1]];
	return static_cast<int64_t>(disp) < static_cast<int64_t>(o.offset) + o.bytes ? by_offset_[lo - 1] : lowir::kNoId;
}

std::vector<uint8_t> tracked_frame_objects(const Function& f, const FrameObjectIndex& index)
{
	const std::vector<FrameObject>& objects = index.objects();
	std::vector<uint8_t> tracked(objects.size(), 1);
	auto untrack = [&](int32_t disp) {
		const uint32_t o = index.find(disp);
		if (o != lowir::kNoId)
			tracked[o] = 0;
	};
	if (f.scratch_bytes != 0)
		untrack(f.scratch_offset);
	if (f.eh.slot < 0)
		untrack(f.eh.slot);
	for (const Inst& inst : f.insts)
		for (unsigned k = 0; k < inst.nops && inst.op != Op::Nop; ++k)
		{
			const Operand& op = inst.ops[k];
			if (!frame_operand(op))
				continue;
			const bool direct = op.index == kNoReg && op.reg == kFrame;
			bool plain = false;
			switch (inst.op)
			{
			case Op::Load:
			case Op::Store:
			case Op::Cmp:
			case Op::Test:
			case Op::FMov:
				plain = true;
				break;
			case Op::CopyBytes:
			case Op::ZeroBytes:
				plain = !(inst.flags & MF_COPY_DYNAMIC);
				break;
			default:
				break;
			}
			const uint32_t o = index.find(op.disp);
			if (o == lowir::kNoId || !direct || !plain ||
				static_cast<int64_t>(op.disp) + access_bytes(inst) >
					static_cast<int64_t>(objects[o].offset) + objects[o].bytes)
			{
				untrack(op.disp);
				// An address at a boundary can denote either the next object's
				// start or the previous object's one-past pointer. Both objects
				// escape, including when the frame's last object ends at zero.
				if ((!direct || inst.op == Op::Lea) && op.disp > INT32_MIN)
					untrack(op.disp - 1);
			}
		}
	return tracked;
}

uint32_t drop_dead_frame_stores(Function& f, const std::vector<FrameObject>& objects)
{
	if (objects.empty() || f.stack_size == 0 || f.variadic || f.eh_records_pushed)
		return 0;
	const uint32_t n = static_cast<uint32_t>(objects.size());
	const FrameObjectIndex index(objects);
	auto find = [&](int32_t disp) { return index.find(disp); };
	// Tracked objects: every access a direct load, store, compare, test or
	// bulk copy of known bytes (no address taken, no index, no implicit
	// use by the emitter).
	const std::vector<uint8_t> tracked = tracked_frame_objects(f, index);
	// Byte indexes of the tracked objects.
	std::vector<uint32_t> base(n, 0);
	uint32_t bytes = 0;
	for (uint32_t k = 0; k < n; ++k)
		if (tracked[k])
		{
			base[k] = bytes;
			bytes += objects[k].bytes;
		}
	if (bytes == 0)
		return 0;
	const uint32_t words = (bytes + 63) / 64;
	const uint32_t blocks = static_cast<uint32_t>(f.blocks.size());
	// Bound scratch and convergence work before considering any rewrite.
	if (static_cast<uint64_t>(blocks) * words > (1u << 20))
		return 0;
	for (const Inst& inst : f.insts)
		if (inst.eh != 0 && (inst.eh > f.eh.landings.size() || f.eh.landings[inst.eh - 1].pad >= blocks))
			return 0;
	std::vector<std::vector<uint64_t>> live_in(blocks, std::vector<uint64_t>(words, 0));
	auto range = [&](const Operand& op, uint32_t count, uint32_t& first, uint32_t& past) {
		if (!frame_operand(op))
			return false;
		const uint32_t o = find(op.disp);
		if (o == lowir::kNoId || !tracked[o])
			return false;
		first = base[o] + static_cast<uint32_t>(op.disp - objects[o].offset);
		past = first + count;
		return true;
	};
	auto any = [&](const std::vector<uint64_t>& live, uint32_t first, uint32_t past) {
		for (uint32_t i = first; i < past; ++i)
			if ((live[i / 64] >> (i % 64)) & 1u)
				return true;
		return false;
	};
	auto set = [&](std::vector<uint64_t>& live, uint32_t first, uint32_t past, bool on) {
		for (uint32_t i = first; i < past; ++i)
		{
			if (on)
				live[i / 64] |= 1ull << (i % 64);
			else
				live[i / 64] &= ~(1ull << (i % 64));
		}
	};
	// One instruction backward: a write kills its bytes (or, none of them
	// live, is dead), a read makes its bytes live.
	auto step = [&](Inst& inst, std::vector<uint64_t>& live, bool rewrite, uint32_t& dropped) {
		if (inst.op == Op::Nop)
			return;
		// A throw leaves at this instruction, before any later store in
		// the block. Merge its pad's live bytes here, not at block exit:
		// a later overwrite on the normal path must not kill them.
		if (inst.eh != 0)
		{
			const uint32_t pad = f.eh.landings[inst.eh - 1].pad;
			for (uint32_t w = 0; w < words; ++w)
				live[w] |= live_in[pad][w];
		}
		uint32_t first = 0, past = 0;
		const bool writes = inst.nops > 0 && frame_operand(inst.ops[0]) &&
			(inst.op == Op::Store || inst.op == Op::CopyBytes || inst.op == Op::ZeroBytes || inst.op == Op::FMov);
		if (writes && range(inst.ops[0], access_bytes(inst), first, past))
		{
			if (!(inst.flags & MF_VOLATILE) && !any(live, first, past))
			{
				if (rewrite)
				{
					inst.op = Op::Nop;
					inst.nops = 0;
					++dropped;
				}
				return;
			}
			set(live, first, past, false);
		}
		const bool stores = inst.nops > 0 && inst.ops[0].kind == OperandKind::Mem &&
			(inst.op == Op::Store || inst.op == Op::CopyBytes || inst.op == Op::ZeroBytes || inst.op == Op::FMov);
		for (unsigned k = stores ? 1 : 0; k < inst.nops; ++k)
			if (frame_operand(inst.ops[k]) && range(inst.ops[k], access_bytes(inst), first, past))
				set(live, first, past, true);
	};
	std::vector<uint32_t> succ;
	uint32_t dropped = 0;
	uint64_t work = 0;
	const uint64_t budget = 64ull * (f.insts.size() + blocks + bytes);
	for (bool changed = true; changed;)
	{
		changed = false;
		for (uint32_t b = blocks; b-- > 0;)
		{
			std::vector<uint64_t> live(words, 0);
			successors(f, b, succ);
			work += static_cast<uint64_t>(words) * (3 + succ.size()) + f.blocks[b].count;
			if (work > budget)
				return 0;
			for (uint32_t s : succ)
				if (s < blocks)
					for (uint32_t w = 0; w < words; ++w)
						live[w] |= live_in[s][w];
			const Block& block = f.blocks[b];
			for (uint32_t i = block.first + block.count; i-- > block.first;)
			{
				work += f.insts[i].eh != 0 ? words : 0;
				step(f.insts[i], live, false, dropped);
			}
			if (live != live_in[b])
			{
				live_in[b] = live;
				changed = true;
			}
		}
	}
	for (uint32_t b = 0; b < blocks; ++b)
	{
		std::vector<uint64_t> live(words, 0);
		successors(f, b, succ);
		for (uint32_t s : succ)
			if (s < blocks)
				for (uint32_t w = 0; w < words; ++w)
					live[w] |= live_in[s][w];
		const Block& block = f.blocks[b];
		for (uint32_t i = block.first + block.count; i-- > block.first;)
			step(f.insts[i], live, true, dropped);
	}
	return dropped;
}

bool order_frame(Function& f, const std::vector<FrameObject>& objects)
{
	if (objects.size() < 2 || f.stack_size == 0 || f.variadic || f.eh_records_pushed)
		return false;
	// The objects by offset, each access's object found by binary search.
	const uint32_t n = static_cast<uint32_t>(objects.size());
	std::vector<uint32_t> by_offset(n), rank(n);
	for (uint32_t k = 0; k < n; ++k)
		by_offset[k] = k;
	std::sort(by_offset.begin(), by_offset.end(),
		[&](uint32_t x, uint32_t y) { return objects[x].offset < objects[y].offset; });
	for (uint32_t r = 0; r < n; ++r)
		rank[by_offset[r]] = r;
	auto find = [&](int32_t disp) -> uint32_t {
		uint32_t lo = 0, hi = n;
		while (lo < hi)
		{
			const uint32_t mid = (lo + hi) / 2;
			if (objects[by_offset[mid]].offset <= disp)
				lo = mid + 1;
			else
				hi = mid;
		}
		if (lo == 0)
			return lowir::kNoId;
		const FrameObject& o = objects[by_offset[lo - 1]];
		return static_cast<int64_t>(disp) < static_cast<int64_t>(o.offset) + o.bytes ? by_offset[lo - 1] : lowir::kNoId;
	};
	auto end_of = [&](uint32_t o) { return static_cast<int64_t>(objects[o].offset) + objects[o].bytes; };
	std::vector<uint64_t> frequency;
	block_frequencies(f, frequency);
	std::vector<uint64_t> weight(n, 0);
	// Per rank: the object stays directly below the next one.
	std::vector<uint8_t> glued(n, 0);
	for (uint32_t b = 0; b < f.blocks.size(); ++b)
	{
		const Block& block = f.blocks[b];
		const uint64_t w = frequency.empty() ? 1 : frequency[b];
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
		{
			const Inst& inst = f.insts[i];
			if (inst.op == Op::VaStart || inst.op == Op::EhPush)
				return false;
			for (unsigned k = 0; k < inst.nops; ++k)
			{
				const Operand& op = inst.ops[k];
				if (!frame_operand(op))
					continue;
				// An address is its object's: the selector keeps the layout
				// of a frame it forms an address outside an object in
				// (lazy_operand), so none here is a neighbor's one-past
				// pointer.
				const uint32_t o = find(op.disp);
				if (o == lowir::kNoId)
					return false;
				// An access running past its object (a merged store) keeps
				// the objects it covers as they are.
				for (uint32_t r = rank[o];
					op.index == kNoReg && static_cast<int64_t>(op.disp) + access_bytes(inst) > end_of(by_offset[r]);
					++r)
				{
					if (r + 1 == n || objects[by_offset[r + 1]].offset != end_of(by_offset[r]))
						return false;
					glued[r] = 1;
				}
				weight[o] += w;
			}
		}
	}
	// The units that move: each run of objects that stay adjacent, laid
	// out inside as it is.
	struct Unit
	{
		uint32_t low, high; // ranks
		uint32_t first;     // its earliest object in the selector's order
		uint32_t bytes, align;
		uint64_t weight;
	};
	std::vector<Unit> units;
	for (uint32_t r = 0; r < n; ++r)
	{
		if (r == 0 || !glued[r - 1])
			units.push_back(Unit{ r, r, by_offset[r], 0, 1, 0 });
		Unit& unit = units.back();
		unit.high = r;
		unit.first = std::min(unit.first, by_offset[r]);
		unit.bytes = static_cast<uint32_t>(end_of(by_offset[r]) - objects[by_offset[unit.low]].offset);
		unit.align = std::max(unit.align, std::max<uint32_t>(objects[by_offset[r]].align, 1));
		unit.weight += weight[by_offset[r]];
	}
	// Allocation order from the frame's top by accesses per byte (the
	// one-byte window holds the most accesses): densest first below RBP;
	// without a frame pointer sparsest first, so the densest end at the
	// bottom, nearest RSP. Ties keep the selector's order.
	std::sort(units.begin(), units.end(), [](const Unit& x, const Unit& y) { return x.first < y.first; });
	const bool bottom = f.omit_frame_pointer;
	std::stable_sort(units.begin(), units.end(), [&](const Unit& x, const Unit& y) {
		const unsigned __int128 dx = static_cast<unsigned __int128>(x.weight) * y.bytes;
		const unsigned __int128 dy = static_cast<unsigned __int128>(y.weight) * x.bytes;
		return bottom ? dx < dy : dx > dy;
	});
	std::vector<int32_t> moved(n);
	uint32_t size = 0;
	bool changed = false;
	for (const Unit& unit : units)
	{
		// (Its lowest byte keeps its alignment phase: so does each object.)
		const int32_t base = objects[by_offset[unit.low]].offset;
		const uint32_t phase = static_cast<uint32_t>(-static_cast<int64_t>(base)) & (unit.align - 1);
		size += unit.bytes;
		size += (phase - size) & (unit.align - 1);
		for (uint32_t r = unit.low; r <= unit.high; ++r)
		{
			const uint32_t k = by_offset[r];
			moved[k] = objects[k].offset - base - static_cast<int32_t>(size);
			changed = changed || moved[k] != objects[k].offset;
		}
	}
	if (!changed)
		return false;
	auto relocate = [&](int32_t& disp) {
		const uint32_t o = find(disp);
		if (o != lowir::kNoId)
			disp += moved[o] - objects[o].offset;
	};
	for (Inst& inst : f.insts)
		for (unsigned k = 0; k < inst.nops; ++k)
			if (frame_operand(inst.ops[k]))
				relocate(inst.ops[k].disp);
	for (FrameBinding& binding : f.bindings)
		if (binding.offset < 0)
			relocate(binding.offset);
	for (DebugSlot& binding : f.debug_slots)
		if (binding.offset < 0)
			relocate(binding.offset);
	if (f.scratch_bytes != 0)
		relocate(f.scratch_offset);
	if (f.eh.slot < 0)
		relocate(f.eh.slot);
	// The preserve area stays below the objects (finish_frame's rule).
	const uint32_t saved = static_cast<uint32_t>(__builtin_popcount(f.preserve_mask));
	if (saved != 0)
	{
		size = align_up(size + 8 * saved, 16);
		f.preserve_offset = -static_cast<int32_t>(size);
	}
	f.stack_size = align_up(size, 16);
	return true;
}

} // namespace native
