#include "native/select/function_selector.h"
#include "native/select/for_each_inst.h"

#include <algorithm>

#include "native/backend_error.h"
#include "native/runtime/eh_record.h"
#include "native/select/frame_layout.h"
#include "native/select/mir_cleanup.h"
#include "native/select/operand_roles.h"
#include "support/internal_error.h"

// Control flow: the block driver, terminators (with compare/branch
// fusion), phi values as parallel transfers on their incoming edges (a
// conditional terminator's edge into a phi block gets its own MIR block so
// an untaken successor's copies never execute), the EH handler stack and
// the per-function driver.
namespace native {

using lowir::ComparePredicate;
using lowir::Instruction;
using lowir::kNoId;
using lowir::Opcode;
using lowir::Type;
using lowir::TypeKind;
using lowir::Value;
using lowir::ValueKind;

namespace {

bool conditional(const Instruction& term)
{
	return term.op == Opcode::Branch || term.op == Opcode::Switch;
}

} // namespace

void FunctionSelector::select(lowir::SymbolId symbol, Function& out)
{
	reset(symbol, out);
	out.wide_bulk = level_ >= 1;
	classify_signature(*signature_, entry_layout_);
	note_parameter_classes();
	build_cfg();
	if (eh_model_ == EhModel::Host)
		analyze_host_eh();
	analyze_slots();
	resolve_aliases();
	find_scaled_offsets();
	count_uses();
	eliminate_dead();
	place_uses();
	find_addresses();
	find_folds();
	find_rmw();
	find_byte_booleans();
	extend_lifetimes();
	compute_clobbers();
	for (uint32_t b = 0; b < blocks_.size(); ++b)
		if (blocks_[b].reachable)
			prepass_block(b);
	// Without a frame pointer (finish_frame decides it from the code: no
	// instruction moves RSP), RBP holds values: no stack allocation, no
	// variadic area, no i128 conversion through the stack, no call with
	// a stack argument area or an over-aligned one.
	// Local objects aligned beyond 16 are addressed from R12, reserved for
	// the function (Function::aligned_base); the frame pointer stays.
	for (uint32_t s = 0; s < body_->slots.size(); ++s)
	{
		const Type type = body_->slots[s].type;
		if (slot_state_[s] == SS_NORMAL && type.is_object() && value_align(type) > 16 &&
			value_align(type) > out.aligned_base)
			out.aligned_base = value_align(type);
	}
	excluded_ |= reserved_gprs(out);
	rbp_free_ = level_ >= 1 && eh_model_ == EhModel::Host && signature_->boundary.arity != ir_model::CAM_VARIADIC &&
		out.aligned_base == 0;
	for_each_inst([&](uint32_t id, uint32_t) {
		const lowir::Instruction& inst = body_->instructions[id];
		if (!rbp_free_ || (marks_[id] & IM_DEAD))
			return;
		if (inst.op == Opcode::StackAlloc || inst.op == Opcode::VaStart || inst.op == Opcode::VaArg)
			rbp_free_ = false;
		else if (inst.op == Opcode::Convert &&
			(inst.type.kind() == TypeKind::I128 || lowir::convert_source_type(inst).kind() == TypeKind::I128))
			rbp_free_ = false;
		else if (inst.op == Opcode::Call)
		{
			const lowir::Signature* sig = program_.call_boundary(inst);
			if (sig == nullptr)
				return;
			const CallLayout& layout = emission_layout(id, sig, false);
			rbp_free_ = layout.stack_bytes == 0 && layout.stack_align <= 16;
		}
	});
	allocate_globals();
	layout_slots();
	emit_blocks();
	finish_frame();
	if (level_ >= 1)
	{
		merge_constant_stores(out);
		forward_return_slots(out);
		forward_home_reloads(out);
		fuse_copies_into_lea(out);
		fuse_adds_and_extensions(out);
		drop_redundant_tests(out);
		drop_redundant_copies(out);
		forward_frame_copies(out, frame_objects_);
		stats_.dead_frame_stores += drop_dead_frame_stores(out, frame_objects_);
		drop_unused_frame(out);
		if (!frame_pinned_)
			stats_.ordered_frames += order_frame(out, frame_objects_) ? 1 : 0;
	}
	const uint32_t mir_instructions = level_ >= 1 ? layout_branches() : static_cast<uint32_t>(out.insts.size());
	++stats_.functions;
	stats_.lowir_instructions += body_->instructions.size();
	stats_.mir_instructions += mir_instructions;
}

namespace {

// The transfers of values spilled around loops on the edge from -> to.
std::pair<std::vector<SplitMove>::const_iterator, std::vector<SplitMove>::const_iterator> split_transfers(
	const std::vector<SplitMove>& moves, uint32_t from, uint32_t to)
{
	SplitMove key;
	key.from = from;
	key.to = to;
	return std::equal_range(moves.begin(), moves.end(), key,
		[](const SplitMove& a, const SplitMove& b) { return a.from != b.from ? a.from < b.from : a.to < b.to; });
}

bool split_edge(const std::vector<SplitMove>& moves, uint32_t from, uint32_t to)
{
	if (moves.empty())
		return false;
	const auto range = split_transfers(moves, from, to);
	return range.first != range.second;
}

} // namespace

// ---------------------------------------------------------------------------
// Blocks

void FunctionSelector::emit_blocks()
{
	const lowir::FunctionBody& body = *body_;
	const bool entry_loops = !blocks_.empty() && blocks_[0].preds != 0;
	uint32_t next = entry_loops ? 1 : 0;
	edge_first_.assign(blocks_.size() + 1, 0);
	edge_mir_.assign(blocks_.size(), kNoId);
	for (uint32_t b = 0; b < blocks_.size(); ++b)
	{
		edge_first_[b] = static_cast<uint32_t>(edges_.size());
		edge_first_[b + 1] = static_cast<uint32_t>(edges_.size());
		if (!blocks_[b].reachable)
			continue;
		// A host landing's pad (when a call names it) and selector block
		// precede its landing block.
		if (eh_model_ == EhModel::Host && eh_landing_index_[b] != kNoId)
		{
			EhLanding& landing = out_->eh.landings[eh_landing_index_[b]];
			if (eh_live_[b] == 2)
				landing.pad = pad_mir_[b] = next++;
			if (eh_selects(b, eh_landing_index_[b]))
				landing.select = select_mir_[b] = next++;
		}
		blocks_[b].mir = next++;
		const lowir::Block& block = body.blocks[blocks_[b].id];
		if (block.last == kNoId || !conditional(body.instructions[block.last]))
			continue;
		for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
		{
			const uint32_t s = succs_[k];
			if (!blocks_[s].has_phi && !eh_entry_edge(s) && !split_edge(split_moves_, b, s))
				continue;
			EdgeBlock edge;
			edge.from = b;
			edge.to = s;
			edge.mir = next++;
			edges_.push_back(edge);
			++stats_.edge_blocks;
		}
		edge_first_[b + 1] = static_cast<uint32_t>(edges_.size());
	}
	switches_.extra_mir = next;
	switches_.trees.clear();
	switches_.nodes.clear();
	if (entry_loops)
	{
		enter_block(0);
		position_ = -1;
		begin_mir_block(0, lowir::own_name(body.blocks[blocks_[0].id]), lowir::own_name(body.blocks[blocks_[0].id]));
		emit_entry();
		emit1(Op::Jmp, TypeKind::Void, Operand::make_label(blocks_[0].mir));
		end_mir_block();
	}
	for (uint32_t b = 0; b < blocks_.size(); ++b)
		if (blocks_[b].reachable)
			emit_block(b);
	// The switch trees' deferred subtrees (each may defer more).
	for (size_t k = 0; k < switches_.nodes.size(); ++k)
	{
		const SwitchNode node = switches_.nodes[k];
		begin_mir_block(0, kNoId, kNoId);
		if (out_->blocks.size() - 1 != node.mir)
			throw InternalError("switch subtree block out of order");
		location_ = switches_.trees[node.tree].location;
		emit_switch_tests(node.tree, node.first, node.past, node.lo, node.hi);
		end_mir_block();
	}
	for (const std::pair<uint32_t, Reg>& cleared : split_homes_.cleared)
		temps_[cleared.first].greg = cleared.second;
	split_homes_.cleared.clear();
}

// Block entry: no block-local value survives an edge; the cross-block
// values whose hulls overlap the block are bound to their registers.
void FunctionSelector::enter_block(uint32_t b)
{
	block_ = b;
	for (unsigned r = 0; r < 32; ++r)
		if (owner_[r] != kNoId)
			release(static_cast<Reg>(r));
	// Values homed in this block (plan_homes) are read from their homes.
	for (const std::pair<uint32_t, Reg>& cleared : split_homes_.cleared)
		temps_[cleared.first].greg = cleared.second;
	split_homes_.cleared.clear();
	if (!split_homes_.temps.empty())
		for (uint32_t k = split_homes_.first[b]; k < split_homes_.first[b + 1]; ++k)
		{
			const uint32_t temp = split_homes_.temps[k];
			split_homes_.cleared.push_back(std::make_pair(temp, temps_[temp].greg));
			temps_[temp].greg = kNoReg;
		}
	occupied_ = 0;
	written_ = 0;
	pinned_ = reserved_.empty() || precise_globals_ ? 0 : reserved_[b];
	exit_block_ = succ_first_[b] == succ_first_[b + 1];
	block_gregs_ = 0;
	if (precise_globals_)
	{
		// O1+: the values live into the block are bound; each is freed
		// after its last use here unless it leaves the block live.
		for (uint32_t k = block_globals_first_[b]; k < block_globals_first_[b + 1]; ++k)
		{
			const GlobalEntry& entry = block_entries_[k];
			TempInfo& t = temps_[entry.temp];
			t.block_last = entry.release;
			if (entry.live_in)
			{
				bind(entry.temp, t.greg);
				block_gregs_ |= 1u << t.greg;
			}
		}
	}
	else if (pinned_ != 0)
	{
		for (uint32_t k = block_globals_first_[b]; k < block_globals_first_[b + 1]; ++k)
			bind(block_globals_[k], temps_[block_globals_[k]].greg);
		if (exit_block_)
			note_hull_last_uses(b);
	}
	retain_parameters(b);
	written_ = 0;
}

// A parameter with a frame home is read from its incoming register in a
// block whose every predecessor (all emitted earlier) left that register
// intact, up to its last use in the block, when nothing clobbers the
// register before then.
void FunctionSelector::retain_parameters(uint32_t b)
{
	const uint32_t params = body_->parameter_count < 32 ? body_->parameter_count : 32;
	if (b == 0)
	{
		avail_in_ = 0;
		for (uint32_t p = 0; p < params; ++p)
			if ((temps_[p].flags & TF_GLOBAL) && temps_[p].greg == kNoReg && incoming_reg(p) != kNoReg &&
				temps_[p].cls == VClass::Gpr)
				avail_in_ |= 1u << p;
		return;
	}
	avail_in_ = 0;
	if (blocks_[b].handler || preds_first_.empty() || preds_first_[b] == preds_first_[b + 1])
		return;
	avail_in_ = ~0u;
	for (uint32_t k = preds_first_[b]; k < preds_first_[b + 1]; ++k)
	{
		const uint32_t q = preds_[k];
		avail_in_ &= emitted_[q] ? avail_out_[q] : 0;
	}
	for (uint32_t p = 0; p < params; ++p)
	{
		// (A value homed only in some blocks is in its register elsewhere.)
		if (!(avail_in_ & (1u << p)) || temps_[p].greg != kNoReg)
			continue;
		const Reg reg = incoming_reg(p);
		const size_t at = static_cast<size_t>(b) * 6 + arg_gpr_slot(reg);
		if (param_last_[at] < 0 || ((occupied_ | pinned_) & (1u << reg)) ||
			(precise_globals_ && (block_def_gregs_[b] & (1u << reg))))
			continue;
		temps_[p].block_last = param_last_[at];
		bind(p, reg);
	}
}

void FunctionSelector::emit_block(uint32_t b)
{
	const lowir::FunctionBody& body = *body_;
	enter_block(b);
	if (eh_model_ == EhModel::Host && eh_landing_index_[b] != kNoId)
		emit_landing_pad(b);
	begin_mir_block(lowir::own_name(body.blocks[blocks_[b].id]), kNoId, kNoId);
	location_ = 0;
	position_ = -1;
	if (b == 0 && blocks_[0].preds == 0)
		emit_entry();
	const size_t before = out_->blocks.size();
	for (lowir::InstId i = body.blocks[blocks_[b].id].first; i != kNoId; i = body.instructions[i].next)
		emit_inst(i);
	if (out_->blocks.size() == before)
		end_mir_block();
	uint32_t intact = avail_in_;
	written_ |= precise_globals_ ? block_gregs_ : pinned_;
	for (uint32_t p = 0; p < body.parameter_count && p < 32; ++p)
		if ((intact & (1u << p)) && (written_ & (1u << incoming_reg(p))))
			intact &= ~(1u << p);
	avail_out_[b] = intact;
	emitted_[b] = 1;
}

// The MIR block a conditional terminator of the current block jumps to for
// LowIR successor `to`: its edge block when `to` has phis or is a host
// catch-all-first landing.
uint32_t FunctionSelector::edge_target(lowir::BlockId to)
{
	const uint32_t s = layout_of_[to];
	return blocks_[s].has_phi || eh_entry_edge(s) || split_edge(split_moves_, block_, s) ? edge_mir_[s]
																						 : blocks_[s].mir;
}

void FunctionSelector::emit_edge_transfers(uint32_t from, uint32_t to)
{
	const lowir::FunctionBody& body = *body_;
	const lowir::BlockId from_id = blocks_[from].id;
	moves_.clear();
	for (lowir::InstId i = body.blocks[blocks_[to].id].first; i != kNoId; i = body.instructions[i].next)
	{
		const Instruction& phi = body.instructions[i];
		if (phi.op != Opcode::Phi)
			break;
		if (!live(i) || (temps_[phi.result].flags & TF_DEAD))
			continue;
		Value value = Value::none();
		if (phi.extra_count <= 16)
		{
			for (uint32_t k = 0; k + 1 < phi.extra_count; k += 2)
				if (body.operands[phi.extra_first + k].index == from_id)
					value = resolve(body.operands[phi.extra_first + k + 1]);
		}
		else
		{
			// A wide join: the phi's (predecessor, operand) pairs are sorted
			// once into a range of wide_phi_pairs_; the last pair naming the
			// predecessor counts, as in the scan above.
			const uint32_t pairs = phi.extra_count / 2;
			if (wide_phi_first_[i] == kNoId)
			{
				wide_phi_first_[i] = static_cast<uint32_t>(wide_phi_pairs_.size());
				for (uint32_t k = 0; k + 1 < phi.extra_count; k += 2)
					wide_phi_pairs_.push_back(std::make_pair(body.operands[phi.extra_first + k].index, k + 1));
				std::sort(wide_phi_pairs_.begin() + wide_phi_first_[i], wide_phi_pairs_.end());
			}
			const std::vector<std::pair<uint32_t, uint32_t>>::const_iterator first = wide_phi_pairs_.begin() +
				wide_phi_first_[i];
			const std::vector<std::pair<uint32_t, uint32_t>>::const_iterator past = std::upper_bound(
				first, first + pairs, std::make_pair(from_id, 0xffffffffu));
			if (past != first && (past - 1)->first == from_id)
				value = resolve(body.operands[phi.extra_first + (past - 1)->second]);
		}
		if (value.kind == ValueKind::None)
			continue;
		Move move;
		const Reg greg = temps_[phi.result].greg;
		move.dst = greg != kNoReg ? Operand::make_reg(greg) : Operand::make_frame(home_of(phi.result));
		move.type = phi.type.is_pointer() ? TypeKind::Ptr : phi.type.kind();
		switch (value.kind)
		{
		case ValueKind::Integer:
		case ValueKind::Float:
			move.src = imm_for(value, phi.type);
			break;
		case ValueKind::Temp:
			move.src = temp_location(value.index);
			// The home of a value an elided extension stands for holds it
			// at its own, narrower width.
			if (move.src.kind == OperandKind::Mem && temps_[value.index].cls == VClass::Gpr &&
				value_bytes(body.temps[value.index].type) < value_bytes(phi.type))
				move.src_type = body.temps[value.index].type.kind();
			break;
		case ValueKind::Slot:
			move.src = Operand::make_frame(slot_offset_[value.index]);
			move.address = true;
			break;
		case ValueKind::Symbol:
			move.src = symbol_operand(value.index, 0);
			break;
		default:
			continue;
		}
		if (move.dst.kind == OperandKind::Reg)
			written_ |= 1u << move.dst.reg;
		moves_.push_back(move);
	}
	// Values spilled around loops: stored entering them, reloaded leaving.
	if (!split_moves_.empty())
	{
		const auto range = split_transfers(split_moves_, from, to);
		for (auto it = range.first; it != range.second; ++it)
		{
			const lowir::Type type = body.temps[it->temp].type;
			const Operand reg = Operand::make_reg(it->reg);
			const Operand home = Operand::make_frame(home_of(it->temp));
			Move move;
			move.dst = it->store ? home : reg;
			move.src = it->store ? reg : home;
			move.type = type.is_pointer() ? TypeKind::Ptr : type.kind();
			if (!it->store)
				written_ |= 1u << reg.reg;
			moves_.push_back(move);
		}
	}
	parallel_moves(moves_);
}

void FunctionSelector::emit_edge_blocks(uint32_t from)
{
	const lowir::FunctionBody& body = *body_;
	for (uint32_t k = edge_first_[from]; k < edge_first_[from + 1]; ++k)
	{
		const EdgeBlock edge = edges_[k];
		begin_mir_block(
			0, lowir::own_name(body.blocks[blocks_[from].id]), lowir::own_name(body.blocks[blocks_[edge.to].id]));
		// Host EH: an edge into a catch-all-first landing, as a jump.
		if (eh_entry_edge(edge.to))
		{
			Inst& clause = emit1(Op::CatchAll, TypeKind::Void, Operand::make_frame(out_->eh.slot + 8));
			clause.aux = static_cast<uint32_t>(eh_entry_selector(edge.to));
		}
		if (blocks_[edge.to].has_phi || split_edge(split_moves_, from, edge.to))
			emit_edge_transfers(from, edge.to);
		emit1(Op::Jmp, TypeKind::Void, Operand::make_label(blocks_[edge.to].mir));
		end_mir_block();
	}
}

// ---------------------------------------------------------------------------
// Terminators

void FunctionSelector::emit_terminator(uint32_t id)
{
	const Instruction& inst = body_->instructions[id];
	switch (inst.op)
	{
	case Opcode::Jump:
	{
		// Host EH: a jump to a resume-only block resumes (host_eh.cpp).
		if (eh_model_ == EhModel::Host && eh_sites_[id] != 0)
		{
			emit_resume(eh_sites_[id]);
			break;
		}
		const uint32_t s = layout_of_[inst.ops[0].index];
		// Host EH: a jump into a catch-all-first landing (host_eh.cpp).
		if (eh_model_ == EhModel::Host)
			if (const int64_t entry = eh_entry_selector(s))
			{
				Inst& clause = emit1(Op::CatchAll, TypeKind::Void, Operand::make_frame(out_->eh.slot + 8));
				clause.aux = static_cast<uint32_t>(entry);
			}
		if (blocks_[s].has_phi || split_edge(split_moves_, block_, s))
			emit_edge_transfers(block_, s);
		emit1(Op::Jmp, TypeKind::Void, Operand::make_label(blocks_[s].mir));
		break;
	}
	case Opcode::Branch:
	case Opcode::Switch:
		for (uint32_t k = edge_first_[block_]; k < edge_first_[block_ + 1]; ++k)
			edge_mir_[edges_[k].to] = edges_[k].mir;
		if (inst.op == Opcode::Branch)
			emit_branch(inst);
		else
			emit_switch(inst);
		break;
	case Opcode::Return:
		emit_return(inst);
		break;
	case Opcode::Unreachable:
		emit(Op::Trap);
		break;
	case Opcode::Throw:
	{
		if (eh_model_ == EhModel::Host)
			throw BackendError("host EH: a throw instruction (host objects throw through the eh_throw runtime role)");
		const Type type = inst.type;
		if (!(type.is_integer() || type.is_pointer()) || type.kind() == TypeKind::I128)
			throw BackendError("throw of a non-integer payload is not supported");
		emit1(Op::Throw, type.kind(), rm_operand(inst.ops[0], type, kSelectScratch));
		break;
	}
	case Opcode::Resume:
		// Host EH: a record left in the frame takes the exception at its
		// landing (its selector block, else its block: past the pad);
		// otherwise the resume terminal.
		emit_resume(eh_model_ == EhModel::Host ? eh_sites_[id] : 0);
		break;
	default:
		throw InternalError("unsupported instruction");
	}
	end_mir_block();
	emit_edge_blocks(block_);
}

// A resume: into the landing block site - 1 (its selector block, else its
// block: past the pad), or through the resume terminal (0, or a jump's
// kResumeTerminal).
void FunctionSelector::emit_resume(uint32_t site)
{
	if (site == 0 || site == kResumeTerminal)
	{
		emit(Op::Resume);
		return;
	}
	const uint32_t target = site - 1;
	emit1(Op::Jmp, TypeKind::Void,
		Operand::make_label(select_mir_[target] != kNoId ? select_mir_[target] : blocks_[target].mir));
}

void FunctionSelector::emit_branch(const Instruction& inst)
{
	const uint32_t on_true = edge_target(inst.ops[1].index);
	const uint32_t on_false = edge_target(inst.ops[2].index);
	const Value cond = resolve(inst.ops[0]);
	if (cond.kind == ValueKind::Temp && (temps_[cond.index].flags & (TF_FUSED | TF_TESTED)))
	{
		emit_compare_jump(body_->instructions[body_->temps[cond.index].def], on_true, on_false);
		return;
	}
	if (cond.kind == ValueKind::Integer)
	{
		const bool taken = (literal(cond).low | literal(cond).high) != 0;
		emit1(Op::Jmp, TypeKind::Void, Operand::make_label(taken ? on_true : on_false));
		return;
	}
	const Type type = type_of(cond);
	Operand c = rm_operand(cond, type, kSelectScratch);
	if (c.kind == OperandKind::Symbol)
	{
		emit1(Op::Jmp, TypeKind::Void, Operand::make_label(on_true));
		return;
	}
	// A value whose definition never executes (temp_location): any bits.
	if (c.kind == OperandKind::Imm)
	{
		emit1(Op::Jmp, TypeKind::Void, Operand::make_label((c.imm | c.imm_high) != 0 ? on_true : on_false));
		return;
	}
	// An i128 condition is true when either half is nonzero.
	if (type.kind() == TypeKind::I128)
	{
		Inst& mir = emit3(Op::WideCmpBranch, TypeKind::I128, c, Operand::make_imm(0), Operand::make_label(on_true));
		mir.sub = static_cast<uint8_t>(ComparePredicate::Ne);
		mir.aux = on_false;
		return;
	}
	// A Boolean's byte (find_byte_booleans).
	const bool boolean = level_ >= 1 && cond.kind == ValueKind::Temp && known_boolean(cond.index);
	emit2(Op::Cmp, boolean ? TypeKind::U8 : type.is_pointer() ? TypeKind::Ptr : type.kind(), c, Operand::make_imm(0));
	Inst& jcc = emit1(Op::Jcc, TypeKind::Void, Operand::make_label(on_true));
	jcc.sub = CC_NE;
	emit1(Op::Jmp, TypeKind::Void, Operand::make_label(on_false));
}

void FunctionSelector::emit_compare_jump(const Instruction& compare, uint32_t on_true, uint32_t on_false)
{
	// A compare with zero of a tested `x & y` (find_folds): test x, y, or
	// bt x, k for `x & (1 << k)`; the branch takes ZF or CF.
	auto emit_test_jump = [&](const Instruction& conj, bool set) {
		const Type type = conj.type;
		const TypeKind kind = type.is_pointer() ? TypeKind::Ptr : type.kind();
		Value x = resolve(conj.ops[0]), y = resolve(conj.ops[1]);
		const auto tested = [&](Value v) { return v.kind == ValueKind::Temp && (temps_[v.index].flags & TF_TESTED); };
		if (tested(x))
			std::swap(x, y);
		Cond cond = set ? CC_NE : CC_E;
		if (tested(y))
		{
			// bt x, k: the bit number in a register or an immediate, x in a register.
			const Instruction& bit = body_->instructions[body_->temps[y.index].def];
			const Operand k = rm_operand(bit.ops[1], body_->temps[y.index].type, kEmitScratch);
			Operand a = rm_operand(x, type, kSelectScratch);
			if (a.kind != OperandKind::Reg)
			{
				load_into(kSelectScratch, a, kind);
				a = Operand::make_reg(kSelectScratch);
			}
			Operand b = k;
			if (b.kind == OperandKind::Mem)
			{
				load_into(kEmitScratch, b, kind);
				b = Operand::make_reg(kEmitScratch);
			}
			emit2(Op::Bt, kind, a, b);
			cond = set ? CC_B : CC_AE;
		}
		else
		{
			Operand a = rm_operand(x, type, kSelectScratch);
			Operand b = rm_operand(y, type, kEmitScratch);
			if (a.kind == OperandKind::Imm || (a.kind != OperandKind::Reg && b.kind == OperandKind::Reg))
				std::swap(a, b);
			if (a.kind == OperandKind::Imm || a.kind == OperandKind::Symbol)
			{
				load_into(kSelectScratch, a, kind);
				a = Operand::make_reg(kSelectScratch);
			}
			if (b.kind == OperandKind::Mem || b.kind == OperandKind::Symbol)
			{
				load_into(kEmitScratch, b, kind);
				b = Operand::make_reg(kEmitScratch);
			}
			emit2(Op::Test, kind, a, b);
		}
		Inst& j = emit1(Op::Jcc, TypeKind::Void, Operand::make_label(on_true));
		j.sub = cond;
		emit1(Op::Jmp, TypeKind::Void, Operand::make_label(on_false));
	};
	// A tested `and` the branch reads directly, or through a compare with zero.
	if (compare.op == Opcode::Binary)
	{
		emit_test_jump(compare, true);
		return;
	}
	const Value tested = compare.op == Opcode::Compare ? resolve(compare.ops[0]) : Value::none();
	if (tested.kind == ValueKind::Temp && (temps_[tested.index].flags & TF_TESTED))
	{
		emit_test_jump(body_->instructions[body_->temps[tested.index].def],
			static_cast<ComparePredicate>(compare.sub) == ComparePredicate::Ne);
		return;
	}
	const Type type = compare.type;
	const TypeKind kind = type.is_pointer() ? TypeKind::Ptr : type.kind();
	const Operand t = Operand::make_label(on_true);
	const Operand f = Operand::make_label(on_false);
	if (kind != TypeKind::I128 && !type.is_float())
	{
		const Cond cond = compare_flags(compare);
		Inst& j = emit1(Op::Jcc, TypeKind::Void, t);
		j.sub = cond;
		emit1(Op::Jmp, TypeKind::Void, f);
		return;
	}
	const ComparePredicate pred = static_cast<ComparePredicate>(compare.sub);
	Operand a = rm_operand(compare.ops[0], type, kSelectScratch);
	Operand b = rm_operand(compare.ops[1], type, kEmitScratch);
	if (kind == TypeKind::I128)
	{
		Inst& mir = emit3(Op::WideCmpBranch, kind, a, b, t);
		mir.sub = static_cast<uint8_t>(pred);
		mir.aux = on_false;
		return;
	}
	if (type.is_float())
	{
		// ucomis: CF for below or unordered, ZF for equal or unordered,
		// PF for unordered. Ordered predicates use the above forms.
		bool swap = pred == ComparePredicate::Lt || pred == ComparePredicate::Le || pred == ComparePredicate::Ugt ||
			pred == ComparePredicate::Uge;
		if (swap)
			std::swap(a, b);
		emit2(Op::FCmp, kind, a, b);
		auto jcc = [&](Cond cond, const Operand& label) {
			Inst& j = emit1(Op::Jcc, TypeKind::Void, label);
			j.sub = cond;
		};
		switch (pred)
		{
		case ComparePredicate::Eq:
			jcc(CC_P, f);
			jcc(CC_E, t);
			break;
		case ComparePredicate::Ne:
			jcc(CC_P, t);
			jcc(CC_NE, t);
			break;
		case ComparePredicate::Gt:
		case ComparePredicate::Lt:
			jcc(CC_A, t);
			break;
		case ComparePredicate::Ge:
		case ComparePredicate::Le:
			jcc(CC_AE, t);
			break;
		// The unordered-or predicates hold on CF (Ult, Ugt) or CF|ZF (Ule,
		// Uge) of the possibly swapped compare, as their materialized forms.
		case ComparePredicate::Ule:
		case ComparePredicate::Uge:
			jcc(CC_A, f);
			emit1(Op::Jmp, TypeKind::Void, t);
			return;
		default:
			jcc(CC_AE, f);
			emit1(Op::Jmp, TypeKind::Void, t);
			return;
		}
		emit1(Op::Jmp, TypeKind::Void, f);
		return;
	}
}

// The flags of an integer compare (or logical not: its operand against
// zero) fused into its reader, and the condition that holds when it does.
Cond FunctionSelector::compare_flags(const Instruction& compare)
{
	const Type type = compare.type;
	const TypeKind kind = type.is_pointer() ? TypeKind::Ptr : type.kind();
	const bool logical_not = compare.op == Opcode::Unary;
	ComparePredicate pred = logical_not ? ComparePredicate::Eq : static_cast<ComparePredicate>(compare.sub);
	Operand a = rm_operand(compare.ops[0], type, kSelectScratch);
	Operand b = logical_not ? Operand::make_imm(0) : rm_operand(compare.ops[1], type, kEmitScratch);
	if (b.kind == OperandKind::Symbol)
	{
		load_into(kEmitScratch, b, TypeKind::Ptr);
		b = Operand::make_reg(kEmitScratch);
	}
	if (a.kind == OperandKind::Symbol)
	{
		load_into(kSelectScratch, a, TypeKind::Ptr);
		a = Operand::make_reg(kSelectScratch);
	}
	if (a.kind == OperandKind::Imm && b.kind != OperandKind::Imm)
	{
		std::swap(a, b);
		pred = swapped(pred);
	}
	if (a.kind == OperandKind::Imm || (a.kind == OperandKind::Mem && b.kind == OperandKind::Mem))
	{
		load_into(kSelectScratch, a, kind);
		a = Operand::make_reg(kSelectScratch);
	}
	emit2(Op::Cmp, logical_not ? kind : compare_width(compare), a, b);
	return int_cond(pred);
}

void FunctionSelector::emit_switch(const Instruction& inst)
{
	const lowir::FunctionBody& body = *body_;
	// Cases compare at the selector's own type, which an extension folded
	// into a narrower source keeps (its register holds the extended value).
	const Value sel = resolve(inst.ops[0]);
	const bool typed = inst.ops[0].kind == ValueKind::Temp;
	const Type type = typed ? type_of(inst.ops[0]) : Type::scalar(TypeKind::I64);
	const TypeKind kind = type.is_pointer() ? TypeKind::Ptr : type.kind();
	const uint32_t fallback = edge_target(inst.ops[1].index);
	if (kind == TypeKind::I128)
	{
		// An i128 selector (in memory) compared with each case's value: the
		// equality in the emitter scratch, then a branch on it.
		const Operand wide = rm_operand(sel, type, kSelectScratch);
		for (uint32_t k = 0; k + 1 < inst.extra_count; k += 2)
		{
			const Value value = resolve(body.operands[inst.extra_first + k]);
			const uint32_t target = edge_target(body.operands[inst.extra_first + k + 1].index);
			Inst& equal = emit3(Op::WideSetcc, TypeKind::I128, Operand::make_reg(kEmitScratch), wide,
				rm_operand(value, type, kEmitScratch));
			equal.sub = static_cast<uint8_t>(ComparePredicate::Eq);
			emit2(Op::Cmp, TypeKind::I64, Operand::make_reg(kEmitScratch), Operand::make_imm(0));
			Inst& j = emit1(Op::Jcc, TypeKind::Void, Operand::make_label(target));
			j.sub = CC_NE;
		}
		emit1(Op::Jmp, TypeKind::Void, Operand::make_label(fallback));
		return;
	}
	Operand s = rm_operand(sel, type, kSelectScratch);
	if (s.kind != OperandKind::Reg)
	{
		load_into(kSelectScratch, s, kind == TypeKind::Ptr ? TypeKind::Ptr : kind);
		s = Operand::make_reg(kSelectScratch);
	}
	else if (eh_model_ == EhModel::Host && sel.kind == ValueKind::Temp && sel.index < body.parameter_count &&
		sel.index < narrow_observed_.size() && !narrow_observed_[sel.index] &&
		narrow_integer(body.temps[sel.index].type.kind()) && body.temps[sel.index].type.kind() != TypeKind::I1)
	{
		// A narrow parameter (or an extension aliased to it) the entry left
		// as the caller passed it (narrow_params_observed): extended here,
		// for the compares and the 64-bit index arithmetic below.
		const TypeKind from = body.temps[sel.index].type.kind();
		emit2(signed_integer(from) ? Op::Sext : Op::Zext, from, Operand::make_reg(kSelectScratch), s);
		s = Operand::make_reg(kSelectScratch);
	}
	if (level_ >= 1 && emit_clustered_switch(inst, s, kind, fallback))
		return;
	for (uint32_t k = 0; k + 1 < inst.extra_count; k += 2)
	{
		const Value value = resolve(body.operands[inst.extra_first + k]);
		const uint32_t target = edge_target(body.operands[inst.extra_first + k + 1].index);
		Operand c = rm_operand(value, Type::scalar(kind), kEmitScratch);
		if (c.kind == OperandKind::Symbol)
		{
			load_into(kEmitScratch, c, TypeKind::Ptr);
			c = Operand::make_reg(kEmitScratch);
		}
		emit2(Op::Cmp, kind, s, c);
		Inst& j = emit1(Op::Jcc, TypeKind::Void, Operand::make_label(target));
		j.sub = CC_E;
	}
	emit1(Op::Jmp, TypeKind::Void, Operand::make_label(fallback));
}

namespace {

// Jump tables (O1+): at least this many cases, at most this many entries,
// and at least one case per kJumpTableSpread entries (GCC's speed ratio).
const uint32_t kJumpTableMinCases = 5;
const uint64_t kJumpTableMaxEntries = 4096;
const uint64_t kJumpTableSpread = 10;
// A bit-test window: cases within 64 values for at most five targets,
// worth it from three clusters. A switch of more targets may take a jump
// table, whose indirect jump mispredicts more than a few predictable
// compares for few targets (as GCC's lowering avoids).
const uint32_t kBitTestTargets = 5;
const uint32_t kBitTestMinClusters = 3;

// A run of consecutive case values with one target.
struct CaseCluster
{
	int64_t lo;
	int64_t hi;
	uint32_t target;
};

bool fits_i32(int64_t v)
{
	return v >= INT32_MIN && v <= INT32_MAX;
}

// The values a register of an integer type holds (canonical form).
void type_bounds(TypeKind kind, int64_t& lo, int64_t& hi)
{
	switch (kind)
	{
	case TypeKind::I8:
		lo = INT8_MIN;
		hi = INT8_MAX;
		return;
	case TypeKind::U8:
		lo = 0;
		hi = UINT8_MAX;
		return;
	case TypeKind::I16:
		lo = INT16_MIN;
		hi = INT16_MAX;
		return;
	case TypeKind::U16:
		lo = 0;
		hi = UINT16_MAX;
		return;
	case TypeKind::I32:
		lo = INT32_MIN;
		hi = INT32_MAX;
		return;
	case TypeKind::U32:
		lo = 0;
		hi = UINT32_MAX;
		return;
	default:
		lo = INT64_MIN;
		hi = INT64_MAX;
		return;
	}
}

bool signed_kind(TypeKind kind)
{
	return kind == TypeKind::I8 || kind == TypeKind::I16 || kind == TypeKind::I32 || kind == TypeKind::I64;
}

} // namespace

// O1+: a switch as tests of its runs of consecutive values per target
// (one compare for a single value or a run reaching the type's bound, a
// subtract and an unsigned compare otherwise), the densest window of at
// least three runs within 64 values for at most five targets last as a
// bounds check and a bit test per target; a dense switch of more targets
// stays a jump table. False (linear compares) when a bound needs more
// than an imm32.
bool FunctionSelector::emit_clustered_switch(
	const Instruction& inst, const Operand& selector, TypeKind kind, uint32_t fallback)
{
	const lowir::FunctionBody& body = *body_;
	std::vector<std::pair<int64_t, uint32_t>> cases;
	for (uint32_t k = 0; k + 1 < inst.extra_count; k += 2)
	{
		const Value value = resolve(body.operands[inst.extra_first + k]);
		if (value.kind != ValueKind::Integer)
			return false;
		cases.push_back(std::make_pair(
			imm_for(value, Type::scalar(kind)).imm, edge_target(body.operands[inst.extra_first + k + 1].index)));
	}
	std::stable_sort(cases.begin(), cases.end(),
		[](const std::pair<int64_t, uint32_t>& a, const std::pair<int64_t, uint32_t>& b) { return a.first < b.first; });
	std::vector<CaseCluster> clusters;
	std::vector<uint32_t> targets;
	for (const std::pair<int64_t, uint32_t>& c : cases)
	{
		if (!clusters.empty() && clusters.back().hi == c.first)
			continue; // a repeated value: its first case decides
		if (!clusters.empty() && clusters.back().target == c.second && clusters.back().hi + 1 == c.first)
			clusters.back().hi = c.first;
		else
			clusters.push_back(CaseCluster{ c.first, c.first, c.second });
		if (std::find(targets.begin(), targets.end(), c.second) == targets.end())
			targets.push_back(c.second);
	}
	// A dense switch of more targets: a bounds check and a jump through a
	// table of its targets (holes take the default). The index is the
	// selector minus the lowest case in r10, compared unsigned against the
	// entry count. Registers hold an integer's canonical 64-bit form
	// (sign- or zero-extended by its type), and so do the cases (imm_for),
	// so the 64-bit difference is the entry for every integer type.
	const int64_t low = cases.front().first, high = cases.back().first;
	const uint64_t span = static_cast<uint64_t>(high) - static_cast<uint64_t>(low);
	if (targets.size() > kBitTestTargets && cases.size() >= kJumpTableMinCases && span < kJumpTableMaxEntries &&
		span + 1 <= kJumpTableSpread * cases.size() && fits_i32(low))
	{
		const uint32_t entries = static_cast<uint32_t>(span + 1);
		const uint32_t first = static_cast<uint32_t>(out_->jump_targets.size());
		out_->jump_targets.resize(first + entries, fallback);
		for (size_t k = cases.size(); k-- > 0;) // the first case of a repeated value wins
			out_->jump_targets[first +
				static_cast<uint32_t>(static_cast<uint64_t>(cases[k].first) - static_cast<uint64_t>(low))] =
				cases[k].second;
		const Operand table_index = Operand::make_reg(kSelectScratch);
		if (selector.reg != kSelectScratch)
			emit2(Op::Mov, TypeKind::I64, table_index, selector);
		if (low != 0)
			emit2(Op::Sub, TypeKind::I64, table_index, Operand::make_imm(low, low < 0 ? -1 : 0));
		emit2(Op::Cmp, TypeKind::I64, table_index, Operand::make_imm(entries - 1));
		Inst& bound = emit1(Op::Jcc, TypeKind::Void, Operand::make_label(fallback));
		bound.sub = CC_A;
		Inst& table = emit1(Op::JumpTable, TypeKind::Void, table_index);
		table.aux = first;
		table.aux2 = entries;
		++stats_.jump_tables;
		return true;
	}
	int64_t tmin = 0, tmax = 0;
	type_bounds(kind, tmin, tmax);
	// Instructions to test one cluster on its own.
	auto test_cost = [&](const CaseCluster& c) { return c.lo == c.hi || c.lo == tmin || c.hi == tmax ? 2 : 4; };
	// The window: the clusters within 64 values for few targets whose own
	// tests cost the most more than the window's (an index, a bounds check,
	// a mask and a bit test per target).
	// (Of windows saving as much, the one covering the most values: the
	// runs left outside it, tested one by one first, are then the fewest
	// values, and their branches mostly fall through.)
	size_t wfirst = clusters.size(), wpast = clusters.size();
	int best = 0;
	int64_t best_cover = 0;
	for (size_t i = 0; i < clusters.size(); ++i)
	{
		std::vector<uint32_t> seen;
		int own = 0;
		int64_t cover = 0;
		for (size_t k = i; k < clusters.size() && clusters[k].hi - clusters[i].lo < 64; ++k)
		{
			if (std::find(seen.begin(), seen.end(), clusters[k].target) == seen.end())
			{
				if (seen.size() == kBitTestTargets)
					break;
				seen.push_back(clusters[k].target);
			}
			own += test_cost(clusters[k]);
			cover += clusters[k].hi - clusters[k].lo + 1;
			const int saved = own - (4 + 3 * static_cast<int>(seen.size()));
			if (k + 1 - i >= kBitTestMinClusters &&
				(saved > best || (saved == best && saved > 0 && cover > best_cover)))
			{
				best = saved;
				best_cover = cover;
				wfirst = i;
				wpast = k + 1;
			}
		}
	}
	for (const CaseCluster& c : clusters)
		if (!fits_i32(c.lo) || !fits_i32(c.hi) || !fits_i32(c.hi - c.lo))
			return false;
	SwitchTree tree;
	tree.selector = selector.reg;
	tree.kind = kind;
	tree.sign = signed_kind(kind);
	tree.fallback = fallback;
	tree.location = location_;
	for (size_t i = 0; i < clusters.size(); ++i)
	{
		if (i >= wfirst && i < wpast)
		{
			if (i == wfirst)
				tree.tests.push_back(SwitchTest{ clusters[wfirst].lo, clusters[wpast - 1].hi, fallback, true });
			const CaseCluster& c = clusters[i];
			const int64_t lo = clusters[wfirst].lo;
			const uint64_t bits = (c.hi - c.lo == 63 ? ~0ull : ((1ull << (c.hi - c.lo + 1)) - 1)) << (c.lo - lo);
			size_t m = 0;
			while (m < tree.masks.size() && tree.masks[m].second != c.target)
				++m;
			if (m == tree.masks.size())
				tree.masks.push_back(std::make_pair(0ull, c.target));
			tree.masks[m].first |= bits;
			continue;
		}
		tree.tests.push_back(SwitchTest{ clusters[i].lo, clusters[i].hi, clusters[i].target, false });
	}
	// O2+: four tests or more with gaps between them (values the fallback
	// takes; a run of consecutive cases, an enumeration's, keeps its
	// order) search by halves. Not a selector in a scratch register,
	// which would not survive into the subtrees' blocks.
	uint32_t gaps = 0;
	for (size_t i = 1; i < tree.tests.size(); ++i)
		gaps += tree.tests[i].lo > tree.tests[i - 1].hi + 1 ? 1 : 0;
	tree.tree = level_ >= 2 && tree.tests.size() >= 4 && gaps >= 2 && selector.kind == OperandKind::Reg &&
		selector.reg < 16 && selector.reg != kSelectScratch && selector.reg != kEmitScratch;
	switches_.trees.push_back(tree);
	emit_switch_tests(
		static_cast<uint32_t>(switches_.trees.size() - 1), 0, static_cast<uint32_t>(tree.tests.size()), tmin, tmax);
	return true;
}

// Switch tree `index`'s tests [first, past), the selector known within
// [lo, hi]: four or more test whether it is below the middle test's lowest
// value (the lower half in a deferred block) and go on with the upper
// half; fewer test each run (one compare when a known bound meets it, a
// subtract and an unsigned compare otherwise), the window last (a bounds
// check unless known, then a bit test per target), then the fallback.
void FunctionSelector::emit_switch_tests(uint32_t index, uint32_t first, uint32_t past, int64_t lo, int64_t hi)
{
	const SwitchTree& t = switches_.trees[index];
	const Operand selector = Operand::make_reg(t.selector);
	auto imm = [](int64_t v) { return Operand::make_imm(v, v < 0 ? -1 : 0); };
	auto jump = [&](Cond cond, uint32_t target) {
		Inst& j = emit1(Op::Jcc, TypeKind::Void, Operand::make_label(target));
		j.sub = cond;
	};
	// selector - v in a scratch register the selector is not in (or in
	// place: a linear switch's selector in the scratch, for its last use).
	auto offset = [&](int64_t v, bool last) {
		const Reg r = t.selector != kSelectScratch || last ? kSelectScratch : kEmitScratch;
		if (t.selector != r)
			emit2(Op::Mov, TypeKind::I64, Operand::make_reg(r), selector);
		if (v != 0)
			emit2(Op::Sub, TypeKind::I64, Operand::make_reg(r), imm(v));
		return r;
	};
	if (t.tree && past - first >= 4)
	{
		const uint32_t middle = (first + past) / 2;
		const int64_t pivot = t.tests[middle].lo;
		const SwitchNode node = { index, first, middle, lo, pivot - 1, switches_.extra_mir++ };
		switches_.nodes.push_back(node);
		emit2(Op::Cmp, t.kind, selector, imm(pivot));
		jump(t.sign ? CC_L : CC_B, node.mir);
		emit_switch_tests(index, middle, past, pivot, hi);
		return;
	}
	// (The upper half's first test of a value the middle compare just
	// compared reads its flags.)
	bool flags = t.tree && first != 0 && past == static_cast<uint32_t>(t.tests.size()) && lo == t.tests[first].lo &&
		out_->insts.size() >= 2 && out_->insts.back().op == Op::Jcc;
	const SwitchTest* window = nullptr;
	for (uint32_t i = first; i < past; ++i)
	{
		const SwitchTest& c = t.tests[i];
		if (c.window)
		{
			window = &c;
			continue;
		}
		if (c.lo <= lo && c.hi >= hi)
		{
			emit1(Op::Jmp, TypeKind::Void, Operand::make_label(c.target));
			return;
		}
		if (c.lo == c.hi)
		{
			if (!(flags && i == first))
				emit2(Op::Cmp, t.kind, selector, imm(c.lo));
			jump(CC_E, c.target);
		}
		else if (c.lo <= lo)
		{
			emit2(Op::Cmp, t.kind, selector, imm(c.hi));
			jump(t.sign ? CC_LE : CC_BE, c.target);
		}
		else if (c.hi >= hi)
		{
			emit2(Op::Cmp, t.kind, selector, imm(c.lo));
			jump(t.sign ? CC_GE : CC_AE, c.target);
		}
		else
		{
			const Reg r = offset(c.lo, false);
			emit2(Op::Cmp, TypeKind::I64, Operand::make_reg(r), imm(c.hi - c.lo));
			jump(CC_BE, c.target);
		}
	}
	if (window != nullptr)
	{
		const Reg r = offset(window->lo, true);
		if (!(window->lo <= lo && window->hi >= hi))
		{
			emit2(Op::Cmp, TypeKind::I64, Operand::make_reg(r), imm(window->hi - window->lo));
			jump(CC_A, t.fallback);
		}
		for (const std::pair<uint64_t, uint32_t>& m : t.masks)
		{
			emit2(Op::Mov, TypeKind::I64, Operand::make_reg(kEmitScratch),
				Operand::make_imm(static_cast<int64_t>(m.first), static_cast<int64_t>(m.first) < 0 ? -1 : 0));
			emit2(Op::Bt, TypeKind::I64, Operand::make_reg(kEmitScratch), Operand::make_reg(r));
			jump(CC_B, m.second);
		}
	}
	emit1(Op::Jmp, TypeKind::Void, Operand::make_label(t.fallback));
}

// ---------------------------------------------------------------------------
// Exception handling (the course's handler-stack runtime ABI; the host
// model's regions are host_eh.cpp's static facts)

void FunctionSelector::emit_eh(uint32_t id)
{
	const Instruction& inst = body_->instructions[id];
	// Host EH regions and leading clauses are static facts (host_eh.cpp);
	// a catch-all after a landing's leading clauses sets the selector no
	// clause set.
	if (eh_model_ == EhModel::Host)
	{
		if (inst.op == Opcode::EhCatchAll && eh_sites_[id] != 0)
		{
			Inst& clause = emit1(Op::CatchAll, TypeKind::Void, Operand::make_frame(out_->eh.slot + 8));
			clause.aux = static_cast<uint32_t>(inst.immediate);
		}
		return;
	}
	switch (inst.op)
	{
	case Opcode::EhTry:
	case Opcode::EhCleanup:
	{
		// A bare `eh_cleanup` is a landing's cleanup clause: it installs no
		// handler, and the landing's own try record becomes a cleanup record
		// its `eh_end` pops (the `exception` read leaves it).
		if (inst.ops[0].kind != ValueKind::Block)
		{
			emit(Op::EhMark);
			return;
		}
		Inst& push = emit(Op::EhPush);
		push.ops[0] = Operand::make_label(blocks_[layout_of_[inst.ops[0].index]].mir);
		push.nops = 1;
		push.aux = inst.op == Opcode::EhTry ? kEhTry : kEhCleanup;
		if (!out_->eh_records_pushed)
			push.aux2 = static_cast<uint32_t>(frame_alloc(kEhRecordBytes, 8));
		return;
	}
	case Opcode::EhEnd:
		emit(Op::EhPop);
		return;
	// A clause of the landing being entered: the runtime matches the
	// exception against the clause's type (none for `...`) and, if no
	// earlier clause of the landing matched, sets the selector.
	case Opcode::EhCatch:
	case Opcode::EhCatchAll:
	{
		Inst& clause = emit(Op::EhCatch);
		clause.aux = static_cast<uint32_t>(inst.immediate);
		if (inst.op == Opcode::EhCatch)
		{
			clause.ops[0] = Operand::make_symbol(inst.ops[0].index);
			clause.nops = 1;
		}
		return;
	}
	default:
		throw BackendError("exception filters are not supported by the native runtime");
	}
}

} // namespace native
