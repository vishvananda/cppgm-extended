#include "native/select/function_selector.h"

#include <algorithm>
#include <functional>

#include "lowir/literals.h"
#include "native/backend_error.h"
#include "support/internal_error.h"

// Operand resolution: the MIR operand naming a LowIR value's current
// location (register, frame home, immediate, symbol), memory operands for
// load/store addresses including lazily folded address expressions, and
// the parallel-move resolver used for call arguments, phi edge transfers
// and parameter relocation.
namespace native {

using lowir::Instruction;
using lowir::kNoId;
using lowir::Literal;
using lowir::Type;
using lowir::TypeKind;
using lowir::Value;
using lowir::ValueKind;

bool operator==(const Operand& a, const Operand& b)
{
	if (a.kind != b.kind)
		return false;
	switch (a.kind)
	{
	case OperandKind::None:
		return true;
	case OperandKind::Reg:
		return a.reg == b.reg;
	case OperandKind::Imm:
	case OperandKind::FImm:
		return a.imm == b.imm && a.imm_high == b.imm_high && a.type == b.type;
	case OperandKind::Mem:
		return a.reg == b.reg && a.index == b.index && a.scale == b.scale && a.disp == b.disp && a.symbol == b.symbol;
	case OperandKind::Symbol:
		return a.symbol == b.symbol && a.disp == b.disp;
	case OperandKind::Label:
		return a.symbol == b.symbol;
	}
	return false;
}

// ---------------------------------------------------------------------------
// MIR construction

Inst& FunctionSelector::emit(Op op, TypeKind type)
{
	out_->insts.push_back(Inst());
	Inst& inst = out_->insts.back();
	inst.op = op;
	inst.type = type;
	inst.location = location_;
	return inst;
}

Inst& FunctionSelector::emit1(Op op, TypeKind type, const Operand& a)
{
	Inst& inst = emit(op, type);
	inst.ops[0] = a;
	inst.nops = 1;
	return inst;
}

Inst& FunctionSelector::emit2(Op op, TypeKind type, const Operand& a, const Operand& b)
{
	Inst& inst = emit(op, type);
	inst.ops[0] = a;
	inst.ops[1] = b;
	inst.nops = 2;
	return inst;
}

Inst& FunctionSelector::emit3(Op op, TypeKind type, const Operand& a, const Operand& b, const Operand& c)
{
	Inst& inst = emit(op, type);
	inst.ops[0] = a;
	inst.ops[1] = b;
	inst.ops[2] = c;
	inst.nops = 3;
	return inst;
}

void FunctionSelector::begin_mir_block(lowir::NameId name, uint32_t edge_from, uint32_t edge_to)
{
	Block block;
	block.name = name;
	block.edge_from = edge_from;
	block.edge_to = edge_to;
	block.first = static_cast<uint32_t>(out_->insts.size());
	block.count = 0;
	block.landing = false;
	out_->blocks.push_back(block);
}

void FunctionSelector::end_mir_block()
{
	Block& block = out_->blocks.back();
	block.count = static_cast<uint32_t>(out_->insts.size()) - block.first;
}

// ---------------------------------------------------------------------------
// Values

Operand FunctionSelector::memory_value_operand(Value value, Type type)
{
	// Memory-class values can be stored contents or numeric bits. A symbol
	// is an address; its ID must never be used to index the literal pool.
	switch (value.kind)
	{
	case ValueKind::Slot:
		return Operand::make_frame(slot_offset_[value.index]);
	case ValueKind::Temp:
		return temp_location(value.index);
	case ValueKind::Integer:
	case ValueKind::Float:
		return imm_for(value, type);
	default:
		throw BackendError("operand requires a numeric literal, not a storage location or symbol");
	}
}

Operand FunctionSelector::imm_for(Value value, Type type) const
{
	const Literal& bits = literal(value);
	if (type.is_float())
	{
		if (value.kind == ValueKind::Float && value.float_kind == type.kind())
			return Operand::make_fimm(type.kind(), bits);
		long double v = 0;
		if (value.kind == ValueKind::Float)
			v = lowir::float_value(value.float_kind, bits);
		else
			v = static_cast<long double>(static_cast<int64_t>(bits.low));
		return Operand::make_fimm(type.kind(), lowir::float_literal(type.kind(), v));
	}
	if (value.kind == ValueKind::Float)
	{
		// A floating literal stored as raw bits of an integer-typed value.
		return Operand::make_imm(static_cast<int64_t>(bits.low), static_cast<int64_t>(bits.high));
	}
	int64_t low = static_cast<int64_t>(bits.low);
	int64_t high = static_cast<int64_t>(bits.high);
	switch (type.kind())
	{
	case TypeKind::I1:
		low &= 1;
		break;
	case TypeKind::I8:
		low = static_cast<int8_t>(low);
		break;
	case TypeKind::U8:
		low &= 0xff;
		break;
	case TypeKind::I16:
		low = static_cast<int16_t>(low);
		break;
	case TypeKind::U16:
		low &= 0xffff;
		break;
	case TypeKind::I32:
		low = static_cast<int32_t>(low);
		break;
	case TypeKind::U32:
		low &= 0xffffffffll;
		break;
	default:
		break;
	}
	if (type.kind() != TypeKind::I128)
		high = low < 0 ? -1 : 0;
	return Operand::make_imm(low, high);
}

bool FunctionSelector::fits_imm32(const Operand& op) const
{
	return op.kind == OperandKind::Imm && op.imm >= INT32_MIN && op.imm <= INT32_MAX;
}

// A folded load's address forms in `scratch`: each operand of the consumer
// uses only its own reserved scratch register.
Operand FunctionSelector::temp_location(uint32_t temp, Reg scratch)
{
	const TempInfo& t = temps_[temp];
	if (t.reg != kNoReg)
		return Operand::make_reg(t.reg);
	// O1+: a function-level value is bound wherever it is live.
	if (precise_globals_ && t.greg != kNoReg)
		throw InternalError("function-level value read where it is not bound");
	if (t.flags & TF_FOLDED)
	{
		const Instruction& load = body_->instructions[body_->temps[temp].def];
		return address_operand(load.ops[0], scratch == kNoReg ? kSelectScratch : scratch);
	}
	if (t.home != 0)
		return Operand::make_frame(t.home);
	if (t.flags & TF_PINNED)
		return Operand::make_reg(t.reg);
	// A value defined in reachable code whose definition the layout emits
	// after this use (a phi of a loop header laid out before all of its
	// predecessors: one entered from a landing and its back edge): its
	// home, which the definition or the phi transfers write. (A value with
	// a function-level register is bound wherever liveness finds it.)
	if ((t.flags & (TF_DEFINED | TF_LAZY)) == TF_DEFINED)
		return Operand::make_frame(home_of(temp));
	// A value whose definition never executes (unreachable or undefined):
	// any bits will do.
	return Operand::make_imm(0);
}

// A homed value reloads at its own type, extending as a register holds it:
// a narrow index reached through an elided extension is its home's low
// bytes extended, never the adjacent frame bytes.
Reg FunctionSelector::temp_reg_or_load(uint32_t temp, Reg scratch)
{
	const Operand loc = temp_location(temp, scratch);
	if (loc.kind == OperandKind::Reg)
		return loc.reg;
	load_into(scratch, loc, body_->temps[temp].type.kind());
	return scratch;
}

void FunctionSelector::load_into(Reg dst, const Operand& src, TypeKind type)
{
	const bool xmm = is_xmm(dst);
	switch (src.kind)
	{
	case OperandKind::Reg:
		if (src.reg != dst)
			emit2(xmm ? Op::FMov : Op::Mov, xmm ? type : TypeKind::Void, Operand::make_reg(dst), src);
		break;
	case OperandKind::Imm:
		emit2(Op::Mov, TypeKind::Void, Operand::make_reg(dst), src);
		break;
	case OperandKind::FImm:
		emit2(Op::FMov, src.type, Operand::make_reg(dst), src);
		break;
	case OperandKind::Mem:
		emit2(xmm ? Op::FMov : Op::Load, type, Operand::make_reg(dst), src);
		break;
	case OperandKind::Symbol:
		emit2(Op::Mov, TypeKind::Void, Operand::make_reg(dst), src);
		break;
	default:
		throw InternalError("unsupported operand transfer");
	}
}

SymbolAccess FunctionSelector::symbol_access(uint32_t symbol) const
{
	// Course objects and executables address every symbol directly.
	if (eh_model_ != EhModel::Host)
		return SymbolAccess::Direct;
	return host_symbol_access(program_, symbol);
}

SymbolAccess host_symbol_access(const lowir::Program& program, lowir::SymbolId symbol)
{
	// Host objects (pa27 README 1, 5): data another module may define --
	// an object declared here (imported, or one a streamed unit defines
	// after this function: the object compiler relaxes that GOT load once
	// the definition follows), or a GNU weak definition another object's
	// may replace -- is reached through its GOT entry, which the linker
	// relaxes where the definition ends up local. A vague-linkage
	// definition is addressed directly, as GCC and Clang do: its copies
	// are the same, and references bind to the one the link keeps. A
	// thread-local object's address is the executing thread's: local exec
	// for one this object defines (the executable holds it), initial exec
	// through its GOT entry otherwise.
	const lowir::Symbol& s = program.symbols[symbol];
	uint8_t binding = s.metadata.binding;
	uint8_t storage = s.metadata.storage;
	bool defined = s.kind == lowir::SymbolKind::GlobalDefinition;
	if (s.kind == lowir::SymbolKind::Unresolved)
	{
		// Referenced before its declaration (a streamed definition). The
		// lowering states every global's binding (set_pending) and whether
		// the unit defines it (then addressed as the definition will be,
		// as a whole-program compile of the LowIR addresses it); one left
		// unstated is a function's, whose address forms directly (the
		// object compiler loads an undefined one's from the GOT instead).
		binding = program.forward_binding(symbol);
		storage = program.forward_storage(symbol);
		defined = program.forward_defined(symbol);
		if (binding == ir_model::SBM_DEFAULT)
			return SymbolAccess::Direct;
	}
	else if (!lowir::is_global_kind(s.kind))
		return SymbolAccess::Direct;
	if (storage == ir_model::GSM_THREAD_LOCAL)
		return defined ? SymbolAccess::TlsLocal : SymbolAccess::TlsInitial;
	if (defined ? binding == ir_model::SBM_WEAK && (s.metadata.flags & lowir::SF_REPLACEABLE) != 0
				: binding != ir_model::SBM_INTERNAL)
		return SymbolAccess::Got;
	return SymbolAccess::Direct;
}

Operand FunctionSelector::symbol_operand(uint32_t symbol, int32_t addend) const
{
	Operand op = Operand::make_symbol(symbol, addend);
	op.access = symbol_access(symbol);
	return op;
}

Operand FunctionSelector::symbol_memory(uint32_t symbol, int32_t addend, Reg scratch)
{
	// A symbol's memory, pc-relative, or at the address its GOT entry or
	// thread-local form gives in `scratch`.
	if (symbol_access(symbol) == SymbolAccess::Direct)
		return Operand::make_symbol_mem(symbol, addend);
	if (scratch == kNoReg)
		throw InternalError("a GOT-addressed object without a scratch register");
	emit2(Op::Mov, TypeKind::Void, Operand::make_reg(scratch), symbol_operand(symbol, 0));
	return Operand::make_mem(scratch, addend);
}

Operand FunctionSelector::rm_operand(Value value, Type type, Reg scratch)
{
	value = resolve(value);
	switch (value.kind)
	{
	case ValueKind::Integer:
	case ValueKind::Float:
		return imm_for(value, type);
	case ValueKind::Temp:
	{
		const TempInfo& t = temps_[value.index];
		if ((t.flags & TF_LAZY) && t.reg == kNoReg)
		{
			// An address value read as a value where it did not fold.
			materialize_address(scratch, addrs_[t.addr]);
			return Operand::make_reg(scratch);
		}
		const Operand loc = temp_location(value.index, scratch);
		const Type own = body_->temps[value.index].type;
		if (loc.kind == OperandKind::Mem && t.cls == VClass::Gpr && value_bytes(own) != value_bytes(type) &&
			scratch != kNoReg)
		{
			// A home holds the value at its own width: read it as such.
			load_into(scratch, loc, own.is_pointer() ? TypeKind::Ptr : own.kind());
			return Operand::make_reg(scratch);
		}
		return loc;
	}
	case ValueKind::Symbol:
		if (type.is_float())
			throw BackendError("floating value requires a numeric value or storage operand");
		if (type.kind() == TypeKind::I128)
			throw BackendError("i128 value requires numeric bits or stored contents, not a symbol address");
		return symbol_operand(value.index, 0);
	case ValueKind::Slot:
		// A slot in a value position reads the value stored there (call
		// arguments and phi transfers take slot addresses separately).
		return Operand::make_frame(slot_offset_[value.index]);
	default:
		throw InternalError("unsupported operand");
	}
}

Operand FunctionSelector::reg_operand(Value value, Type type, Reg scratch)
{
	const Operand op = rm_operand(value, type, scratch);
	if (op.kind == OperandKind::Reg)
		return op;
	load_into(scratch, op, type.is_pointer() ? TypeKind::Ptr : type.kind());
	return Operand::make_reg(scratch);
}

// The memory operand of a lazy address. It reloads homed parts into
// `scratch` only: when base and index both need reloading they combine
// there (the index loaded at its own type and scaled, the full-width base
// added from its home), unless the caller passes a `spare` register for the
// index, so the operand leaves the other reserved scratch register free.
Operand FunctionSelector::lazy_operand(const AddrExpr& expr, Reg scratch, Reg spare)
{
	Operand op;
	switch (expr.kind)
	{
	case AddrExpr::Frame:
		op = Operand::make_frame(static_cast<int32_t>(slot_offset_[expr.base] + expr.disp));
		frame_pinned_ = frame_pinned_ || expr.disp < 0 ||
			static_cast<uint64_t>(expr.disp) >=
				std::max<uint64_t>(8, (value_bytes(body_->slots[expr.base].type) + 7) & ~7ull);
		break;
	case AddrExpr::Symbol:
		op = symbol_memory(expr.base, static_cast<int32_t>(expr.disp), scratch);
		break;
	case AddrExpr::Temp:
	{
		const Operand base = temp_location(expr.base, scratch);
		if (base.kind != OperandKind::Reg && expr.index != kNoId && spare == kNoReg &&
			temp_location(expr.index, scratch).kind != OperandKind::Reg)
		{
			temp_reg_or_load(expr.index, scratch);
			if (expr.scale > 1)
				emit2(Op::Shl, TypeKind::I64, Operand::make_reg(scratch), Operand::make_imm(log2_of(expr.scale)));
			emit2(Op::Add, TypeKind::I64, Operand::make_reg(scratch), base);
			return Operand::make_mem(scratch, static_cast<int32_t>(expr.disp));
		}
		op = Operand::make_mem(temp_reg_or_load(expr.base, scratch), static_cast<int32_t>(expr.disp));
		break;
	}
	case AddrExpr::Absolute:
		op = Operand::make_mem(kNoReg, static_cast<int32_t>(expr.disp));
		break;
	case AddrExpr::Home:
		op = Operand::make_frame(static_cast<int32_t>(home_of(expr.base) + expr.disp));
		frame_pinned_ = frame_pinned_ || expr.disp < 0 ||
			static_cast<uint64_t>(expr.disp) >=
				std::max<uint64_t>(8, (value_bytes(body_->temps[expr.base].type) + 7) & ~7ull);
		break;
	}
	if (expr.index != kNoId)
	{
		op.index = temp_reg_or_load(expr.index, op.reg == scratch && spare != kNoReg ? spare : scratch);
		op.scale = expr.scale;
	}
	return op;
}

Operand FunctionSelector::address_operand(Value storage, Reg scratch)
{
	storage = resolve(storage);
	switch (storage.kind)
	{
	case ValueKind::Slot:
		return Operand::make_frame(slot_offset_[storage.index]);
	case ValueKind::Symbol:
		return symbol_memory(storage.index, 0, scratch);
	case ValueKind::Integer:
		return Operand::make_mem(kNoReg, static_cast<int32_t>(literal(storage).low));
	case ValueKind::Temp:
	{
		const TempInfo& t = temps_[storage.index];
		if ((t.flags & TF_LAZY) && t.addr != kNoId)
			return lazy_operand(addrs_[t.addr], scratch);
		if (t.cls == VClass::Memory)
			return temp_location(storage.index);
		return Operand::make_mem(temp_reg_or_load(storage.index, scratch), 0);
	}
	default:
		throw BackendError("unsupported address operand");
	}
}

void FunctionSelector::materialize_address(Reg dst, const AddrExpr& expr, bool exclusive)
{
	if (expr.kind == AddrExpr::Symbol && expr.index == kNoId)
	{
		emit2(Op::Mov, TypeKind::Void, Operand::make_reg(dst),
			symbol_operand(expr.base, static_cast<int32_t>(expr.disp)));
		return;
	}
	if (expr.kind == AddrExpr::Absolute && expr.index == kNoId)
	{
		emit2(Op::Mov, TypeKind::Void, Operand::make_reg(dst), Operand::make_imm(expr.disp));
		return;
	}
	// Homed components load into the reserved scratch registers: an
	// allocatable dst may already be another component's (dying) register.
	// Only an instruction forming no other operand (`exclusive`) may use
	// both.
	const bool scratch_dst = dst == kSelectScratch || dst == kEmitScratch;
	const Reg work = scratch_dst ? dst : kSelectScratch;
	const Operand mem = lazy_operand(
		expr, work, !exclusive ? kNoReg : (work == kSelectScratch ? kEmitScratch : kSelectScratch));
	if (mem.reg == dst && mem.index == kNoReg && mem.disp == 0)
		return;
	if (mem.index == kNoReg && mem.disp == 0 && mem.reg < 32 && mem.symbol == kNoId)
	{
		emit2(Op::Mov, TypeKind::Void, Operand::make_reg(dst), Operand::make_reg(mem.reg));
		return;
	}
	emit2(Op::Lea, TypeKind::Void, Operand::make_reg(dst), mem);
}

// ---------------------------------------------------------------------------
// Parallel moves

namespace {

// True if `move` reads the location `dst` written by a move of `type`.
bool reads(const Move& move, const Operand& dst, lowir::TypeKind type)
{
	if (dst.kind == OperandKind::Reg)
		return move.src.uses_reg(dst.reg);
	if (dst.kind == OperandKind::Mem && move.src.kind == OperandKind::Mem && !move.address)
		return move.src.reg == dst.reg && move.src.index == dst.index && move.src.symbol == dst.symbol &&
			move.src.disp < dst.disp + static_cast<int32_t>(register_bytes(type)) &&
			dst.disp < move.src.disp + static_cast<int32_t>(register_bytes(move.read_type()));
	return false;
}

void redirect(Move& move, const Operand& from, lowir::TypeKind type, Reg to)
{
	if (from.kind == OperandKind::Reg)
	{
		if (move.src.kind == OperandKind::Reg && move.src.reg == from.reg)
			move.src.reg = to;
		else if (move.src.kind == OperandKind::Mem)
		{
			if (move.src.reg == from.reg)
				move.src.reg = to;
			if (move.src.index == from.reg)
				move.src.index = to;
		}
		return;
	}
	if (reads(move, from, type))
		move.src = Operand::make_reg(to);
}

} // namespace

void FunctionSelector::emit_move(const Move& move)
{
	const Operand& dst = move.dst;
	const Operand& src = move.src;
	const bool float_type = move.type == TypeKind::F32 || move.type == TypeKind::F64;
	if (dst.kind == OperandKind::Reg)
	{
		if (move.address)
			emit2(Op::Lea, TypeKind::Void, dst, src);
		else
			load_into(dst.reg, src, src.kind == OperandKind::Mem ? move.read_type() : move.type);
		return;
	}
	// A memory destination.
	const TypeKind int_type = float_type ? (move.type == TypeKind::F32 ? TypeKind::I32 : TypeKind::I64) : move.type;
	if (move.address)
	{
		emit2(Op::Lea, TypeKind::Void, Operand::make_reg(kEmitScratch), src);
		emit2(Op::Store, TypeKind::Ptr, dst, Operand::make_reg(kEmitScratch));
		return;
	}
	switch (src.kind)
	{
	case OperandKind::Reg:
		if (is_xmm(src.reg))
			emit2(Op::FMov, move.type, dst, src);
		else
			emit2(Op::Store, int_type, dst, src);
		break;
	case OperandKind::Imm:
		emit2(Op::Store, int_type, dst, src);
		break;
	case OperandKind::FImm:
		emit2(Op::FMov, src.type, dst, src);
		break;
	case OperandKind::Mem:
		emit2(
			Op::Load, move.src_type == TypeKind::Void ? int_type : move.src_type, Operand::make_reg(kEmitScratch), src);
		emit2(Op::Store, int_type, dst, Operand::make_reg(kEmitScratch));
		break;
	case OperandKind::Symbol:
		emit2(Op::Mov, TypeKind::Void, Operand::make_reg(kEmitScratch), src);
		emit2(Op::Store, TypeKind::Ptr, dst, Operand::make_reg(kEmitScratch));
		break;
	default:
		throw InternalError("unsupported move");
	}
}

// Moves are emitted in the order of the original quadratic resolver: the
// first pending move (by index) whose destination no other pending move
// reads, and when every pending destination is read, the first pending
// move's destination is saved in scratch and its readers redirected there.
// Reader counts and indexes of the register and memory locations each move
// reads keep the work O(n log n) in the moves and the reads between them.
void FunctionSelector::parallel_moves(std::vector<Move>& moves)
{
	size_t n = 0;
	for (size_t i = 0; i < moves.size(); ++i)
		if (moves[i].address || moves[i].dst != moves[i].src)
			moves[n++] = moves[i];
	moves.resize(n);
	if (n == 0)
		return;
	// A single move depends on no other.
	if (n == 1)
	{
		emit_move(moves[0]);
		moves.clear();
		return;
	}
	int32_t writer[32];
	for (unsigned r = 0; r < 32; ++r)
		writer[r] = -1;
	pm_mem_dsts_.clear();
	pm_mem_srcs_.clear();
	for (uint32_t i = 0; i < n; ++i)
	{
		const Move& m = moves[i];
		if (m.dst.kind == OperandKind::Reg)
			writer[m.dst.reg] = static_cast<int32_t>(i);
		else if (m.dst.kind == OperandKind::Mem)
			pm_mem_dsts_.push_back(i);
		if (m.src.kind == OperandKind::Mem && !m.address)
			pm_mem_srcs_.push_back(i);
	}
	// Memory locations ordered by base, index, symbol and displacement; a
	// typed move touches at most 8 bytes, so overlaps lie within 8 bytes.
	auto location_less = [](const Operand& a, int32_t a_disp, const Operand& b, int32_t b_disp) {
		if (a.reg != b.reg)
			return a.reg < b.reg;
		if (a.index != b.index)
			return a.index < b.index;
		if (a.symbol != b.symbol)
			return a.symbol < b.symbol;
		return a_disp < b_disp;
	};
	std::sort(pm_mem_dsts_.begin(), pm_mem_dsts_.end(), [&](uint32_t a, uint32_t b) {
		return location_less(moves[a].dst, moves[a].dst.disp, moves[b].dst, moves[b].dst.disp) ||
			(!location_less(moves[b].dst, moves[b].dst.disp, moves[a].dst, moves[a].dst.disp) && a < b);
	});
	// The sources as they were: a redirected source (a register now) keeps
	// its place in the order the searches assume.
	pm_src_at_.resize(n);
	for (uint32_t i = 0; i < n; ++i)
		pm_src_at_[i] = moves[i].src;
	std::sort(pm_mem_srcs_.begin(), pm_mem_srcs_.end(), [&](uint32_t a, uint32_t b) {
		return location_less(pm_src_at_[a], pm_src_at_[a].disp, pm_src_at_[b], pm_src_at_[b].disp) ||
			(!location_less(pm_src_at_[b], pm_src_at_[b].disp, pm_src_at_[a], pm_src_at_[a].disp) && a < b);
	});
	// The range of `sorted` (memory destinations or sources) that may
	// overlap `op`: its location at displacements in (op.disp - 8, op.disp + 8].
	auto near = [&](const std::vector<uint32_t>& sorted, bool by_dst, const Operand& op, size_t& lo, size_t& hi) {
		auto operand = [&](uint32_t k) -> const Operand& { return by_dst ? moves[k].dst : pm_src_at_[k]; };
		lo = static_cast<size_t>(std::lower_bound(sorted.begin(), sorted.end(), 0u, [&](uint32_t k, uint32_t) {
			return location_less(operand(k), operand(k).disp, op, op.disp - 7);
		}) - sorted.begin());
		hi = lo;
		while (hi < sorted.size() && !location_less(op, op.disp + 8, operand(sorted[hi]), operand(sorted[hi]).disp))
			++hi;
	};
	// Moves reading each register (a register source, a memory base or
	// index), in CSR form.
	uint32_t reg_first[33] = { 0 };
	auto src_regs = [](const Move& m, Reg* regs) {
		unsigned count = 0;
		if (m.src.kind == OperandKind::Reg)
			regs[count++] = m.src.reg;
		else if (m.src.kind == OperandKind::Mem)
		{
			if (m.src.reg < 32)
				regs[count++] = m.src.reg;
			if (m.src.index != kNoReg && m.src.index != m.src.reg)
				regs[count++] = m.src.index;
		}
		return count;
	};
	Reg regs[2];
	for (uint32_t j = 0; j < n; ++j)
		for (unsigned k = src_regs(moves[j], regs); k-- > 0;)
			++reg_first[regs[k] + 1];
	for (unsigned r = 0; r < 32; ++r)
		reg_first[r + 1] += reg_first[r];
	pm_reg_readers_.resize(reg_first[32]);
	{
		uint32_t fill[32];
		for (unsigned r = 0; r < 32; ++r)
			fill[r] = reg_first[r];
		for (uint32_t j = 0; j < n; ++j)
			for (unsigned k = src_regs(moves[j], regs); k-- > 0;)
				pm_reg_readers_[fill[regs[k]]++] = j;
	}
	// The moves whose destination `m` reads (once per way), into pm_written_.
	auto written_by = [&](const Move& m) {
		pm_written_.clear();
		if (m.src.kind == OperandKind::Reg)
		{
			if (writer[m.src.reg] >= 0)
				pm_written_.push_back(static_cast<uint32_t>(writer[m.src.reg]));
			return;
		}
		if (m.src.kind != OperandKind::Mem)
			return;
		if (m.src.reg < 32 && writer[m.src.reg] >= 0)
			pm_written_.push_back(static_cast<uint32_t>(writer[m.src.reg]));
		if (m.src.index != kNoReg && writer[m.src.index] >= 0)
			pm_written_.push_back(static_cast<uint32_t>(writer[m.src.index]));
		if (m.address)
			return;
		size_t lo, hi;
		near(pm_mem_dsts_, true, m.src, lo, hi);
		for (; lo != hi; ++lo)
			if (reads(m, moves[pm_mem_dsts_[lo]].dst, moves[pm_mem_dsts_[lo]].type))
				pm_written_.push_back(pm_mem_dsts_[lo]);
	};
	pm_readers_.assign(n, 0);
	pm_done_.assign(n, 0);
	for (uint32_t j = 0; j < n; ++j)
	{
		written_by(moves[j]);
		for (size_t k = 0; k < pm_written_.size(); ++k)
			if (pm_written_[k] != j)
				++pm_readers_[pm_written_[k]];
	}
	pm_ready_.clear();
	auto ready = [&](uint32_t i) {
		pm_ready_.push_back(i);
		std::push_heap(pm_ready_.begin(), pm_ready_.end(), std::greater<uint32_t>());
	};
	for (uint32_t i = 0; i < n; ++i)
		if (pm_readers_[i] == 0)
			ready(i);
	size_t pending = n;
	uint32_t first_pending = 0;
	while (pending != 0)
	{
		if (!pm_ready_.empty())
		{
			std::pop_heap(pm_ready_.begin(), pm_ready_.end(), std::greater<uint32_t>());
			const uint32_t i = pm_ready_.back();
			pm_ready_.pop_back();
			emit_move(moves[i]);
			pm_done_[i] = 1;
			--pending;
			written_by(moves[i]);
			for (size_t w = 0; w < pm_written_.size(); ++w)
			{
				const uint32_t k = pm_written_[w];
				if (k != i && !pm_done_[k] && --pm_readers_[k] == 0)
					ready(k);
			}
			continue;
		}
		// Every pending destination is still read: save the first pending
		// move's destination in scratch and redirect its readers there.
		while (pm_done_[first_pending])
			++first_pending;
		const uint32_t m0 = first_pending;
		const Operand dst = moves[m0].dst;
		const bool xmm = (dst.kind == OperandKind::Reg && is_xmm(dst.reg)) || moves[m0].type == TypeKind::F32 ||
			moves[m0].type == TypeKind::F64;
		const Reg save = xmm ? static_cast<Reg>(XMM0 + 14) : kSelectScratch;
		Move saving;
		saving.dst = Operand::make_reg(save);
		saving.src = dst;
		saving.type = moves[m0].type;
		emit_move(saving);
		const lowir::TypeKind saved_type = moves[m0].type;
		auto redirect_reader = [&](uint32_t j) {
			if (pm_done_[j] || !reads(moves[j], dst, saved_type))
				return;
			written_by(moves[j]);
			for (size_t w = 0; w < pm_written_.size(); ++w)
				if (pm_written_[w] != j && !pm_done_[pm_written_[w]])
					--pm_readers_[pm_written_[w]];
			redirect(moves[j], dst, saved_type, save);
			written_by(moves[j]);
			for (size_t w = 0; w < pm_written_.size(); ++w)
				if (pm_written_[w] != j && !pm_done_[pm_written_[w]])
					++pm_readers_[pm_written_[w]];
		};
		if (dst.kind == OperandKind::Reg)
		{
			// Readers only ever lose registers, so the initial index covers them.
			for (uint32_t k = reg_first[dst.reg]; k < reg_first[dst.reg + 1]; ++k)
				redirect_reader(pm_reg_readers_[k]);
		}
		else
		{
			size_t lo, hi;
			near(pm_mem_srcs_, false, dst, lo, hi);
			for (; lo != hi; ++lo)
				redirect_reader(pm_mem_srcs_[lo]);
		}
		if (pm_readers_[m0] != 0)
			throw InternalError("unresolved parallel move");
		ready(m0);
	}
	moves.clear();
}

} // namespace native
