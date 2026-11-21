#include "aarch64/selector.h"

#include "aarch64/error.h"
#include "opt/function_ir.h"

namespace aarch64 {

using namespace lowir;
namespace {
const unsigned result_reg = 9, left = 10, right = 11;
const Type word = Type::scalar(TypeKind::I64);

bool zero_extended_from_narrower(const FunctionBody& body, Value value, Type type)
{
	if (!value.is(ValueKind::Temp) || body.temps[value.index].type != type)
		return false;
	const InstId id = body.temps[value.index].def;
	if (id == kNoId)
		return false;
	const auto& def = body.instructions[id];
	return def.op == Opcode::Convert && def.type == type && static_cast<ConvertOp>(def.sub) == ConvertOp::ZExt &&
		integer_bits(convert_source_type(def).kind()) < integer_bits(type.kind());
}
}

void Selector::binary(const lowir::Instruction& inst)
{
	const BinaryOp op = static_cast<BinaryOp>(inst.sub);
	const unsigned bits = integer_bits(inst.type.kind());
	value(left, inst.ops[0], inst.type);
	value(right, inst.ops[1], inst.type);
	const bool div = op == BinaryOp::Div || op == BinaryOp::Mod;
	if (div || op == BinaryOp::UDiv || op == BinaryOp::UMod || op == BinaryOp::Shr || op == BinaryOp::UShr)
	{
		extend(left, bits, div || op == BinaryOp::Shr);
		extend(right, bits, div);
	}
	static const Op ops[] = { Op::Add, Op::Sub, Op::Mul, Op::SDiv, Op::SDiv, Op::UDiv, Op::UDiv, Op::And, Op::Orr,
		Op::Eor, Op::Lsl, Op::Asr, Op::Lsr };
	out_.emit(ops[inst.sub], result_reg, left, right);
	if (op == BinaryOp::Mod || op == BinaryOp::UMod)
	{
		out_.emit(Op::Mul, result_reg, result_reg, right);
		out_.emit(Op::Sub, result_reg, left, result_reg);
	}
}

void Selector::select(const lowir::Instruction& inst, BlockId block)
{
	if (inst.result != kNoId &&
		(rematerialized(inst.result) || (!aliases_.empty() && aliases_[inst.result] != inst.result)))
		return;
	if ((inst.type.kind() == TypeKind::I128 ||
			(inst.op == Opcode::Convert && convert_source_type(inst).kind() == TypeKind::I128)) &&
		wide(inst))
		return;
	if (floating(inst))
	{
		float_instruction(inst);
		return;
	}
	switch (inst.op)
	{
	case Opcode::Phi:
		return;
	case Opcode::Const:
	case Opcode::Copy:
		if (vectors_[inst.result] != kNoId && inst.type.is_float())
		{
			save_fp(fp_operand(inst.ops[0], inst.type, vectors_[inst.result]), inst.result);
			return;
		}
		if (inst.type.kind() == TypeKind::V128)
		{
			save_vector(vector_value(inst.ops[0], 16), inst.result);
			return;
		}
		if (memory_value(inst.type))
		{
			home_address(10, inst.result);
			copy_value(10, inst.ops[0], inst.type);
			return;
		}
		if (level_)
		{
			const unsigned dest = registers_[inst.result] == kNoId ? result_reg : registers_[inst.result];
			value(dest, inst.ops[0], inst.type);
			save(dest, inst.result, inst.ops[0].is(ValueKind::Temp) || inst.ops[0].is(ValueKind::Integer));
			return;
		}
		value(result_reg, inst.ops[0], inst.type);
		break;
	case Opcode::Addr:
	{
		const auto dest = registers_[inst.result] == kNoId ? result_reg : registers_[inst.result];
		address(dest, inst.ops[0]);
		save(dest, inst.result, true);
		return;
	}
	case Opcode::CopyObject:
	{
		if (!call_result_slots_.empty() && inst.ops[0].is(ValueKind::Temp) &&
			call_result_slots_[inst.ops[0].index] != kNoId)
			return;
		const unsigned dst = address_register(inst.ops[1], 10);
		const unsigned src = address_register(inst.ops[0], 11);
		// LowIR copyobj permits disjoint spans or the same address.
		// In either case a forward copy is sufficient.
		if (level_ && dst != src && inst.immediate > 32)
			copy_run(dst, src, inst.immediate, false);
		else
			copy_bytes(dst, src, inst.immediate);
		return;
	}
	case Opcode::ZeroInit:
		copy_bytes(address_register(inst.ops[0], 10), 31, inst.immediate);
		return;
	case Opcode::Load:
		if (inst.type.kind() == TypeKind::V128 && vector_page_access(inst))
			return;
		if (inst.type.kind() == TypeKind::V128)
		{
			const unsigned base = address_register(inst.ops[0], 10);
			const unsigned dest = vectors_[inst.result] == kNoId ? 16 : vectors_[inst.result];
			out_.emit(Op::LdrQ, dest, base);
			save_vector(dest, inst.result);
			return;
		}
		if (memory_value(inst.type))
		{
			home_address(10, inst.result);
			copy_bytes(10, address_register(inst.ops[0], 11), storage_bytes(inst.type));
			return;
		}
		if (level_)
		{
			scalar_access(inst);
			return;
		}
		address(left, inst.ops[0]);
		access(false, result_reg, left, 0, storage_bytes(inst.type));
		break;
	case Opcode::Store:
		if (inst.type.kind() == TypeKind::V128 && vector_page_access(inst))
			return;
		if (inst.type.kind() == TypeKind::V128)
		{
			const unsigned base = address_register(inst.ops[1], 10);
			out_.emit(Op::StrQ, vector_value(inst.ops[0], 16), base);
			return;
		}
		if (memory_value(inst.type))
		{
			copy_value(address_register(inst.ops[1], 10), inst.ops[0], inst.type);
			return;
		}
		if (level_)
		{
			scalar_access(inst);
			return;
		}
		address(left, inst.ops[1]);
		value(right, inst.ops[0], inst.type);
		access(true, right, left, 0, storage_bytes(inst.type));
		return;
	case Opcode::Index:
	{
		address(left, inst.ops[0]);
		const Type index = type_of(inst.ops[1], word);
		value(right, inst.ops[1], index);
		extend(right, integer_bits(index.kind()), !is_unsigned(index.kind()) && index.kind() != TypeKind::I1);
		const uint64_t stride = storage_bytes(inst.type);
		if (level_ && stride && (stride & (stride - 1)) == 0)
		{
			unsigned shift = 0;
			while ((uint64_t(1) << shift) != stride)
				++shift;
			out_.emit(Op::Add, result_reg, left, right, shift << 10);
			break;
		}
		constant(result_reg, stride);
		out_.emit(Op::Mul, result_reg, right, result_reg);
		out_.emit(Op::Add, result_reg, left, result_reg);
		break;
	}
	case Opcode::Vector:
		vector_instruction(inst);
		return;
	case Opcode::Binary:
		if (level_ && simple_binary(inst))
			return;
		binary(inst);
		break;
	case Opcode::Compare:
		compare(inst);
		return;
	case Opcode::Convert:
	{
		const auto op = static_cast<ConvertOp>(inst.sub);
		const Type from = convert_source_type(inst);
		if (op != ConvertOp::SExt && op != ConvertOp::ZExt && op != ConvertOp::Trunc)
			throw AArch64Error("unsupported AArch64 conversion");
		const auto to_bits = integer_bits(inst.type.kind());
		if (level_ && (op != ConvertOp::SExt || to_bits == 32 || to_bits == 64))
		{
			const unsigned src = operand_register(inst.ops[0], from, left);
			const unsigned dest = registers_[inst.result] == kNoId ? result_reg : registers_[inst.result];
			const unsigned bits = integer_bits(op == ConvertOp::Trunc ? inst.type.kind() : from.kind());
			const bool zero_extend = op == ConvertOp::ZExt ||
				(op == ConvertOp::SExt && zero_extended_from_narrower(body_, inst.ops[0], from));
			if ((zero_extend || (op == ConvertOp::Trunc && loose(inst.result))) && inst.ops[0].is(ValueKind::Temp) &&
				!memory_value(from))
				move(dest, src);
			else if (bits == 32 && op != ConvertOp::SExt)
				out_.emit(Op::OrrW, dest, 31, src);
			else if (bits && bits < 64)
				out_.emit(op == ConvertOp::SExt ? (to_bits == 32 ? Op::SbfmW : Op::Sbfm) : Op::Ubfm, dest, src, 0,
					(bits - 1) << 10);
			else
				move(dest, src);
			save(dest, inst.result, true);
			return;
		}
		value(result_reg, inst.ops[0], from);
		extend(result_reg, integer_bits(from.kind()), op == ConvertOp::SExt);
		break;
	}
	case Opcode::Unary:
		integer_unary(inst);
		return;
	case Opcode::Switch:
		switch_edges(inst, block);
		return;
	case Opcode::AtomicLoad:
	case Opcode::AtomicStore:
	case Opcode::AtomicAddFetch:
	case Opcode::AtomicExchange:
	case Opcode::AtomicCompareExchange:
	case Opcode::AtomicThreadFence:
	case Opcode::AtomicSignalFence:
		atomic(inst);
		return;
	case Opcode::Select:
		if (level_ && !memory_value(inst.type) && !inst.ops[1].is(ValueKind::Symbol) &&
			!inst.ops[2].is(ValueKind::Symbol))
		{
			const unsigned cond = condition_register(inst.ops[0], result_reg);
			out_.emit(word_condition(inst.ops[0]) ? Op::SubsW : Op::Subs, 31, cond, 31);
			choose(inst, 1);
			return;
		}
		move(result_reg, condition_register(inst.ops[0], result_reg));
		value(left, inst.ops[1], inst.type);
		value(right, inst.ops[2], inst.type);
		out_.emit(Op::Subs, 31, result_reg, 31);
		out_.emit(Op::Csel, result_reg, left, right, 1u << 12);
		break;
	case Opcode::EhTry:
	case Opcode::EhCleanup:
	case Opcode::EhCatch:
	case Opcode::EhFilter:
	case Opcode::EhEnd:
	case Opcode::EhCatchAll:
		return;
	case Opcode::Exception:
	case Opcode::ExceptionSelector:
		exception_value(inst);
		return;
	case Opcode::Resume:
		resume(site(inst));
		return;
	case Opcode::Jump:
		if (site(inst))
			resume(site(inst));
		else
			edge(block, inst.ops[0].index, true);
		return;
	case Opcode::Branch:
		conditional_edges(
			block, inst, word_condition(inst.ops[0]) ? Op::CbzW : Op::Cbz, condition_register(inst.ops[0], left));
		return;
	case Opcode::StackAlloc:
		stack_alloc(inst);
		return;
	case Opcode::StackSave:
		out_.emit(Op::AddI, result_reg, 31);
		break;
	case Opcode::StackRestore:
		value(left, inst.ops[0], program_.pointer_type());
		out_.emit(Op::AddI, 31, left);
		return;
	case Opcode::FrameAddress:
		move(result_reg, 29);
		break;
	case Opcode::VaStart:
		emit_va_start(inst);
		return;
	case Opcode::VaArg:
		emit_va_arg(inst);
		return;
	case Opcode::Call:
		call(inst);
		return;
	case Opcode::Return:
		epilogue(inst);
		return;
	case Opcode::Unreachable:
		out_.emit(Op::Brk);
		return;
	default:
		throw AArch64Error(std::string("unsupported AArch64 instruction: ") + spelling(inst.op));
	}
	save(result_reg, inst.result);
}

unsigned Selector::incoming_scalar(Value param) const
{
	if (!param.is(ValueKind::Temp) || param.index >= body_.parameter_count)
		return kNoId;
	Arguments args(named_.darwin);
	for (unsigned i = 0; i <= param.index; ++i)
	{
		const auto& p = program_.parameters_of(signature_)[i];
		if (p.pass == ir_model::PPM_INDIRECT_RESULT)
			return kNoId;
		const auto arg = args.next(p.type, p.shape);
		if (i == param.index && arg.bank == Bank::General && !memory_value(p.type) &&
			(p.type.is_integer() || p.type.is_pointer()))
			return arg.first;
	}
	return kNoId;
}

// Recognize a direct parameter, or a scalar field of a parameter, tested
// once at entry. Every skipped definition has exactly one use in the test.
bool Selector::entry_guard(BlockId entry, EntryGuard& guard) const
{
	const auto& bb = body_.blocks[entry];
	if (bb.last == kNoId)
		return false;
	const auto& branch = body_.instructions[bb.last];
	if (branch.op != Opcode::Branch || !branch.ops[0].is(ValueKind::Temp))
		return false;
	InstId id = bb.first;
	Value expected = Value::none();
	if (body_.instructions[id].op == Opcode::Index)
	{
		const auto& index = body_.instructions[id];
		if (uses_[index.result] != 1 || !index.ops[1].is(ValueKind::Integer))
			return false;
		const uint64_t stride = storage_bytes(index.type), count = body_.literals[index.ops[1].index].low;
		if (!stride || count > 32760 / stride)
			return false;
		guard.offset = count * stride;
		guard.reg = incoming_scalar(index.ops[0]);
		expected = Value::temp(index.result);
		id = index.next;
	}
	if (body_.instructions[id].op == Opcode::Load)
	{
		const auto& load = body_.instructions[id];
		if ((!load.type.is_integer() && !load.type.is_pointer()) || (load.flags & IF_VOLATILE) ||
			uses_[load.result] != 1)
			return false;
		if (expected.is(ValueKind::None))
			guard.reg = incoming_scalar(load.ops[0]);
		else if (!(expected == load.ops[0]))
			return false;
		guard.bytes = storage_bytes(load.type);
		if (guard.bytes > 8 || load.type.kind() == TypeKind::I1 || guard.offset % guard.bytes ||
			guard.offset / guard.bytes >= 4096)
			return false;
		expected = Value::temp(load.result);
		id = load.next;
	}
	else if (!expected.is(ValueKind::None))
		return false;
	const bool compared = body_.instructions[id].op == Opcode::Compare;
	const auto& last = body_.instructions[id];
	const Value tested = compared ? last.ops[0] : branch.ops[0];
	if (!tested.is(ValueKind::Temp))
		return false;
	if (expected.is(ValueKind::None))
		guard.reg = incoming_scalar(tested);
	else if (!(tested == expected))
		return false;
	guard.bits = integer_bits(body_.temps[tested.index].type.kind());
	if (guard.reg == kNoId || (!guard.bytes && guard.bits && guard.bits < 32))
		return false;
	if (!compared)
		return id == bb.last;
	if (last.next != bb.last || uses_[last.result] != 1 || !(Value::temp(last.result) == branch.ops[0]) ||
		last.type != body_.temps[tested.index].type || !last.ops[1].is(ValueKind::Integer) ||
		(last.sub != static_cast<unsigned>(ComparePredicate::Eq) &&
			last.sub != static_cast<unsigned>(ComparePredicate::Ne)))
		return false;
	const uint64_t mask = !guard.bits || guard.bits == 64 ? ~0ull : (1ull << guard.bits) - 1;
	const uint64_t literal = body_.literals[last.ops[1].index].low & mask;
	if (literal > 4095)
		return false;
	guard.literal = literal;
	guard.equal = last.sub == static_cast<unsigned>(ComparePredicate::Eq);
	return true;
}

BlockId Selector::early_return()
{
	if (level_ < 2 || named_.darwin || !frame_ || out_.exceptions || !body_.code_labels.empty() ||
		program_.symbols[symbol_].metadata.tls_for != kNoId)
		return kNoId;
	const auto entry = body_.layout.front();
	EntryGuard guard;
	if (!entry_guard(entry, guard))
		return kNoId;
	const auto& branch = body_.instructions[body_.blocks[entry].last];
	for (unsigned side = 1; side <= 2; ++side)
	{
		const auto fast = branch.ops[side].index, slow = branch.ops[3 - side].index;
		const auto& b = body_.blocks[fast];
		if (b.first != b.last || b.last == kNoId || fast == slow)
			continue;
		const auto& ret = body_.instructions[b.last];
		const Type type = signature_.result;
		const unsigned width = integer_bits(type.kind());
		if (ret.op != Opcode::Return || !(type.is_void() || type.is_pointer() || (width && width <= 64)))
			continue;
		const Value value = ret.ops[0];
		const auto returned = incoming_scalar(value);
		if (!type.is_void() && !value.is(ValueKind::Integer) &&
			(returned == kNoId || type != body_.temps[value.index].type || (width && width < 32)))
			continue;
		opt::Cfg cfg;
		opt::build_cfg_light(program_, body_, cfg, true);
		if (cfg.pred_count(entry))
			return kNoId;
		const auto skip = static_cast<uint32_t>(out_.labels.size());
		out_.labels.push_back(kNoId);
		const unsigned reg = guard.bytes ? 9 : guard.reg;
		if (guard.bytes)
			access(false, reg, guard.reg, guard.offset, guard.bytes);
		const bool nonzero = (side == 2) != guard.equal;
		const bool word = guard.bits && guard.bits <= 32;
		if (guard.literal)
		{
			out_.emit(word ? Op::SubsIW : Op::SubsIX, 31, reg, 0, guard.literal << 10);
			out_.reference(Op::BCond, Fixup::Block, skip, nonzero ? 1 : 0);
		}
		else
			out_.reference(
				nonzero ? (word ? Op::CbnzW : Op::Cbnz) : (word ? Op::CbzW : Op::Cbz), Fixup::Block, skip, reg);
		if (!type.is_void())
		{
			if (value.is(ValueKind::Integer))
				constant(0, body_.literals[value.index].low & (width && width < 64 ? (1ull << width) - 1 : ~0ull));
			else if (width == 32)
				out_.emit(Op::OrrW, 0, 31, returned);
			else
				move(0, returned);
		}
		out_.emit(Op::Ret, 0, 30);
		out_.bind(skip);
		return slow;
	}
	return kNoId;
}

void Selector::run()
{
	plan();
	find_loose_values();
	find_chosen_increments();
	const BlockId slow_entry = minimize_size_ ? kNoId : early_return();
	stack_adjust(true);
	if (tls_vectors_ != kNoId)
	{
		base_address(16, 31, tls_vectors_);
		for (unsigned v = 0; v < 32; ++v)
			out_.emit(Op::StrD, v, 16, 0, v << 10);
	}
	saved_registers(true);
	if (frame_pointer_ != kNoId)
		base_address(29, 31, frame_pointer_);
	if (dynamic_stack_)
	{
		out_.emit(Op::AddI, 28, 31);
		out_.frame_rule(FrameRule::Register, 28);
	}
	frame_code_.prologue_end = static_cast<uint32_t>(out_.code.size());
	frame_code_.prologue_events = static_cast<uint32_t>(out_.frame_events.size());
	if (result_.indirect)
		frame_code_.incoming |= 1u << 8;
	if (result_.bank == Bank::General && !result_.indirect)
		frame_code_.returned = ((1u << result_.pieces) - 1) << result_.first;
	if (result_reg_ != kNoId)
		move(result_reg_, 8);
	else if (result_pointer_ != kNoId)
		access(true, 8, frame_base(), result_pointer_, 8);
	save_variadic_registers();
	for (unsigned i = 0; i < body_.parameter_count; ++i)
	{
		const auto& param = program_.parameters_of(signature_)[i];
		const Argument arg = param.pass == ir_model::PPM_INDIRECT_RESULT ? Argument{ Bank::General, 8, 1, 8, false }
																		 : named_.next(param.type, param.shape);
		if (arg.bank == Bank::General)
			frame_code_.incoming |= ((1u << arg.pieces) - 1) << arg.first;
		parameter(i, arg);
	}
	const auto placed = order_blocks();
	const auto& layout = placed.empty() ? body_.layout : placed;
	auto elided = entry_elided_.begin();
	plan_forwarding(layout);
	for (size_t index = 0; index < layout.size(); ++index)
	{
		const BlockId block = layout[index];
		if (forwarded(block))
			continue;
		if (!deferred_edges_.empty() && !out_.code.empty())
		{
			const auto op = out_.code.back().op;
			if (op == Op::B || op == Op::Ret || op == Op::Br || op == Op::Brk)
				flush_deferred_edges();
		}
		// A landing prologue can precede its LowIR block label.
		fallthrough_ = level_ ? next_emitted(layout, index) : kNoId;
		if (out_.exceptions && fallthrough_ != kNoId)
		{
			const auto next = out_.exceptions->layout_of[fallthrough_];
			if (!out_.exceptions->analysis.blocks_[next].reachable ||
				out_.exceptions->analysis.eh_landing_index_[next] != kNoId)
				fallthrough_ = kNoId;
		}
		if (out_.exceptions)
		{
			const auto& plan = *out_.exceptions;
			if (!plan.analysis.blocks_[plan.layout_of[block]].reachable)
				continue;
			landing(block);
		}
		out_.bind(block);
		entry_moves(block);
		build_held_constants(block);
		if (slow_entry != kNoId && block == body_.layout.front())
		{
			edge(block, slow_entry, true);
			continue;
		}
		for (InstId i = body_.blocks[block].first; i != kNoId; i = body_.instructions[i].next)
		{
			if (elided != entry_elided_.end() && *elided == i)
			{
				++elided;
				continue;
			}
			const auto& inst = body_.instructions[i];
			if (!word_accesses_.empty() && word_accesses_[i] && register_access(inst))
				continue;
			// Late address rewriting can leave integer arithmetic with no
			// remaining readers, and minimum formation a comparison. An
			// increment that a selection makes itself is not computed.
			if (level_ && ((inst.op == Opcode::Binary && inst.type.is_integer()) || inst.op == Opcode::Compare) &&
				(uses_[inst.result] == 0 || chosen_increment(Value::temp(inst.result))))
				continue;
			if (inst.op == Opcode::Call && calls_[call_indices_[i]].tail)
			{
				call(inst);
				i = inst.next;
				continue;
			}
			if (level_ && inst.op == Opcode::Call)
			{
				const auto last = inst.type.is_object() ? projected_call(inst) : branch_on_result(inst, block);
				if (last != kNoId)
				{
					i = last;
					continue;
				}
			}
			if (level_ &&
				((inst.op == Opcode::Unary && inst.sub == static_cast<unsigned>(UnaryOp::BitNot)) ||
					(inst.op == Opcode::Binary && inst.sub == static_cast<unsigned>(BinaryOp::Mul))))
			{
				const auto last = arithmetic_operand(inst);
				if (last != kNoId)
				{
					i = last;
					continue;
				}
			}
			if (level_ && (inst.op == Opcode::Copy || (inst.op == Opcode::Binary && inst.type.is_float())))
			{
				const auto last = magnitude(inst);
				if (last != kNoId)
				{
					i = last;
					continue;
				}
			}
			if (level_ &&
				(inst.op == Opcode::Index || inst.op == Opcode::Binary || inst.op == Opcode::Convert ||
					inst.op == Opcode::Addr))
			{
				InstId last = inst.op != Opcode::Convert ? address_sequence(i)
					: inst.type.kind() == TypeKind::I128 ? wide_high_product(inst)
														 : truncated_operand(inst, block);
				if (last == kNoId && inst.op == Opcode::Convert && inst.type.kind() == TypeKind::I64)
					last = address_sequence(i);
				if (last == kNoId && inst.op == Opcode::Convert && inst.type.kind() == TypeKind::I64)
					last = widening_product(inst);
				if (last == kNoId && inst.op == Opcode::Binary)
					last = bitfield(inst);
				if (last == kNoId && inst.op == Opcode::Binary)
				{
					const auto op = static_cast<BinaryOp>(inst.sub);
					last = op == BinaryOp::And ? bit_branch(inst, block)
						: op == BinaryOp::Sub  ? narrow_range_branch(inst, block)
						: op != BinaryOp::Add  ? shifted_operand(inst, block)
											   : kNoId;
				}
				if (last != kNoId)
				{
					i = last;
					continue;
				}
			}
			if (level_ && (inst.op == Opcode::Load || inst.op == Opcode::Store || inst.op == Opcode::Vector))
			{
				if (inst.op == Opcode::Load && load_returns_directly(inst))
				{
					transfer(true, result_, inst.ops[0], inst.type);
					finish();
					i = inst.next;
					continue;
				}
				auto last = inst.op == Opcode::Vector ? vector_combination(inst) : paired_access(inst);
				if (last == kNoId && inst.op == Opcode::Load && !memory_value(inst.type))
					last = signed_access(inst);
				if (last == kNoId && inst.op == Opcode::Load)
					last = vector_load(inst);
				if (last != kNoId)
				{
					i = last;
					continue;
				}
			}
			if (level_ && inst.op == Opcode::Compare)
			{
				const bool fp = inst.type.is_float();
				auto last = !fp ? compare_select_pair(inst) : kNoId;
				if (!fp && last == kNoId)
					last = compare_pair(inst, block);
				if (last == kNoId)
					last = compare_before_increment(inst, block);
				if (!fp && last == kNoId)
					last = added_condition(inst);
				if (last != kNoId || (fp ? float_consumer(inst, block) : compare_consumer(inst, block)))
				{
					i = last == kNoId ? inst.next : last;
					continue;
				}
			}
			select(inst, block);
		}
	}
	flush_deferred_edges();
	bind_forwarded();
	if (level_ && body_.code_labels.empty())
		forward_reloads(out_, frame_code_.prologue_end, frame_base(), spill_homes_, bit_tests_, named_.darwin);
	if (level_ && !named_.darwin && body_.code_labels.empty())
		pair_accesses(out_, frame_code_.prologue_end, bit_tests_, orders_memory(body_));
	if (level_ && !dynamic_stack_ && body_.code_labels.empty())
		share_edge_moves(out_, bit_tests_);
	// Tail sharing may redirect a branch. Check the final target before
	// choosing the shorter reach of TBZ or TBNZ.
	if (!bit_tests_.empty())
		fold_bit_tests(out_, bit_tests_);
	// Exception regions, a frame record, a dynamic or variadic frame and
	// Darwin's compact unwind keep the prologue at the entry.
	if (level_ && !minimize_size_ && frame_ && slow_entry == kNoId && !named_.darwin && !out_.exceptions &&
		!dynamic_stack_ && frame_pointer_ == kNoId && variadic_area_ == kNoId && tls_vectors_ == kNoId &&
		body_.code_labels.empty() && program_.symbols[symbol_].metadata.tls_for == kNoId)
		shrink_wrap(out_, frame_code_);
	if (level_ >= 2 && !dynamic_stack_ && body_.code_labels.empty())
	{
		copy_short_tests(out_);
		simplify_branches(out_);
		orient_branches(out_, branch_estimates_);
		conditional_compares(out_, block_frequency_);
		return_directly(out_);
		drop_repeated_compares(out_);
	}
	relax_branches(out_);
}

} // namespace aarch64
