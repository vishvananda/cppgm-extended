#include "wasm/mir.h"

namespace wasm {

namespace {

bool local_operation(Op op)
{
	return op == Op::LocalGet || op == Op::LocalSet || op == Op::LocalTee;
}

void count_reads(const Function& function, std::vector<uint32_t>& reads)
{
	reads.assign(function.locals.size(), 0);
	for (const Instruction& inst : function.instructions)
		if (inst.op == Op::LocalGet)
			++reads[function.operands[inst.operands.first]];
}

// The values an operation without an effect takes to leave one: a local or a
// constant read, or integer arithmetic that cannot trap. -1 for the others.
int value_inputs(Op op)
{
	const unsigned code = static_cast<unsigned>(op);
	if (op == Op::LocalGet || op == Op::I32Const || op == Op::I64Const)
		return 0;
	if (op == Op::I32Eqz || op == Op::I64Eqz || (code >= 0x67 && code <= 0x69) || (code >= 0x79 && code <= 0x7b) ||
		op == Op::I32WrapI64 || op == Op::I64ExtendI32S || op == Op::I64ExtendI32U || (code >= 0xc0 && code <= 0xc4))
		return 1;
	if ((code >= 0x46 && code <= 0x4f) || (code >= 0x51 && code <= 0x5a) || (code >= 0x6a && code <= 0x6c) ||
		(code >= 0x71 && code <= 0x78) || (code >= 0x7c && code <= 0x7e) || (code >= 0x83 && code <= 0x8a))
		return 2;
	return -1;
}

// Where the expression that leaves the value on top of the stack before
// `end` begins, when every operation of it may go with that value; else
// `end`. The expression may begin after a value another local keeps
// (`stored`): that local's `local.tee` is then a `local.set`. A relocated
// operand that goes is marked 2: its relocation goes too.
size_t unread_value(
	const Function& function, size_t end, std::vector<uint8_t>& relocated, std::vector<uint32_t>& reads, bool& stored)
{
	size_t need = 1, at = end;
	stored = false;
	while (need != 0 && at != 0 && !stored)
	{
		const Op op = function.instructions[at - 1].op;
		stored = op == Op::LocalTee && need == 1 && at != end;
		if (stored)
			break;
		const int inputs = value_inputs(op);
		if (inputs < 0)
			return end;
		need += static_cast<size_t>(inputs);
		--need;
		--at;
	}
	if (need != 0 && !stored)
		return end;
	for (size_t i = at; i < end; ++i)
	{
		const Instruction& inst = function.instructions[i];
		if (inst.op == Op::LocalGet)
			--reads[function.operands[inst.operands.first]];
		else if (inst.operands.count != 0 && relocated[inst.operands.first] != 0)
			relocated[inst.operands.first] = 2;
	}
	return at;
}

// The integer comparison that holds when `op` does not; Nop when `op` is not
// an integer comparison of two values.
Op inverse_comparison(Op op)
{
	static const uint8_t kInverse[10] = { 1, 0, 8, 9, 6, 7, 4, 5, 2, 3 };
	const unsigned code = static_cast<unsigned>(op);
	if (code < 0x46 || code > 0x5a || code == 0x50)
		return Op::Nop;
	const unsigned base = code > 0x50 ? 0x51 : 0x46;
	return static_cast<Op>(base + kInverse[code - base]);
}

// Whether `op` leaves 0 or 1: a comparison or a zero test.
bool leaves_truth(Op op)
{
	const unsigned code = static_cast<unsigned>(op);
	return code >= 0x45 && code <= 0x66;
}

// A truth test reads only whether its value is zero: the test of a widened
// 32-bit value is the test of that value, and a doubled negation before an
// `if`, a `br_if` or a `select` goes. A negated integer comparison is the
// opposite comparison, and a truth value negated twice is that value. A
// widening that is narrowed at once goes too.
void fold_truth_tests(Function& function)
{
	size_t kept = 0;
	for (const Instruction& inst : function.instructions)
	{
		const Op last = kept != 0 ? function.instructions[kept - 1].op : Op::Nop;
		if (inst.op == Op::I64Eqz && last == Op::I64ExtendI32U)
		{
			function.instructions[kept - 1].op = Op::I32Eqz;
			continue;
		}
		if (inst.op == Op::I32WrapI64 && (last == Op::I64ExtendI32U || last == Op::I64ExtendI32S))
		{
			--kept;
			continue;
		}
		if (inst.op == Op::I32Eqz && inverse_comparison(last) != Op::Nop)
		{
			function.instructions[kept - 1].op = inverse_comparison(last);
			continue;
		}
		if (inst.op == Op::I32Eqz && last == Op::I32Eqz && kept >= 2 &&
			leaves_truth(function.instructions[kept - 2].op))
		{
			--kept;
			continue;
		}
		if ((inst.op == Op::If || inst.op == Op::BrIf || inst.op == Op::Select) && kept >= 2 && last == Op::I32Eqz &&
			function.instructions[kept - 2].op == Op::I32Eqz)
			kept -= 2;
		function.instructions[kept++] = inst;
	}
	function.instructions.resize(kept);
}

} // namespace

void cleanup_locals(Function& function, bool optimize)
{
	std::vector<uint32_t> uses;
	if (optimize)
	{
		count_reads(function, uses);
		size_t kept = 0;
		for (size_t i = 0; i < function.instructions.size(); ++i)
		{
			Instruction inst = function.instructions[i];
			if (inst.op == Op::LocalSet && i + 1 < function.instructions.size())
			{
				const Instruction& next = function.instructions[i + 1];
				const uint64_t id = function.operands[inst.operands.first];
				if (next.op == Op::LocalGet && function.operands[next.operands.first] == id)
				{
					++i;
					if (uses[id] == 1)
						continue;
					inst.op = Op::LocalTee;
				}
			}
			function.instructions[kept++] = inst;
		}
		function.instructions.resize(kept);
		fold_truth_tests(function);
		// A value that no instruction reads is not stored, and not computed
		// when its operations have no effect: a step whose uses all went into
		// memory offsets. Removing one may leave the values it read unread.
		std::vector<uint8_t> relocated(function.operands.size() + 1, 0);
		for (const Relocation& relocation : function.relocations)
			relocated[relocation.offset] = 1;
		count_reads(function, uses);
		for (bool again = true; again;)
		{
			again = false;
			kept = 0;
			for (size_t i = 0; i < function.instructions.size(); ++i)
			{
				Instruction inst = function.instructions[i];
				if ((inst.op == Op::LocalSet || inst.op == Op::LocalTee) &&
					uses[function.operands[inst.operands.first]] == 0)
				{
					if (inst.op == Op::LocalTee)
						continue;
					bool stored = false;
					const size_t begin = unread_value(function, kept, relocated, uses, stored);
					if (begin != kept)
					{
						kept = begin;
						if (stored)
							function.instructions[kept - 1].op = Op::LocalSet;
						again = true;
						continue;
					}
					// (A value another local keeps is only stored there.)
					if (kept != 0 && function.instructions[kept - 1].op == Op::LocalTee)
					{
						function.instructions[kept - 1].op = Op::LocalSet;
						continue;
					}
					inst.op = Op::Drop;
					inst.operands.count = 0;
				}
				function.instructions[kept++] = inst;
			}
			function.instructions.resize(kept);
		}
		size_t named = 0;
		for (const Relocation& relocation : function.relocations)
			if (relocated[relocation.offset] != 2)
				function.relocations[named++] = relocation;
		function.relocations.resize(named);
	}
	// Parameters retain their public ABI indices, including unused parameters.
	// Ordinary locals need no allocation if no surviving instruction uses them.
	uses.assign(function.locals.size(), 0);
	for (const Instruction& inst : function.instructions)
		if (local_operation(inst.op))
			uses[function.operands[inst.operands.first]] = 1;
	uint32_t next = function.parameter_count;
	for (uint32_t i = 0; i < function.locals.size(); ++i)
		if (i < function.parameter_count)
			uses[i] = i;
		else if (uses[i] != 0)
		{
			uses[i] = next;
			function.locals[next++] = function.locals[i];
		}
	function.locals.resize(next);
	for (const Instruction& inst : function.instructions)
		if (local_operation(inst.op))
		{
			auto& operand = function.operands[inst.operands.first];
			operand = uses[operand];
		}
}

} // namespace wasm
