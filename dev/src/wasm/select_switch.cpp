#include "wasm/selector.h"

#include <algorithm>

namespace wasm {

using namespace lowir;

namespace {

// The operations of a selector's tests, in its own width: a selector of 32
// bits or fewer is tested as an i32, in the order of its view.
struct SwitchOps
{
	Op constant, subtract, equal, below, within, outside;
};

SwitchOps switch_ops(Type type)
{
	if (value_type(type) != ValueType::I32)
		return { Op::I64Const, Op::I64Sub, Op::I64Eq, Op::I64LtS, Op::I64LeU, Op::I64GtU };
	return { Op::I32Const, Op::I32Sub, Op::I32Eq, is_unsigned(type.kind()) ? Op::I32LtU : Op::I32LtS, Op::I32LeU,
		Op::I32GtU };
}

// A literal of the selector's width (an i32 constant is written sign-extended).
uint64_t switch_literal(Type type, uint64_t bits)
{
	return value_type(type) == ValueType::I32
		? static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(static_cast<uint32_t>(bits))))
		: bits;
}

// The value a case literal has in `type`.
int64_t case_value(Type type, uint64_t bits)
{
	const unsigned width = integer_bits(type.kind());
	if (width < 64)
	{
		bits &= (uint64_t(1) << width) - 1;
		if (!is_unsigned(type.kind()) && (bits & (uint64_t(1) << (width - 1))))
			bits |= ~((uint64_t(1) << width) - 1);
	}
	return static_cast<int64_t>(bits);
}

} // namespace

// The selector as its conversion to 64 bits read it, when every case is a
// value of that narrower type: the tests then need no wide copy.
bool Selector::narrow_selector(const lowir::Instruction& inst, Value& operand, Type& type) const
{
	if (!operand.is(ValueKind::Temp) || type.kind() != TypeKind::I64 || body_.temps[operand.index].def == kNoId)
		return false;
	const lowir::Instruction& def = body_.instructions[body_.temps[operand.index].def];
	const auto operation = static_cast<ConvertOp>(def.sub);
	if (def.op != Opcode::Convert || (operation != ConvertOp::SExt && operation != ConvertOp::ZExt))
		return false;
	const unsigned bits = integer_bits(convert_source_type(def).kind());
	if (bits != 8 && bits != 16 && bits != 32)
		return false;
	const bool sign = operation == ConvertOp::SExt;
	const Type narrow = Type::scalar(bits == 8 ? (sign ? TypeKind::I8 : TypeKind::U8)
			: bits == 16                       ? (sign ? TypeKind::I16 : TypeKind::U16)
											   : (sign ? TypeKind::I32 : TypeKind::U32));
	for (uint32_t i = 0; i < inst.extra_count; i += 2)
	{
		const Value literal = body_.operands[inst.extra_first + i];
		if (!literal.is(ValueKind::Integer) ||
			case_value(narrow, body_.literals[literal.index].low) !=
				static_cast<int64_t>(body_.literals[literal.index].low))
			return false;
	}
	operand = def.ops[0];
	type = narrow;
	return true;
}

bool Selector::select_switch(const lowir::Instruction& inst)
{
	Value operand = inst.ops[0];
	Type type = operand_type(operand, Type::scalar(TypeKind::I64));
	if (!type.is_integer() || type.kind() == TypeKind::I1 || type.kind() == TypeKind::I128)
		return false;
	narrow_selector(inst, operand, type);
	std::vector<std::pair<int64_t, uint32_t>> cases;
	for (uint32_t i = 0; i < inst.extra_count; i += 2)
	{
		const Value literal = body_.operands[inst.extra_first + i];
		if (!literal.is(ValueKind::Integer))
			return false;
		cases.push_back(
			{ case_value(type, body_.literals[literal.index].low), body_.operands[inst.extra_first + i + 1].index });
	}
	std::vector<machine::CaseCluster> clusters;
	std::vector<uint32_t> targets;
	machine::cluster_switch_cases(cases, clusters, targets, static_cast<uint32_t>(cases.size()));
	const uint32_t selector = add_local(value_type(type));
	viewed(operand, type, !is_unsigned(type.kind()));
	out_.emit(Op::LocalSet, { selector });
	if (machine::dense_switch(cases, 4096, 4, 4))
		switch_table(cases, targets, selector, type, inst.ops[1].index);
	else
		switch_tree(clusters, 0, static_cast<uint32_t>(clusters.size()), selector, type, inst.ops[1].index);
	return true;
}

void Selector::switch_tree(const std::vector<machine::CaseCluster>& clusters, uint32_t first, uint32_t past,
	uint32_t selector, Type type, BlockId fallback)
{
	const SwitchOps ops = switch_ops(type);
	if (past - first > 3)
	{
		const uint32_t middle = first + (past - first) / 2;
		out_.emit(Op::LocalGet, { selector });
		out_.emit(ops.constant, { switch_literal(type, static_cast<uint64_t>(clusters[middle].lo)) });
		out_.emit(ops.below);
		out_.emit(Op::If, { block_type(ValueType::Void) });
		++nesting_;
		switch_tree(clusters, first, middle, selector, type, fallback);
		out_.emit(Op::Else);
		switch_tree(clusters, middle, past, selector, type, fallback);
		--nesting_;
		out_.emit(Op::End);
		return;
	}
	for (uint32_t i = first; i < past; ++i)
	{
		const auto& c = clusters[i];
		out_.emit(Op::LocalGet, { selector });
		out_.emit(ops.constant, { switch_literal(type, static_cast<uint64_t>(c.lo)) });
		if (c.lo == c.hi)
			out_.emit(ops.equal);
		else
		{
			out_.emit(ops.subtract);
			out_.emit(
				ops.constant, { switch_literal(type, static_cast<uint64_t>(c.hi) - static_cast<uint64_t>(c.lo)) });
			out_.emit(ops.within);
		}
		out_.emit(Op::If, { block_type(ValueType::Void) });
		++nesting_;
		edge(current_block_, c.target);
		--nesting_;
		out_.emit(Op::End);
	}
	edge(current_block_, fallback);
}

void Selector::switch_table(const std::vector<std::pair<int64_t, uint32_t>>& cases,
	const std::vector<uint32_t>& targets, uint32_t selector, Type type, BlockId fallback)
{
	const SwitchOps ops = switch_ops(type);
	const uint64_t low = static_cast<uint64_t>(cases.front().first);
	const uint32_t span = static_cast<uint32_t>(static_cast<uint64_t>(cases.back().first) - low);
	out_.emit(Op::LocalGet, { selector });
	if (low != 0)
	{
		out_.emit(ops.constant, { switch_literal(type, low) });
		out_.emit(ops.subtract);
		out_.emit(Op::LocalTee, { selector });
	}
	out_.emit(ops.constant, { switch_literal(type, span) });
	out_.emit(ops.outside);
	out_.emit(Op::If, { block_type(ValueType::Void) });
	++nesting_;
	edge(current_block_, fallback);
	--nesting_;
	out_.emit(Op::End);
	const uint32_t count = static_cast<uint32_t>(targets.size());
	std::vector<uint32_t> depths(body_.blocks.size(), count);
	for (uint32_t i = 0; i < count; ++i)
		depths[targets[i]] = i;
	std::vector<uint32_t> table(span + 2, count);
	for (size_t i = cases.size(); i-- > 0;)
		table[static_cast<uint64_t>(cases[i].first) - low] = depths[cases[i].second];
	for (uint32_t i = 0; i <= count; ++i)
		open_label(Op::Block, kNoId);
	out_.emit(Op::LocalGet, { selector });
	if (value_type(type) != ValueType::I32)
		out_.emit(Op::I32WrapI64);
	out_.emit(Op::BrTable);
	for (uint32_t depth : table)
		out_.operands.push_back(depth);
	out_.instructions.back().operands.count = static_cast<uint32_t>(table.size());
	for (BlockId target : targets)
	{
		close_label();
		edge(current_block_, target);
	}
	close_label();
	edge(current_block_, fallback);
}

} // namespace wasm
