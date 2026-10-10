#include "native/object/function_census.h"

#include <algorithm>

#include "support/telemetry.h"

namespace native {

#if CPPGM_TELEMETRY_ENABLED

namespace {

bool frame_operand(const Operand& operand)
{
	return operand.kind == OperandKind::Mem && operand.reg == kFrame && operand.symbol == lowir::kNoId;
}

// An instruction after which control does not reach the next one.
bool unconditional(const Inst& inst)
{
	switch (inst.op)
	{
	case Op::Jmp:
	case Op::Ret:
	case Op::FRet:
	case Op::TailJump:
	case Op::JumpTable:
	case Op::WideCmpBranch:
	case Op::Throw:
	case Op::Resume:
	case Op::Exit:
	case Op::Trap:
		return true;
	case Op::Call:
		return (inst.flags & MF_NORETURN) != 0;
	default:
		return false;
	}
}

void add_successor(std::vector<uint32_t>& successors, uint32_t block)
{
	if (std::find(successors.begin(), successors.end(), block) == successors.end())
		successors.push_back(block);
}

// The blocks control reaches from `block`: its jumps' and jump tables'
// targets, an eh_push's handler, and the next block unless the block ends
// control.
void block_successors(const Function& f, uint32_t block, std::vector<uint32_t>& successors)
{
	const uint32_t count = static_cast<uint32_t>(f.blocks.size());
	const Block& b = f.blocks[block];
	bool falls_through = true;
	for (uint32_t i = b.first; i < b.first + b.count; ++i)
	{
		const Inst& inst = f.insts[i];
		if (inst.op == Op::JumpTable)
		{
			for (uint32_t k = inst.aux; k < inst.aux + inst.aux2; ++k)
				add_successor(successors, f.jump_targets[k]);
		}
		else if (inst.op == Op::Jcc || inst.op == Op::Jmp || inst.op == Op::EhPush || inst.op == Op::WideCmpBranch)
		{
			for (unsigned k = 0; k < inst.nops; ++k)
				if (inst.ops[k].kind == OperandKind::Label && inst.ops[k].symbol < count)
					add_successor(successors, inst.ops[k].symbol);
			if (inst.op == Op::WideCmpBranch && inst.aux < count)
				add_successor(successors, inst.aux);
		}
		falls_through = falls_through && !unconditional(inst);
	}
	if (falls_through && block + 1 < count)
		add_successor(successors, block + 1);
}

// Whether `target` is reachable from the entry without entering `omitted`
// (the entry itself is never omitted).
bool reaches_without(const std::vector<std::vector<uint32_t>>& successors, uint32_t target, uint32_t omitted)
{
	if (successors.empty() || omitted == 0)
		return false;
	std::vector<bool> seen(successors.size(), false);
	std::vector<uint32_t> work(1, 0);
	seen[0] = true;
	while (!work.empty())
	{
		const uint32_t block = work.back();
		work.pop_back();
		if (block == target)
			return true;
		for (uint32_t next : successors[block])
		{
			if (next == omitted || seen[next])
				continue;
			seen[next] = true;
			work.push_back(next);
		}
	}
	return false;
}

struct Loop
{
	uint32_t header;
	std::vector<bool> members;
};

void field(std::string& line, const char* name, uint64_t value)
{
	line += ' ';
	line += name;
	line += '=';
	line += std::to_string(value);
}

// One line per natural loop: an edge latch -> header is a back edge when
// every path from the entry to the latch passes the header; the loop is the
// header with the blocks that reach the latch without passing it, and the
// back edges to one header make one loop.
void append_loops(
	const lowir::Program& program, const Function& f, const std::string& symbol, std::vector<std::string>& lines)
{
	const uint32_t count = static_cast<uint32_t>(f.blocks.size());
	std::vector<std::vector<uint32_t>> successors(count), predecessors(count);
	for (uint32_t b = 0; b < count; ++b)
	{
		block_successors(f, b, successors[b]);
		for (uint32_t next : successors[b])
			predecessors[next].push_back(b);
	}
	std::vector<bool> reachable(count, false);
	std::vector<uint32_t> work;
	if (count != 0)
	{
		reachable[0] = true;
		work.push_back(0);
	}
	while (!work.empty())
	{
		const uint32_t b = work.back();
		work.pop_back();
		for (uint32_t next : successors[b])
		{
			if (!reachable[next])
			{
				reachable[next] = true;
				work.push_back(next);
			}
		}
	}
	std::vector<Loop> loops;
	for (uint32_t latch = 0; latch < count; ++latch)
	{
		if (!reachable[latch])
			continue;
		for (uint32_t header : successors[latch])
		{
			if (header != latch && reaches_without(successors, latch, header))
				continue;
			size_t loop = 0;
			while (loop < loops.size() && loops[loop].header != header)
				++loop;
			if (loop == loops.size())
				loops.push_back(Loop{ header, std::vector<bool>(count, false) });
			std::vector<bool>& members = loops[loop].members;
			members[header] = true;
			if (!members[latch])
			{
				members[latch] = true;
				if (latch != header)
					work.push_back(latch);
			}
			while (!work.empty())
			{
				const uint32_t b = work.back();
				work.pop_back();
				for (uint32_t previous : predecessors[b])
				{
					if (members[previous])
						continue;
					members[previous] = true;
					work.push_back(previous);
				}
			}
		}
	}
	std::sort(loops.begin(), loops.end(), [](const Loop& a, const Loop& b) { return a.header < b.header; });
	const uint32_t callee_saved = static_cast<uint32_t>(__builtin_popcount(f.preserve_mask));
	for (size_t loop = 0; loop < loops.size(); ++loop)
	{
		const std::vector<bool>& members = loops[loop].members;
		uint64_t blocks = 0, instructions = 0, calls = 0, eh = 0, frame_operands = 0;
		for (uint32_t b = 0; b < count; ++b)
		{
			if (!members[b])
				continue;
			++blocks;
			for (uint32_t i = f.blocks[b].first; i < f.blocks[b].first + f.blocks[b].count; ++i)
			{
				const Inst& inst = f.insts[i];
				if (inst.op == Op::Nop)
					continue;
				++instructions;
				if (inst.op == Op::Call || inst.op == Op::TailJump)
					++calls;
				if ((inst.op >= Op::EhPush && inst.op <= Op::CatchAll) || inst.eh != 0)
					++eh;
				for (unsigned k = 0; k < inst.nops; ++k)
					frame_operands += frame_operand(inst.ops[k]) ? 1 : 0;
			}
		}
		uint64_t depth = 0;
		for (size_t outer = 0; outer < loops.size(); ++outer)
		{
			if (outer == loop)
				continue;
			bool contains = true, larger = false;
			for (uint32_t b = 0; b < count; ++b)
			{
				contains = contains && (!members[b] || loops[outer].members[b]);
				larger = larger || (loops[outer].members[b] && !members[b]);
			}
			depth += contains && larger ? 1 : 0;
		}
		const uint32_t header = loops[loop].header;
		const lowir::NameId name = f.blocks[header].name;
		std::string line = "loop_census symbol=" + symbol +
			" header_label=" + (name != 0 ? std::string(program.spelling(name)) : "block_" + std::to_string(header));
		field(line, "header", header);
		field(line, "blocks", blocks);
		field(line, "mir", instructions);
		field(line, "calls", calls);
		field(line, "eh", eh);
		field(line, "depth", depth);
		field(line, "frame_operands", frame_operands);
		field(line, "frame_bindings", f.bindings.size());
		field(line, "callee_saved", callee_saved);
		lines.push_back(line);
	}
}

} // namespace

void append_function_census(const lowir::Program& program, const Function& mir, const SelectStats& before,
	const SelectStats& after, std::vector<std::string>& lines)
{
	// Movement: frame loads and stores, register-to-register copies.
	uint64_t loads = 0, stores = 0, copies = 0;
	for (const Block& block : mir.blocks)
	{
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
		{
			const Inst& inst = mir.insts[i];
			const bool move = inst.op == Op::Mov || inst.op == Op::FMov;
			if ((inst.op == Op::Load || inst.op == Op::FMov) && inst.ops[0].is_reg() && frame_operand(inst.ops[1]))
				++loads;
			else if ((inst.op == Op::Store || inst.op == Op::FMov) && frame_operand(inst.ops[0]))
				++stores;
			else if (move && inst.ops[0].is_reg() && inst.ops[1].is_reg() && inst.ops[0].reg != inst.ops[1].reg)
				++copies;
		}
	}
	const std::string symbol = program.spelling(program.symbols[mir.symbol].name);
	std::string line = "function_census symbol=" + symbol;
	field(line, "mir", after.mir_instructions - before.mir_instructions);
	field(line, "mov_loads", loads);
	field(line, "mov_stores", stores);
	field(line, "mov_copies", copies);
	field(line, "planned", after.global_registers - before.global_registers);
	field(line, "grants", after.register_grants - before.register_grants);
	field(line, "releases", after.register_releases - before.register_releases);
	field(line, "spills", after.spills - before.spills);
	field(line, "frame_homes", after.homes - before.homes);
	field(line, "grant_busy", after.grant_busy - before.grant_busy);
	field(line, "grant_busy_parameters", after.grant_busy_parameters - before.grant_busy_parameters);
	field(line, "grant_busy_values", after.grant_busy_values - before.grant_busy_values);
	field(line, "defined_in_plan", after.defined_in_plan - before.defined_in_plan);
	field(line, "defined_frame", after.defined_frame - before.defined_frame);
	field(line, "defined_other_register", after.defined_other_register - before.defined_other_register);
	lines.push_back(line);
	append_loops(program, mir, symbol, lines);
}

#else

void append_function_census(
	const lowir::Program&, const Function&, const SelectStats&, const SelectStats&, std::vector<std::string>&)
{}

#endif

} // namespace native
