#include "native/select/function_selector.h"

#include "native/image/global_data.h"

namespace native {

using lowir::Instruction;
using lowir::kNoId;
using lowir::Opcode;
using lowir::Value;
using lowir::ValueKind;

namespace {

// Padding is paid on entry, but a forward scan confined to one cache line
// has little repetition to repay it. Use bounded typed provenance, keeping
// the existing counted-loop policy when no stronger estimate is available.
bool unit_scan_index(
	const lowir::FunctionBody& body, const std::vector<uint32_t>& block_of, uint32_t header, Value value)
{
	if (!value.is(ValueKind::Temp) || body.temps[value.index].def == kNoId)
		return false;
	const uint32_t def = body.temps[value.index].def;
	const Instruction& phi = body.instructions[def];
	if (phi.op != Opcode::Phi || block_of[def] != header)
		return false;
	bool initial = false, step = false;
	for (uint32_t n = 1; n < phi.extra_count; n += 2)
	{
		const Value incoming = body.operands[phi.extra_first + n];
		if (incoming.is(ValueKind::Integer))
		{
			if (static_cast<int64_t>(body.literals[incoming.index].low) < 0)
				return false;
			initial = true;
			continue;
		}
		if (!incoming.is(ValueKind::Temp) || body.temps[incoming.index].def == kNoId)
			return false;
		const Instruction& add = body.instructions[body.temps[incoming.index].def];
		if (add.op != Opcode::Binary || add.sub != static_cast<uint8_t>(lowir::BinaryOp::Add))
			return false;
		const Value amount = add.ops[0] == value ? add.ops[1] : add.ops[0];
		if (!(add.ops[0] == value || add.ops[1] == value) || !amount.is(ValueKind::Integer) ||
			body.literals[amount.index].low != 1 || body.literals[amount.index].high != 0)
			return false;
		step = true;
	}
	return initial && step;
}

class SmallScan
{
public:
	SmallScan(const lowir::Program& program, const lowir::FunctionBody& body, const std::vector<uint32_t>& block_of,
		uint32_t header)
		: program_(program), body_(body), block_of_(block_of), header_(header)
	{}

	bool address(Value value)
	{
		if (budget_ == 0)
			return false;
		--budget_;
		if (value.is(ValueKind::Symbol))
		{
			const lowir::Symbol& symbol = program_.symbols[value.index];
			if (symbol.kind != lowir::SymbolKind::GlobalDefinition ||
				symbol.metadata.storage != ir_model::GSM_READONLY ||
				symbol.metadata.binding != ir_model::SBM_INTERNAL || (symbol.metadata.flags & lowir::SF_REPLACEABLE))
				return false;
			uint32_t align = 1;
			const lowir::Global& global = program_.globals[symbol.detail];
			if ((global.typed && global.type.bytes > 64) || global.item_count > 64)
				return false;
			return global_size(program_, global, align) <= 64;
		}
		// A null alternative is harmless: a defined load must exclude it.
		if (value.is(ValueKind::Integer))
			return body_.literals[value.index].low == 0 && body_.literals[value.index].high == 0;
		if (!value.is(ValueKind::Temp) || body_.temps[value.index].def == kNoId)
			return false;
		const Instruction& inst = body_.instructions[body_.temps[value.index].def];
		if (inst.op == Opcode::Addr || inst.op == Opcode::Copy)
			return address(inst.ops[0]);
		if (inst.op == Opcode::Index)
		{
			if (!inst.ops[1].is(ValueKind::Integer))
			{
				if (!unit_scan_index(body_, block_of_, header_, inst.ops[1]))
					return false;
				stepped_ = true;
			}
			return address(inst.ops[0]);
		}
		if (inst.op != Opcode::Phi || block_of_[body_.temps[value.index].def] == header_)
			return false;
		for (uint32_t n = 1; n < inst.extra_count; n += 2)
			if (!address(body_.operands[inst.extra_first + n]))
				return false;
		return inst.extra_count != 0;
	}

	bool stepped() const { return stepped_; }

private:
	const lowir::Program& program_;
	const lowir::FunctionBody& body_;
	const std::vector<uint32_t>& block_of_;
	uint32_t header_;
	unsigned budget_ = 128;
	bool stepped_ = false;
};

} // namespace

bool FunctionSelector::small_table_scan(uint32_t first, uint32_t end) const
{
	const std::vector<uint32_t>& members = loop_splitter_.loop_blocks;
	SmallScan scan(program_, *body_, inst_block_, members[first]);
	bool loaded = false;
	for (uint32_t n = first; n < end; ++n)
	{
		const lowir::Block& block = body_->blocks[blocks_[members[n]].id];
		for (uint32_t id = block.first; id != kNoId; id = body_->instructions[id].next)
		{
			if (marks_[id] & IM_DEAD)
				continue;
			const Instruction& inst = body_->instructions[id];
			if (inst.op == Opcode::Load)
			{
				if ((inst.flags & lowir::IF_VOLATILE) || !scan.address(inst.ops[0]))
					return false;
				loaded = true;
			}
			else if (!(inst.op == Opcode::Phi || inst.op == Opcode::Copy || inst.op == Opcode::Addr ||
						 inst.op == Opcode::Index || inst.op == Opcode::Binary || inst.op == Opcode::Unary ||
						 inst.op == Opcode::Compare || inst.op == Opcode::Convert || inst.op == Opcode::Select ||
						 inst.op == Opcode::Jump || inst.op == Opcode::Branch))
				return false;
		}
	}
	return loaded && scan.stepped();
}

// Whether `condition` compares a counter of the loop `epoch` marks (a phi of
// its header, or that phi plus or minus a literal) with a literal or a value
// defined outside the loop.
bool FunctionSelector::counter_test(
	Value condition, const std::vector<uint32_t>& mark, uint32_t epoch, uint32_t header) const
{
	const lowir::FunctionBody& body = *body_;
	if (!condition.is(ValueKind::Temp) || body.temps[condition.index].def == kNoId)
		return false;
	const Instruction& compare = body.instructions[body.temps[condition.index].def];
	if (compare.op != Opcode::Compare)
		return false;
	const auto outside = [&](Value v) {
		if (!v.is(ValueKind::Temp))
			return !v.is(ValueKind::None);
		const lowir::InstId def = body.temps[v.index].def;
		return def == kNoId || mark[inst_block_[def]] != epoch;
	};
	const auto header_phi = [&](Value v) {
		if (!v.is(ValueKind::Temp) || body.temps[v.index].def == kNoId)
			return false;
		const lowir::InstId def = body.temps[v.index].def;
		return body.instructions[def].op == Opcode::Phi && inst_block_[def] == header;
	};
	const auto stepped = [&](Value v) {
		// Widening may leave an exact narrowing at the original counter test.
		if (v.is(ValueKind::Temp) && body.temps[v.index].def != kNoId)
		{
			const Instruction& conversion = body.instructions[body.temps[v.index].def];
			if (conversion.op == Opcode::Convert && conversion.sub == static_cast<uint8_t>(lowir::ConvertOp::Trunc) &&
				(conversion.flags & lowir::IF_EXACT) != 0)
				v = conversion.ops[0];
		}
		if (!v.is(ValueKind::Temp) || body.temps[v.index].def == kNoId)
			return false;
		const Instruction& step = body.instructions[body.temps[v.index].def];
		if (step.op == Opcode::Binary &&
			(step.sub == static_cast<uint8_t>(lowir::BinaryOp::Add) ||
				step.sub == static_cast<uint8_t>(lowir::BinaryOp::Sub)))
			return (header_phi(step.ops[0]) && step.ops[1].is_literal()) ||
				(header_phi(step.ops[1]) && step.ops[0].is_literal());
		return header_phi(v);
	};
	return (stepped(compare.ops[0]) && outside(compare.ops[1])) || (stepped(compare.ops[1]) && outside(compare.ops[0]));
}

// O2+: retain loop membership and the stronger counted-exit fact. The
// encoder uses placement estimates before aligning data-dependent cycles.
void FunctionSelector::mark_loop_blocks()
{
	const lowir::FunctionBody& body = *body_;
	loop_depths(); // Shared with allocation and parameter observation.
	const std::vector<uint32_t>& first = loop_splitter_.loop_first;
	const std::vector<uint32_t>& members = loop_splitter_.loop_blocks;
	const uint32_t n = static_cast<uint32_t>(blocks_.size());
	if (first.size() < 2)
		return;
	std::vector<uint32_t> owner(n, kNoId), size(n, 0);
	for (uint32_t k = 0; k + 1 < first.size(); ++k)
		for (uint32_t m = first[k]; m < first[k + 1]; ++m)
		{
			const uint32_t b = members[m];
			if (owner[b] == kNoId || first[k + 1] - first[k] < size[b])
			{
				owner[b] = k;
				size[b] = first[k + 1] - first[k];
			}
		}
	std::vector<uint32_t> mark(n, kNoId);
	std::vector<uint8_t> counted(first.size(), 0), align(first.size(), 0);
	for (uint32_t k = 0; k + 1 < first.size(); ++k)
	{
		for (uint32_t m = first[k]; m < first[k + 1]; ++m)
			mark[members[m]] = k;
		const uint32_t header = members[first[k]];
		bool exits = false, all = true;
		for (uint32_t m = first[k]; m < first[k + 1] && all; ++m)
		{
			const uint32_t b = members[m];
			const lowir::InstId last = body.blocks[blocks_[b].id].last;
			if (last == kNoId)
				continue;
			const Instruction& term = body.instructions[last];
			bool leaves = false;
			for (uint32_t e = succ_first_[b]; e < succ_first_[b + 1]; ++e)
				leaves = leaves || mark[succs_[e]] != k;
			if (term.op == Opcode::Jump || (term.op == Opcode::Branch && !leaves))
				continue;
			all = term.op == Opcode::Branch && counter_test(term.ops[0], mark, k, header);
			exits = true;
		}
		counted[k] = all && exits;
		align[k] = counted[k] || !small_table_scan(first[k], first[k + 1]);
	}
	for (uint32_t b = 0; b < n; ++b)
		if (owner[b] != kNoId && blocks_[b].mir < out_->blocks.size())
		{
			Block& block = out_->blocks[blocks_[b].mir];
			block.loop = align[owner[b]] != 0;
			block.counted = counted[owner[b]] != 0;
		}
	for (const EdgeBlock& edge : edges_)
		if (owner[edge.from] != kNoId && owner[edge.to] != kNoId && edge.mir < out_->blocks.size())
		{
			Block& block = out_->blocks[edge.mir];
			block.loop = align[owner[edge.from]] != 0 && align[owner[edge.to]] != 0;
			block.counted = owner[edge.from] == owner[edge.to] && counted[owner[edge.from]] != 0;
		}
}

} // namespace native
