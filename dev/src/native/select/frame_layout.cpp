#include <algorithm>
#include <vector>

#include "native/mir/type_kinds.h"
#include "native/select/block_placement.h"
#include "native/select/frame_layout.h"

namespace native {

namespace {

bool frame_operand(const Operand& op)
{
	return op.kind == OperandKind::Mem && op.symbol == lowir::kNoId && (op.reg == kFrame || op.index == kFrame) &&
		op.disp < 0;
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
				untrack(op.disp);
		}
	return tracked;
}

uint32_t drop_dead_frame_stores(Function& f, const std::vector<FrameObject>& objects)
{
	if (objects.empty() || f.stack_size == 0 || f.variadic || f.eh_records_pushed || !f.eh.landings.empty())
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
	const uint32_t blocks = static_cast<uint32_t>(f.blocks.size());
	std::vector<std::vector<uint64_t>> live_in(blocks, std::vector<uint64_t>(words, 0));
	std::vector<uint32_t> succ;
	uint32_t dropped = 0;
	for (bool changed = true; changed;)
	{
		changed = false;
		for (uint32_t b = blocks; b-- > 0;)
		{
			std::vector<uint64_t> live(words, 0);
			successors(f, b, succ);
			for (uint32_t s : succ)
				if (s < blocks)
					for (uint32_t w = 0; w < words; ++w)
						live[w] |= live_in[s][w];
			const Block& block = f.blocks[b];
			for (uint32_t i = block.first + block.count; i-- > block.first;)
				step(f.insts[i], live, false, dropped);
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
	std::vector<uint32_t> by_offset(objects.size());
	for (uint32_t k = 0; k < objects.size(); ++k)
		by_offset[k] = k;
	std::sort(by_offset.begin(), by_offset.end(),
		[&](uint32_t x, uint32_t y) { return objects[x].offset < objects[y].offset; });
	auto find = [&](int32_t disp) -> uint32_t {
		uint32_t lo = 0, hi = static_cast<uint32_t>(by_offset.size());
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
	std::vector<uint64_t> frequency;
	block_frequencies(f, frequency);
	std::vector<uint64_t> weight(objects.size(), 0);
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
				// Accesses stay inside one object (a merged store may not).
				const uint32_t o = find(op.disp);
				if (o == lowir::kNoId ||
					(op.index == kNoReg &&
						static_cast<int64_t>(op.disp) + access_bytes(inst) >
							static_cast<int64_t>(objects[o].offset) + objects[o].bytes))
					return false;
				weight[o] += w;
			}
		}
	}
	// Allocation order from the frame's top by accesses per byte (the
	// one-byte window holds the most accesses): densest first below RBP;
	// without a frame pointer sparsest first, so the densest end at the
	// bottom, nearest RSP. Ties keep the selector's order.
	std::vector<uint32_t> order(objects.size());
	for (uint32_t k = 0; k < objects.size(); ++k)
		order[k] = k;
	const bool bottom = f.omit_frame_pointer;
	std::stable_sort(order.begin(), order.end(), [&](uint32_t x, uint32_t y) {
		const unsigned __int128 dx = static_cast<unsigned __int128>(weight[x]) * objects[y].bytes;
		const unsigned __int128 dy = static_cast<unsigned __int128>(weight[y]) * objects[x].bytes;
		return bottom ? dx < dy : dx > dy;
	});
	std::vector<int32_t> moved(objects.size());
	uint32_t size = 0;
	bool changed = false;
	for (uint32_t k : order)
	{
		size = align_up(size + objects[k].bytes, std::max<uint32_t>(objects[k].align, 1));
		moved[k] = -static_cast<int32_t>(size);
		changed = changed || moved[k] != objects[k].offset;
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
