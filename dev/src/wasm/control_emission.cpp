#include "wasm/selector.h"

#include "wasm/exceptions.h"

namespace wasm {

using namespace lowir;

bool Selector::branch_label(uint32_t target)
{
	for (auto label = labels_.rbegin(); label != labels_.rend(); ++label)
		if (label->target == target)
		{
			out_.emit(Op::Br, { nesting_ - 1 - label->depth });
			return true;
		}
	return false;
}

void Selector::open_label(Op op, BlockId target)
{
	out_.emit(op, { block_type(ValueType::Void) });
	labels_.push_back(Label{ target, nesting_++, op == Op::Loop });
}

void Selector::close_label()
{
	out_.emit(Op::End);
	labels_.pop_back();
	--nesting_;
}

void Selector::region_child(uint32_t child)
{
	if (child < control_.block_count)
		block(child);
	else
	{
		const uint32_t id = child - control_.block_count + 1;
		if (control_.regions[id].dispatch)
			dispatch_region(id);
		else
		{
			open_label(Op::Loop, control_.regions[id].header);
			region(id);
			close_label();
		}
	}
}

void Selector::dispatch_region(uint32_t id)
{
	const auto& children = control_.regions[id].children;
	const uint32_t count = static_cast<uint32_t>(children.size());
	dispatch_labels_.push_back(Label{ id, nesting_, false });
	open_label(Op::Loop, kNoId);
	for (uint32_t i = 0; i <= count; ++i)
		open_label(Op::Block, kNoId);
	out_.emit(Op::LocalGet, { region_locals_[id] });
	out_.emit(Op::BrTable);
	for (uint32_t i = 0; i <= count; ++i)
		out_.operands.push_back(i);
	out_.instructions.back().operands.count = count + 1;
	for (uint32_t child : children)
	{
		close_label();
		region_child(child);
	}
	close_label();
	out_.emit(Op::Unreachable);
	close_label();
	dispatch_labels_.pop_back();
	out_.emit(Op::Unreachable);
}

void Selector::region(uint32_t id)
{
	const auto& children = control_.regions[id].children;
	auto entry = [&](uint32_t child) {
		return child >= control_.block_count && control_.regions[child - control_.block_count + 1].dispatch
			? child
			: control_.entry(child);
	};
	for (uint32_t i = static_cast<uint32_t>(children.size()); i > 1; --i)
		open_label(Op::Block, entry(children[i - 1]));
	for (uint32_t i = 0; i < children.size(); ++i)
	{
		if (i != 0)
			close_label();
		region_child(children[i]);
	}
}

bool Selector::structured()
{
	const opt::Cfg* edges = exceptions_ != nullptr && exceptions_->active ? &exceptions_->control : nullptr;
	if (!structure_control(program_, body_, control_, edges))
		return false;
	if (!control_.dispatch_region.empty())
	{
		region_locals_.assign(control_.regions.size(), kNoId);
		for (uint32_t i = 0; i < control_.regions.size(); ++i)
			if (control_.regions[i].dispatch)
				region_locals_[i] = add_local(ValueType::I32);
	}
	position_.assign(body_.blocks.size(), kNoId);
	loop_first_.assign(body_.blocks.size(), kNoId);
	loop_last_.assign(body_.blocks.size(), kNoId);
	uint32_t next = 0;
	number_blocks(0, next);
	region(0);
	out_.emit(Op::Unreachable);
	return true;
}

void Selector::number_blocks(uint32_t region, uint32_t& next)
{
	for (uint32_t child : control_.regions[region].children)
		if (child < control_.block_count)
			position_[child] = next++;
		else
		{
			const uint32_t id = child - control_.block_count + 1;
			const uint32_t first = next;
			number_blocks(id, next);
			if (!control_.regions[id].dispatch && next != first)
			{
				loop_first_[control_.regions[id].header] = first;
				loop_last_[control_.regions[id].header] = next - 1;
			}
		}
}

// Marks each loop header one of whose phis is read outside its loop: by an
// instruction there, or by a phi there on an edge that leaves the loop.
void Selector::find_carried()
{
	std::vector<BlockId> home(body_.temps.size(), kNoId);
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
			i = body_.instructions[i].next)
			home[body_.instructions[i].result] = b;
	escaped_.assign(body_.blocks.size(), 0);
	const auto inside = [&](BlockId header, BlockId block) {
		return position_[block] != kNoId && position_[block] >= loop_first_[header] &&
			position_[block] <= loop_last_[header];
	};
	const auto mark = [&](Value v, BlockId from, BlockId at) {
		if (!v.is(ValueKind::Temp) || home[v.index] == kNoId)
			return;
		const BlockId header = home[v.index];
		if (loop_first_[header] == kNoId || !inside(header, from) || !inside(header, at))
			escaped_[header] = 1;
	};
	// A use may read again what a copy, a step or a conversion that made its
	// operand read (an access computes its address from the step's operands).
	const auto read = [&](Value v, BlockId from, BlockId at) {
		for (unsigned depth = 0; depth < 20 && v.is(ValueKind::Temp); ++depth)
		{
			mark(v, from, at);
			const InstId id = body_.temps[v.index].def;
			if (id == kNoId)
				break;
			const auto& def = body_.instructions[id];
			if (def.op != Opcode::Copy && def.op != Opcode::Index && def.op != Opcode::Convert)
				break;
			mark(def.ops[1], from, at);
			// (A step by a product reads the product's first operand.)
			const InstId made = def.op == Opcode::Index && def.ops[1].is(ValueKind::Temp)
				? body_.temps[def.ops[1].index].def
				: kNoId;
			if (made != kNoId && body_.instructions[made].op == Opcode::Binary)
				mark(body_.instructions[made].ops[0], from, at);
			v = def.ops[0];
		}
	};
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const auto& inst = body_.instructions[i];
			if (inst.op == Opcode::Phi)
			{
				for (uint32_t k = 0; k + 1 < inst.extra_count; k += 2)
					read(body_.operands[inst.extra_first + k + 1], body_.operands[inst.extra_first + k].index, b);
				continue;
			}
			for (const Value& operand : inst.ops)
				read(operand, b, b);
			for (uint32_t k = 0; k < inst.extra_count; ++k)
				read(body_.operands[inst.extra_first + k], b, b);
		}
}

// Whether a `br_if` may go to `to` where the branch otherwise goes to `other`:
// `to` has an open label and takes no moves, or it is the header of a loop
// that `other` leaves and whose phis are read only inside it, so their moves
// may run before the test's outcome is known.
bool Selector::branch_if_target(BlockId to, BlockId other, uint32_t& depth, bool& moves)
{
	if (exceptions_ != nullptr && exceptions_->active && exceptions_->entry_selectors[to] != 0)
		return false;
	const Label* found = nullptr;
	for (auto label = labels_.rbegin(); label != labels_.rend() && found == nullptr; ++label)
		if (label->target == to)
			found = &*label;
	if (found == nullptr)
		return false;
	depth = nesting_ - 1 - found->depth;
	const InstId first = body_.blocks[to].first;
	moves = first != kNoId && body_.instructions[first].op == Opcode::Phi;
	if (!moves)
		return true;
	if (!found->loop || loop_first_[to] == kNoId || position_[other] == kNoId ||
		(position_[other] >= loop_first_[to] && position_[other] <= loop_last_[to]))
		return false;
	if (escaped_.empty())
		find_carried();
	return escaped_[to] == 0;
}

// `branch c, T, F` as the test, the moves of a loop's back edge, `br_if` and
// the other edge, in place of an `if` around the first edge.
bool Selector::conditional_branch(const lowir::Instruction& inst)
{
	if (pc_ != kNoId || position_.empty() || inst.ops[1].index == inst.ops[2].index)
		return false;
	uint32_t depth[2] = { 0, 0 };
	bool moves[2] = { false, false }, usable[2];
	for (unsigned k = 0; k < 2; ++k)
		usable[k] = branch_if_target(inst.ops[1 + k].index, inst.ops[2 - k].index, depth[k], moves[k]);
	// A back edge first: the loop's own transfer is then the one branch.
	unsigned pick = 2;
	for (unsigned k = 0; k < 2 && pick == 2; ++k)
		if (usable[k] && loop_first_[inst.ops[1 + k].index] != kNoId && moves[k])
			pick = k;
	for (unsigned k = 0; k < 2 && pick == 2; ++k)
		if (usable[k])
			pick = k;
	if (pick == 2)
		return false;
	condition(inst.ops[0]);
	if (pick == 1)
		out_.emit(Op::I32Eqz);
	if (moves[pick])
		phi_copies(current_block_, inst.ops[1 + pick].index);
	out_.emit(Op::BrIf, { depth[pick] });
	edge(current_block_, inst.ops[2 - pick].index);
	return true;
}

} // namespace wasm
