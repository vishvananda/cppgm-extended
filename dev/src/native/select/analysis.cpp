#include "native/select/function_selector.h"
#include "native/select/for_each_inst.h"

#include <stdexcept>
#include <string>

#include "native/select/operand_roles.h"

// Per-function analysis ahead of selection: the reachable CFG, slots whose
// stores are dead or whose one entry store can stand for every load, value
// aliases (constants, representation-preserving copies, zero indexes),
// use counts and dead pure values, then the placement facts the allocator
// consumes: defining positions, last uses within the defining block, values
// that need a frame home, lazily folded addresses, loads folded into their
// sole adjacent consumer and compares fused into the following branch.
namespace native {

using lowir::BlockId;
using lowir::Instruction;
using lowir::kNoId;
using lowir::Literal;
using lowir::Opcode;
using lowir::Type;
using lowir::TypeKind;
using lowir::Value;
using lowir::ValueKind;

FunctionSelector::FunctionSelector(const lowir::Program& program, SelectStats& stats) : program_(program), stats_(stats)
{}

void FunctionSelector::reset(lowir::SymbolId symbol, Function& out)
{
	const lowir::Symbol& sym = program_.symbols[symbol];
	const lowir::Function& function = program_.functions[sym.detail];
	body_ = function.body.get();
	signature_ = &program_.signatures[function.signature];
	symbol_ = symbol;
	out_ = &out;
	out.clear();
	out.symbol = symbol;
	const lowir::FunctionBody& body = *body_;
	blocks_.assign(body.layout.size(), BlockInfo());
	layout_of_.assign(body.blocks.size(), kNoId);
	succ_first_.assign(body.layout.size() + 1, 0);
	succs_.clear();
	edges_.clear();
	wide_phi_first_.assign(body.instructions.size(), kNoId);
	wide_phi_pairs_.clear();
	temps_.assign(body.temps.size(), TempInfo());
	alias_.assign(body.temps.size(), Value::none());
	addrs_.clear();
	pos_.assign(body.instructions.size(), -1);
	inst_block_.assign(body.instructions.size(), kNoId);
	marks_.assign(body.instructions.size(), 0);
	call_layouts_.reset(body.instructions.size());
	slot_state_.assign(body.slots.size(), SS_UNUSED);
	slot_value_.assign(body.slots.size(), Value::none());
	slot_store_.assign(body.slots.size(), kNoId);
	slot_offset_.assign(body.slots.size(), 0);
	extra_literals_.clear();
	for (unsigned i = 0; i < 32; ++i)
		owner_[i] = kNoId;
	occupied_ = pinned_ = used_callee_saved_ = 0;
	frame_size_ = 0;
	frame_objects_.clear();
	frame_pinned_ = false;
	block_ = 0;
	position_ = 0;
	location_ = 0;
	sret_home_ = 0;
	va_save_area_ = 0;
	named_gprs_ = named_xmms_ = named_stack_ = 0;
	needs_scratch_area_ = false;
	scratch_area_ = signature_->result.is_float() || signature_->boundary.arity == ir_model::CAM_VARIADIC;
	for (uint32_t p = 0; p < signature_->parameter_count; ++p)
		if (program_.parameters_of(*signature_)[p].type.is_float())
			scratch_area_ = true;
	out.locations = body.locations;
}

lowir::Value FunctionSelector::resolve(Value value) const
{
	while (value.kind == ValueKind::Temp && alias_[value.index].kind != ValueKind::None)
		value = alias_[value.index];
	return value;
}

Type FunctionSelector::type_of(Value value) const
{
	if (value.kind == ValueKind::Temp)
		return body_->temps[value.index].type;
	return Type::scalar(TypeKind::Ptr);
}

const Literal& FunctionSelector::literal(Value value) const
{
	const uint32_t pool = static_cast<uint32_t>(body_->literals.size());
	return value.index < pool ? body_->literals[value.index] : extra_literals_[value.index - pool];
}

Value FunctionSelector::add_literal(const Literal& bits)
{
	extra_literals_.push_back(bits);
	return Value::make(ValueKind::Integer, static_cast<uint32_t>(body_->literals.size() + extra_literals_.size() - 1));
}

// Whether an instruction handles a floating value: its type, an operand, or
// the parameters or result of the signature it calls.
bool FunctionSelector::floating_inst(const Instruction& inst) const
{
	if (inst.type.is_float())
		return true;
	bool floating = false;
	for_each_operand(*body_, inst, [&](Value v, OperandRole) {
		if (v.kind == ValueKind::Float || (v.kind == ValueKind::Temp && body_->temps[v.index].type.is_float()))
			floating = true;
	});
	const lowir::Signature* sig = inst.op == Opcode::Call ? program_.call_boundary(inst) : nullptr;
	if (sig != nullptr)
	{
		floating = floating || sig->result.is_float();
		for (uint32_t p = 0; p < sig->parameter_count; ++p)
			floating = floating || program_.parameters_of(*sig)[p].type.is_float();
	}
	return floating;
}

// ---------------------------------------------------------------------------
// CFG

void FunctionSelector::build_cfg()
{
	const lowir::FunctionBody& body = *body_;
	const uint32_t n = static_cast<uint32_t>(body.layout.size());
	for (uint32_t b = 0; b < n; ++b)
	{
		blocks_[b].id = body.layout[b];
		layout_of_[body.layout[b]] = b;
	}
	std::vector<uint32_t>& handlers = handlers_;
	std::vector<uint32_t>& handler_first = handler_first_;
	handlers.clear();
	handler_first.assign(n + 1, 0);
	// Distinct successors in CSR form; `added` marks those of block b.
	std::vector<uint32_t> added(n, kNoId);
	for (uint32_t b = 0; b < n; ++b)
	{
		handler_first[b] = static_cast<uint32_t>(handlers.size());
		succ_first_[b] = static_cast<uint32_t>(succs_.size());
		const lowir::Block& block = body.blocks[blocks_[b].id];
		int32_t pos = 0;
		for (lowir::InstId i = block.first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			pos_[i] = pos++;
			inst_block_[i] = b;
			if (pos == 1 && inst.op == Opcode::Phi)
				blocks_[b].has_phi = true;
			if ((inst.op == Opcode::EhTry || inst.op == Opcode::EhCleanup) && inst.ops[0].kind == ValueKind::Block)
				handlers.push_back(layout_of_[inst.ops[0].index]);
			if (!scratch_area_ && floating_inst(inst))
				scratch_area_ = true;
			if (inst.op == Opcode::Call && (program_.symbols[symbol_].metadata.flags & lowir::SF_KEEP_ALIAS))
				scratch_area_ = true;
		}
		blocks_[b].count = static_cast<uint32_t>(pos);
		if (block.last == kNoId)
			continue;
		const Instruction& term = body.instructions[block.last];
		auto add = [&](BlockId target) {
			const uint32_t s = layout_of_[target];
			if (added[s] != b)
			{
				added[s] = b;
				succs_.push_back(s);
			}
		};
		if (term.op == Opcode::Jump)
			add(term.ops[0].index);
		else if (term.op == Opcode::Branch)
		{
			add(term.ops[1].index);
			add(term.ops[2].index);
		}
		else if (term.op == Opcode::Switch)
		{
			add(term.ops[1].index);
			for (uint32_t k = 1; k < term.extra_count; k += 2)
				add(body.operands[term.extra_first + k].index);
		}
	}
	handler_first[n] = static_cast<uint32_t>(handlers.size());
	succ_first_[n] = static_cast<uint32_t>(succs_.size());
	if (n == 0)
		return;
	std::vector<uint32_t> work(1, 0);
	blocks_[0].reachable = true;
	while (!work.empty())
	{
		const uint32_t b = work.back();
		work.pop_back();
		auto visit = [&](uint32_t s) {
			if (!blocks_[s].reachable)
			{
				blocks_[s].reachable = true;
				work.push_back(s);
			}
		};
		for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
			visit(succs_[k]);
		for (uint32_t k = handler_first[b]; k < handler_first[b + 1]; ++k)
		{
			blocks_[handlers[k]].handler = true;
			visit(handlers[k]);
		}
	}
	for (uint32_t b = 0; b < n; ++b)
	{
		if (!blocks_[b].reachable)
			continue;
		for (lowir::InstId i = body.blocks[blocks_[b].id].first; i != kNoId; i = body.instructions[i].next)
			marks_[i] |= IM_REACHABLE;
	}
	reachable_preds();
}

// Each block's reachable predecessors: their count and CSR lists.
void FunctionSelector::reachable_preds()
{
	const uint32_t n = static_cast<uint32_t>(blocks_.size());
	for (uint32_t b = 0; b < n; ++b)
		blocks_[b].preds = 0;
	for (uint32_t b = 0; b < n; ++b)
		if (blocks_[b].reachable)
			for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
				++blocks_[succs_[k]].preds;
	preds_first_.assign(n + 1, 0);
	for (uint32_t b = 0; b < n; ++b)
		preds_first_[b + 1] = preds_first_[b] + blocks_[b].preds;
	preds_.assign(preds_first_[n], 0);
	std::vector<uint32_t> fill(preds_first_.begin(), preds_first_.end() - 1);
	for (uint32_t b = 0; b < n; ++b)
		if (blocks_[b].reachable)
			for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
				preds_[fill[succs_[k]]++] = b;
}

// ---------------------------------------------------------------------------
// Slots

void FunctionSelector::analyze_slots()
{
	const lowir::FunctionBody& body = *body_;
	const size_t count = body.slots.size();
	if (count == 0)
		return;
	std::vector<uint32_t> loads(count, 0);
	std::vector<uint32_t> stores(count, 0);
	std::vector<uint8_t> escaped(count, 0);
	std::vector<uint8_t> stored_in_entry(count, 0);
	const bool entry_has_preds = blocks_.empty() || blocks_[0].preds != 0;
	for_each_inst([&](uint32_t id, uint32_t b) {
		const Instruction& inst = body.instructions[id];
		const bool is_volatile = (inst.flags & lowir::IF_VOLATILE) != 0;
		if (inst.op == Opcode::Load && inst.ops[0].kind == ValueKind::Slot)
		{
			const uint32_t s = inst.ops[0].index;
			slot_state_[s] = SS_NORMAL;
			++loads[s];
			if (is_volatile || inst.type != body.slots[s].type || (b == 0 && !stored_in_entry[s]))
				escaped[s] = 1;
			return;
		}
		if (inst.op == Opcode::Store && inst.ops[1].kind == ValueKind::Slot)
		{
			const uint32_t s = inst.ops[1].index;
			slot_state_[s] = SS_NORMAL;
			++stores[s];
			if (b == 0)
				stored_in_entry[s] = 1;
			if (stores[s] == 1 && b == 0)
				slot_store_[s] = id;
			else
				slot_store_[s] = kNoId;
			if (is_volatile || inst.type != body.slots[s].type)
				escaped[s] = 2;
			if (inst.ops[0].kind == ValueKind::Slot)
				escaped[inst.ops[0].index] = 1;
			return;
		}
		for_each_operand(body, inst, [&](Value v, OperandRole) {
			if (v.kind == ValueKind::Slot)
			{
				slot_state_[v.index] = SS_NORMAL;
				escaped[v.index] = 1;
			}
		});
	});
	// Unoptimized source debugging keeps named objects in their actual slots.
	// Generated slots and optimized compilations retain the usual promotion.
	if (level_ == 0 && program_.debug && symbol_ < program_.debug->functions.size())
		for (const lowir::DebugLocal& local : program_.debug->functions[symbol_].locals)
			if (local.slot < count)
			{
				escaped[local.slot] = 1;
				slot_state_[local.slot] = SS_NORMAL;
			}
	for (uint32_t s = 0; s < count; ++s)
	{
		if (slot_state_[s] == SS_UNUSED || escaped[s] == 1)
			continue;
		const Type type = body.slots[s].type;
		const bool scalar = type.is_integer()
			? type.kind() != TypeKind::I128
			: (type.is_pointer() || type.kind() == TypeKind::F32 || type.kind() == TypeKind::F64);
		// The stored value stands for the loads only when it is the slot's
		// value as stored: a temporary of a compatible type (a store
		// converts others implicitly), not another slot read later.
		const Value stored = slot_store_[s] != kNoId ? body.instructions[slot_store_[s]].ops[0] : Value::none();
		const bool stands_for = stored.kind != ValueKind::Slot &&
			(stored.kind != ValueKind::Temp || alias_compatible(type, body.temps[stored.index].type));
		if (loads[s] == 0 && escaped[s] == 0)
			slot_state_[s] = SS_DEAD;
		else if (escaped[s] == 0 && stores[s] == 1 && slot_store_[s] != kNoId && !entry_has_preds && scalar &&
			stands_for)
		{
			slot_state_[s] = SS_PROMOTED;
			slot_value_[s] = body.instructions[slot_store_[s]].ops[0];
			marks_[slot_store_[s]] |= IM_DEAD;
			++stats_.promoted_slots;
			continue;
		}
		else
			continue;
		++stats_.dead_slots;
	}
	for_each_inst([&](uint32_t id, uint32_t) {
		const Instruction& inst = body.instructions[id];
		if (inst.op == Opcode::Store && inst.ops[1].kind == ValueKind::Slot &&
			slot_state_[inst.ops[1].index] == SS_DEAD)
			marks_[id] |= IM_DEAD;
	});
}

// ---------------------------------------------------------------------------
// Aliases

// True if every normalized register value of type `from` is also the
// normalized value of the same low bits read as `to` (registers hold
// narrow integers sign- or zero-extended to 64 bits per their type).
bool FunctionSelector::alias_compatible(Type to, Type from) const
{
	if (to == from)
		return true;
	const TypeKind t = to.kind();
	const TypeKind f = from.kind();
	const bool t_int = to.is_integer() || to.is_pointer();
	const bool f_int = from.is_integer() || from.is_pointer();
	if (!t_int || !f_int || t == TypeKind::I128 || f == TypeKind::I128)
		return false;
	const unsigned tb = to.is_pointer() ? 64 : lowir::integer_bits(t);
	const unsigned fb = from.is_pointer() ? 64 : lowir::integer_bits(f);
	if (f == TypeKind::I1)
		return true;
	if (tb == 64)
		return true;
	if (tb <= fb)
		return false;
	// Widening within 64 bits: a zero-extended source fits any wider type,
	// a sign-extended one only a wider signed type.
	if (!signed_narrow(f))
		return true;
	return signed_narrow(t);
}

namespace {

// `bits` read at integer type `kind`: wrapped and extended per the kind.
Literal normalize_literal(TypeKind kind, Literal bits)
{
	switch (kind)
	{
	case TypeKind::I1:
		bits.low &= 1;
		bits.high = 0;
		break;
	case TypeKind::I8:
		bits.low = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int8_t>(bits.low)));
		break;
	case TypeKind::U8:
		bits.low &= 0xff;
		break;
	case TypeKind::I16:
		bits.low = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(bits.low)));
		break;
	case TypeKind::U16:
		bits.low &= 0xffff;
		break;
	case TypeKind::I32:
		bits.low = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(bits.low)));
		break;
	case TypeKind::U32:
		bits.low &= 0xffffffffu;
		break;
	default:
		return bits;
	}
	bits.high = static_cast<int64_t>(bits.low) < 0 ? ~0ull : 0;
	return bits;
}

} // namespace

// True if the temp's value is 0 or 1: a compare or compare-exchange result
// or an i1 value.
bool FunctionSelector::known_boolean(uint32_t temp, unsigned depth) const
{
	const lowir::Temp& t = body_->temps[temp];
	if (t.def == kNoId || temp < body_->parameter_count)
		return false;
	const Instruction& def = body_->instructions[t.def];
	// A phi merging Booleans (0 or 1 literals, materialized Booleans) holds
	// one: `a || b` lowered as a merge.
	if (def.op == Opcode::Phi && depth < 4 && t.type.is_integer() && t.type.kind() != TypeKind::I128)
	{
		for (uint32_t k = 0; k + 1 < def.extra_count; k += 2)
		{
			const Value v = body_->operands[def.extra_first + k + 1];
			if (v.kind == ValueKind::Integer ? (literal(v).high != 0 || literal(v).low > 1)
											 : v.kind != ValueKind::Temp || !known_boolean(v.index, depth + 1))
				return false;
		}
		return def.extra_count != 0;
	}
	// So does a select of Booleans.
	if (def.op == Opcode::Select && depth < 4)
	{
		for (unsigned k = 1; k < 3; ++k)
		{
			const Value v = def.ops[k];
			if (v.kind == ValueKind::Integer ? (literal(v).high != 0 || literal(v).low > 1)
											 : v.kind != ValueKind::Temp || !known_boolean(v.index, depth + 1))
				return false;
		}
		return true;
	}
	const Opcode op = def.op;
	return op == Opcode::Compare || op == Opcode::AtomicCompareExchange ||
		(t.type.kind() == TypeKind::I1 && op != Opcode::Call && op != Opcode::Load && op != Opcode::AtomicLoad);
}

namespace {

unsigned width_bits(TypeKind kind)
{
	switch (kind)
	{
	case TypeKind::I1:
		return 1;
	case TypeKind::I8:
	case TypeKind::U8:
		return 8;
	case TypeKind::I16:
	case TypeKind::U16:
		return 16;
	case TypeKind::I32:
	case TypeKind::U32:
		return 32;
	case TypeKind::I64:
		return 64;
	default:
		return 0;
	}
}

} // namespace

void FunctionSelector::resolve_aliases()
{
	const lowir::FunctionBody& body = *body_;
	// O1+: per temporary, the low bits its value occupies when it is known
	// to be non-negative (64: unknown): zero-extending loads and
	// extensions, compares, and/or/xor/add/mul/shifts by constants of such
	// values, phis of them (inputs laid out earlier). A value that fits its
	// own type's and another's capacity has one canonical register form
	// for both.
	fits_.assign(temps_.size(), 64);
	std::vector<uint8_t> done(level_ >= 1 ? temps_.size() : 0, 0);
	auto bits = [&](Value v) -> unsigned {
		if (v.kind == ValueKind::Integer)
		{
			const Literal& l = literal(v);
			return l.high != 0 || static_cast<int64_t>(l.low) < 0 ? 64u
				: l.low == 0                                      ? 0u
																  : 64u - __builtin_clzll(l.low);
		}
		return v.kind == ValueKind::Temp && v.index < done.size() && done[v.index] ? fits_[v.index] : 64u;
	};
	auto shift_count = [&](Value v) -> int {
		return v.kind == ValueKind::Integer && literal(v).high == 0 && literal(v).low < 64
			? static_cast<int>(literal(v).low)
			: -1;
	};
	for_each_inst([&](uint32_t id, uint32_t) {
		const Instruction& inst = body.instructions[id];
		if (level_ < 1 || (marks_[id] & IM_DEAD) || inst.result == kNoId || !inst.type.is_integer() ||
			inst.type.kind() == TypeKind::I128)
			return;
		const TypeKind kind = inst.type.kind();
		const unsigned width = width_bits(kind);
		unsigned r = 64;
		switch (inst.op)
		{
		case Opcode::Load:
			r = unsigned_narrow(kind) || kind == TypeKind::I1 ? width : 64;
			break;
		case Opcode::Compare:
			r = 1;
			break;
		case Opcode::Unary:
			// A bit scan's index: below 64.
			if (static_cast<lowir::UnaryOp>(inst.sub) == lowir::UnaryOp::CountTrailingZeros ||
				static_cast<lowir::UnaryOp>(inst.sub) == lowir::UnaryOp::CountLeadingZeros)
				r = 6;
			break;
		case Opcode::Copy:
			r = bits(inst.ops[0]);
			break;
		case Opcode::Convert:
		{
			const lowir::ConvertOp op = static_cast<lowir::ConvertOp>(inst.sub);
			const unsigned from = width_bits(lowir::convert_source_type(inst).kind());
			const unsigned b = bits(inst.ops[0]);
			if (from == 0)
				break;
			if (op == lowir::ConvertOp::ZExt)
				r = std::min(b, from);
			else if (op == lowir::ConvertOp::SExt)
				r = b < from ? b : 64;
			else if (op == lowir::ConvertOp::Trunc)
				r = b <= width ? b : 64;
			break;
		}
		case Opcode::Binary:
		{
			const unsigned a = bits(inst.ops[0]), c = bits(inst.ops[1]);
			const int k = shift_count(inst.ops[1]);
			switch (static_cast<lowir::BinaryOp>(inst.sub))
			{
			case lowir::BinaryOp::And:
				r = std::min(a, c);
				break;
			case lowir::BinaryOp::Or:
			case lowir::BinaryOp::Xor:
				r = std::max(a, c);
				break;
			case lowir::BinaryOp::Add:
				r = std::max(a, c) < 63 ? std::max(a, c) + 1 : 64;
				break;
			case lowir::BinaryOp::Mul:
				r = a + c <= 63 ? a + c : 64;
				break;
			case lowir::BinaryOp::Shl:
				r = k >= 0 && a + static_cast<unsigned>(k) <= 63 ? a + static_cast<unsigned>(k) : 64;
				break;
			case lowir::BinaryOp::UShr:
				if (k >= 0)
				{
					const unsigned in = std::min(a, width);
					r = in > static_cast<unsigned>(k) ? in - static_cast<unsigned>(k) : 0;
				}
				break;
			case lowir::BinaryOp::Shr:
				// Arithmetic: below the type's sign bit only.
				if (k >= 0 && a < width)
					r = a > static_cast<unsigned>(k) ? a - static_cast<unsigned>(k) : 0;
				break;
			default:
				break;
			}
			break;
		}
		case Opcode::Phi:
		{
			r = 0;
			for (uint32_t k = 0; k + 1 < inst.extra_count && r < 64; k += 2)
				r = std::max(r, bits(body.operands[inst.extra_first + k + 1]));
			break;
		}
		case Opcode::Select:
			r = std::max(bits(inst.ops[1]), bits(inst.ops[2]));
			break;
		default:
			break;
		}
		// An unsigned narrow value is zero-extended in its register.
		if (unsigned_narrow(kind) && r > width)
			r = width;
		if (r > canonical_bits(kind) && !unsigned_narrow(kind))
			r = 64;
		fits_[inst.result] = static_cast<uint8_t>(r);
		done[inst.result] = 1;
	});
	// O1+: an integer truncation every use of which reads only its low
	// bits -- an add, sub or mul of its type, a store or compare of its
	// type or narrower, a narrower truncation -- stands for its source
	// (whose upper bits those ignore). Not one a phi, branch, extension or
	// any other instruction reads, nor the stored value of a promoted slot
	// (its store emits nothing; the slot's loads stand for the value, read
	// in full by their own readers).
	std::vector<uint8_t> low_only(level_ >= 1 ? temps_.size() : 0, 0);
	if (level_ >= 1)
	{
		auto narrows = [&](const Instruction& inst) {
			return inst.op == Opcode::Convert && static_cast<lowir::ConvertOp>(inst.sub) == lowir::ConvertOp::Trunc &&
				inst.type.is_integer() && inst.type.kind() != TypeKind::I128 && inst.type.kind() != TypeKind::I1 &&
				lowir::convert_source_type(inst).is_integer() &&
				lowir::convert_source_type(inst).kind() != TypeKind::I128;
		};
		for_each_inst([&](uint32_t id, uint32_t) {
			const Instruction& inst = body.instructions[id];
			if (!(marks_[id] & IM_DEAD) && inst.result != kNoId && narrows(inst) && inst.ops[0].kind == ValueKind::Temp)
				low_only[inst.result] = 1;
		});
		for (uint32_t s = 0; s < slot_state_.size(); ++s)
			if (slot_state_[s] == SS_PROMOTED && slot_value_[s].kind == ValueKind::Temp &&
				slot_value_[s].index < low_only.size() && low_only[slot_value_[s].index] == 1)
				low_only[slot_value_[s].index] = 2;
		for_each_inst([&](uint32_t id, uint32_t) {
			const Instruction& inst = body.instructions[id];
			if (marks_[id] & IM_DEAD)
				return;
			uint32_t position = 0;
			auto visit = [&](const Value& v) {
				const uint32_t k = position++;
				if (v.kind != ValueKind::Temp || v.index >= low_only.size() || low_only[v.index] != 1)
					return;
				const unsigned bits = width_bits(body.temps[v.index].type.kind());
				bool ok = false;
				switch (inst.op)
				{
				case Opcode::Binary:
				{
					const lowir::BinaryOp op = static_cast<lowir::BinaryOp>(inst.sub);
					ok = k < 2 &&
						(op == lowir::BinaryOp::Add || op == lowir::BinaryOp::Sub || op == lowir::BinaryOp::Mul) &&
						inst.type.kind() == body.temps[v.index].type.kind();
					break;
				}
				case Opcode::Store:
					ok = k == 0 && inst.type.is_integer() && width_bits(inst.type.kind()) <= bits &&
						!(inst.flags & lowir::IF_VOLATILE);
					break;
				case Opcode::Compare:
					ok = k < 2 && inst.type.kind() == body.temps[v.index].type.kind();
					break;
				case Opcode::Convert:
					ok = k == 0 && narrows(inst) && width_bits(inst.type.kind()) <= bits;
					break;
				default:
					break;
				}
				if (!ok)
					low_only[v.index] = 2;
			};
			if (inst.op == Opcode::Phi)
			{
				for (uint32_t k = 1; k < inst.extra_count; k += 2)
				{
					const Value v = body.operands[inst.extra_first + k];
					if (v.kind == ValueKind::Temp && v.index < low_only.size())
						low_only[v.index] = low_only[v.index] ? 2 : 0;
				}
				return;
			}
			for_each_operand(body, inst, [&](Value v, OperandRole) { visit(v); });
		});
	}
	for_each_inst([&](uint32_t id, uint32_t) {
		const Instruction& inst = body.instructions[id];
		if ((marks_[id] & IM_DEAD) || inst.result == kNoId)
			return;
		const uint32_t r = inst.result;
		Value to = Value::none();
		if (inst.op == Opcode::Const)
		{
			to = inst.ops[0];
			if (to.kind == ValueKind::Integer && inst.type.is_integer() && inst.type.kind() != TypeKind::I128)
			{
				const Literal norm = normalize_literal(inst.type.kind(), literal(to));
				const Literal& old = literal(to);
				if (norm.low != old.low || norm.high != old.high)
					to = add_literal(norm);
			}
		}
		else if (inst.op == Opcode::Copy)
		{
			const Value src = resolve(inst.ops[0]);
			if (src.kind == ValueKind::Integer && (inst.type.is_integer() || inst.type.is_pointer()) &&
				inst.type.kind() != TypeKind::I128)
			{
				const Literal norm = normalize_literal(inst.type.kind(), literal(src));
				to = (norm.low == literal(src).low && norm.high == literal(src).high) ? src : add_literal(norm);
			}
			else if (src.kind == ValueKind::Float && inst.type.is_float())
				to = src;
			else if (src.kind == ValueKind::Temp &&
				(alias_compatible(inst.type, body.temps[src.index].type) ||
					((inst.type.is_integer() && inst.type.kind() != TypeKind::I128) &&
						(known_boolean(src.index) ||
							fits_[src.index] <= std::min(canonical_bits(inst.type.kind()),
													canonical_bits(body.temps[src.index].type.kind()))))))
				to = src;
		}
		else if (inst.op == Opcode::Load && inst.ops[0].kind == ValueKind::Slot &&
			slot_state_[inst.ops[0].index] == SS_PROMOTED)
		{
			to = resolve(slot_value_[inst.ops[0].index]);
			if (to.kind == ValueKind::Integer && inst.type.is_integer() && inst.type.kind() != TypeKind::I128)
			{
				const Literal norm = normalize_literal(inst.type.kind(), literal(to));
				if (norm.low != literal(to).low || norm.high != literal(to).high)
					to = add_literal(norm);
			}
		}
		else if (inst.op == Opcode::Index)
		{
			// A zero index of a pointer is the pointer; of an object value it
			// is the address of the object's home.
			const Value off = resolve(inst.ops[1]);
			const Value base = resolve(inst.ops[0]);
			if (off.kind == ValueKind::Integer && literal(off).low == 0 && base.kind == ValueKind::Temp &&
				body.temps[base.index].type.is_pointer())
				to = base;
		}
		else if (inst.op == Opcode::Convert)
		{
			// Extensions that keep the normalized register form, and any
			// integer conversion of a materialized Boolean (0 or 1 is
			// normalized at every width).
			const Value src = resolve(inst.ops[0]);
			const TypeKind from = lowir::convert_source_type(inst).kind();
			const TypeKind dest = inst.type.kind();
			const lowir::ConvertOp op = static_cast<lowir::ConvertOp>(inst.sub);
			const bool ints = (inst.type.is_integer() || inst.type.is_pointer()) && dest != TypeKind::I128 &&
				from != TypeKind::I128 &&
				(lowir::convert_source_type(inst).is_integer() || lowir::convert_source_type(inst).is_pointer());
			if (src.kind == ValueKind::Temp && ints)
			{
				// Sign-extending an i1 makes 1 all ones.
				const bool boolean = known_boolean(src.index) &&
					!(op == lowir::ConvertOp::SExt && from == TypeKind::I1);
				const bool extension = (op == lowir::ConvertOp::ZExt &&
										   (from == TypeKind::U8 || from == TypeKind::U16 || from == TypeKind::U32 ||
											   from == TypeKind::I1) &&
										   alias_compatible(inst.type, lowir::convert_source_type(inst))) ||
					(op == lowir::ConvertOp::SExt && signed_narrow(from) &&
						alias_compatible(inst.type, lowir::convert_source_type(inst)));
				// A non-negative value the conversion keeps (O1+): within the
				// bits a zero-extension keeps, below the sign bit a sign
				// extension copies, and a canonical value of the result type.
				const unsigned kept = op == lowir::ConvertOp::ZExt ? width_bits(from)
					: op == lowir::ConvertOp::SExt                 ? width_bits(from) - 1
					: op == lowir::ConvertOp::Trunc                ? 64u
																   : 0u;
				const bool same = fits_[src.index] <= std::min(kept, canonical_bits(dest));
				const bool low = op == lowir::ConvertOp::Trunc && r < low_only.size() && low_only[r] == 1;
				if (boolean || extension || same || low)
					to = src;
			}
		}
		if (to.kind == ValueKind::None)
			return;
		alias_[r] = to;
		temps_[r].flags |= TF_ALIAS;
	});
}

// O1+: a byte offset `x * K` (a multiply, or a shift left) only indexes
// read: K = c * s with s in {2, 4, 8} the addresses' scale and c one lea
// (or a copy and a short shift, which fuse into one) or nothing -- the
// offset holds x * c (an alias of x when c is 1). Every
// reader an index of bytes from a base other than a symbol (whose
// address takes the offset as its index, find_addresses).
void FunctionSelector::find_scaled_offsets()
{
	const lowir::FunctionBody& body = *body_;
	if (level_ < 1)
	{
		scale_of_.clear();
		return;
	}
	scale_of_.assign(temps_.size(), 0);
	// Candidates: the offset temps, with x and the scale.
	std::vector<uint32_t> candidates;
	std::vector<uint8_t> state(temps_.size(), 0); // 1 candidate, 2 disqualified
	std::vector<Value> source(temps_.size());
	std::vector<uint64_t> factor(temps_.size(), 0);
	for_each_inst([&](uint32_t id, uint32_t) {
		const Instruction& inst = body.instructions[id];
		if ((marks_[id] & IM_DEAD) || inst.op != Opcode::Binary || inst.result == kNoId ||
			(temps_[inst.result].flags & TF_ALIAS) ||
			(inst.type.kind() != TypeKind::I64 && inst.type.kind() != TypeKind::Ptr))
			return;
		const lowir::BinaryOp op = static_cast<lowir::BinaryOp>(inst.sub);
		Value x = resolve(inst.ops[0]), k = resolve(inst.ops[1]);
		if (op == lowir::BinaryOp::Mul && x.kind == ValueKind::Integer)
			std::swap(x, k);
		if ((op != lowir::BinaryOp::Mul && op != lowir::BinaryOp::Shl) || x.kind != ValueKind::Temp ||
			k.kind != ValueKind::Integer || literal(k).high != 0)
			return;
		const uint64_t K = op == lowir::BinaryOp::Shl ? (literal(k).low < 7 ? uint64_t(1) << literal(k).low : 0)
													  : literal(k).low;
		if (K < 2 || K > 72)
			return;
		uint8_t scale = 0;
		for (uint8_t s = 8; s >= 2 && scale == 0; s = static_cast<uint8_t>(s / 2))
		{
			const uint64_t c = K % s == 0 ? K / s : 0;
			if (c == 1 || c == 2 || c == 3 || c == 4 || c == 5 || c == 8 || c == 9)
				scale = s;
		}
		if (scale == 0)
			return;
		state[inst.result] = 1;
		source[inst.result] = x;
		factor[inst.result] = scale;
		candidates.push_back(inst.result);
	});
	if (candidates.empty())
		return;
	// Every read an index's offset (the operand itself, not an alias).
	for_each_inst([&](uint32_t id, uint32_t) {
		const Instruction& inst = body.instructions[id];
		if ((marks_[id] & IM_DEAD) || (inst.result != kNoId && (temps_[inst.result].flags & TF_ALIAS)))
			return;
		const bool index = inst.op == Opcode::Index && value_bytes(inst.type) == 1 &&
			resolve(inst.ops[0]).kind != ValueKind::Symbol;
		auto check = [&](Value v, unsigned position) {
			const Value r = resolve(v);
			if (r.kind == ValueKind::Temp && state[r.index] == 1 &&
				!(index && position == 1 && v.kind == ValueKind::Temp && v.index == r.index))
				state[r.index] = 2;
		};
		if (inst.op == Opcode::Phi)
		{
			for (uint32_t k = 1; k < inst.extra_count; k += 2)
				check(body.operands[inst.extra_first + k], 0);
			return;
		}
		if (inst.op == Opcode::Index)
		{
			check(inst.ops[0], 0);
			check(inst.ops[1], 1);
			return;
		}
		for_each_operand(body, inst, [&](Value v, OperandRole) { check(v, 0); });
	});
	for (uint32_t m : candidates)
	{
		if (state[m] != 1)
			continue;
		const Instruction& def = body.instructions[body.temps[m].def];
		const lowir::BinaryOp op = static_cast<lowir::BinaryOp>(def.sub);
		Value k = resolve(def.ops[1]);
		if (k.kind != ValueKind::Integer)
			k = resolve(def.ops[0]);
		const uint64_t K = op == lowir::BinaryOp::Shl ? uint64_t(1) << literal(k).low : literal(k).low;
		scale_of_[m] = static_cast<uint8_t>(factor[m]);
		if (K == factor[m])
		{
			alias_[m] = source[m];
			temps_[m].flags |= TF_ALIAS;
		}
	}
}

// ---------------------------------------------------------------------------
// Uses

void FunctionSelector::count_uses()
{
	const lowir::FunctionBody& body = *body_;
	for_each_inst([&](uint32_t id, uint32_t) {
		const Instruction& inst = body.instructions[id];
		if ((marks_[id] & IM_DEAD) || (inst.result != kNoId && (temps_[inst.result].flags & TF_ALIAS)))
			return;
		auto count = [&](Value v) {
			v = resolve(v);
			if (v.kind == ValueKind::Temp)
				++temps_[v.index].uses;
		};
		if (inst.op == Opcode::Phi)
		{
			for (uint32_t k = 1; k < inst.extra_count; k += 2)
				count(body.operands[inst.extra_first + k]);
			return;
		}
		for_each_operand(body, inst, [&](Value v, OperandRole) { count(v); });
	});
}

namespace {

bool is_pure(const Instruction& inst)
{
	switch (inst.op)
	{
	case Opcode::Copy:
	case Opcode::Phi:
	case Opcode::Addr:
	case Opcode::Index:
	case Opcode::Unary:
	case Opcode::Binary:
	case Opcode::Compare:
	case Opcode::Select:
	case Opcode::Convert:
	case Opcode::ExceptionSelector:
	case Opcode::StackAlloc:
		return true;
	// `exception` also takes a catch landing's handler record.
	case Opcode::Exception:
		return false;
	case Opcode::Load:
		return (inst.flags & lowir::IF_VOLATILE) == 0;
	default:
		return false;
	}
}

} // namespace

void FunctionSelector::eliminate_dead()
{
	const lowir::FunctionBody& body = *body_;
	for (uint32_t b = static_cast<uint32_t>(blocks_.size()); b-- > 0;)
	{
		if (!blocks_[b].reachable)
			continue;
		block_insts_.clear();
		for (lowir::InstId i = body.blocks[blocks_[b].id].first; i != kNoId; i = body.instructions[i].next)
			block_insts_.push_back(i);
		for (size_t k = block_insts_.size(); k-- > 0;)
		{
			const uint32_t i = block_insts_[k];
			const Instruction& inst = body.instructions[i];
			if (!(marks_[i] & IM_DEAD) && inst.result != kNoId && !(temps_[inst.result].flags & TF_ALIAS) &&
				temps_[inst.result].uses == 0 && is_pure(inst))
			{
				marks_[i] |= IM_DEAD;
				temps_[inst.result].flags |= TF_DEAD;
				auto drop = [&](Value v) {
					v = resolve(v);
					if (v.kind == ValueKind::Temp && temps_[v.index].uses > 0)
						--temps_[v.index].uses;
				};
				if (inst.op == Opcode::Phi)
				{
					for (uint32_t k = 1; k < inst.extra_count; k += 2)
						drop(body.operands[inst.extra_first + k]);
				}
				else
					for_each_operand(body, inst, [&](Value v, OperandRole) { drop(v); });
			}
		}
	}
}

void FunctionSelector::note_use(Value value, uint32_t inst, uint32_t block, int32_t pos, bool value_use)
{
	value = resolve(value);
	if (value.kind != ValueKind::Temp)
		return;
	TempInfo& t = temps_[value.index];
	++t.uses;
	t.user = inst;
	if (value_use)
		t.flags |= TF_VALUE_USE;
	if (t.block == block && pos >= t.def && !(t.flags & TF_GLOBAL))
	{
		if (pos > t.last)
			t.last = pos;
	}
	else
		t.flags |= TF_GLOBAL;
}

void FunctionSelector::place_uses()
{
	const lowir::FunctionBody& body = *body_;
	const bool entry_loops = !blocks_.empty() && blocks_[0].preds != 0;
	for (uint32_t p = 0; p < body.parameter_count; ++p)
	{
		TempInfo& t = temps_[p];
		t.block = 0;
		t.def = -1;
		t.flags |= TF_PARAM | TF_DEFINED;
		if (entry_loops)
			t.flags |= TF_GLOBAL;
	}
	for_each_inst([&](uint32_t id, uint32_t b) {
		const Instruction& inst = body.instructions[id];
		if (inst.result == kNoId || (marks_[id] & IM_DEAD) || (temps_[inst.result].flags & TF_ALIAS))
			return;
		TempInfo& t = temps_[inst.result];
		t.block = b;
		t.def = inst.op == Opcode::Phi ? -1 : pos_[id];
		t.flags |= TF_DEFINED;
		if (inst.op == Opcode::Phi)
			t.flags |= TF_GLOBAL;
	});
	for (size_t i = 0; i < temps_.size(); ++i)
	{
		temps_[i].uses = 0;
		temps_[i].user = kNoId;
	}
	for_each_inst([&](uint32_t id, uint32_t b) {
		const Instruction& inst = body.instructions[id];
		if ((marks_[id] & IM_DEAD) || (inst.result != kNoId && (temps_[inst.result].flags & TF_ALIAS)))
			return;
		if (inst.op == Opcode::Phi)
		{
			for (uint32_t k = 0; k + 1 < inst.extra_count; k += 2)
			{
				const uint32_t pred = layout_of_[body.operands[inst.extra_first + k].index];
				if (pred == kNoId || !blocks_[pred].reachable)
					continue;
				note_use(body.operands[inst.extra_first + k + 1], id, pred,
					static_cast<int32_t>(blocks_[pred].count) - 1, true);
			}
			return;
		}
		for_each_operand(
			body, inst, [&](Value v, OperandRole role) { note_use(v, id, b, pos_[id], role == OperandRole::Value); });
	});
	for (size_t i = 0; i < temps_.size(); ++i)
	{
		TempInfo& t = temps_[i];
		const Type type = body.temps[i].type;
		if (type.is_object() || type.kind() == TypeKind::F80 || type.kind() == TypeKind::I128)
			t.cls = VClass::Memory;
		else if (type.is_float())
			t.cls = VClass::Xmm;
		else if (!type.is_void())
			t.cls = VClass::Gpr;
		if (!(t.flags & TF_DEFINED) && t.uses > 0)
			t.flags |= TF_GLOBAL;
	}
}

// ---------------------------------------------------------------------------
// Addresses and folds

bool FunctionSelector::address_expr(uint32_t id, AddrExpr& expr) const
{
	const Instruction& inst = body_->instructions[id];
	if (inst.op == Opcode::Addr)
	{
		expr = AddrExpr();
		if (inst.ops[0].kind == ValueKind::Slot)
		{
			expr.kind = AddrExpr::Frame;
			expr.base = inst.ops[0].index;
		}
		else
		{
			expr.kind = AddrExpr::Symbol;
			expr.base = inst.ops[0].index;
		}
		return true;
	}
	if (inst.op != Opcode::Index)
		return false;
	const uint32_t size = value_bytes(inst.type);
	const Value base = resolve(inst.ops[0]);
	expr = AddrExpr();
	switch (base.kind)
	{
	case ValueKind::Slot:
		expr.kind = AddrExpr::Frame;
		expr.base = base.index;
		break;
	case ValueKind::Symbol:
		expr.kind = AddrExpr::Symbol;
		expr.base = base.index;
		break;
	case ValueKind::Temp:
		if (temps_[base.index].cls == VClass::Memory)
		{
			expr.kind = AddrExpr::Home;
			expr.base = base.index;
		}
		else if ((temps_[base.index].flags & TF_LAZY) && temps_[base.index].addr != kNoId)
			expr = addrs_[temps_[base.index].addr];
		else
		{
			expr.kind = AddrExpr::Temp;
			expr.base = base.index;
		}
		break;
	case ValueKind::Integer:
		expr.kind = AddrExpr::Absolute;
		expr.disp = static_cast<int64_t>(literal(base).low);
		break;
	default:
		return false;
	}
	const Value off = resolve(inst.ops[1]);
	if (off.kind == ValueKind::Integer)
	{
		const int64_t k = static_cast<int64_t>(literal(off).low);
		const int64_t limit = int64_t(1) << 30;
		if (k > limit || k < -limit || static_cast<int64_t>(size) * k > limit ||
			static_cast<int64_t>(size) * k < -limit)
			return false;
		expr.disp += k * static_cast<int64_t>(size);
		if (expr.disp > limit || expr.disp < -limit)
			return false;
		return true;
	}
	if (off.kind != ValueKind::Temp)
		return false;
	if (expr.index != kNoId || expr.kind == AddrExpr::Symbol || !(size == 1 || size == 2 || size == 4 || size == 8))
		return false;
	// A base and an index both reloaded from homes combine by adding the
	// base from its home (operands.cpp): a register base is full width.
	if (expr.kind == AddrExpr::Temp && register_bytes(body_->temps[expr.base].type.kind()) != 8)
		return false;
	expr.index = off.index;
	expr.scale = static_cast<uint8_t>(size);
	// A scaled byte offset (find_scaled_offsets).
	const Value raw = inst.ops[1];
	if (size == 1 && raw.kind == ValueKind::Temp && raw.index < scale_of_.size() && scale_of_[raw.index] != 0)
		expr.scale = scale_of_[raw.index];
	return true;
}

void FunctionSelector::find_addresses()
{
	const lowir::FunctionBody& body = *body_;
	// O1+: the values read as values other than as a call's argument (a
	// frame address read only so rematerializes at each argument).
	std::vector<uint8_t> other_value_use;
	if (level_ >= 1)
	{
		other_value_use.assign(temps_.size(), 0);
		for_each_inst([&](uint32_t id, uint32_t) {
			const Instruction& inst = body.instructions[id];
			if ((marks_[id] & IM_DEAD) || (inst.result != kNoId && (temps_[inst.result].flags & TF_ALIAS)))
				return;
			auto mark = [&](Value v) {
				v = resolve(v);
				if (v.kind == ValueKind::Temp)
					other_value_use[v.index] = 1;
			};
			if (inst.op == Opcode::Phi)
			{
				for (uint32_t k = 1; k < inst.extra_count; k += 2)
					mark(body.operands[inst.extra_first + k]);
				return;
			}
			if (inst.op == Opcode::Call)
			{
				mark(inst.ops[0]);
				return;
			}
			for_each_operand(body, inst, [&](Value v, OperandRole role) {
				if (role == OperandRole::Value)
					mark(v);
			});
		});
	}
	// A base that stops being lazy after an index composed through it (a
	// later index cannot combine with it) leaves that index's expression
	// naming parts nothing reads there: the pass runs again (O1+; -O0
	// keeps its shapes) with such bases never lazy, until none stops.
	std::vector<uint8_t> eager(temps_.size(), 0);
	for (bool again = true; again;)
	{
		again = false;
		addrs_.clear();
		for_each_inst([&](uint32_t id, uint32_t) {
			const Instruction& inst = body.instructions[id];
			if ((inst.op != Opcode::Addr && inst.op != Opcode::Index) || inst.result == kNoId)
				return;
			TempInfo& t = temps_[inst.result];
			t.addr = kNoId;
			if (t.flags & TF_LAZY)
			{
				t.flags &= ~TF_LAZY;
				--stats_.lazy_addresses;
			}
		});
		for_each_inst([&](uint32_t id, uint32_t) {
			const Instruction& inst = body.instructions[id];
			if ((inst.op != Opcode::Addr && inst.op != Opcode::Index) || (marks_[id] & IM_DEAD) ||
				(temps_[inst.result].flags & TF_ALIAS))
				return;
			AddrExpr expr;
			if (!address_expr(id, expr))
			{
				// A lazy base this index cannot combine with (a symbol with a
				// register index, a second index) becomes a register value.
				const Value base = resolve(inst.ops[0]);
				if (inst.op != Opcode::Index || base.kind != ValueKind::Temp || !(temps_[base.index].flags & TF_LAZY))
					return;
				temps_[base.index].flags &= ~TF_LAZY;
				--stats_.lazy_addresses;
				eager[base.index] = 1;
				again = level_ >= 1;
				if (!address_expr(id, expr))
					return;
			}
			TempInfo& t = temps_[inst.result];
			t.addr = static_cast<uint32_t>(addrs_.size());
			addrs_.push_back(expr);
			// A frame or pc-relative symbol address, or a register's plus a
			// displacement, passed to calls (and read through) holds no
			// register: each argument takes it again (emit_call), as a slot
			// argument does; one live across a call would take a callee-saved
			// register, and the reads fold.
			const bool remat = level_ >= 1 &&
				(expr.kind == AddrExpr::Frame || expr.kind == AddrExpr::Temp ||
					(expr.kind == AddrExpr::Symbol && symbol_access(expr.base) == SymbolAccess::Direct)) &&
				expr.index == kNoId && !other_value_use[inst.result];
			if (((t.flags & TF_VALUE_USE) && !remat) || t.uses == 0)
				return;
			// An address read in other blocks folds there too at O1+ when its
			// register parts are function-level values themselves (their live
			// ranges take the folded uses, register_allocation.cpp).
			const bool registers = expr.kind == AddrExpr::Temp || expr.index != kNoId;
			auto global_part = [&](uint32_t part) {
				return part == kNoId || ((temps_[part].flags & TF_GLOBAL) && temps_[part].cls == VClass::Gpr);
			};
			if ((registers && (t.flags & TF_GLOBAL) &&
					!(level_ >= 1 && global_part(expr.kind == AddrExpr::Temp ? expr.base : kNoId) &&
						global_part(expr.index))) ||
				eager[inst.result])
				return;
			t.flags |= TF_LAZY;
			++stats_.lazy_addresses;
		});
	}
}

// O1+: an eq/ne compare of a Boolean with 0 or 1 compares the byte (the
// Boolean's setcc may leave bits above it, find_byte_booleans).
TypeKind FunctionSelector::compare_width(const Instruction& compare) const
{
	const TypeKind kind = compare.type.is_pointer() ? TypeKind::Ptr : compare.type.kind();
	const lowir::ComparePredicate pred = static_cast<lowir::ComparePredicate>(compare.sub);
	if (level_ < 1 || compare.op != Opcode::Compare || !compare.type.is_integer() || kind == TypeKind::I128 ||
		register_bytes(kind) == 1 || (pred != lowir::ComparePredicate::Eq && pred != lowir::ComparePredicate::Ne))
		return kind;
	for (unsigned k = 0; k < 2; ++k)
	{
		const Value v = resolve(compare.ops[k]);
		const Value o = resolve(compare.ops[1 - k]);
		if (v.kind == ValueKind::Temp && known_boolean(v.index) && o.kind == ValueKind::Integer &&
			literal(o).high == 0 && literal(o).low <= 1)
			return TypeKind::U8;
	}
	return kind;
}

// O1+: the materialized Booleans (compares) and one-byte call results
// (the psABI defines their low byte only) whose low byte is all any
// reader reads, so they need no extension (a compare's setcc writes that
// byte, bits 1-7 zero). Reading the byte: a one-byte store of the value, a
// compare at one byte, a compare (eq/ne) of a Boolean with 0 or 1, a
// branch on a Boolean or a one-byte value (both test the byte,
// emit_compare/emit_branch),
// and (host objects) a one-byte argument of a call to a function this
// unit defines (which extends a parameter it reads wider on entry). A
// phi, or an and, or or xor, passes its readers' needs to its operands;
// aliases (extensions, truncations of Booleans) pass reads through.
// Grown to a fixed point (uses may precede definitions in layout order).
void FunctionSelector::find_byte_booleans()
{
	const lowir::FunctionBody& body = *body_;
	if (level_ < 1)
		return;
	byte_only_.assign(temps_.size(), 0);
	bool any = false;
	for_each_inst([&](uint32_t id, uint32_t) {
		const Opcode op = body.instructions[id].op;
		any = any || ((op == Opcode::Compare || op == Opcode::Call) && !(marks_[id] & IM_DEAD));
	});
	if (!any)
		return;
	std::vector<uint8_t> wide(temps_.size(), 0);
	auto byte_literal = [&](Value v) {
		v = resolve(v);
		return v.kind == ValueKind::Integer && literal(v).high == 0 && literal(v).low <= 1;
	};
	// To the least fixed point. Only phis, and/or/xor and selects read
	// their operands according to `wide`; every other read marks the same
	// values each round, so rounds after the first revisit those alone.
	bool grown = false;
	// A read of v (by a reader that reads the low byte only when `safe`).
	auto read = [&](Value v, bool safe) {
		const Value r = resolve(v);
		if (r.kind == ValueKind::Temp && !safe && !wide[r.index])
		{
			wide[r.index] = 1;
			grown = true;
		}
	};
	auto boolean = [&](Value v) {
		const Value r = resolve(v);
		return r.kind == ValueKind::Temp && known_boolean(r.index);
	};
	std::vector<uint32_t> again;
	bool collecting = true;
	auto visit = [&](uint32_t id) {
		const Instruction& inst = body.instructions[id];
		if ((marks_[id] & IM_DEAD) || (inst.result != kNoId && (temps_[inst.result].flags & TF_ALIAS)))
			return;
		switch (inst.op)
		{
		case Opcode::Phi:
			if (collecting)
				again.push_back(id);
			for (uint32_t k = 1; k < inst.extra_count; k += 2)
				read(body.operands[inst.extra_first + k], !wide[inst.result]);
			return;
		case Opcode::Binary:
		{
			const lowir::BinaryOp op = static_cast<lowir::BinaryOp>(inst.sub);
			if ((op == lowir::BinaryOp::And || op == lowir::BinaryOp::Or || op == lowir::BinaryOp::Xor) &&
				inst.type.is_integer() && inst.type.kind() != TypeKind::I128)
			{
				if (collecting)
					again.push_back(id);
				read(inst.ops[0], !wide[inst.result]);
				read(inst.ops[1], !wide[inst.result]);
				return;
			}
			break;
		}
		case Opcode::Branch:
		case Opcode::Select:
		{
			// A select also passes its readers' needs to its operands.
			const Value r = resolve(inst.ops[0]);
			read(inst.ops[0],
				boolean(inst.ops[0]) ||
					(r.kind == ValueKind::Temp && body.temps[r.index].type.is_integer() &&
						register_bytes(body.temps[r.index].type.kind()) == 1));
			if (inst.op == Opcode::Select)
			{
				if (collecting)
					again.push_back(id);
				read(inst.ops[1], !wide[inst.result]);
				read(inst.ops[2], !wide[inst.result]);
			}
			return;
		}
		case Opcode::Store:
			read(inst.ops[0], register_bytes(inst.type.kind()) == 1);
			read(inst.ops[1], false);
			return;
		case Opcode::Compare:
		{
			const bool byte = inst.type.is_integer() && register_bytes(inst.type.kind()) == 1;
			const lowir::ComparePredicate pred = static_cast<lowir::ComparePredicate>(inst.sub);
			const bool test = pred == lowir::ComparePredicate::Eq || pred == lowir::ComparePredicate::Ne;
			read(inst.ops[0], byte || (test && boolean(inst.ops[0]) && byte_literal(inst.ops[1])));
			read(inst.ops[1], byte || (test && boolean(inst.ops[1]) && byte_literal(inst.ops[0])));
			return;
		}
		case Opcode::Call:
		{
			const Value target = resolve(inst.ops[0]);
			bool own = false;
			if (eh_model_ == EhModel::Host && target.kind == ValueKind::Symbol)
			{
				const lowir::Symbol& s = program_.symbols[target.index];
				own = s.kind == lowir::SymbolKind::FunctionDefinition && s.metadata.binding != ir_model::SBM_WEAK &&
					(s.metadata.flags & lowir::SF_REPLACEABLE) == 0;
			}
			const lowir::Signature* sig = own ? program_.call_boundary(inst) : nullptr;
			const lowir::Parameter* params = sig != nullptr ? program_.parameters_of(*sig) : nullptr;
			read(inst.ops[0], false);
			for (uint32_t k = 0; k < inst.extra_count; ++k)
				read(body.operands[inst.extra_first + k],
					params != nullptr && k < sig->parameter_count && params[k].type.is_integer() &&
						register_bytes(params[k].type.kind()) == 1);
			return;
		}
		default:
			break;
		}
		for_each_operand(body, inst, [&](Value v, OperandRole) { read(v, false); });
	};
	for_each_inst([&](uint32_t id, uint32_t) { visit(id); });
	collecting = false;
	while (grown)
	{
		grown = false;
		for (uint32_t id : again)
			visit(id);
	}
	for_each_inst([&](uint32_t id, uint32_t) {
		const Instruction& inst = body.instructions[id];
		if (inst.result == kNoId || (marks_[id] & IM_DEAD))
			return;
		if (inst.op == Opcode::Compare ||
			(inst.op == Opcode::Call && inst.type.is_integer() && register_bytes(inst.type.kind()) == 1))
			byte_only_[inst.result] = !wide[inst.result];
	});
}

namespace {

// The farthest a load folds into its user (find_folds), in instructions.
const int32_t kFoldDistance = 8;

} // namespace

void FunctionSelector::find_folds()
{
	const lowir::FunctionBody& body = *body_;
	// O1+: `x & y` (or `x & (1 << k)`) a branch tests, directly or as a
	// compare with zero fused into it, folds into the branch's test x, y
	// (bt x, k): its register operands live to the branch.
	auto test_fold = [&](Value x, uint32_t branch_id) {
		if (level_ < 1 || x.kind != ValueKind::Temp || x.index < body.parameter_count)
			return;
		const Type type = body.temps[x.index].type;
		if (!(type.is_integer() || type.is_pointer()) || type.kind() == TypeKind::I128 || type.kind() == TypeKind::I1)
			return;
		auto single = [&](Value v, lowir::BinaryOp op) -> const Instruction* {
			if (v.kind != ValueKind::Temp || v.index < body.parameter_count || body.temps[v.index].def == kNoId)
				return nullptr;
			const lowir::InstId def = body.temps[v.index].def;
			const TempInfo& t = temps_[v.index];
			const Instruction& d = body.instructions[def];
			if (d.op != Opcode::Binary || static_cast<lowir::BinaryOp>(d.sub) != op || d.type != type ||
				(t.flags & (TF_GLOBAL | TF_ALIAS | TF_FOLDED | TF_LAZY)) || t.uses != 1 || (marks_[def] & IM_DEAD) ||
				inst_block_[def] != inst_block_[branch_id])
				return nullptr;
			return &d;
		};
		const Instruction* conj = single(x, lowir::BinaryOp::And);
		if (conj == nullptr)
			return;
		// No operand a folded load (its read would move past the
		// instructions between the and and the branch).
		for (unsigned k = 0; k < 2; ++k)
		{
			const Value v = resolve(conj->ops[k]);
			if (v.kind == ValueKind::Temp && (temps_[v.index].flags & TF_FOLDED))
				return;
		}
		temps_[x.index].flags |= TF_TESTED;
		const int32_t at = pos_[branch_id];
		auto extend = [&](Value v) {
			v = resolve(v);
			if (v.kind != ValueKind::Temp)
				return;
			TempInfo& c = temps_[v.index];
			if (c.block == inst_block_[branch_id] && !(c.flags & TF_GLOBAL) && c.last < at)
				c.last = at;
		};
		for (unsigned k = 0; k < 2; ++k)
		{
			const Value v = resolve(conj->ops[k]);
			const Instruction* bit = type.is_integer() && value_bytes(type) >= 4 ? single(v, lowir::BinaryOp::Shl)
																				 : nullptr;
			const Value one = bit != nullptr ? resolve(bit->ops[0]) : Value::none();
			if (bit != nullptr && one.kind == ValueKind::Integer && literal(one).low == 1 && literal(one).high == 0)
			{
				temps_[v.index].flags |= TF_TESTED;
				extend(bit->ops[1]);
				extend(conj->ops[1 - k]);
				return;
			}
		}
		extend(conj->ops[0]);
		extend(conj->ops[1]);
	};

	for_each_inst([&](uint32_t id, uint32_t b) {
		const Instruction& inst = body.instructions[id];
		if ((marks_[id] & IM_DEAD) || inst.result == kNoId)
			return;
		TempInfo& t = temps_[inst.result];
		if ((t.flags & (TF_ALIAS | TF_GLOBAL)) || t.uses != 1 || t.user == kNoId)
			return;
		const uint32_t user_id = t.user;
		if (sunk_compare(id))
		{
			// Fused into the branch ending the block past other instructions:
			// the operands live until the branch.
			t.flags |= TF_FUSED;
			++stats_.fused_compares;
			for (unsigned k = 0; k < 2; ++k)
			{
				const Value v = inst.ops[k].kind == ValueKind::None ? Value::none() : resolve(inst.ops[k]);
				if (v.kind != ValueKind::Temp)
					continue;
				TempInfo& c = temps_[v.index];
				if (c.block == b && !(c.flags & TF_GLOBAL) && c.last < pos_[user_id])
					c.last = pos_[user_id];
			}
			return;
		}
		if (inst_block_[user_id] != b || pos_[user_id] <= pos_[id])
			return;
		// O1+: a load folds into a later user of its block past a few
		// instructions that write no memory (a hash step's rotation between
		// the field load and the xor reading it); everything else folds
		// into the next instruction only.
		if (pos_[user_id] != pos_[id] + 1)
		{
			if (level_ < 1 || inst.op != Opcode::Load || pos_[user_id] - pos_[id] > kFoldDistance)
				return;
			for (lowir::InstId i = inst.next; i != kNoId && i != user_id; i = body.instructions[i].next)
			{
				const Instruction& between = body.instructions[i];
				if (marks_[i] & IM_DEAD)
					continue;
				const bool quiet = between.op == Opcode::Binary || between.op == Opcode::Unary ||
					between.op == Opcode::Convert || between.op == Opcode::Compare || between.op == Opcode::Copy ||
					between.op == Opcode::Const || between.op == Opcode::Index || between.op == Opcode::Addr ||
					between.op == Opcode::Select ||
					(between.op == Opcode::Load && (between.flags & lowir::IF_VOLATILE) == 0);
				if (!quiet ||
					(between.op == Opcode::Binary &&
						(static_cast<lowir::BinaryOp>(between.sub) == lowir::BinaryOp::Div ||
							static_cast<lowir::BinaryOp>(between.sub) == lowir::BinaryOp::UDiv ||
							static_cast<lowir::BinaryOp>(between.sub) == lowir::BinaryOp::Mod ||
							static_cast<lowir::BinaryOp>(between.sub) == lowir::BinaryOp::UMod)))
					return;
			}
		}
		const Instruction& user = body.instructions[user_id];
		const Value self = Value::temp(inst.result);
		const bool left = resolve(user.ops[0]) == self;
		const bool right = resolve(user.ops[1]) == self;
		const bool logical_not = inst.op == Opcode::Unary &&
			static_cast<lowir::UnaryOp>(inst.sub) == lowir::UnaryOp::Not &&
			(inst.type.is_integer() || inst.type.is_pointer()) && inst.type.kind() != TypeKind::I128;
		// O2+: an integer compare the next instruction's select reads.
		if ((inst.op == Opcode::Compare || logical_not) && user.op == Opcode::Select && left &&
			!(resolve(user.ops[1]) == self) && !(resolve(user.ops[2]) == self) && inst.type.kind() != TypeKind::I128 &&
			(inst.type.is_integer() || inst.type.is_pointer()))
		{
			t.flags |= TF_FUSED;
			++stats_.fused_compares;
			return;
		}
		if ((inst.op == Opcode::Compare || logical_not) && user.op == Opcode::Branch && left)
		{
			const TypeKind k = inst.type.kind();
			if (k != TypeKind::F80)
			{
				t.flags |= TF_FUSED;
				++stats_.fused_compares;
				const Instruction& cmp = inst;
				const Value zero = cmp.op == Opcode::Compare ? resolve(cmp.ops[1]) : Value::none();
				const lowir::ComparePredicate pred = static_cast<lowir::ComparePredicate>(cmp.sub);
				if (zero.kind == ValueKind::Integer && (literal(zero).low | literal(zero).high) == 0 &&
					(pred == lowir::ComparePredicate::Eq || pred == lowir::ComparePredicate::Ne))
					test_fold(resolve(cmp.ops[0]), user_id);
			}
			return;
		}
		if (inst.op == Opcode::Binary && static_cast<lowir::BinaryOp>(inst.sub) == lowir::BinaryOp::And &&
			user.op == Opcode::Branch && left)
		{
			test_fold(self, user_id);
			return;
		}
		// O1+: a load only the next instruction's branch tests compares in
		// memory (cmp byte [m], 0).
		const bool branch_test = level_ >= 1 && user.op == Opcode::Branch && left;
		// (O1+: a pointer read as an i64 through a retyping copy, the copy
		// an alias, reads the same eight bytes.)
		const bool retyped = level_ >= 1 &&
			((user.type.kind() == TypeKind::I64 && inst.type.is_pointer()) ||
				(user.type.is_pointer() && inst.type.kind() == TypeKind::I64));
		if (inst.op != Opcode::Load || (inst.flags & lowir::IF_VOLATILE) || t.cls != VClass::Gpr || left == right ||
			(user.type != inst.type && !branch_test && !retyped))
			return;
		const TypeKind k = inst.type.kind();
		bool ok = branch_test && k != TypeKind::I128;
		// (A Boolean's byte compares in memory at O1+; -O0 keeps the
		// reference's shapes.)
		if (user.op == Opcode::Compare)
			ok = (k != TypeKind::I1 || level_ >= 1) && !sunk_compare(user_id);
		else if (user.op == Opcode::Binary)
		{
			const lowir::BinaryOp op = static_cast<lowir::BinaryOp>(user.sub);
			const bool wide = k == TypeKind::I64 || k == TypeKind::Ptr || k == TypeKind::I32 || k == TypeKind::U32;
			const bool commutative = op == lowir::BinaryOp::Add || op == lowir::BinaryOp::And ||
				op == lowir::BinaryOp::Or || op == lowir::BinaryOp::Xor || op == lowir::BinaryOp::Mul;
			ok = wide && (commutative || (op == lowir::BinaryOp::Sub && right));
		}
		if (!ok)
			return;
		t.flags |= TF_FOLDED;
		++stats_.folded_loads;
		const Value address = resolve(inst.ops[0]);
		if (address.kind == ValueKind::Temp)
		{
			TempInfo& a = temps_[address.index];
			if (a.block == b && a.last < pos_[user_id])
				a.last = pos_[user_id];
		}
	});
}

// O1+: `store (op (load m), x), m`, the load and the operation used once
// each and nothing writing memory between the load and the store, is the
// operation on memory: add, sub, and, or, xor [m], x (x an immediate or a
// register value), or a pointer index by a constant adding its byte
// offset. A narrow store of a wider operation's truncation, its loaded
// operand extended from the store's width, is the narrow operation (the
// low bits depend on the low bits only). The load folds; the extension,
// the operation and the truncation emit nothing; x is read at the store.
// And `(x << k) | (x >> (w - k))` of a 64-bit or u32 x (w its width, 0 < k
// < w; the shifts in either order, or an xor or add, whose operands' bits
// are disjoint too), each shift used once, by the or in its block, is a
// rotation of x: the shifts emit nothing (TF_ROTATED, read at the or) and
// the or rotates x (emit_binary).
void FunctionSelector::find_rmw()
{
	const lowir::FunctionBody& body = *body_;
	rotate_.assign(level_ >= 1 ? temps_.size() : 0, 0);
	if (level_ < 1)
		return;
	const uint16_t taken = TF_GLOBAL | TF_ALIAS | TF_DEAD | TF_LAZY | TF_FUSED | TF_TESTED | TF_ROTATED | TF_RMW;
	// The definition of v, a value of block b used once, free to fold.
	auto single = [&](Value v, uint32_t b, uint16_t also) -> lowir::InstId {
		if (v.kind != ValueKind::Temp || v.index < body.parameter_count)
			return kNoId;
		const lowir::InstId def = body.temps[v.index].def;
		const TempInfo& t = temps_[v.index];
		if (def == kNoId || (marks_[def] & IM_DEAD) || inst_block_[def] != b || t.uses != 1 ||
			(t.flags & (taken | also)) || t.cls != VClass::Gpr)
			return kNoId;
		return def;
	};
	auto convert = [&](const Instruction& inst, lowir::ConvertOp a, lowir::ConvertOp c) {
		const lowir::ConvertOp op = static_cast<lowir::ConvertOp>(inst.sub);
		return inst.op == Opcode::Convert && (op == a || op == c) && inst.type.is_integer() &&
			lowir::convert_source_type(inst).is_integer() && inst.type.kind() != TypeKind::I128 &&
			lowir::convert_source_type(inst).kind() != TypeKind::I128;
	};
	for_each_inst([&](uint32_t id, uint32_t b) {
		const Instruction& store = body.instructions[id];
		if (store.op != Opcode::Store || (marks_[id] & IM_DEAD) || (store.flags & lowir::IF_VOLATILE))
			return;
		const TypeKind kind = store.type.kind();
		if (!(store.type.is_integer() || store.type.is_pointer()) || kind == TypeKind::I128 || kind == TypeKind::I1)
			return;
		const uint32_t bytes = value_bytes(store.type);
		const Value v = resolve(store.ops[0]);
		lowir::InstId op_id = single(v, b, TF_FOLDED);
		if (op_id == kNoId)
			return;
		lowir::InstId trunc_id = kNoId;
		if (convert(body.instructions[op_id], lowir::ConvertOp::Trunc, lowir::ConvertOp::Trunc) && bytes < 8 &&
			body.instructions[op_id].type.kind() == kind)
		{
			trunc_id = op_id;
			op_id = single(resolve(body.instructions[trunc_id].ops[0]), b, TF_FOLDED);
			if (op_id == kNoId)
				return;
		}
		const Instruction& op = body.instructions[op_id];
		// A wider integer operation stored narrowly (its truncation elided
		// or not).
		const bool narrow = store.type.is_integer() && op.op == Opcode::Binary && op.type.is_integer() &&
			value_bytes(op.type) > bytes;
		if (!narrow && (trunc_id != kNoId || body.temps[op.result].type.kind() != kind))
			return;
		// The load of m, through an extension from the store's width when
		// the store is narrower.
		lowir::InstId ext_id = kNoId;
		auto load_of = [&](Value x) -> lowir::InstId {
			ext_id = kNoId;
			lowir::InstId d = single(x, b, 0);
			if (d != kNoId && narrow && convert(body.instructions[d], lowir::ConvertOp::ZExt, lowir::ConvertOp::SExt))
			{
				if (value_bytes(lowir::convert_source_type(body.instructions[d])) != bytes)
					return kNoId;
				ext_id = d;
				d = single(resolve(body.instructions[d].ops[0]), b, 0);
			}
			if (d == kNoId)
				return kNoId;
			const Instruction& load = body.instructions[d];
			if (load.op != Opcode::Load || (load.flags & lowir::IF_VOLATILE) || load.type.kind() != kind ||
				!(resolve(load.ops[0]) == resolve(store.ops[1])))
				return kNoId;
			return d;
		};
		lowir::InstId load_id = kNoId;
		Value other = Value::none();
		if (op.op == Opcode::Binary && !op.type.is_float() && op.type.kind() != TypeKind::I128 &&
			value_bytes(op.type) >= bytes)
		{
			const lowir::BinaryOp bop = static_cast<lowir::BinaryOp>(op.sub);
			if (bop != lowir::BinaryOp::Add && bop != lowir::BinaryOp::Sub && bop != lowir::BinaryOp::And &&
				bop != lowir::BinaryOp::Or && bop != lowir::BinaryOp::Xor)
				return;
			const Value a = resolve(op.ops[0]), c = resolve(op.ops[1]);
			if (a == c)
				return;
			load_id = load_of(a);
			other = c;
			if (load_id == kNoId && bop != lowir::BinaryOp::Sub)
			{
				load_id = load_of(c);
				other = a;
			}
			if (other.kind == ValueKind::Integer)
			{
				if (bytes == 8 && !fits_imm32(imm_for(other, store.type)))
					return;
			}
			else if (other.kind != ValueKind::Temp || (temps_[other.index].flags & (TF_FOLDED | TF_RMW)) ||
				temps_[other.index].cls != VClass::Gpr)
				return;
		}
		else if (op.op == Opcode::Index && kind == TypeKind::Ptr && trunc_id == kNoId)
		{
			const Value c = resolve(op.ops[1]);
			if (c.kind != ValueKind::Integer || literal(c).high != 0 || literal(c).low > 0x7fffffffull ||
				literal(c).low * value_bytes(op.type) > 0x7fffffffull)
				return;
			load_id = load_of(resolve(op.ops[0]));
		}
		if (load_id == kNoId)
			return;
		// Nothing between the load and the store writes memory.
		for (lowir::InstId i = body.instructions[load_id].next; i != id && i != kNoId; i = body.instructions[i].next)
		{
			if (marks_[i] & IM_DEAD)
				continue;
			switch (body.instructions[i].op)
			{
			case Opcode::Binary:
			case Opcode::Unary:
			case Opcode::Convert:
			case Opcode::Compare:
			case Opcode::Select:
			case Opcode::Index:
			case Opcode::Addr:
			case Opcode::Copy:
			case Opcode::Const:
			case Opcode::Load:
				break;
			default:
				return;
			}
		}
		temps_[body.instructions[load_id].result].flags |= TF_FOLDED;
		temps_[op.result].flags |= TF_RMW;
		if (ext_id != kNoId)
			temps_[body.instructions[ext_id].result].flags |= TF_RMW;
		if (trunc_id != kNoId)
			temps_[body.instructions[trunc_id].result].flags |= TF_RMW;
		++stats_.folded_loads;
		if (other.kind == ValueKind::Temp)
		{
			TempInfo& x = temps_[other.index];
			if (x.block == b && !(x.flags & TF_GLOBAL) && x.last < pos_[id])
				x.last = pos_[id];
		}
	});
	// Rotations.
	const uint16_t folded = taken | TF_FOLDED;
	// The count of `x op k` defining v, used once, by `user` in block b, a
	// constant below the width; 0 otherwise.
	auto shift = [&](Value v, uint32_t user, uint32_t b, Type type, lowir::BinaryOp op, Value& x) -> uint64_t {
		if (v.kind != ValueKind::Temp || v.index < body.parameter_count)
			return 0;
		const lowir::InstId def = body.temps[v.index].def;
		const TempInfo& t = temps_[v.index];
		if (def == kNoId || (marks_[def] & IM_DEAD) || inst_block_[def] != b || t.uses != 1 || t.user != user ||
			(t.flags & folded) || t.cls != VClass::Gpr)
			return 0;
		const Instruction& d = body.instructions[def];
		if (d.op != Opcode::Binary || static_cast<lowir::BinaryOp>(d.sub) != op || d.type != type)
			return 0;
		const Value k = resolve(d.ops[1]);
		if (k.kind != ValueKind::Integer || literal(k).high != 0 || literal(k).low >= value_bytes(type) * 8)
			return 0;
		x = resolve(d.ops[0]);
		return literal(k).low;
	};
	for_each_inst([&](uint32_t id, uint32_t b) {
		const Instruction& inst = body.instructions[id];
		if ((marks_[id] & IM_DEAD) || inst.op != Opcode::Binary || inst.result == kNoId ||
			(temps_[inst.result].flags & (TF_ALIAS | TF_DEAD)))
			return;
		const lowir::BinaryOp op = static_cast<lowir::BinaryOp>(inst.sub);
		const TypeKind kind = inst.type.kind();
		if ((op != lowir::BinaryOp::Or && op != lowir::BinaryOp::Xor && op != lowir::BinaryOp::Add) ||
			(kind != TypeKind::I64 && kind != TypeKind::U32))
			return;
		const uint64_t width = value_bytes(inst.type) * 8;
		for (unsigned k = 0; k < 2; ++k)
		{
			const Value l = resolve(inst.ops[k]);
			const Value r = resolve(inst.ops[1 - k]);
			Value xl, xr;
			const uint64_t left = shift(l, id, b, inst.type, lowir::BinaryOp::Shl, xl);
			const uint64_t right = left != 0 ? shift(r, id, b, inst.type, lowir::BinaryOp::UShr, xr) : 0;
			if (right == 0 || left + right != width || !(xl == xr))
				continue;
			temps_[l.index].flags |= TF_ROTATED;
			temps_[r.index].flags |= TF_ROTATED;
			rotate_[inst.result] = static_cast<uint8_t>(left);
			if (xl.kind == ValueKind::Temp)
			{
				TempInfo& c = temps_[xl.index];
				if (c.block == b && !(c.flags & TF_GLOBAL) && c.last < pos_[id])
					c.last = pos_[id];
			}
			return;
		}
	});
}

// An integer compare or logical not whose only use is the condition of the
// branch ending its block, with other instructions between them: it is
// evaluated at the branch (README goal 7).
bool FunctionSelector::sunk_compare(uint32_t id) const
{
	const lowir::FunctionBody& body = *body_;
	const Instruction& inst = body.instructions[id];
	if (inst.result == kNoId || !(inst.type.is_integer() || inst.type.is_pointer()) ||
		inst.type.kind() == TypeKind::I128)
		return false;
	if (inst.op != Opcode::Compare &&
		!(inst.op == Opcode::Unary && static_cast<lowir::UnaryOp>(inst.sub) == lowir::UnaryOp::Not))
		return false;
	const TempInfo& t = temps_[inst.result];
	if ((t.flags & (TF_ALIAS | TF_GLOBAL)) || t.uses != 1 || t.user == kNoId)
		return false;
	const Instruction& user = body.instructions[t.user];
	return user.op == Opcode::Branch && inst_block_[t.user] == inst_block_[id] && pos_[t.user] > pos_[id] + 1 &&
		resolve(user.ops[0]) == Value::temp(inst.result);
}

// Where an instruction reads its operands: a folded load at its consumer,
// a fused compare at its branch, a tested and/shl at its compare's, a
// rotation's shift at its or.
int32_t FunctionSelector::read_pos(uint32_t id) const
{
	const Instruction& inst = body_->instructions[id];
	if (inst.result != kNoId)
	{
		const TempInfo& t = temps_[inst.result];
		if ((t.flags & (TF_FOLDED | TF_FUSED | TF_TESTED | TF_ROTATED | TF_RMW)) && t.user != kNoId &&
			inst_block_[t.user] == inst_block_[id])
			return read_pos(t.user);
	}
	return pos_[id];
}

// The parts of a lazy address are read wherever it folds: their register
// intervals in the defining block extend to its last use. That includes
// a part with a frame home (TF_GLOBAL), whose register is read until its
// in-block last use: an instruction reading the folded address may write
// one of its fixed registers before reading its operands (a compare's
// immediate scratch), which the part's mask must exclude.
void FunctionSelector::extend_lifetimes()
{
	for (size_t i = temps_.size(); i-- > 0;)
	{
		const TempInfo& t = temps_[i];
		if (!(t.flags & TF_LAZY) || t.addr == kNoId)
			continue;
		const AddrExpr& expr = addrs_[t.addr];
		const uint32_t parts[2] = { expr.kind == AddrExpr::Temp ? expr.base : kNoId, expr.index };
		for (unsigned k = 0; k < 2; ++k)
		{
			if (parts[k] == kNoId)
				continue;
			TempInfo& c = temps_[parts[k]];
			if (c.block == t.block && c.last < t.last)
				c.last = t.last;
		}
	}
}

} // namespace native
