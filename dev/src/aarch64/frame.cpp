#include "aarch64/selector.h"
#include "aarch64/logical_immediate.h"

#include <algorithm>
#include "lowir/literals.h"
#include "machine/frame_lifetimes.h"
#include "opt/function_ir.h"
#include "aarch64/error.h"

namespace aarch64 {

using namespace lowir;

Selector::Selector(const Program& program, SymbolId symbol, Function& out, int level, bool pic,
	const std::vector<uint32_t>& leaf_registers, bool minimize_size, bool contract_float)
	: program_(program), symbol_(symbol), body_(*program.functions[program.symbols[symbol].detail].body),
	  signature_(program.signatures[program.functions[program.symbols[symbol].detail].signature]), out_(out),
	  level_(level), pic_(pic), minimize_size_(minimize_size), contract_float_(contract_float),
	  leaf_registers_(leaf_registers), named_(target_model::is_darwin(program.target))
{}

uint32_t Selector::reserve(uint64_t size, uint32_t alignment)
{
	if (size > UINT32_MAX)
		throw AArch64Error("unsupported AArch64 frame alignment or size");
	if (alignment > 16)
	{
		size += alignment - 16;
		alignment = 16;
	}
	const uint64_t start = (uint64_t(frame_) + alignment - 1) & ~uint64_t(alignment - 1);
	if (start + size > INT32_MAX)
		throw AArch64Error("AArch64 stack frame exceeds 2 GiB");
	frame_ = start + size;
	return start;
}

void Selector::plan_slots()
{
	const auto owners = level_ ? machine::sequential_copy_slots(program_, body_) : std::vector<uint32_t>();
	for (uint32_t i = 0; i < body_.slots.size(); ++i)
	{
		const Type type = body_.slots[i].type;
		if (i == result_slot_)
			slots_.push_back(kNoId);
		else if (!owners.empty() && owners[i] < i && owners[i] != result_slot_)
			slots_.push_back(slots_[owners[i]]);
		else
			slots_.push_back(reserve((storage_bytes(type) + 7) & ~uint64_t(7), data_alignment(type)));
	}
}

void Selector::plan()
{
	out_.labels.resize(body_.blocks.size(), kNoId);
	uint32_t outgoing = 0;
	bool has_phi = false;
	result_ = classify(signature_.result, signature_.boundary.result_shape);
	const auto tls_access = [&](Value v) {
		return named_.darwin && v.is(ValueKind::Symbol) &&
			program_.symbols[v.index].metadata.storage == ir_model::GSM_THREAD_LOCAL;
	};
	bool calls = named_.darwin && std::any_of(body_.operands.begin(), body_.operands.end(), tls_access),
		 starts_variadic = false, eh = false;
	bool tail_safe = level_ >= 2 && signature_.boundary.arity != ir_model::CAM_VARIADIC &&
		program_.symbols[symbol_].metadata.tls_for == kNoId;
	bool has_tail = false;
	for (BlockId block : body_.layout)
	{
		for (InstId i = body_.blocks[block].first; i != kNoId; i = body_.instructions[i].next)
		{
			const auto& inst = body_.instructions[i];
			if (named_.darwin && !calls)
				for (const auto operand : inst.ops)
					calls |= tls_access(operand);
			tail_safe &= inst.op != Opcode::Asm && !(inst.op == Opcode::Addr && !inst.ops[0].is(ValueKind::Symbol));
			dynamic_stack_ |= inst.op == Opcode::StackSave || inst.op == Opcode::StackAlloc ||
				inst.op == Opcode::StackRestore;
			if (inst.op == Opcode::FrameAddress)
				frame_pointer_ = 0;
			starts_variadic |= inst.op == Opcode::VaStart;
			eh |= (inst.op >= Opcode::EhTry && inst.op <= Opcode::Resume) || inst.op == Opcode::Exception ||
				inst.op == Opcode::ExceptionSelector;
			calls |= inst.op == Opcode::Resume;
			has_phi |= inst.op == Opcode::Phi;
			if (inst.op == Opcode::Call)
			{
				if (call_indices_.empty())
					call_indices_.resize(body_.instructions.size(), kNoId);
				call_indices_[i] = calls_.size();
				calls_.push_back(call_layout(inst));
				auto& layout = calls_.back();
				layout.block = block;
				if (!leaf_registers_.empty() && inst.ops[0].is(ValueKind::Symbol))
				{
					layout.preserved_callers = leaf_registers_[inst.ops[0].index];
					if (layout.preserved_callers)
						for (const auto& arg : layout.args)
							if (arg.bank == Bank::General)
								layout.preserved_callers &= ~(((1u << arg.pieces) - 1) << arg.first);
				}
				const bool required = inst.sub == lowir::kTailCall;
				layout.tail = required || (tail_safe && tail_compatible(inst, layout));
				has_tail |= layout.tail && !required;
				calls |= !layout.tail;
				outgoing = std::max(outgoing, calls_.back().size);
			}
		}
	}
	if (has_tail && tail_safe && !body_.slots.empty())
	{
		const auto escaped = opt::escaped_slots(body_, true);
		tail_safe = std::none_of(escaped.begin(), escaped.end(), [](uint8_t used) { return used != 0; });
	}
	if (has_tail && (!tail_safe || eh || starts_variadic || dynamic_stack_ || frame_pointer_ != kNoId))
	{
		for (InstId i = 0; i < call_indices_.size(); ++i)
			if (call_indices_[i] != kNoId && body_.instructions[i].sub != lowir::kTailCall)
				calls_[call_indices_[i]].tail = false;
		calls = true;
	}
	outgoing_ = (outgoing + 15) & ~15u;
	reserve(outgoing_, 16);
	if (eh)
		prepare_exceptions();
	if (starts_variadic && !named_.darwin)
		variadic_area_ = reserve(64 + 128, 16);
	if (result_.indirect)
	{
		if (!level_ || calls)
			result_pointer_ = reserve(8, 8);
		if (level_ && !eh && !body_.slots.empty())
		{
			const auto slot = machine::returned_object_slot(body_, signature_.result, {});
			if (slot != kNoId && body_.slots[slot].type == signature_.result && !opt::escaped_slots(body_, true)[slot])
				result_slot_ = slot;
		}
	}
	if (calls || dynamic_stack_ || frame_pointer_ != kNoId)
	{
		if (named_.darwin || frame_pointer_ != kNoId)
		{
			frame_pointer_ = reserve(8, 16);
			preserved_.push_back({ 29, frame_pointer_ });
		}
		preserved_.push_back({ 30, reserve(8, 8) });
	}
	allocate();
	// The existing result-pointer home can save a spare nonvolatile register.
	// The pointer then survives calls without a reload at each output access.
	if (level_ >= 2 && result_pointer_ != kNoId && !eh && !dynamic_stack_ &&
		program_.symbols[symbol_].metadata.tls_for == kNoId)
	{
		unsigned used = 0;
		for (const auto& saved : preserved_)
			used |= 1u << saved.first;
		const unsigned available = (0x3ffu << 19) & ~used;
		if (available)
		{
			result_reg_ = __builtin_ctz(available);
			preserved_.insert(preserved_.begin(), std::make_pair(result_reg_, result_pointer_));
		}
	}
	if (named_.darwin && program_.symbols[symbol_].metadata.tls_for != kNoId)
	{
		// Darwin C++ TLS accessors may call initializers. Their callers
		// retain these GP registers and every scalar SIMD lane across the call.
		for (unsigned reg : { 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 10u, 11u, 12u, 13u, 14u })
			preserved_.push_back({ reg, reserve(8, 8) });
		tls_vectors_ = reserve(32 * 8, 16);
	}
	phis_.reset(body_.instructions.size());
	if (has_phi)
		phi_area_ = reserve(plan_phis(), 8);
	reserve(0, 16);
	stack_pair_ = level_ && frame_ <= 504 && preserved_.size() >= 2 && preserved_[0].second == 0 &&
		preserved_[1].second == 8;
}

void Selector::move(unsigned to, unsigned from)
{
	if (to != from)
		out_.emit(Op::Orr, to, 31, from);
}

void Selector::constant(unsigned reg, uint64_t bits)
{
	if (!level_ || bits < 65536)
	{
		out_.emit(Op::MovZ, reg, 0, 0, (bits & 65535) << 5);
		for (unsigned shift = 16; shift < 64; shift += 16)
			if (const unsigned half = (bits >> shift) & 65535)
				out_.emit(Op::MovK, reg, 0, 0, ((shift / 16) << 21) | (half << 5));
		return;
	}
	const unsigned width = bits >> 32 ? 64 : 32;
	unsigned zeros = 0, ones = 0;
	for (unsigned shift = 0; shift < width; shift += 16)
	{
		const unsigned half = (bits >> shift) & 65535;
		zeros += half == 0;
		ones += half == 65535;
	}
	uint32_t mask = 0;
	if (std::max(zeros, ones) + 1 < width / 16 && logical_immediate(bits, width, mask))
	{
		out_.emit(width == 64 ? Op::OrrImm : Op::OrrImmW, reg, 31, 0, mask);
		return;
	}
	// Reciprocal multipliers often differ from a repeated bit mask in
	// only one halfword. ORR and one MOVK can then replace four moves.
	if (width == 64 && std::max(zeros, ones) <= 1)
	{
		const uint64_t repeated = ((bits >> 32) & 65535) * 0x0001000100010001ull;
		uint64_t changed = bits ^ repeated;
		if (changed && logical_immediate(repeated, 64, mask))
		{
			const unsigned halfword = __builtin_ctzll(changed) / 16;
			if ((changed >> (halfword * 16)) <= 65535)
			{
				out_.emit(Op::OrrImm, reg, 31, 0, mask);
				out_.emit(Op::MovK, reg, 0, 0, (halfword << 21) | (((bits >> (halfword * 16)) & 65535) << 5));
				return;
			}
		}
	}
	const bool invert = ones > zeros;
	const unsigned fill = invert ? 65535 : 0;
	bool first = true;
	for (unsigned shift = 0; shift < width; shift += 16)
	{
		const unsigned half = (bits >> shift) & 65535;
		if (half == fill && (!first || shift + 16 < width))
			continue;
		const Op op = !first ? Op::MovK : !invert ? Op::MovZ : width == 32 ? Op::MovNW : Op::MovN;
		out_.emit(op, reg, 0, 0, ((shift / 16) << 21) | ((first ? half ^ fill : half) << 5));
		first = false;
	}
}

void Selector::save_bank(bool store, bool simd, const std::vector<std::pair<unsigned, uint32_t>>& regs, size_t first)
{
	const auto rule = store ? FrameRule::Save : FrameRule::Restore;
	const unsigned dwarf_base = simd ? 64 : 0;
	for (size_t i = first; i < regs.size(); ++i)
	{
		const auto current = regs[i];
		const uint32_t offset = current.second;
		if (level_ && i + 1 < regs.size() && offset <= 504 && regs[i + 1].second == offset + 8)
		{
			const auto next = regs[++i];
			const Op pair = simd ? (store ? Op::StpD : Op::LdpD) : (store ? Op::StpX : Op::LdpX);
			out_.emit(pair, current.first, 31, 0, (next.first << 10) | ((offset / 8) << 15));
			out_.frame_rule(rule, dwarf_base + next.first, frame_ - next.second);
		}
		else if (simd)
		{
			const bool scaled = offset / 8 <= 4095;
			if (!scaled)
				base_address(16, 31, offset);
			out_.emit(store ? Op::StrD : Op::LdrD, current.first, scaled ? 31 : 16, 0, scaled ? (offset / 8) << 10 : 0);
		}
		else
			access(store, current.first, 31, offset, 8);
		out_.frame_rule(rule, dwarf_base + current.first, frame_ - offset);
	}
}

void Selector::saved_registers(bool store)
{
	save_bank(store, false, preserved_, stack_pair_ ? 2 : 0);
	save_bank(store, true, preserved_fp_, 0);
}

void Selector::stack_adjust(bool subtract)
{
	if (stack_pair_)
	{
		const auto first = preserved_[0].first, second = preserved_[1].first;
		const unsigned words = (subtract ? 0u - frame_ / 8 : frame_ / 8) & 127;
		out_.emit(subtract ? Op::StpXPre : Op::LdpXPost, first, 31, 0, (words << 15) | (second << 10));
		out_.frame_rule(FrameRule::Offset, 31, subtract ? frame_ : 0);
		const auto rule = subtract ? FrameRule::Save : FrameRule::Restore;
		out_.frame_rule(rule, first, frame_);
		out_.frame_rule(rule, second, frame_ - 8);
		return;
	}
	const Op op = subtract ? Op::SubI : Op::AddI;
	uint32_t amount = frame_;
	while (amount >= 4096)
	{
		const uint32_t pages = std::min(amount >> 12, 4095u);
		out_.emit(op, 31, 31, 0, (1u << 22) | (pages << 10));
		amount -= pages << 12;
		out_.frame_rule(FrameRule::Offset, 31, subtract ? frame_ - amount : amount);
	}
	if (amount)
	{
		out_.emit(op, 31, 31, 0, amount << 10);
		out_.frame_rule(FrameRule::Offset, 31, subtract ? frame_ : 0);
	}
}

// SP and the fixed frame base are 16-byte aligned. Over-aligned objects
// reserve padding inside the frame, leaving DWARF save offsets constant.
void Selector::base_address(unsigned reg, unsigned base, uint32_t offset, unsigned alignment)
{
	if (alignment > 16)
		offset += alignment - 16;
	if (offset <= 4095)
		out_.emit(Op::AddI, reg, base, 0, offset << 10);
	else
	{
		const unsigned scratch = reg == base ? 16 : reg;
		constant(scratch, offset);
		out_.emit(Op::Add, reg, base, scratch, 0x206000);
	}
	if (alignment > 16)
	{
		unsigned bits = 0;
		for (; (1u << bits) < alignment; ++bits)
		{
		}
		out_.emit(Op::AndI, reg, reg, 0, ((64 - bits) << 16) | ((63 - bits) << 10));
	}
}

void Selector::result_address(unsigned reg)
{
	if (result_reg_ != kNoId)
		move(reg, result_reg_);
	else if (result_pointer_ == kNoId)
		move(reg, 8);
	else
		access(false, reg, frame_base(), result_pointer_, 8);
}

void Selector::home_address(unsigned reg, TempId temp)
{
	if (!call_result_slots_.empty() && call_result_slots_[temp] != kNoId)
		address(reg, Value::slot(call_result_slots_[temp]));
	else
		base_address(reg, frame_base(), homes_[temp], data_alignment(body_.temps[temp].type));
}

void Selector::stack_alloc(const lowir::Instruction& inst)
{
	const Type size = type_of(inst.ops[0], Type::scalar(TypeKind::I64));
	value(11, inst.ops[0], size);
	out_.emit(Op::AddI, 9, 31);
	out_.emit(Op::Sub, 9, 9, 11);
	out_.emit(Op::AndI, 9, 9, 0, (60u << 16) | (59u << 10));
	out_.emit(Op::AddI, 31, 9);
	// Keep space for this function's outgoing stack arguments at the new SP.
	base_address(9, 9, outgoing_);
	save(9, inst.result);
}

void Selector::extend(unsigned reg, unsigned bits, bool sign)
{
	if (bits != 0 && bits < 64)
		out_.emit(sign ? Op::Sbfm : Op::Ubfm, reg, reg, 0, (bits - 1) << 10);
}

void Selector::access(bool store, unsigned reg, unsigned base, uint32_t offset, unsigned bytes)
{
	Op op;
	switch (bytes)
	{
	case 1:
		op = store ? Op::StrB : Op::LdrB;
		break;
	case 2:
		op = store ? Op::StrH : Op::LdrH;
		break;
	case 4:
		op = store ? Op::StrW : Op::LdrW;
		break;
	case 8:
		op = store ? Op::StrX : Op::LdrX;
		break;
	default:
		throw AArch64Error("unsupported AArch64 scalar memory width");
	}
	if (offset % bytes || offset / bytes >= 4096)
	{
		unsigned scratch = 16;
		if (base == scratch || (store && reg == scratch))
			scratch = (base == 17 || (store && reg == 17)) ? 15 : 17;
		constant(scratch, offset);
		// ADD (extended register), UXTX. This form can use SP as the base.
		out_.emit(Op::Add, scratch, base, scratch, 0x206000);
		base = scratch;
		offset = 0;
	}
	out_.emit(op, reg, base, 0, (offset / bytes) << 10);
}

Type Selector::type_of(Value v, Type fallback) const
{
	if (v.kind == ValueKind::Temp)
		return body_.temps[v.index].type;
	if (v.kind == ValueKind::Slot)
		return body_.slots[v.index].type;
	return fallback;
}

unsigned Selector::address_register(Value operand, unsigned scratch)
{
	if (operand.is(ValueKind::Temp) && registers_[operand.index] != kNoId)
		return registers_[operand.index];
	address(scratch, operand);
	return scratch;
}

void Selector::address(unsigned reg, Value operand)
{
	if (operand.is(ValueKind::Slot) && operand.index == result_slot_)
		result_address(reg);
	else if (operand.kind == ValueKind::Slot)
		base_address(reg, frame_base(), slots_[operand.index], data_alignment(body_.slots[operand.index].type));
	else if (operand.kind == ValueKind::Temp && memory_value(body_.temps[operand.index].type))
		home_address(reg, operand.index);
	else if (operand.kind == ValueKind::Symbol)
	{
		const auto& symbol = program_.symbols[operand.index];
		if (symbol.metadata.storage == ir_model::GSM_THREAD_LOCAL)
		{
			if (named_.darwin)
				darwin_tls_address(reg, operand.index);
			else if (pic_ || symbol.kind != SymbolKind::GlobalDefinition)
			{
				const unsigned tp = reg == 16 ? 17 : 16;
				out_.reference(Op::Adrp, Fixup::TlsPage, operand.index, reg);
				out_.reference(Op::LdrX, Fixup::TlsGotLoad, operand.index, reg, reg);
				out_.emit(Op::ReadThreadPointer, tp);
				out_.emit(Op::Add, reg, tp, reg);
			}
			else
			{
				out_.emit(Op::ReadThreadPointer, reg);
				out_.reference(Op::AddPageI, Fixup::TlsHi, operand.index, reg, reg);
				out_.reference(Op::AddI, Fixup::TlsLo, operand.index, reg, reg);
			}
			return;
		}
		// An absent weak ELF import must have address zero. A direct
		// page-relative reference can instead resolve to the instruction's page.
		const bool weak_import = !named_.darwin && symbol.metadata.binding == ir_model::SBM_WEAK &&
			(symbol.kind == SymbolKind::FunctionDeclaration || symbol.kind == SymbolKind::GlobalDeclaration);
		const bool got = weak_import || (pic_ && symbol.metadata.binding != ir_model::SBM_INTERNAL);
		out_.reference(Op::Adrp, got ? Fixup::GotPage : Fixup::Page, operand.index, reg);
		out_.reference(got ? Op::LdrX : Op::AddI, got ? Fixup::GotLoad : Fixup::PageOffset, operand.index, reg, reg);
	}
	else
		value(reg, operand, program_.pointer_type());
}

void Selector::value(unsigned reg, Value operand, Type type)
{
	if ((!type.is_integer() && !type.is_pointer() && !type.is_float()) || storage_bytes(type) > 8)
		throw AArch64Error("unsupported AArch64 scalar type");
	switch (operand.kind)
	{
	case ValueKind::Integer:
	{
		const auto& literal = body_.literals[operand.index];
		const unsigned bits = integer_bits(type.kind());
		const uint64_t mask = bits && bits < 64 ? (uint64_t(1) << bits) - 1 : UINT64_MAX;
		constant(reg, type.is_float() ? integer_as_float(type.kind(), literal).low : literal.low & mask);
		break;
	}
	case ValueKind::Float:
		constant(reg, body_.literals[operand.index].low);
		break;
	case ValueKind::Temp:
		if (memory_value(body_.temps[operand.index].type))
		{
			address(reg, operand);
			if (!type.is_pointer())
				access(false, reg, reg, 0, storage_bytes(type));
		}
		else if (rematerialized(operand.index))
			address(reg, Value::slot(remat_slots_[operand.index]));
		else if (vectors_[operand.index] != kNoId)
			fp_move(false, vectors_[operand.index], reg, body_.temps[operand.index].type);
		else if (registers_[operand.index] != kNoId)
			move(reg, registers_[operand.index]);
		// A single's home is written four bytes wide from a SIMD register; the
		// load leaves the high bits of its general register clear.
		else
			access(false, reg, frame_base(), homes_[operand.index],
				body_.temps[operand.index].type.kind() == TypeKind::F32 ? 4 : 8);
		break;
	case ValueKind::Slot:
		address(reg, operand);
		access(false, reg, reg, 0, storage_bytes(type));
		break;
	case ValueKind::Symbol:
		address(reg, operand);
		break;
	default:
		throw AArch64Error("unsupported AArch64 operand");
	}
}

void Selector::save(unsigned reg, TempId temp, bool normalized)
{
	const Type type = body_.temps[temp].type;
	if ((!type.is_integer() && !type.is_pointer() && !type.is_float()) || storage_bytes(type) > 8)
		throw AArch64Error("unsupported AArch64 result type");
	const auto bits = integer_bits(type.kind());
	// A loose value's readers take only its low bits (narrow.cpp).
	if (loose(temp))
		normalized = true;
	if (!normalized && bits && bits < 64 && registers_[temp] != kNoId)
	{
		const auto dest = registers_[temp];
		if (bits == 32)
			out_.emit(Op::OrrW, dest, 31, reg);
		else
			out_.emit(Op::Ubfm, dest, reg, 0, (bits - 1) << 10);
		return;
	}
	if (!normalized)
		extend(reg, bits, false);
	if (vectors_[temp] != kNoId)
		fp_move(true, vectors_[temp], reg, type);
	else if (registers_[temp] != kNoId)
		move(registers_[temp], reg);
	else
		access(true, reg, frame_base(), homes_[temp], 8);
}

} // namespace aarch64
