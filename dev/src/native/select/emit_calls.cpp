#include "native/select/function_selector.h"
#include "native/select/for_each_inst.h"

#include <algorithm>
#include <string>

#include "native/select/operand_roles.h"
#include "support/internal_error.h"

// The call boundary: incoming parameters at function entry, outgoing
// argument setup (stack area, then one parallel move into the argument
// registers), call results and returns, per the System V classification
// of native/select/call_abi.h.
namespace native {

using lowir::Instruction;
using lowir::kNoId;
using lowir::Opcode;
using lowir::Type;
using lowir::TypeKind;
using lowir::Value;
using lowir::ValueKind;

namespace {

uint32_t align16(uint32_t value)
{
	return (value + 15) & ~15u;
}

TypeKind move_type(Type type)
{
	if (type.is_pointer())
		return TypeKind::Ptr;
	if (type.is_object() || type.kind() == TypeKind::I128)
		return TypeKind::I64;
	return type.kind();
}

// A boundary's result classes (argument_classes).
uint8_t result_classes(const lowir::Signature* sig)
{
	return sig != nullptr ? sig->boundary.result_classes : 0;
}

} // namespace

void FunctionSelector::classify_signature(const lowir::Signature& sig, CallLayout& layout)
{
	const lowir::Parameter* params = program_.parameters_of(sig);
	arg_types_.clear();
	arg_classes_.clear();
	for (uint32_t i = 0; i < sig.parameter_count; ++i)
	{
		arg_types_.push_back(params[i].type);
		arg_classes_.push_back(params[i].classes);
	}
	classify_call(
		arg_types_.data(), arg_classes_.data(), sig.parameter_count, sig.result, result_classes(&sig), layout);
}

// Every boundary follows the classes (psABI 3.2.3, lowir.md "Object ABI
// Conventions") its LowIR states for object parameters, results and
// variadic arguments (the body's table, in call order: InstIds grow as
// calls are made), in host and course objects and executables alike; an
// object without them is INTEGER throughout.
uint8_t FunctionSelector::argument_classes(uint32_t inst, uint32_t argument, const lowir::Signature* sig) const
{
	if (sig != nullptr && argument < sig->parameter_count)
		return program_.parameters_of(*sig)[argument].classes;
	const std::vector<lowir::ArgumentClasses>& table = body_->argument_classes;
	std::vector<lowir::ArgumentClasses>::const_iterator it = std::lower_bound(table.begin(), table.end(), inst,
		[](const lowir::ArgumentClasses& entry, uint32_t call) { return entry.call < call; });
	for (; it != table.end() && it->call == inst; ++it)
		if (it->argument == argument)
			return it->classes;
	return 0;
}

// The layout of call `inst` with argument types arg_types_.
void FunctionSelector::classify_arguments(uint32_t inst, const lowir::Signature* sig, CallLayout& layout)
{
	const uint32_t count = static_cast<uint32_t>(arg_types_.size());
	arg_classes_.resize(count);
	for (uint32_t a = 0; a < count; ++a)
		arg_classes_[a] = arg_types_[a].is_object() || arg_types_[a].kind() == TypeKind::I128
			? argument_classes(inst, a, sig)
			: 0;
	classify_call(
		arg_types_.data(), arg_classes_.data(), count, body_->instructions[inst].type, result_classes(sig), layout);
}

// The layout of call `inst` (boundary `sig`) for its emission-time
// argument types (call_argument_types; in arg_types_ already when `typed`),
// classified once per function: the body, the boundary and the classes
// stay fixed.
const CallLayout& FunctionSelector::emission_layout(uint32_t inst, const lowir::Signature* sig, bool typed)
{
	if (const CallLayout* known = call_layouts_.known(inst))
		return *known;
	if (!typed)
		call_argument_types(*body_, body_->instructions[inst], *sig, program_.parameters_of(*sig), arg_types_);
	CallLayout& layout = call_layouts_.add(inst);
	classify_arguments(inst, sig, layout);
	return layout;
}

// ---------------------------------------------------------------------------
// Entry

// Register conventions of a function with vector parameters, as the
// reference allocates them: its integer parameters' registers are not
// taken over by a result at their last use, and with two or more integer
// parameters r8 holds no value.
void FunctionSelector::note_parameter_classes()
{
	unsigned gprs = 0;
	unsigned xmms = 0;
	for (size_t p = 0; p < entry_layout_.args.size(); ++p)
	{
		const ArgLayout& arg = entry_layout_.args[p];
		if (arg.part_count != 1 || entry_layout_.parts[arg.first_part].reg == kNoReg)
			continue;
		if (arg.cls == ArgClass::Gpr)
			++gprs;
		else if (arg.cls == ArgClass::Xmm)
			++xmms;
	}
	vector_params_ = xmms != 0;
	excluded_ = vector_params_ && gprs >= 2 ? 1u << R8 : 0;
}

namespace {

// The bits of a narrow integer type, 0 for another.
uint32_t narrow_bits(lowir::TypeKind kind)
{
	switch (kind)
	{
	case lowir::TypeKind::I8:
	case lowir::TypeKind::U8:
		return 8;
	case lowir::TypeKind::I16:
	case lowir::TypeKind::U16:
		return 16;
	case lowir::TypeKind::I32:
	case lowir::TypeKind::U32:
		return 32;
	default:
		return 0;
	}
}

} // namespace

// Whether bitwise `and` instruction `inst` masks with a constant below its
// type's sign bit: its result is the same extended value whatever the
// other operand's bits above the type's width.
bool FunctionSelector::small_mask(const Instruction& inst) const
{
	const uint32_t bits = narrow_bits(inst.type.kind());
	for (unsigned k = 0; k < 2 && bits != 0; ++k)
	{
		const Value v = resolve(inst.ops[k]);
		if (v.kind == ValueKind::Integer)
		{
			const int64_t mask = static_cast<int64_t>(literal(v).low);
			return mask >= 0 && mask < (static_cast<int64_t>(1) << (bits - 1));
		}
	}
	return false;
}

// Host objects: the narrow integer parameters a use may read beyond their
// width (per parameter, in `observed`): any use but a compare (at its
// width), a stored value, a call argument, an add,
// subtract or multiply (whose result is extended again), an `and` with a
// small constant mask or a truncation -- directly, through a value of the
// same type (a promoted slot's load) or through bitwise operations whose
// results only such uses read -- and any use through a value of another
// type (a widening conversion) or a phi.
void FunctionSelector::narrow_params_observed(std::vector<uint8_t>& observed)
{
	const lowir::FunctionBody& body = *body_;
	const uint32_t params = body.parameter_count;
	observed.assign(params, 0);
	// Per temp, the parameters (the first 64; later ones count as read) its
	// bits above its width may come from: a narrow parameter's own, and a
	// bitwise and, or or xor's (whose result is not extended again) from
	// its operands'. Grown to a fixed point, as uses may precede their
	// definitions in layout order (each round adds bits).
	std::vector<uint64_t>& from = narrow_from_;
	from.assign(body.temps.size(), 0);
	for (uint32_t p = 0; p < params; ++p)
		if (narrow_integer(body.temps[p].type.kind()))
		{
			if (p < 64)
				from[p] = static_cast<uint64_t>(1) << p;
			else
				observed[p] = 1;
		}
	const auto mark = [&](uint64_t bits) {
		for (uint32_t p = 0; bits != 0; ++p, bits >>= 1)
			if (bits & 1)
				observed[p] = 1;
	};
	bool depths = false; // loop_depths() run for this function (the allocator's may be another's)
	for (bool grown = true; grown;)
	{
		grown = false;
		for_each_inst([&](uint32_t id, uint32_t b) {
			const Instruction& inst = body.instructions[id];
			if (marks_[id] & IM_DEAD)
				return;
			// A use through a value of another type (a widening conversion
			// the selector aliases) reads the register at that type.
			const auto same = [&](Value v, Value r) {
				return v.kind == ValueKind::Temp && body.temps[v.index].type.kind() == body.temps[r.index].type.kind();
			};
			const auto check = [&](Value v, bool safe) {
				const Value r = resolve(v);
				if (r.kind == ValueKind::Temp && from[r.index] != 0 && (!safe || !same(v, r)))
					mark(from[r.index]);
			};
			if (inst.op == Opcode::Phi)
			{
				for (uint32_t k = 1; k < inst.extra_count; k += 2)
					check(body.operands[inst.extra_first + k], false);
				return;
			}
			const lowir::BinaryOp op = static_cast<lowir::BinaryOp>(inst.sub);
			bool safe = false;
			switch (inst.op)
			{
			case Opcode::Compare:
			{
				// A compare reads its operands at the compare's type, which
				// may be wider than a narrow operand's own (`cmp ne i64 v, 0`
				// is the spelling of a `&&`/`||` operand's truth, lowir.md).
				const unsigned bytes = inst.type.is_integer() ? register_bytes(inst.type.kind()) : 8;
				for (unsigned k = 0; k < 2; ++k)
				{
					const Value v = inst.ops[k];
					const Value r = resolve(v);
					if (r.kind == ValueKind::Temp && from[r.index] != 0 &&
						(!same(v, r) || register_bytes(body.temps[v.index].type.kind()) < bytes))
						mark(from[r.index]);
				}
				return;
			}
			case Opcode::Binary:
				safe = op == lowir::BinaryOp::Add || op == lowir::BinaryOp::Sub || op == lowir::BinaryOp::Mul ||
					(op == lowir::BinaryOp::And && small_mask(inst));
				if (!safe && (op == lowir::BinaryOp::And || op == lowir::BinaryOp::Or || op == lowir::BinaryOp::Xor) &&
					narrow_integer(inst.type.kind()) && inst.result != kNoId)
				{
					uint64_t bits = from[inst.result];
					for (unsigned k = 0; k < 2; ++k)
					{
						const Value r = resolve(inst.ops[k]);
						if (r.kind != ValueKind::Temp || from[r.index] == 0)
							continue;
						if (same(inst.ops[k], r))
							bits |= from[r.index];
						else
							mark(from[r.index]);
					}
					grown = grown || bits != from[inst.result];
					from[inst.result] = bits;
					return;
				}
				break;
			case Opcode::Unary:
				// A bit scan reads its operand at its own width.
				safe = static_cast<lowir::UnaryOp>(inst.sub) == lowir::UnaryOp::CountTrailingZeros ||
					static_cast<lowir::UnaryOp>(inst.sub) == lowir::UnaryOp::CountLeadingZeros;
				break;
			case Opcode::Convert:
				// An extension aliased to its source: its uses read the
				// register (checked there).
				if (inst.result != kNoId && (temps_[inst.result].flags & TF_ALIAS))
					return;
				safe = static_cast<lowir::ConvertOp>(inst.sub) == lowir::ConvertOp::Trunc;
				break;
			// A branch tests its condition at its type; a switch outside
			// loops on a parameter (or an extension of one) extends it first
			// (emit_switch).
			case Opcode::Branch:
				safe = level_ >= 1;
				break;
			case Opcode::Switch:
				if (level_ >= 1 && !depths)
				{
					loop_depths();
					depths = true;
				}
				if (level_ >= 1 && b < ra_depth_.size() && ra_depth_[b] == 0)
				{
					const Value r = resolve(inst.ops[0]);
					if (r.kind == ValueKind::Temp && r.index < params &&
						body.temps[r.index].type.kind() != TypeKind::I1)
						return;
				}
				break;
			case Opcode::Store:
				check(inst.ops[0], true);
				check(inst.ops[1], false);
				return;
			case Opcode::Call:
				check(inst.ops[0], false);
				for (uint32_t k = 0; k < inst.extra_count; ++k)
					check(body.operands[inst.extra_first + k], true);
				return;
			default:
				break;
			}
			for_each_operand(body, inst, [&](Value v, OperandRole) { check(v, safe); });
		});
	}
}

void FunctionSelector::emit_entry()
{
	const lowir::FunctionBody& body = *body_;
	const CallLayout& layout = entry_layout_;
	const lowir::Parameter* params = program_.parameters_of(*signature_);
	out_->variadic = signature_->boundary.arity == ir_model::CAM_VARIADIC;
	out_->return_type = signature_->result;
	for (uint32_t p = 0; p < body.parameter_count; ++p)
	{
		AbiParam param;
		param.name = params[p].name;
		param.type = params[p].type;
		param.first_part = static_cast<uint32_t>(out_->parts.size());
		const ArgLayout& arg = layout.args[p];
		param.part_count = arg.part_count;
		for (uint32_t k = 0; k < arg.part_count; ++k)
		{
			const ArgPart& part = layout.parts[arg.first_part + k];
			AbiPart abi;
			abi.reg = part.reg;
			abi.stack = part.reg == kNoReg ? 16 + part.stack : 0;
			abi.type = arg.part_count > 1 || params[p].type.is_object() ? part_type(part) : params[p].type.kind();
			out_->parts.push_back(abi);
		}
		out_->params.push_back(param);
	}
	if (layout.sret)
	{
		sret_home_ = frame_alloc(8, 8);
		emit2(Op::Store, TypeKind::Ptr, Operand::make_frame(sret_home_), Operand::make_reg(RDI));
	}
	if (out_->variadic)
	{
		va_save_area_ = frame_alloc(176, 16);
		FrameBinding binding;
		binding.kind = BindingKind::Temp;
		binding.name = 0;
		binding.label = "va-register-save";
		binding.offset = va_save_area_;
		binding.type = Type::object(176, 4);
		out_->bindings.push_back(binding);
		for (unsigned k = 0; k < 6; ++k)
			emit2(Op::Store, TypeKind::I64, Operand::make_frame(va_save_area_ + 8 * static_cast<int32_t>(k)),
				Operand::make_reg(kArgGprs[k]));
		// Whole XMM registers (psABI 3.5.7, Figure 3.33): a binary128 or
		// 16-byte vector argument's SSEUP half is read from the save area
		// by any function the va_list reaches, not only by this one.
		for (unsigned k = 0; k < 8; ++k)
			emit2(Op::FMov, TypeKind::I128, Operand::make_frame(va_save_area_ + 48 + 16 * static_cast<int32_t>(k)),
				Operand::make_reg(static_cast<Reg>(XMM0 + k)));
		named_gprs_ = layout.gprs;
		named_xmms_ = layout.xmms;
		named_stack_ = layout.stack_bytes;
	}
	if (eh_model_ == EhModel::Host)
		narrow_params_observed(narrow_observed_);
	const size_t normalized = out_->insts.size();
	uint32_t incoming = 0;
	for (uint32_t p = 0; p < body.parameter_count; ++p)
	{
		const ArgLayout& arg = layout.args[p];
		if (temps_[p].uses != 0 && arg.part_count == 1 && layout.parts[arg.first_part].reg != kNoReg)
		{
			incoming |= 1u << layout.parts[arg.first_part].reg;
			// A host object's callers may be compiled elsewhere: the psABI
			// (3.2.3) leaves the bits above a narrow integer argument's width
			// unspecified (_Bool: above bit 7), and registers here hold
			// integers extended to 64 bits, so one a use may read beyond its
			// width is extended on entry.
			if (eh_model_ == EhModel::Host && temps_[p].cls == VClass::Gpr && !params[p].type.is_object() &&
				narrow_integer(params[p].type.kind()) && narrow_observed_[p])
				normalize(layout.parts[arg.first_part].reg, params[p].type.kind());
		}
	}
	// Block-local parameters that keep their incoming register are bound
	// first, so relocation targets avoid them; every transfer out of an
	// incoming register is then one parallel move.
	for (uint32_t p = 0; p < body.parameter_count; ++p)
	{
		TempInfo& t = temps_[p];
		const ArgLayout& arg = layout.args[p];
		if (t.uses == 0 || t.cls != VClass::Gpr || arg.part_count != 1 || t.greg != kNoReg)
			continue;
		const Reg reg = layout.parts[arg.first_part].reg;
		if (reg != kNoReg && !(t.forbid & (1u << reg)) && !((pinned_ | occupied_) & (1u << reg)) && t.last >= 0)
			bind(p, reg);
	}
	moves_.clear();
	for (uint32_t p = 0; p < body.parameter_count; ++p)
	{
		TempInfo& t = temps_[p];
		const ArgLayout& arg = layout.args[p];
		if (t.uses == 0)
			continue;
		const Type type = params[p].type;
		if (arg.part_count == 0 && arg.cls == ArgClass::None)
		{
			// A host object's empty class takes no register and no stack
			// (psABI 3.2.3: no eightbyte has a class): its own home.
			home_of(p);
			continue;
		}
		if (arg.part_count == 0 || layout.parts[arg.first_part].reg == kNoReg)
		{
			// Passed in memory: loaded once into a hull register; a scalar is
			// otherwise copied through rax to its parameter slot (the
			// reference frame layout); other values keep the incoming area.
			const int32_t incoming = 16 + layout.parts[arg.first_part].stack;
			if (t.greg != kNoReg)
			{
				Move load;
				load.dst = Operand::make_reg(t.greg);
				load.src = Operand::make_frame(incoming);
				load.type = move_type(type);
				moves_.push_back(load);
				t.home = incoming;
			}
			else if (t.cls == VClass::Gpr)
			{
				const int32_t home = home_of(p);
				emit2(Op::Load, move_type(type), Operand::make_reg(RAX), Operand::make_frame(incoming));
				emit2(Op::Store, move_type(type), Operand::make_frame(home), Operand::make_reg(RAX));
				t.flags |= TF_HOMED;
				continue;
			}
			t.home = incoming;
			continue;
		}
		Move move;
		move.type = is_xmm(layout.parts[arg.first_part].reg) ? type.kind() : move_type(type);
		move.src = Operand::make_reg(layout.parts[arg.first_part].reg);
		if (t.cls == VClass::Memory)
		{
			// Each eightbyte from its register into the (padded) home.
			const int32_t home = home_of(p);
			for (uint32_t k = 0; k < arg.part_count; ++k)
			{
				const ArgPart& part = layout.parts[arg.first_part + k];
				move.dst = Operand::make_frame(home + static_cast<int32_t>(part.offset));
				move.src = Operand::make_reg(part.reg);
				move.type = part_type(part);
				moves_.push_back(move);
			}
			continue;
		}
		if (t.greg != kNoReg)
			move.dst = Operand::make_reg(t.greg);
		else if (t.flags & TF_GLOBAL)
			move.dst = Operand::make_frame(home_of(p));
		else if (t.cls == VClass::Xmm)
		{
			// A vector parameter lives in its parameter slot (a conservative
			// spill at the ABI boundary, as the reference lays frames out).
			move.dst = Operand::make_frame(home_of(p));
			t.flags |= TF_HOMED;
		}
		else if (t.reg == layout.parts[arg.first_part].reg)
			continue;
		else
		{
			// The incoming register is clobbered inside the parameter's
			// local interval: relocate it, or keep it in its home.
			const Reg target = allocate(p, incoming);
			if (target != kNoReg)
			{
				bind(p, target);
				move.dst = Operand::make_reg(target);
			}
			else
			{
				move.dst = Operand::make_frame(home_of(p));
				t.flags |= TF_HOMED;
			}
		}
		moves_.push_back(move);
	}
	parallel_moves(moves_);
	if (level_ >= 1)
		fold_entry_extensions(normalized);
}

// O1+: an incoming register extended on entry and then read once, by a
// copy into another register before anything writes it, extends in the
// copy (`mov %esi,%esi; mov %rsi,%r12` -> `mov %esi,%r12d`): the parameter
// lives in the copy's register, the incoming one is dead after it.
void FunctionSelector::fold_entry_extensions(size_t first)
{
	std::vector<Inst>& insts = out_->insts;
	for (size_t z = first; z < insts.size(); ++z)
	{
		Inst& ext = insts[z];
		if ((ext.op != Op::Zext && ext.op != Op::Sext) || ext.nops != 1 || ext.ops[0].kind != OperandKind::Reg)
			continue;
		const Reg r = ext.ops[0].reg;
		size_t reader = insts.size();
		unsigned readers = 0;
		for (size_t i = z + 1; i < insts.size(); ++i)
		{
			const Inst& inst = insts[i];
			bool reads = false, writes = false;
			for (unsigned k = 0; k < inst.nops; ++k)
			{
				const Operand& op = inst.ops[k];
				if (op.kind == OperandKind::Mem && (op.reg == r || op.index == r))
					reads = true;
				else if (op.kind == OperandKind::Reg && op.reg == r)
				{
					const bool destination = k == 0 && inst.op != Op::Store && inst.op != Op::Cmp &&
						inst.op != Op::Test;
					const bool pure_write = destination &&
						(inst.op == Op::Mov || inst.op == Op::Load || inst.op == Op::Lea);
					writes = writes || destination;
					reads = reads || !pure_write;
				}
			}
			if (reads)
			{
				++readers;
				reader = i;
			}
			if (writes || readers > 1)
				break;
		}
		if (readers != 1)
			continue;
		Inst& copy = insts[reader];
		if (copy.op != Op::Mov || copy.nops != 2 || copy.ops[0].kind != OperandKind::Reg || copy.ops[0].reg >= 16 ||
			copy.ops[1].kind != OperandKind::Reg || copy.ops[1].reg != r)
			continue;
		copy.op = ext.op;
		copy.type = ext.type;
		ext.op = Op::Nop;
		ext.nops = 0;
	}
}

// ---------------------------------------------------------------------------
// Calls

// A call's arguments: each stack argument stored into its slot, and the
// parallel moves (moves_) that set the argument registers, rdi for an
// indirect result and al for `...`. A register argument formed after those
// moves goes to `late` (register, address).
void FunctionSelector::stage_call_arguments(const Instruction& inst, const lowir::Signature* sig,
	const lowir::Parameter* params, const CallLayout& layout, uint32_t& arg_mask,
	std::vector<std::pair<Reg, uint32_t>>& late, uint32_t& late_mask)
{
	const lowir::FunctionBody& body = *body_;
	for (uint32_t a = 0; a < inst.extra_count; ++a)
	{
		const ArgLayout& arg = layout.args[a];
		const Type type = arg_types_[a];
		const Value v = resolve(body.operands[inst.extra_first + a]);
		const bool memory_value = type.is_object() || type.kind() == TypeKind::F80 || type.kind() == TypeKind::I128;
		Operand src;
		bool address = false;
		const TypeKind value_type = move_type(type);
		TypeKind src_type = TypeKind::Void;
		if (memory_value)
		{
			src = memory_value_operand(v, type);
		}
		else if (v.kind == ValueKind::Slot)
		{
			// A slot passes its storage to an indirect_result or by_address
			// parameter, otherwise its value, of the slot's type (the
			// validator's rule).
			src = Operand::make_frame(slot_offset_[v.index]);
			address = a < sig->parameter_count && params[a].pass != ir_model::PPM_DIRECT;
		}
		else if (a < sig->parameter_count && params[a].pass == ir_model::PPM_BY_ADDRESS &&
			body.operands[inst.extra_first + a].kind == ValueKind::Temp &&
			!body.temps[body.operands[inst.extra_first + a].index].type.is_pointer())
		{
			// A scalar temporary (or a constant it stands for) passed to an
			// addressable-storage boundary: copy it into fresh storage and
			// pass that storage's address. Never the temp's own home, which
			// the callee could write through.
			const lowir::Temp& passed = body.temps[body.operands[inst.extra_first + a].index];
			const Type scalar = passed.type;
			const uint64_t bytes = (static_cast<uint64_t>(value_bytes(scalar)) + 7) & ~7ull;
			const Operand storage = Operand::make_frame(
				frame_alloc(bytes == 0 ? 8 : bytes, value_align(scalar) < 8 ? 8 : value_align(scalar)));
			add_binding(BindingKind::Temp, passed.name, storage.disp, scalar);
			const Operand from = v.kind == ValueKind::Temp ? temp_location(v.index) : rm_operand(v, scalar, kNoReg);
			if (scalar.is_object())
			{
				Inst& mir = emit2(Op::CopyBytes, TypeKind::Void, storage, from);
				mir.aux = value_bytes(scalar);
				mir.aux2 = value_align(scalar);
			}
			else if (scalar.kind() == TypeKind::F80)
				emit2(Op::FMov, TypeKind::F80, storage, from);
			else if (scalar.kind() == TypeKind::I128)
			{
				Inst& mir = emit2(Op::Wide, TypeKind::I128, storage, from);
				mir.sub = static_cast<uint8_t>(WideOp::Move);
			}
			else
			{
				Move store;
				store.dst = storage;
				store.src = from;
				store.type = move_type(scalar);
				if (from.kind == OperandKind::Mem && v.kind == ValueKind::Temp &&
					value_bytes(body.temps[v.index].type) < value_bytes(scalar))
					store.src_type = move_type(body.temps[v.index].type);
				emit_move(store);
			}
			src = storage;
			address = true;
		}
		else if (v.kind == ValueKind::Temp && (temps_[v.index].flags & TF_LAZY) && temps_[v.index].addr != kNoId)
		{
			// An address that holds no register (find_addresses: a frame
			// slot's, or a register's plus a displacement): formed as the
			// argument. With its base register waiting in its home, a
			// register argument's forms after the moves, in the argument's
			// register.
			const AddrExpr& expr = addrs_[temps_[v.index].addr];
			if (expr.kind == AddrExpr::Temp && arg.cls != ArgClass::Memory &&
				temp_location(expr.base).kind != OperandKind::Reg)
			{
				const Reg reg = layout.parts[arg.first_part].reg;
				late.push_back(std::make_pair(reg, temps_[v.index].addr));
				late_mask |= 1u << reg;
				arg_mask |= 1u << reg;
				continue;
			}
			src = lazy_operand(expr, arg.cls == ArgClass::Memory ? kSelectScratch : kNoReg);
			address = true;
		}
		else if (v.kind == ValueKind::Temp)
		{
			// Every argument is read in one parallel move: name the temp's
			// own location (read at its own width and written at the
			// argument's), never a shared scratch.
			src = temp_location(v.index);
			if (src.kind == OperandKind::Mem && !memory_value)
				src_type = move_type(body.temps[v.index].type);
		}
		else
			src = rm_operand(v, type, kSelectScratch);
		if (arg.cls == ArgClass::Memory)
		{
			const ArgPart& part = layout.parts[arg.first_part];
			const Operand slot = Operand::make_mem(RSP, part.stack);
			if (type.is_object())
			{
				Inst& mir = emit2(Op::CopyBytes, TypeKind::Void, slot, src);
				mir.aux = part.size;
				mir.aux2 = 8;
			}
			else if (type.kind() == TypeKind::F80)
				emit2(Op::FMov, TypeKind::F80, slot, src);
			else if (type.kind() == TypeKind::I128)
			{
				Inst& mir = emit2(Op::Wide, TypeKind::I128, slot, src);
				mir.sub = static_cast<uint8_t>(WideOp::Move);
			}
			else
			{
				Move move;
				move.dst = slot;
				move.src = src;
				move.type = value_type;
				move.src_type = src_type;
				move.address = address;
				// A constant is stored after the register arguments are set,
				// through the encoder scratch.
				if (!address && (src.kind == OperandKind::Imm || src.kind == OperandKind::Symbol))
					stack_constants_.push_back(move);
				else
					emit_move(move);
			}
			continue;
		}
		for (uint32_t k = 0; k < arg.part_count; ++k)
		{
			const ArgPart& part = layout.parts[arg.first_part + k];
			Move move;
			move.dst = Operand::make_reg(part.reg);
			move.type = memory_value ? part_type(part) : value_type;
			move.src_type = src_type;
			move.address = address;
			if (memory_value && src.kind == OperandKind::Imm)
			{
				const int64_t bits = part.offset == 0 ? src.imm : src.imm_high;
				move.src = Operand::make_imm(bits);
				if (is_xmm(part.reg))
				{
					lowir::Literal literal = lowir::Literal();
					literal.low = static_cast<uint64_t>(bits);
					move.src = Operand::make_fimm(move.type, literal);
				}
			}
			else if (memory_value)
				move.src = src.displaced(static_cast<int32_t>(part.offset));
			else
				move.src = src;
			arg_mask |= 1u << part.reg;
			moves_.push_back(move);
		}
	}
	// Integer argument registers are set before vector ones.
	std::stable_partition(moves_.begin(), moves_.end(),
		[](const Move& m) { return !(m.dst.kind == OperandKind::Reg && is_xmm(m.dst.reg)); });
	if (layout.sret)
	{
		Move move;
		move.dst = Operand::make_reg(RDI);
		move.src = Operand::make_frame(home_of(inst.result));
		move.address = true;
		arg_mask |= 1u << RDI;
		moves_.push_back(move);
	}
	if (sig->boundary.arity == ir_model::CAM_VARIADIC)
	{
		Move move;
		move.dst = Operand::make_reg(RAX);
		move.src = Operand::make_imm(layout.xmms);
		moves_.push_back(move);
		arg_mask |= 1u << RAX;
	}
}

void FunctionSelector::emit_call(uint32_t id)
{
	const lowir::FunctionBody& body = *body_;
	const Instruction& inst = body.instructions[id];
	// The runtime's trap (__builtin_trap) is the trap instruction itself.
	if (inst.ops[0].kind == ValueKind::Symbol && program_.trap_function(inst.ops[0].index))
	{
		emit(Op::Trap);
		return;
	}
	const lowir::Signature* sig = program_.call_boundary(inst);
	if (sig == nullptr)
		throw InternalError("indirect call without a signature");
	const lowir::Parameter* params = program_.parameters_of(*sig);
	// A long double literal passed to `...` is X87 class (psABI 3.2.3:
	// memory, 16-aligned).
	call_argument_types(body, inst, *sig, params, arg_types_);
	const CallLayout& layout = emission_layout(id, sig, true);
	const uint32_t stack_bytes = align16(layout.stack_bytes);
	const Operand rsp = Operand::make_reg(RSP);
	// psABI 3.2.3: the argument area of an argument aligned beyond 16 bytes
	// is aligned as it is (a callee's va_arg rounds the overflow area's
	// address up): rsp realigned below the frame for the call, and restored
	// from a frame slot after it.
	const Operand saved_rsp = layout.stack_align > 16 ? Operand::make_frame(frame_alloc(8, 8)) : Operand();
	if (layout.stack_align > 16)
		emit2(Op::Store, TypeKind::I64, saved_rsp, rsp);
	if (stack_bytes != 0)
		emit2(Op::Sub, TypeKind::I64, rsp, Operand::make_imm(stack_bytes));
	if (layout.stack_align > 16)
		emit2(Op::And, TypeKind::I64, rsp, Operand::make_imm(-static_cast<int64_t>(layout.stack_align)));
	moves_.clear();
	stack_constants_.clear();
	uint32_t arg_mask = 0;
	// Register arguments formed after the moves: (register, address).
	std::vector<std::pair<Reg, uint32_t>> late;
	uint32_t late_mask = 0;
	stage_call_arguments(inst, sig, params, layout, arg_mask, late, late_mask);
	const bool variadic = sig->boundary.arity == ir_model::CAM_VARIADIC;
	Operand target;
	const Value callee = resolve(inst.ops[0]);
	// A call through the address of a global variable calls the pointer
	// stored in that cell.
	uint32_t cell = kNoId;
	int32_t cell_offset = 0;
	if (callee.kind == ValueKind::Symbol && lowir::is_global_kind(program_.symbols[callee.index].kind))
		cell = callee.index;
	else if (callee.kind == ValueKind::Temp && temps_[callee.index].addr != kNoId)
	{
		const AddrExpr& expr = addrs_[temps_[callee.index].addr];
		if (expr.kind == AddrExpr::Symbol && expr.index == kNoId &&
			lowir::is_global_kind(program_.symbols[expr.base].kind))
		{
			cell = expr.base;
			cell_offset = static_cast<int32_t>(expr.disp);
		}
	}
	if (cell != kNoId && symbol_access(cell) != SymbolAccess::Direct)
	{
		// An imported cell's address forms in the encoder scratch with the
		// register arguments (the parallel moves keep it).
		Move move;
		move.dst = Operand::make_reg(kEmitScratch);
		move.src = symbol_operand(cell, cell_offset);
		moves_.push_back(move);
		target = Operand::make_mem(kEmitScratch, 0);
	}
	else if (cell != kNoId)
		target = Operand::make_symbol_mem(cell, cell_offset);
	else if (callee.kind == ValueKind::Symbol)
		target = Operand::make_symbol(callee.index);
	else
	{
		target = rm_operand(callee, Type::scalar(TypeKind::Ptr), kSelectScratch);
		bool overwritten = false;
		for (size_t m = 0; m < moves_.size(); ++m)
			if (target.uses_reg(moves_[m].dst.reg))
				overwritten = true;
		for (Reg r = 0; r < 16; ++r)
			if ((late_mask & (1u << r)) && target.uses_reg(r))
				overwritten = true;
		if (target.kind == OperandKind::Imm || target.kind == OperandKind::Symbol || overwritten)
		{
			Move move;
			move.dst = Operand::make_reg(kEmitScratch);
			move.src = target;
			moves_.push_back(move);
			target = Operand::make_reg(kEmitScratch);
		}
	}
	const bool scratch_target = target.uses_reg(kEmitScratch);
	for (size_t k = 0; k < stack_constants_.size() && scratch_target; ++k)
		emit_move(stack_constants_[k]);
	parallel_moves(moves_);
	for (const std::pair<Reg, uint32_t>& arg : late)
	{
		const AddrExpr& expr = addrs_[arg.second];
		load_into(arg.first, temp_location(expr.base), TypeKind::Ptr);
		emit2(Op::Lea, TypeKind::Ptr, Operand::make_reg(arg.first),
			Operand::make_mem(arg.first, static_cast<int32_t>(expr.disp)));
	}
	for (size_t k = 0; k < stack_constants_.size() && !scratch_target; ++k)
	{
		const Move& move = stack_constants_[k];
		emit2(Op::Mov, TypeKind::Void, Operand::make_reg(kEmitScratch), move.src);
		emit2(Op::Store, move.type, move.dst, Operand::make_reg(kEmitScratch));
	}
	// The course runtime's builtin memcpy is the string copy itself
	// (runtime/runtime.cpp): a direct call whose result is unused copies
	// in place, its arguments in the call's registers (host objects call
	// the C library's).
	if (eh_model_ == EhModel::Course && callee.kind == ValueKind::Symbol && cell == kNoId && !variadic &&
		inst.extra_count == 3 && sig->parameter_count == 3 && params[0].type.is_pointer() &&
		params[1].type.is_pointer() && params[2].type.kind() == TypeKind::I64 &&
		params[0].pass == ir_model::PPM_DIRECT && params[1].pass == ir_model::PPM_DIRECT &&
		params[2].pass == ir_model::PPM_DIRECT && sig->result.is_pointer() &&
		(inst.result == kNoId || (temps_[inst.result].flags & TF_DEAD) || temps_[inst.result].uses == 0) &&
		program_.builtin_memcpy(callee.index))
	{
		emit(Op::CopyBytes).flags = MF_COPY_DYNAMIC;
		++stats_.dynamic_copies;
		return;
	}
	Inst& call = emit1(Op::Call, TypeKind::Void, target);
	call.aux = arg_mask;
	call.aux2 = layout.stack_bytes;
	if (variadic)
		call.flags |= MF_VARIADIC;
	if (sig->boundary.unwind == ir_model::CUM_NO)
		call.flags |= MF_UNWIND_NO;
	if (sig->boundary.returns == ir_model::CRM_NORETURN)
		call.flags |= MF_NORETURN;
	// O1+: a direct strlen call of one pointer (in rdi) with a 64-bit
	// result first probes the string's first 16 bytes inline and calls
	// only when they hold no NUL (the probe writes rax, xmm0 and xmm1,
	// which the call clobbers anyway). The course runtime's strlen is a
	// byte loop; host objects call the C library's vectorized strlen,
	// where the probe ran 22% slower on 16-47 byte strings (and 28% faster
	// on short ones), so they keep the plain call.
	if (level_ >= 1 && eh_model_ == EhModel::Course && cell == kNoId && callee.kind == ValueKind::Symbol && !variadic &&
		inst.extra_count == 1 && sig->parameter_count == 1 && params[0].type.is_pointer() &&
		params[0].pass == ir_model::PPM_DIRECT && sig->result.kind() == TypeKind::I64 && !(call.flags & MF_NORETURN) &&
		layout.stack_bytes == 0 && program_.strlen_function(callee.index))
	{
		call.flags |= MF_STRLEN_PREFIX;
		++stats_.strlen_prefixes;
	}
	if (eh_model_ == EhModel::Host)
		call.eh = eh_sites_[id];
	if (layout.stack_align > 16)
		emit2(Op::Load, TypeKind::I64, rsp, saved_rsp);
	else if (stack_bytes != 0)
		emit2(Op::Add, TypeKind::I64, rsp, Operand::make_imm(stack_bytes));
	if (inst.result == kNoId || (temps_[inst.result].flags & TF_DEAD) || temps_[inst.result].uses == 0)
	{
		// An unused f80 (or long double object) result still occupies st0
		// (a complex long double's st1 too): pop it (the x87 stack has eight
		// registers).
		if (layout.result == ArgClass::X87 || layout.result == ArgClass::X87Pair)
			emit(Op::FStp, TypeKind::F80);
		if (layout.result == ArgClass::X87Pair)
			emit(Op::FStp, TypeKind::F80);
		return;
	}
	const TempInfo& t = temps_[inst.result];
	const TypeKind kind = inst.type.kind();
	if (layout.result == ArgClass::Memory)
		return;
	if (t.cls == VClass::Memory)
	{
		// O1+: a result in registers whose only use is a copy right after
		// the call (past folded address arithmetic) goes straight to the copy's
		// destination (no home, no copy: and no 16-byte reload of two 8-byte
		// stores). Only when its eightbytes are all its bytes: a part store
		// may not write past it.
		const lowir::FunctionBody& body = *body_;
		const Instruction* copy = t.uses == 1 && t.user != kNoId ? &body.instructions[t.user] : nullptr;
		bool adjacent = copy != nullptr && inst_block_[t.user] == block_ && pos_[t.user] > pos_[id];
		for (lowir::InstId i = inst.next; adjacent && i != t.user; i = body.instructions[i].next)
			adjacent = i != kNoId &&
				(body.instructions[i].op == Opcode::Addr || body.instructions[i].op == Opcode::Index) &&
				(temps_[body.instructions[i].result].flags & (TF_LAZY | TF_DEAD)) != 0;
		if (level_ >= 1 && adjacent && copy->op == Opcode::CopyObject &&
			resolve(copy->ops[0]) == Value::temp(inst.result) &&
			static_cast<uint64_t>(copy->immediate) == value_bytes(inst.type) &&
			value_bytes(inst.type) == 8u * layout.result_parts && layout.result != ArgClass::X87 &&
			layout.result != ArgClass::X87Pair && !(resolve(copy->ops[1]) == Value::temp(inst.result)))
		{
			const Operand dst = address_operand(copy->ops[1], kSelectScratch);
			if (dst.kind == OperandKind::Mem)
			{
				for (uint32_t k = 0; k < layout.result_parts; ++k)
				{
					const ArgPart& part = layout.result_part[k];
					const Operand to = dst.displaced(static_cast<int32_t>(part.offset));
					if (is_xmm(part.reg))
						emit2(Op::FMov, part_type(part), to, Operand::make_reg(part.reg));
					else
						emit2(Op::Store, TypeKind::I64, to, Operand::make_reg(part.reg));
				}
				temps_[inst.result].flags |= TF_STORED;
				return;
			}
		}
		// The result's eightbytes from their registers into its home.
		const Operand home = memory_result(inst);
		if (layout.result == ArgClass::X87)
			emit1(Op::FStp, TypeKind::F80, home);
		else if (layout.result == ArgClass::X87Pair)
		{
			emit1(Op::FStp, TypeKind::F80, home);
			emit1(Op::FStp, TypeKind::F80, home.displaced(16));
		}
		else
			for (uint32_t k = 0; k < layout.result_parts; ++k)
			{
				const ArgPart& part = layout.result_part[k];
				const Operand to = home.displaced(static_cast<int32_t>(part.offset));
				if (is_xmm(part.reg))
					emit2(Op::FMov, part_type(part), to, Operand::make_reg(part.reg));
				else
					emit2(Op::Store, TypeKind::I64, to, Operand::make_reg(part.reg));
			}
		return;
	}
	if (t.cls == VClass::Xmm)
	{
		const Reg d = result_reg(inst, XMM0);
		load_into(d, Operand::make_reg(XMM0), kind);
		finish_result(inst, d);
		return;
	}
	// A straight-line function keeps the result in rax; with more blocks it
	// moves to an allocated register, as the reference places it. A block-
	// local result feeding one two-address operation goes to its home when
	// the call's arguments occupy every general caller-saved register.
	const uint32_t general = ((1u << R8) | (1u << R9) | (1u << RDI) | (1u << RSI)) & ~excluded_;
	const uint32_t caller_saved = kCallerSavedGprs & kAllocatableGprs;
	if (t.greg == kNoReg && !(t.flags & TF_GLOBAL) && (t.forbid & caller_saved) != caller_saved &&
		(general & ~arg_mask) == 0 && t.uses == 1 && t.user != kNoId && body.instructions[t.user].op == Opcode::Binary)
	{
		normalize(RAX, kind);
		temps_[inst.result].flags |= TF_HOMED;
		finish_result(inst, RAX);
		return;
	}
	// A result stored or returned by the next instruction stays in rax too.
	bool terminal = false, stored = false;
	if (t.uses == 1 && t.user != kNoId && inst_block_[t.user] == block_ && pos_[t.user] == position_ + 1)
	{
		const Instruction& user = body.instructions[t.user];
		const Value self = Value::temp(inst.result);
		stored = user.op == Opcode::Store && user.type == inst.type && resolve(user.ops[0]) == self &&
			!(resolve(user.ops[1]) == self);
		terminal = stored || (user.op == Opcode::Return && user.type == inst.type);
	}
	// O1+: a callee this unit defines (no other object's definition can
	// replace it) returns a narrow result extended, as every register here
	// holds one (a returned narrow parameter is extended on entry).
	const Value target_symbol = resolve(inst.ops[0]);
	bool own = false;
	if (level_ >= 1 && target_symbol.kind == ValueKind::Symbol)
	{
		const lowir::Symbol& s = program_.symbols[target_symbol.index];
		own = s.kind == lowir::SymbolKind::FunctionDefinition && s.metadata.binding != ir_model::SBM_WEAK &&
			(s.metadata.flags & lowir::SF_REPLACEABLE) == 0;
	}
	// O1+ prefers rax wherever no instruction of the result's interval
	// clobbers it. A byte result only a branch reads (which tests its byte)
	// needs no normalization.
	const Reg d = result_reg(inst, level_ >= 1 || body.layout.size() == 1 || terminal ? RAX : kNoReg);
	load_into(d, Operand::make_reg(RAX), TypeKind::I64);
	const bool byte_tested = level_ >= 1 && register_bytes(kind) == 1 && t.uses == 1 && t.user != kNoId &&
		body.instructions[t.user].op == Opcode::Branch && !(t.flags & (TF_GLOBAL | TF_HOMED)) && d != kSelectScratch &&
		d != kEmitScratch;
	if (!stored && !byte_tested && !own && !(level_ >= 1 && byte_only_[inst.result]))
		normalize(d, kind);
	finish_result(inst, d);
}

// ---------------------------------------------------------------------------
// Returns and variadic state

void FunctionSelector::emit_return(const Instruction& inst)
{
	const Type type = inst.type;
	const TypeKind kind = type.kind();
	if (type.is_void())
	{
		emit(Op::Ret);
		return;
	}
	const Value v = resolve(inst.ops[0]);
	if (v.kind == ValueKind::Temp && type.is_float() && body_->temps[v.index].type.is_float() &&
		body_->temps[v.index].type.kind() != kind)
	{
		// A floating return of another floating width converts implicitly.
		const TypeKind from = body_->temps[v.index].type.kind();
		const ConvKind conv = lowir::float_rank(kind) > lowir::float_rank(from) ? ConvKind::FPExt : ConvKind::FPTrunc;
		needs_scratch_area_ = true;
		if (kind == TypeKind::F80)
		{
			const Operand home = Operand::make_frame(frame_alloc(16, 16));
			Inst& mir = emit2(Op::Convert, from, home, temp_location(v.index));
			mir.type2 = kind;
			mir.sub = static_cast<uint8_t>(conv);
			emit1(Op::FRet, kind, home);
			return;
		}
		Inst& mir = emit2(Op::Convert, from, Operand::make_reg(XMM0), temp_location(v.index));
		mir.type2 = kind;
		mir.sub = static_cast<uint8_t>(conv);
		emit1(Op::Ret, kind, Operand::make_reg(XMM0));
		return;
	}
	if (kind == TypeKind::F80)
	{
		emit1(Op::FRet, kind, rm_operand(v, type, kSelectScratch));
		return;
	}
	if (type.is_object() || kind == TypeKind::I128)
	{
		const Operand src = memory_value_operand(v, type);
		const uint32_t bytes = value_bytes(type);
		const CallLayout& layout = entry_layout_;
		if (type.is_object() && layout.result == ArgClass::X87Pair)
		{
			// A host object's complex long double: its imaginary part into
			// st1, its real part into st0.
			emit2(Op::FRet, TypeKind::F80, src, src.displaced(16));
			return;
		}
		if (bytes > 16 || (type.is_object() && layout.result == ArgClass::Memory))
		{
			emit2(Op::Load, TypeKind::Ptr, Operand::make_reg(RAX), Operand::make_frame(sret_home_));
			Inst& copy = emit2(Op::CopyBytes, TypeKind::Void, Operand::make_reg(RAX), src);
			copy.aux = bytes;
			copy.aux2 = value_align(type);
			emit1(Op::Ret, TypeKind::Void, Operand::make_reg(RAX));
			return;
		}
		if (type.is_object() && layout.result == ArgClass::X87)
		{
			// A host object's long double object returns in st0.
			if (src.kind == OperandKind::Imm)
			{
				lowir::Literal literal = lowir::Literal();
				literal.low = static_cast<uint64_t>(src.imm);
				literal.high = static_cast<uint64_t>(src.imm_high);
				emit1(Op::FRet, TypeKind::F80, Operand::make_fimm(TypeKind::F80, literal));
			}
			else
				emit1(Op::FRet, TypeKind::F80, src);
			return;
		}
		if (type.is_object() && layout.result == ArgClass::Mixed)
		{
			// A host object's classified eightbytes into rax/rdx, xmm0/xmm1.
			for (uint32_t k = 0; k < layout.result_parts; ++k)
			{
				const ArgPart& part = layout.result_part[k];
				const Operand to = Operand::make_reg(part.reg);
				if (src.kind == OperandKind::Imm)
				{
					lowir::Literal literal = lowir::Literal();
					literal.low = static_cast<uint64_t>(part.offset == 0 ? src.imm : src.imm_high);
					if (is_xmm(part.reg))
						emit2(Op::FMov, part_type(part), to, Operand::make_fimm(part_type(part), literal));
					else
						emit2(Op::Mov, TypeKind::Void, to, Operand::make_imm(static_cast<int64_t>(literal.low)));
				}
				else if (is_xmm(part.reg))
					emit2(Op::FMov, part_type(part), to, src.displaced(static_cast<int32_t>(part.offset)));
				else
				{
					// A last eightbyte of at most four bytes loads as such: a
					// 4-byte store of it forwards (the upper half is padding).
					const bool half = level_ >= 1 && bytes - part.offset <= 4;
					emit2(Op::Load, half ? TypeKind::U32 : TypeKind::I64, to,
						src.displaced(static_cast<int32_t>(part.offset)));
				}
			}
			emit(Op::Ret);
			return;
		}
		if (src.kind == OperandKind::Imm)
		{
			emit2(Op::Mov, TypeKind::Void, Operand::make_reg(RAX), Operand::make_imm(src.imm));
			emit2(Op::Mov, TypeKind::Void, Operand::make_reg(RDX), Operand::make_imm(src.imm_high));
		}
		else
		{
			emit2(Op::Load, TypeKind::I64, Operand::make_reg(RAX), src);
			if (bytes > 8)
				emit2(Op::Load, TypeKind::I64, Operand::make_reg(RDX), src.displaced(8));
		}
		emit(Op::Ret);
		return;
	}
	const Operand src = rm_operand(v, type, kSelectScratch);
	if (type.is_float())
	{
		if (src.kind == OperandKind::Reg)
		{
			emit1(Op::Ret, kind, src);
			return;
		}
		load_into(XMM0, src, kind);
		emit1(Op::Ret, kind, Operand::make_reg(XMM0));
		return;
	}
	// O1+: a narrow result returns extended at its own width (callers in
	// this unit read it so: emit_call), also from a value of another width.
	if (level_ >= 1 && src.kind == OperandKind::Reg && v.kind == ValueKind::Temp && narrow_integer(kind) &&
		body_->temps[v.index].type.kind() != kind && (v.index >= fits_.size() || fits_[v.index] > canonical_bits(kind)))
	{
		load_into(RAX, src, TypeKind::I64);
		normalize(RAX, kind);
		emit1(Op::Ret, kind, Operand::make_reg(RAX));
		return;
	}
	if (src.kind == OperandKind::Reg)
	{
		emit1(Op::Ret, kind, src);
		return;
	}
	if (src.kind == OperandKind::Mem)
		emit2(Op::Load, move_type(type), Operand::make_reg(RAX), src);
	else
		load_into(RAX, src, TypeKind::I64);
	emit1(Op::Ret, kind, Operand::make_reg(RAX));
}

void FunctionSelector::emit_va_start(const Instruction& inst)
{
	if (va_save_area_ == 0)
		throw InternalError("va_start outside a variadic function");
	const Operand list = address_operand(inst.ops[0], kSelectScratch);
	Inst& mir = emit2(Op::VaStart, TypeKind::Void, list, Operand::make_frame(va_save_area_));
	mir.aux = (8 * named_gprs_) | ((48 + 16 * named_xmms_) << 16);
	mir.aux2 = 16 + named_stack_;
}

void FunctionSelector::emit_va_arg_object(const Instruction& inst, uint8_t classes, const Operand& list)
{
	// Another argument read into its memory home (psABI 3.5.7): the
	// eightbytes of an object passed in registers (its stated classes; an
	// object without them, the course's boundary's, each INTEGER), all
	// from the register save area if they fit, else the whole object, as a
	// MEMORY or X87 one always, from the overflow area at the alignment
	// call_abi's stack placement gives it (a member function pointer's
	// 8, an __int128's and a long double's 16). MIR: classes in `sub`
	// (OC_MEMORY for the overflow area alone), bytes in `aux`, overflow
	// alignment in `aux2`.
	const Type type = inst.type;
	const uint32_t bytes = value_bytes(type);
	uint32_t align = value_align(type);
	if (type.kind() == TypeKind::F80)
		classes = lowir::OC_STATED | lowir::OC_X87;
	else if (type.kind() == TypeKind::I128 && classes != 0)
		align = 8;
	if (classes == 0)
		classes = bytes > 16 ? lowir::OC_STATED | lowir::OC_MEMORY
							 : static_cast<uint8_t>(lowir::OC_STATED | lowir::EB_INTEGER |
								   (bytes > 8 ? lowir::EB_INTEGER << 2 : lowir::EB_NONE));
	if (bytes > 16 || (classes & (lowir::OC_MEMORY | lowir::OC_X87 | lowir::OC_COMPLEX_X87)) != 0)
		classes = lowir::OC_STATED | lowir::OC_MEMORY;
	Inst& mir = emit2(Op::VaArg, TypeKind::Obj, memory_result(inst), list);
	mir.sub = classes;
	mir.aux = bytes;
	mir.aux2 = align < 8 ? 8 : align;
}

} // namespace native
