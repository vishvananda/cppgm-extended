#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"

#include <algorithm>

namespace opt {

using namespace lowir;

namespace {

// The most instructions of a nest this pass reads.
const size_t kNestInstructions = 64;
// How deep an address's arithmetic is read.
const unsigned kStrideDepth = 8;
// A value of varying_ beyond 1: the temp of a phi of one input.
const uint8_t kRelay = 2;

// Loop interchange at O3: an inner loop that folds a sum over a stride, each
// sum stored by the outer loop, as in a matrix product:
//
//   for j: { s = start; for k: s = s + a[k] * b[k][j]; r[j] = s; }
//
// becomes a loop of first values and a nest with the loops swapped:
//
//   for j: r[j] = start;
//   for k: for j: r[j] = r[j] + a[k] * b[k][j];
//
// Each r[j] takes the same operations in the same order, so floating sums stay
// exact, and the new inner loop walks consecutive elements. The loops of the
// nest are rotated, so each runs at least one pass, and the inner loop's count
// does not depend on the outer loop's pass. What the outer loop stores is apart
// from everything the inner loop reads.
class LoopInterchange
{
public:
	explicit LoopInterchange(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();

private:
	bool read_shape(uint32_t inner);
	bool read_outer();
	bool read_exit();
	bool read_inner();
	bool varies(Value v) const;
	bool defined_in(Value v, BlockId block) const;
	bool same_value(Value a, Value b) const;
	bool step_of(const Instruction& phi, int64_t& out) const;
	bool stride(Value v, BlockId loop, int64_t& out, unsigned depth) const;
	bool pays();
	Value copy_of(Value v) const;
	BlockId add_first_values();
	void set_block(BlockId block, const std::vector<InstId>& list);
	void rewrite(BlockId preheader);

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	LoopForest forest_;

	// The nest: outer_ runs inner_, whose exit is exit_; exit_ goes round or to
	// after_.
	BlockId outer_ = kNoId, inner_ = kNoId, exit_ = kNoId, after_ = kNoId;
	std::vector<InstId> outer_phis_, outer_code_, inner_phis_, inner_code_, exit_code_;
	std::vector<InstId> relays_; // the exit's phis of one input
	InstId first_store_ = kNoId, last_store_ = kNoId, sum_ = kNoId;
	Value start_ = Value::none(), cell_ = Value::none();
	std::vector<uint8_t> varying_; // per temp: changes with the outer pass, or read from memory
	std::vector<Value> copies_;    // per temp: its value in the loop of first values
};

bool LoopInterchange::varies(Value v) const
{
	return v.is(ValueKind::Temp) && varying_[v.index] != 0;
}

bool LoopInterchange::defined_in(Value v, BlockId block) const
{
	if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
		return false;
	const InstId def = body_.temps[v.index].def;
	return def != kNoId && c_.block_of[def] == block;
}

// Equal values: literals by what they hold, since each mention has its own.
bool LoopInterchange::same_value(Value a, Value b) const
{
	if (a.kind != b.kind)
		return false;
	if (a.is(ValueKind::Integer) || a.is(ValueKind::Float))
	{
		const Literal& x = body_.literals[a.index];
		const Literal& y = body_.literals[b.index];
		return x.low == y.low && x.high == y.high && a.float_kind == b.float_kind;
	}
	return a == b;
}

bool LoopInterchange::read_shape(uint32_t inner)
{
	const Loop& k = forest_.loops[inner];
	if (k.blocks.size() != 1 || k.parent == kNoId)
		return false;
	const Loop& j = forest_.loops[k.parent];
	if (j.blocks.size() != 3 || j.header == k.header)
		return false;
	outer_ = j.header;
	inner_ = k.header;
	exit_ = kNoId;
	for (BlockId b : j.blocks)
		if (b != outer_ && b != inner_)
			exit_ = b;
	const Instruction& enter = body_.instructions[body_.blocks[outer_].last];
	const Instruction& repeat = body_.instructions[body_.blocks[inner_].last];
	const Instruction& back = body_.instructions[body_.blocks[exit_].last];
	if (enter.op != Opcode::Jump || enter.ops[0].index != inner_ || repeat.op != Opcode::Branch ||
		back.op != Opcode::Branch)
		return false;
	if (!((repeat.ops[1].index == inner_ && repeat.ops[2].index == exit_) ||
			(repeat.ops[2].index == inner_ && repeat.ops[1].index == exit_)))
		return false;
	if (back.ops[1].index != outer_ && back.ops[2].index != outer_)
		return false;
	after_ = back.ops[1].index == outer_ ? back.ops[2].index : back.ops[1].index;
	return after_ != outer_ && after_ != exit_ && after_ != inner_;
}

// The outer header: counters, and pure code with at most one store.
bool LoopInterchange::read_outer()
{
	outer_phis_.clear();
	outer_code_.clear();
	first_store_ = kNoId;
	const InstId last = body_.blocks[outer_].last;
	for (InstId i = body_.blocks[outer_].first; i != last; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		if (inst.op == Opcode::Phi)
		{
			if (inst.extra_count != 4)
				return false;
			outer_phis_.push_back(i);
			varying_[inst.result] = 1;
		}
		else if (inst.op == Opcode::Store && first_store_ == kNoId && (inst.flags & IF_VOLATILE) == 0)
			first_store_ = i;
		else if (is_pure(inst) && inst.extra_count == 0 && inst.op != Opcode::Load)
		{
			outer_code_.push_back(i);
			varying_[inst.result] = 1;
		}
		else
			return false;
	}
	return !outer_phis_.empty();
}

// The block after the inner loop: the store of the sum, and the outer loop's
// own stepping, which reads nothing of the inner loop.
bool LoopInterchange::read_exit()
{
	exit_code_.clear();
	relays_.clear();
	last_store_ = kNoId;
	const InstId last = body_.blocks[exit_].last;
	for (InstId i = body_.blocks[exit_].first; i != last; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		if (inst.op == Opcode::Store && last_store_ == kNoId && (inst.flags & IF_VOLATILE) == 0)
		{
			last_store_ = i;
			continue;
		}
		if (inst.op == Opcode::Phi && inst.extra_count == 2)
		{
			relays_.push_back(i);
			varying_[inst.result] = kRelay;
			continue;
		}
		if (!is_pure(inst) || inst.op == Opcode::Phi || inst.op == Opcode::Load || inst.extra_count != 0)
			return false;
		bool clean = true;
		for_each_operand(body_, inst, [&](const Value& v) {
			clean = clean && !defined_in(v, inner_) && !(v.is(ValueKind::Temp) && varying_[v.index] == kRelay);
		});
		if (!clean)
			return false;
		exit_code_.push_back(i);
		varying_[inst.result] = 1;
	}
	if (last_store_ == kNoId || defined_in(body_.instructions[last].ops[0], inner_))
		return false;
	cell_ = body_.instructions[last_store_].ops[1];
	if (defined_in(cell_, inner_) || (cell_.is(ValueKind::Temp) && varying_[cell_.index] == kRelay))
		return false;
	for (InstId phi : outer_phis_)
		if (defined_in(phi_incoming(body_, body_.instructions[phi], exit_), inner_))
			return false;
	return true;
}

// The inner loop: one sum, the value stored, and counters that do not change
// with the outer pass, as its branch does not.
bool LoopInterchange::read_inner()
{
	inner_phis_.clear();
	inner_code_.clear();
	sum_ = kNoId;
	Value stored = body_.instructions[last_store_].ops[0];
	if (stored.is(ValueKind::Temp) && varying_[stored.index] == kRelay)
		stored = body_.operands[body_.instructions[body_.temps[stored.index].def].extra_first + 1];
	InstId i = body_.blocks[inner_].first;
	for (; body_.instructions[i].op == Opcode::Phi; i = body_.instructions[i].next)
	{
		const Instruction& phi = body_.instructions[i];
		if (phi.extra_count != 4 || phi.type.is_object())
			return false;
		if (phi_incoming(body_, phi, inner_) == stored)
		{
			if (sum_ != kNoId)
				return false;
			sum_ = i;
			start_ = phi_incoming(body_, phi, outer_);
			varying_[phi.result] = 1;
		}
		else
			inner_phis_.push_back(i);
	}
	if (sum_ == kNoId || varies(start_))
		return false;
	const InstId last = body_.blocks[inner_].last;
	for (; i != last; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		if (inner_code_.size() == kNestInstructions)
			return false;
		bool moves = inst.op == Opcode::Load;
		if (moves && (inst.flags & IF_VOLATILE) != 0)
			return false;
		if (!moves && (!is_pure(inst) || inst.extra_count != 0))
			return false;
		for_each_operand(body_, inst, [&](const Value& v) { moves = moves || varies(v); });
		varying_[inst.result] = moves;
		inner_code_.push_back(i);
	}
	if (varies(body_.instructions[last].ops[0]))
		return false;
	for (InstId phi : inner_phis_)
	{
		const Instruction& inst = body_.instructions[phi];
		if (varies(phi_incoming(body_, inst, outer_)) || varies(phi_incoming(body_, inst, inner_)))
			return false;
	}
	return true;
}

// A counter's step per pass, from its latch value.
bool LoopInterchange::step_of(const Instruction& phi, int64_t& out) const
{
	const BlockId loop = c_.block_of[body_.temps[phi.result].def];
	const BlockId latch = loop == outer_ ? exit_ : inner_;
	Value next = phi_incoming(body_, phi, latch);
	if (!next.is(ValueKind::Temp) || next.index < body_.parameter_count)
		return false;
	const Instruction* step = &body_.instructions[body_.temps[next.index].def];
	if (step->op == Opcode::Convert && static_cast<ConvertOp>(step->sub) == ConvertOp::SExt &&
		step->ops[0].is(ValueKind::Temp))
		step = &body_.instructions[body_.temps[step->ops[0].index].def];
	if ((step->op != Opcode::Binary && step->op != Opcode::Index) || !step->ops[1].is(ValueKind::Integer))
		return false;
	const int64_t by = static_cast<int64_t>(body_.literals[step->ops[1].index].low);
	if (step->op == Opcode::Index)
	{
		out = by * static_cast<int64_t>(storage_bytes(step->type));
		return true;
	}
	const BinaryOp op = static_cast<BinaryOp>(step->sub);
	if (op != BinaryOp::Add && op != BinaryOp::Sub)
		return false;
	out = op == BinaryOp::Add ? by : -by;
	return true;
}

// What `v` adds on each pass of the loop headed by `loop`.
bool LoopInterchange::stride(Value v, BlockId loop, int64_t& out, unsigned depth) const
{
	out = 0;
	if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
		return true;
	const InstId def = body_.temps[v.index].def;
	if (def == kNoId || !in_loop(forest_, forest_.innermost[outer_], c_.block_of[def]))
		return true;
	if (depth == 0)
		return false;
	const Instruction& inst = body_.instructions[def];
	int64_t a = 0, b = 0;
	switch (inst.op)
	{
	case Opcode::Phi:
		return c_.block_of[def] != loop || step_of(inst, out);
	case Opcode::Copy:
	case Opcode::Convert:
		return (inst.op == Opcode::Copy || static_cast<ConvertOp>(inst.sub) == ConvertOp::SExt) &&
			stride(inst.ops[0], loop, out, depth - 1);
	case Opcode::Index:
		if (!stride(inst.ops[0], loop, a, depth - 1) || !stride(inst.ops[1], loop, b, depth - 1))
			return false;
		out = a + b * static_cast<int64_t>(storage_bytes(inst.type));
		return true;
	case Opcode::Binary:
	{
		const BinaryOp op = static_cast<BinaryOp>(inst.sub);
		if (!stride(inst.ops[0], loop, a, depth - 1) || !stride(inst.ops[1], loop, b, depth - 1))
			return false;
		if (op == BinaryOp::Add || op == BinaryOp::Sub)
			out = op == BinaryOp::Add ? a + b : a - b;
		else if (op == BinaryOp::Mul && inst.ops[1].is(ValueKind::Integer))
			out = a * static_cast<int64_t>(body_.literals[inst.ops[1].index].low);
		else
			return a == 0 && b == 0;
		return true;
	}
	default:
		return false;
	}
}

// Whether the sums' cells and everything read are apart, each read keeps to
// consecutive elements or one place as the outer loop steps, and the inner
// loop walks some array by more than one element.
bool LoopInterchange::pays()
{
	const Located cell = locate(body_, cell_);
	if (!cell.root.is(ValueKind::Symbol) && !cell.root.is(ValueKind::Slot))
		return false;
	const Instruction& store = body_.instructions[last_store_];
	int64_t step = 0;
	if (!stride(cell_, outer_, step, kStrideDepth) || step != static_cast<int64_t>(storage_bytes(store.type)))
		return false;
	bool blocked = false;
	for (InstId id : inner_code_)
	{
		const Instruction& inst = body_.instructions[id];
		if (inst.op != Opcode::Load)
			continue;
		const Located from = locate(body_, inst.ops[0]);
		if ((!from.root.is(ValueKind::Symbol) && !from.root.is(ValueKind::Slot)) || from.root == cell.root)
			return false;
		const int64_t bytes = static_cast<int64_t>(storage_bytes(inst.type));
		int64_t along = 0, across = 0;
		if (!stride(inst.ops[0], outer_, across, kStrideDepth) || !stride(inst.ops[0], inner_, along, kStrideDepth) ||
			(across != 0 && across != bytes))
			return false;
		blocked = blocked || (along != 0 && along != bytes && along != -bytes);
	}
	if (!blocked)
		return false;
	if (first_store_ == kNoId)
		return true;
	const Instruction& first = body_.instructions[first_store_];
	return first.ops[1] == cell_ && first.type.kind() == store.type.kind() && same_value(first.ops[0], start_);
}

Value LoopInterchange::copy_of(Value v) const
{
	return v.is(ValueKind::Temp) && !copies_[v.index].is(ValueKind::None) ? copies_[v.index] : v;
}

// The loop of first values: the outer loop's counters and addresses again, and
// the store of the sum's start.
BlockId LoopInterchange::add_first_values()
{
	const BlockId block = append_block(body_, BlockKind::ForBody);
	copies_.assign(body_.temps.size(), Value::none());
	std::vector<InstId> list;
	auto clone = [&](InstId id) {
		Instruction copy = body_.instructions[id];
		copy.next = kNoId;
		const InstId made = append_instruction(body_, copy);
		list.push_back(made);
		return made;
	};
	std::vector<InstId> phis;
	for (InstId id : outer_phis_)
	{
		const Instruction phi = body_.instructions[id];
		const InstId made = clone(id);
		// (The list grows as it is read.)
		const Value edges[4] = { body_.operands[phi.extra_first], body_.operands[phi.extra_first + 1],
			body_.operands[phi.extra_first + 2], body_.operands[phi.extra_first + 3] };
		const uint32_t first = append_operands(body_, edges, 4);
		body_.instructions[made].extra_first = first;
		const TempId temp = append_temp(body_, phi.type, made);
		body_.instructions[made].result = temp;
		copies_[phi.result] = Value::temp(temp);
		phis.push_back(made);
	}
	auto copy_code = [&](InstId id) {
		const InstId made = clone(id);
		Instruction& inst = body_.instructions[made];
		for (Value& v : inst.ops)
			v = copy_of(v);
		if (inst.result != kNoId)
		{
			const TempId was = inst.result;
			const TempId temp = append_temp(body_, body_.temps[was].type, made);
			body_.instructions[made].result = temp;
			copies_[was] = Value::temp(temp);
		}
		return made;
	};
	for (InstId id : outer_code_)
		copy_code(id);
	for (InstId id : exit_code_)
		copy_code(id);
	const InstId write = clone(last_store_);
	body_.instructions[write].ops[0] = start_;
	body_.instructions[write].ops[1] = copy_of(cell_);
	const Instruction back = body_.instructions[body_.blocks[exit_].last];
	const InstId branch = clone(body_.blocks[exit_].last);
	Instruction& out = body_.instructions[branch];
	out.ops[0] = copy_of(back.ops[0]);
	const uint32_t stay = back.ops[1].index == outer_ ? 1 : 2;
	out.ops[stay] = Value::block(block);
	out.ops[3 - stay] = Value::block(inner_);
	for (InstId id : phis)
	{
		const Instruction& phi = body_.instructions[id];
		for (uint32_t k = 0; k < 4; k += 2)
			if (body_.operands[phi.extra_first + k].index == exit_)
			{
				body_.operands[phi.extra_first + k] = Value::block(block);
				body_.operands[phi.extra_first + k + 1] = copy_of(body_.operands[phi.extra_first + k + 1]);
			}
	}
	set_block(block, list);
	return block;
}

void LoopInterchange::set_block(BlockId block, const std::vector<InstId>& list)
{
	body_.blocks[block].first = list.front();
	body_.blocks[block].last = list.back();
	for (size_t k = 0; k < list.size(); ++k)
		body_.instructions[list[k]].next = k + 1 < list.size() ? list[k + 1] : kNoId;
}

void LoopInterchange::rewrite(BlockId preheader)
{
	const BlockId first = add_first_values();
	const InstId enter = body_.blocks[outer_].last, repeat = body_.blocks[inner_].last, back = body_.blocks[exit_].last;

	body_.instructions[body_.blocks[preheader].last].ops[0] = Value::block(first);
	rename_phi_incoming(body_, outer_, preheader, inner_);
	rename_phi_incoming(body_, outer_, exit_, outer_);
	rename_phi_incoming(body_, inner_, outer_, first);
	rename_phi_incoming(body_, inner_, inner_, exit_);

	// The sum's phi becomes the load of its cell, at the head of the code.
	Instruction load = make_instruction(Opcode::Load, body_.instructions[sum_].type);
	load.result = body_.instructions[sum_].result;
	load.ops[0] = cell_;
	load.flags = body_.instructions[last_store_].flags;
	body_.instructions[sum_] = load;

	std::vector<InstId> joined;
	joined.insert(joined.end(), outer_phis_.begin(), outer_phis_.end());
	joined.insert(joined.end(), outer_code_.begin(), outer_code_.end());
	joined.insert(joined.end(), exit_code_.begin(), exit_code_.end());
	joined.push_back(sum_);
	joined.insert(joined.end(), inner_code_.begin(), inner_code_.end());
	for (InstId id : relays_)
	{
		Instruction copy = make_instruction(Opcode::Copy, body_.instructions[id].type);
		copy.result = body_.instructions[id].result;
		copy.ops[0] = body_.operands[body_.instructions[id].extra_first + 1];
		body_.instructions[id] = copy;
		joined.push_back(id);
	}
	joined.push_back(last_store_);
	Instruction& turn = body_.instructions[back];
	const uint32_t leave = turn.ops[1].index == outer_ ? 2 : 1;
	turn.ops[leave] = Value::block(exit_);
	joined.push_back(back);
	set_block(outer_, joined);

	std::vector<InstId> head = inner_phis_;
	body_.instructions[enter].ops[0] = Value::block(outer_);
	head.push_back(enter);
	set_block(inner_, head);

	Instruction& again = body_.instructions[repeat];
	const uint32_t done = again.ops[1].index == inner_ ? 2 : 1;
	again.ops[done] = Value::block(after_);
	set_block(exit_, std::vector<InstId>(1, repeat));

	// (Definitions stay before their uses in the layout.)
	body_.layout.erase(std::find(body_.layout.begin(), body_.layout.end(), inner_));
	const auto where = std::find(body_.layout.begin(), body_.layout.end(), outer_);
	const BlockId placed[2] = { first, inner_ };
	body_.layout.insert(where, placed, placed + 2);
	++c_.stats.loops_interchanged;
}

bool LoopInterchange::run()
{
	c_.ensure_cfg();
	find_loops(body_, cfg_, forest_);
	if (forest_.loops.size() < 2)
		return false;
	c_.entry_block_of(interchange_loops);
	bool changed = false;
	std::vector<uint8_t> tried(body_.blocks.size(), 0);
	for (unsigned round = 0; round < 16; ++round)
	{
		bool one = false;
		for (uint32_t inner = 0; inner < forest_.loops.size() && !one; ++inner)
		{
			if (tried[forest_.loops[inner].header] != 0 || !read_shape(inner))
				continue;
			tried[inner_] = 1;
			varying_.assign(body_.temps.size(), 0);
			if (!read_outer() || !read_exit() || !read_inner() || !pays())
				continue;
			uint32_t parent = forest_.loops[inner].parent;
			if (forest_.loops[parent].preheader == kNoId)
			{
				one = make_loop_preheader(body_, cfg_, forest_, parent);
				tried[inner_] = 0;
				continue;
			}
			rewrite(forest_.loops[parent].preheader);
			one = true;
		}
		if (!one)
			break;
		changed = true;
		c_.invalidate_cfg();
		c_.ensure_cfg();
		find_loops(body_, cfg_, forest_);
		map_instruction_blocks(body_, c_.block_of);
		c_.invalidate_block_of();
		tried.resize(body_.blocks.size(), 0);
	}
	return changed;
}

} // namespace

bool interchange_loops(FunctionContext& context)
{
	LoopInterchange pass(context);
	return pass.run();
}

} // namespace opt
