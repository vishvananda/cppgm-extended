#include "opt/passes.h"

#include <algorithm>
#include <vector>

namespace opt {

using namespace lowir;

namespace {

// The most square roots and divisions issued before their first reader.
constexpr uint32_t kInFlight = 4;
// The most instructions one square root or division brings forward.
constexpr uint32_t kSlice = 24;

bool long_latency(const Instruction& inst)
{
	switch (inst.op)
	{
	case Opcode::Unary:
		return inst.type.is_float() && static_cast<UnaryOp>(inst.sub) == UnaryOp::Sqrt;
	case Opcode::Binary:
		return inst.type.is_float() && static_cast<BinaryOp>(inst.sub) == BinaryOp::Div;
	case Opcode::Vector:
		switch (static_cast<VectorOp>(inst.sub))
		{
		case VectorOp::Sqrtpd:
		case VectorOp::Sqrtps:
		case VectorOp::Divpd:
		case VectorOp::Divps:
			return true;
		default:
			return false;
		}
	default:
		return false;
	}
}

// What may move up within its block: no effect, no trap, not a phi.
bool movable(const Instruction& inst)
{
	if (!is_pure(inst) || inst.op == Opcode::Phi)
		return false;
	if (inst.op != Opcode::Binary || inst.type.is_float())
		return true;
	const BinaryOp op = static_cast<BinaryOp>(inst.sub);
	return op != BinaryOp::Div && op != BinaryOp::Mod && op != BinaryOp::UDiv && op != BinaryOp::UMod;
}

// A square root or a division waits for its operands and then takes many
// cycles; what reads it waits behind it in the scheduler and keeps the next,
// independent one from starting. Walking each block in order, an instruction
// about to read one whose result no instruction has read yet first brings
// forward the next ones of the block with their pure operands, up to
// kInFlight unread at once, and not past a call: no floating register
// survives one.
class LongLatency
{
public:
	explicit LongLatency(FunctionContext& context) : body_(context.body) {}

	bool run();

private:
	bool here(InstId i) const { return block_[i] == epoch_; }
	bool emitted(InstId i) const { return emitted_[i] == epoch_; }
	void emit(InstId i);
	bool pull(InstId root);
	bool order(BlockId b);

	FunctionBody& body_;
	std::vector<uint32_t> block_;   // per instruction: the epoch of the block it is in
	std::vector<uint32_t> emitted_; // per instruction: placed in the new order
	std::vector<uint32_t> unread_;  // per instruction: a long one placed, not yet read
	std::vector<uint32_t> barrier_; // per position: the position of the next call
	std::vector<uint32_t> position_;
	std::vector<InstId> seq_, out_, longs_, stack_, slice_;
	uint32_t epoch_ = 0;
	uint32_t in_flight_ = 0;
};

void LongLatency::emit(InstId i)
{
	const Instruction& inst = body_.instructions[i];
	for_each_operand(body_, inst, [&](const Value& v) {
		if (v.kind != ValueKind::Temp)
			return;
		const InstId def = body_.temps[v.index].def;
		if (def != kNoId && unread_[def] == epoch_)
		{
			unread_[def] = 0;
			--in_flight_;
		}
	});
	emitted_[i] = epoch_;
	out_.push_back(i);
	if (long_latency(inst))
	{
		unread_[i] = epoch_;
		++in_flight_;
	}
}

// Places `root` and what it reads in this block that is not placed yet, in
// an order defining each before its readers; false, placing nothing, when
// one of them cannot move or there are more than kSlice.
bool LongLatency::pull(InstId root)
{
	slice_.clear();
	stack_.assign(1, root);
	while (!stack_.empty())
	{
		const InstId i = stack_.back();
		if (emitted(i) || std::find(slice_.begin(), slice_.end(), i) != slice_.end())
		{
			stack_.pop_back();
			continue;
		}
		bool ready = true, fits = true;
		for_each_operand(body_, body_.instructions[i], [&](const Value& v) {
			if (v.kind != ValueKind::Temp)
				return;
			const InstId def = body_.temps[v.index].def;
			if (def == kNoId || !here(def) || emitted(def) ||
				std::find(slice_.begin(), slice_.end(), def) != slice_.end())
				return;
			// (Pushed again when already below: it must be placed first.)
			if (!movable(body_.instructions[def]) || stack_.size() > 2 * kSlice)
				fits = false;
			else
			{
				stack_.push_back(def);
				ready = false;
			}
		});
		if (!fits)
			return false;
		if (ready)
		{
			slice_.push_back(i);
			stack_.pop_back();
			if (slice_.size() > kSlice)
				return false;
		}
	}
	for (InstId i : slice_)
		emit(i);
	return true;
}

bool LongLatency::order(BlockId b)
{
	++epoch_;
	seq_.clear();
	longs_.clear();
	for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
	{
		block_[i] = epoch_;
		position_[i] = static_cast<uint32_t>(seq_.size());
		seq_.push_back(i);
		if (long_latency(body_.instructions[i]))
			longs_.push_back(i);
	}
	if (longs_.size() < 2)
		return false;
	barrier_.resize(seq_.size() + 1);
	barrier_[seq_.size()] = static_cast<uint32_t>(seq_.size());
	for (size_t k = seq_.size(); k-- > 0;)
		barrier_[k] = body_.instructions[seq_[k]].op == Opcode::Call ? static_cast<uint32_t>(k) : barrier_[k + 1];
	out_.clear();
	in_flight_ = 0;
	size_t next = 0;
	for (size_t k = 0; k < seq_.size(); ++k)
	{
		const InstId i = seq_[k];
		if (emitted(i))
			continue;
		bool waits = false;
		for_each_operand(body_, body_.instructions[i], [&](const Value& v) {
			waits = waits ||
				(v.kind == ValueKind::Temp && body_.temps[v.index].def != kNoId &&
					unread_[body_.temps[v.index].def] == epoch_);
		});
		while (waits && in_flight_ < kInFlight && next < longs_.size())
		{
			const InstId l = longs_[next];
			if (!emitted(l) && (position_[l] >= barrier_[k] || !pull(l)))
				break;
			++next;
		}
		if (!emitted(i))
			emit(i);
		while (next < longs_.size() && emitted(longs_[next]))
			++next;
	}
	bool moved = false;
	for (size_t k = 0; k < seq_.size() && !moved; ++k)
		moved = out_[k] != seq_[k];
	if (!moved)
		return false;
	body_.blocks[b].first = out_.front();
	for (size_t k = 0; k + 1 < out_.size(); ++k)
		body_.instructions[out_[k]].next = out_[k + 1];
	body_.instructions[out_.back()].next = kNoId;
	body_.blocks[b].last = out_.back();
	return true;
}

bool LongLatency::run()
{
	size_t longs = 0;
	for (const Instruction& inst : body_.instructions)
		longs += long_latency(inst) ? 1 : 0;
	if (longs < 2)
		return false;
	const size_t n = body_.instructions.size();
	block_.assign(n, 0);
	emitted_.assign(n, 0);
	unread_.assign(n, 0);
	position_.assign(n, 0);
	bool changed = false;
	for (BlockId b : body_.layout)
		changed = order(b) || changed;
	return changed;
}

} // namespace

bool issue_long_latency(FunctionContext& context)
{
	LongLatency pass(context);
	return pass.run();
}

} // namespace opt
