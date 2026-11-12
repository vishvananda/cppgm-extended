#include "wasm/selector.h"

#include <algorithm>

namespace wasm {

using namespace lowir;

bool Selector::select_switch(const lowir::Instruction& inst)
{
	const Type type = operand_type(inst.ops[0], Type::scalar(TypeKind::I64));
	if (inst.extra_count < 8 || !type.is_integer() || type.kind() == TypeKind::I1 || type.kind() == TypeKind::I128)
		return false;
	std::vector<std::pair<int64_t, uint32_t>> cases;
	for (uint32_t i = 0; i < inst.extra_count; i += 2)
	{
		const Value literal = body_.operands[inst.extra_first + i];
		if (!literal.is(ValueKind::Integer))
			return false;
		uint64_t bits = body_.literals[literal.index].low;
		const unsigned width = integer_bits(type.kind());
		if (width < 64)
		{
			bits &= (uint64_t(1) << width) - 1;
			if (!is_unsigned(type.kind()) && (bits & (uint64_t(1) << (width - 1))))
				bits |= ~((uint64_t(1) << width) - 1);
		}
		cases.push_back({ static_cast<int64_t>(bits), body_.operands[inst.extra_first + i + 1].index });
	}
	std::vector<machine::CaseCluster> clusters;
	std::vector<uint32_t> targets;
	machine::cluster_switch_cases(cases, clusters, targets, static_cast<uint32_t>(cases.size()));
	const uint32_t selector = add_local(ValueType::I64);
	value(inst.ops[0], type);
	if (value_type(type) == ValueType::I32)
		out_.emit(is_unsigned(type.kind()) ? Op::I64ExtendI32U : Op::I64ExtendI32S);
	out_.emit(Op::LocalSet, { selector });
	if (machine::dense_switch(cases, 4096, 4, 4))
		switch_table(cases, targets, selector, inst.ops[1].index);
	else
		switch_tree(clusters, 0, static_cast<uint32_t>(clusters.size()), selector, inst.ops[1].index);
	return true;
}

void Selector::switch_tree(const std::vector<machine::CaseCluster>& clusters, uint32_t first, uint32_t past,
	uint32_t selector, BlockId fallback)
{
	if (past - first > 3)
	{
		const uint32_t middle = first + (past - first) / 2;
		out_.emit(Op::LocalGet, { selector });
		out_.emit(Op::I64Const, { static_cast<uint64_t>(clusters[middle].lo) });
		out_.emit(Op::I64LtS);
		out_.emit(Op::If, { block_type(ValueType::Void) });
		++nesting_;
		switch_tree(clusters, first, middle, selector, fallback);
		out_.emit(Op::Else);
		switch_tree(clusters, middle, past, selector, fallback);
		--nesting_;
		out_.emit(Op::End);
		return;
	}
	for (uint32_t i = first; i < past; ++i)
	{
		const auto& c = clusters[i];
		out_.emit(Op::LocalGet, { selector });
		out_.emit(Op::I64Const, { static_cast<uint64_t>(c.lo) });
		if (c.lo == c.hi)
			out_.emit(Op::I64Eq);
		else
		{
			out_.emit(Op::I64Sub);
			out_.emit(Op::I64Const, { static_cast<uint64_t>(c.hi) - static_cast<uint64_t>(c.lo) });
			out_.emit(Op::I64LeU);
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
	const std::vector<uint32_t>& targets, uint32_t selector, BlockId fallback)
{
	const uint64_t low = static_cast<uint64_t>(cases.front().first);
	const uint32_t span = static_cast<uint32_t>(static_cast<uint64_t>(cases.back().first) - low);
	out_.emit(Op::LocalGet, { selector });
	out_.emit(Op::I64Const, { low });
	out_.emit(Op::I64Sub);
	out_.emit(Op::LocalTee, { selector });
	out_.emit(Op::I64Const, { span });
	out_.emit(Op::I64GtU);
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
