#include "opt/passes.h"
#include "opt/scalar_fold.h"

namespace opt {

using namespace lowir;

namespace {

enum Lattice : uint8_t
{
	L_TOP, // not yet known (no executable definition)
	L_CONSTANT,
	L_BOTTOM // overdefined
};

// The operand type an instruction reads its operands at.
Type operand_type(const Instruction& inst)
{
	return inst.op == Opcode::Convert ? convert_source_type(inst) : inst.type;
}

class Propagator
{
public:
	explicit Propagator(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	// The containers, kept across runs for their capacity (each run
	// assigns or fills every one it reads; value_ is read only where the
	// lattice says constant, written with it).
	struct Pool
	{
		std::vector<uint8_t> lattice, executable, edge;
		std::vector<Constant> value;
		std::vector<uint32_t> succ_slot, user_first, users, user_operand, phi_first, phis, slot_input_first,
			slot_inputs, label_slot;
		std::vector<BlockId> block_work;
		std::vector<TempId> temp_work;
	};
	void exchange(Pool& pool)
	{
		lattice_.swap(pool.lattice);
		executable_.swap(pool.executable);
		edge_.swap(pool.edge);
		value_.swap(pool.value);
		succ_slot_.swap(pool.succ_slot);
		user_first_.swap(pool.user_first);
		users_.swap(pool.users);
		user_operand_.swap(pool.user_operand);
		phi_first_.swap(pool.phi_first);
		phis_.swap(pool.phis);
		slot_input_first_.swap(pool.slot_input_first);
		slot_inputs_.swap(pool.slot_inputs);
		label_slot_.swap(pool.label_slot);
		block_work_.swap(pool.block_work);
		temp_work_.swap(pool.temp_work);
	}
	bool run();

private:
	Lattice state(Value v, Constant& out) const;
	void lower(TempId temp, Lattice to, const Constant* value);
	void meet_input(TempId phi, Value input);
	void evaluate(InstId id);
	void evaluate_phi(InstId id);
	void evaluate_terminator(BlockId block, const Instruction& term);
	bool readonly_global(const Instruction& inst, Constant& out) const;
	void mark_successor(BlockId from, uint32_t k);
	void mark_edge(BlockId from, BlockId to);
	void visit_block(BlockId block);
	void index_phis();
	bool rewrite();

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	std::vector<uint8_t> lattice_;
	std::vector<Constant> value_;
	std::vector<uint8_t> executable_; // per block
	std::vector<uint8_t> edge_;       // per predecessor slot (cfg preds)
	std::vector<uint32_t> succ_slot_; // per cfg succs entry: its predecessor slot in the target
	// Users of each temporary (CSR): the instruction, and for a phi the
	// operand index of the input naming it (kNoId otherwise).
	std::vector<uint32_t> user_first_, users_, user_operand_;
	// Phi inputs by edge (a new edge meets one input of each phi): per
	// block, its phis (phi_first_) and, per (predecessor slot, phi), the
	// operand index of that edge's value (kNoId without one); per phi label
	// operand, its predecessor slot.
	std::vector<uint32_t> phi_first_, phis_;
	std::vector<uint32_t> slot_input_first_, slot_inputs_;
	std::vector<uint32_t> label_slot_;
	bool phis_present_ = false; // the phi tables are built only when true
	const std::vector<BlockId>* block_of_ = nullptr;
	std::vector<BlockId> block_work_;
	std::vector<TempId> temp_work_;
};

Lattice Propagator::state(Value v, Constant& out) const
{
	if (literal_constant(body_, v, out))
		return L_CONSTANT;
	if (!v.is(ValueKind::Temp))
		return L_BOTTOM;
	const Lattice l = static_cast<Lattice>(lattice_[v.index]);
	if (l == L_CONSTANT)
		out = value_[v.index];
	return l;
}

void Propagator::lower(TempId temp, Lattice to, const Constant* value)
{
	const Lattice from = static_cast<Lattice>(lattice_[temp]);
	if (to == from && (to != L_CONSTANT || same_constant(body_.temps[temp].type, value_[temp], *value)))
		return;
	// Monotone: a second, different constant is overdefined.
	if (from == L_BOTTOM)
		return;
	if (from == L_CONSTANT && to == L_CONSTANT)
		to = L_BOTTOM;
	if (to == L_TOP)
		return;
	lattice_[temp] = to;
	if (to == L_CONSTANT)
		value_[temp] = *value;
	temp_work_.push_back(temp);
}

bool Propagator::readonly_global(const Instruction& inst, Constant& out) const
{
	if ((inst.flags & IF_VOLATILE) != 0 || !inst.ops[0].is(ValueKind::Symbol))
		return false;
	const Symbol& symbol = c_.program.symbols[inst.ops[0].index];
	if (!constant_contents(c_.program, inst.ops[0].index, c_.unwritten) ||
		symbol.metadata.binding != ir_model::SBM_INTERNAL)
		return false;
	const Global& global = c_.program.globals[symbol.detail];
	if (!global.typed || global.type != inst.type || !inst.type.is_integer() || inst.type.kind() == TypeKind::I128)
		return false;
	if (global.init == GlobalInit::Zero)
	{
		Literal zero = { 0, 0 };
		out = integer_constant(inst.type, zero);
		return true;
	}
	if (global.init != GlobalInit::Constant || global.constant.kind != ValueKind::Integer)
		return false;
	out = integer_constant(inst.type, global.constant.bits);
	return true;
}

void Propagator::evaluate(InstId id)
{
	const Instruction& inst = body_.instructions[id];
	if (inst.op == Opcode::Phi)
	{
		evaluate_phi(id);
		return;
	}
	if (inst.result == kNoId)
	{
		if (is_terminator(inst.op))
			evaluate_terminator((*block_of_)[id], inst);
		return;
	}
	Constant a, b, r;
	Lattice la = L_CONSTANT, lb = L_CONSTANT;
	bool folded = false;
	const Type type = operand_type(inst);
	switch (inst.op)
	{
	case Opcode::Const:
	case Opcode::Copy:
		la = state(inst.ops[0], a);
		folded = la == L_CONSTANT && fold_copy(inst.type, a, r);
		break;
	case Opcode::Unary:
		la = state(inst.ops[0], a);
		folded = la == L_CONSTANT && fold_unary(static_cast<UnaryOp>(inst.sub), inst.type, a, r);
		break;
	case Opcode::Binary:
		la = state(inst.ops[0], a);
		lb = state(inst.ops[1], b);
		folded = la == L_CONSTANT && lb == L_CONSTANT &&
			fold_binary(static_cast<BinaryOp>(inst.sub), inst.type, a, b, r);
		break;
	case Opcode::Compare:
		la = state(inst.ops[0], a);
		lb = state(inst.ops[1], b);
		folded = la == L_CONSTANT && lb == L_CONSTANT &&
			fold_compare(static_cast<ComparePredicate>(inst.sub), inst.type, a, b, r);
		break;
	case Opcode::Convert:
		la = state(inst.ops[0], a);
		folded = la == L_CONSTANT && fold_convert(static_cast<ConvertOp>(inst.sub), inst.type, type, a, r);
		break;
	case Opcode::Select:
	{
		// A known condition takes its operand's state.
		Constant c;
		const Lattice lc = state(inst.ops[0], c);
		if (lc != L_CONSTANT || c.kind != ValueKind::Integer)
		{
			la = lc == L_TOP ? L_TOP : L_BOTTOM;
			break;
		}
		la = state((c.bits.low | c.bits.high) != 0 ? inst.ops[1] : inst.ops[2], a);
		folded = la == L_CONSTANT && fold_copy(inst.type, a, r);
		break;
	}
	case Opcode::Load:
		folded = readonly_global(inst, r);
		la = folded ? L_CONSTANT : L_BOTTOM;
		break;
	default:
		la = L_BOTTOM;
		break;
	}
	if (folded)
		lower(inst.result, L_CONSTANT, &r);
	else if (la == L_TOP || lb == L_TOP)
		return;
	else
		lower(inst.result, L_BOTTOM, nullptr);
}

// One more input of a phi: the lattice only falls, so the meet of the
// phi's value with the input's is the meet over all its inputs.
void Propagator::meet_input(TempId phi, Value input)
{
	if (lattice_[phi] == L_BOTTOM)
		return;
	Constant in;
	const Lattice l = state(input, in);
	if (l != L_TOP)
		lower(phi, l, l == L_CONSTANT ? &in : nullptr);
}

void Propagator::evaluate_phi(InstId id)
{
	const Instruction& phi = body_.instructions[id];
	if (lattice_[phi.result] == L_BOTTOM)
		return;
	Lattice meet = L_TOP;
	Constant value;
	for (uint32_t k = 0; k < phi.extra_count && meet != L_BOTTOM; k += 2)
	{
		const uint32_t slot = label_slot_[phi.extra_first + k];
		if (slot == kNoId || !edge_[slot])
			continue;
		Constant in;
		const Lattice l = state(body_.operands[phi.extra_first + k + 1], in);
		if (l == L_TOP)
			continue;
		if (l == L_BOTTOM || (meet == L_CONSTANT && !same_constant(phi.type, value, in)))
			meet = L_BOTTOM;
		else if (meet == L_TOP)
		{
			meet = L_CONSTANT;
			value = in;
		}
	}
	if (meet != L_TOP)
		lower(phi.result, meet, &value);
}

void Propagator::evaluate_terminator(BlockId block, const Instruction& term)
{
	Constant c;
	switch (term.op)
	{
	case Opcode::Jump:
		mark_edge(block, term.ops[0].index);
		break;
	case Opcode::Branch:
	{
		const Lattice l = state(term.ops[0], c);
		if (l == L_TOP)
			return;
		Literal bits;
		if (l == L_CONSTANT && c.kind == ValueKind::Integer)
		{
			bits = c.bits;
			mark_edge(block, (bits.low | bits.high) != 0 ? term.ops[1].index : term.ops[2].index);
			return;
		}
		mark_edge(block, term.ops[1].index);
		mark_edge(block, term.ops[2].index);
		break;
	}
	case Opcode::Switch:
	{
		const Lattice l = state(term.ops[0], c);
		if (l == L_TOP)
			return;
		const Type type = !term.type.is_void() ? term.type
			: term.ops[0].is(ValueKind::Temp)  ? body_.temps[term.ops[0].index].type
											   : Type::scalar(TypeKind::I64);
		if (l == L_CONSTANT && c.kind == ValueKind::Integer)
		{
			BlockId target = term.ops[1].index;
			for (uint32_t k = 0; k < term.extra_count; k += 2)
			{
				Constant label;
				if (literal_constant(body_, body_.operands[term.extra_first + k], label) &&
					same_constant(type, label, c))
				{
					target = body_.operands[term.extra_first + k + 1].index;
					break;
				}
			}
			mark_edge(block, target);
			return;
		}
		// Every case target and the default: the block's successors.
		for (uint32_t k = cfg_.succ_first[block]; k < cfg_.succ_first[block + 1]; ++k)
			mark_successor(block, k);
		break;
	}
	default:
		break;
	}
}

void Propagator::mark_edge(BlockId from, BlockId to)
{
	for (uint32_t k = cfg_.succ_first[from]; k < cfg_.succ_first[from + 1]; ++k)
		if (cfg_.succs[k] == to)
		{
			mark_successor(from, k);
			return;
		}
}

// The edge to `from`'s k-th successor is executable: the target is visited
// once, and a later edge into it meets its value into each of its phis.
void Propagator::mark_successor(BlockId from, uint32_t k)
{
	(void)from;
	const BlockId to = cfg_.succs[k];
	const uint32_t slot = succ_slot_[k];
	if (edge_[slot])
		return;
	edge_[slot] = 1;
	if (!executable_[to])
	{
		executable_[to] = 1;
		block_work_.push_back(to);
		return;
	}
	if (!phis_present_)
		return;
	const uint32_t phis = phi_first_[to + 1] - phi_first_[to];
	const uint32_t row = slot_input_first_[to] + (slot - cfg_.pred_first[to]) * phis;
	for (uint32_t j = 0; j < phis; ++j)
		if (slot_inputs_[row + j] != kNoId)
			meet_input(body_.instructions[phis_[phi_first_[to] + j]].result, body_.operands[slot_inputs_[row + j]]);
}

void Propagator::visit_block(BlockId block)
{
	for (InstId i = body_.blocks[block].first; i != kNoId; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		evaluate(i);
		if ((inst.op == Opcode::EhTry || inst.op == Opcode::EhCleanup) && inst.ops[0].is(ValueKind::Block) &&
			!executable_[inst.ops[0].index])
		{
			executable_[inst.ops[0].index] = 1;
			block_work_.push_back(inst.ops[0].index);
		}
		if (inst.op == Opcode::Call && c_.program.may_end_block(inst))
			return; // a noreturn call: nothing after it runs
	}
}

// The edge indexes: each successor's predecessor slot (build_cfg fills
// predecessors by source block, then successor order), each phi label's
// slot, and per block a (slot, phi) table of input operands. Linear in
// edges plus phi operands.
void Propagator::index_phis()
{
	const uint32_t n = cfg_.block_count;
	succ_slot_.assign(cfg_.succs.size(), 0);
	std::vector<uint32_t> fill(cfg_.pred_first.begin(), cfg_.pred_first.end() - 1);
	for (BlockId b = 0; b < n; ++b)
		for (uint32_t k = cfg_.succ_first[b]; k < cfg_.succ_first[b + 1]; ++k)
			succ_slot_[k] = fill[cfg_.succs[k]]++;
	// A body without phis (most, before promotion) needs no phi tables.
	phis_present_ = false;
	for (BlockId b : body_.layout)
		phis_present_ = phis_present_ ||
			(body_.blocks[b].first != kNoId && body_.instructions[body_.blocks[b].first].op == Opcode::Phi);
	if (!phis_present_)
		return;
	label_slot_.assign(body_.operands.size(), kNoId);
	phi_first_.assign(n + 1, 0);
	phis_.clear();
	slot_input_first_.assign(n + 1, 0);
	slot_inputs_.clear();
	std::vector<uint32_t> slot_of(n, kNoId);
	for (BlockId b = 0; b < n; ++b)
	{
		phi_first_[b] = static_cast<uint32_t>(phis_.size());
		slot_input_first_[b] = static_cast<uint32_t>(slot_inputs_.size());
		if ((cfg_.flags[b] & BF_PLACED) == 0)
			continue;
		for (InstId i = body_.blocks[b].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
			i = body_.instructions[i].next)
			phis_.push_back(i);
		const uint32_t phis = static_cast<uint32_t>(phis_.size()) - phi_first_[b];
		if (phis == 0)
			continue;
		for (uint32_t p = cfg_.pred_first[b]; p < cfg_.pred_first[b + 1]; ++p)
			slot_of[cfg_.preds[p]] = p;
		slot_inputs_.resize(slot_inputs_.size() + static_cast<size_t>(phis) * cfg_.pred_count(b), kNoId);
		for (uint32_t j = 0; j < phis; ++j)
		{
			const Instruction& phi = body_.instructions[phis_[phi_first_[b] + j]];
			for (uint32_t k = 0; k < phi.extra_count; k += 2)
			{
				const BlockId pred = body_.operands[phi.extra_first + k].index;
				const uint32_t slot = pred < n ? slot_of[pred] : kNoId;
				if (slot == kNoId || slot < cfg_.pred_first[b] || slot >= cfg_.pred_first[b + 1] ||
					cfg_.preds[slot] != pred)
					continue;
				label_slot_[phi.extra_first + k] = slot;
				uint32_t& input = slot_inputs_[slot_input_first_[b] + (slot - cfg_.pred_first[b]) * phis + j];
				if (input == kNoId)
					input = phi.extra_first + k + 1;
			}
		}
	}
	phi_first_[n] = static_cast<uint32_t>(phis_.size());
	slot_input_first_[n] = static_cast<uint32_t>(slot_inputs_.size());
}

bool Propagator::rewrite()
{
	bool changed = false;
	TempId first = body_.parameter_count;
	while (first < body_.temps.size() && lattice_[first] != L_CONSTANT)
		++first;
	if (first == body_.temps.size())
		return false; // no constant to substitute
	std::vector<Value> literal(body_.temps.size(), Value::none());
	for (TempId t = first; t < body_.temps.size(); ++t)
		if (lattice_[t] == L_CONSTANT)
		{
			const Constant& v = value_[t];
			literal[t] = literal_value(body_, v.kind, v.float_kind, v.bits);
		}
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			Instruction& inst = body_.instructions[i];
			uint32_t position = 0;
			for_each_operand(body_, inst, [&](Value& v) {
				const bool callee = inst.op == Opcode::Call && position++ == 0;
				if (v.is(ValueKind::Temp) && !literal[v.index].is(ValueKind::None) && !callee)
				{
					v = literal[v.index];
					changed = true;
					++c_.stats.propagated;
				}
			});
		}
	return changed;
}

bool Propagator::run()
{
	c_.ensure_cfg();
	if (cfg_.rpo.empty())
		return false;
	const size_t temps = body_.temps.size();
	lattice_.assign(temps, L_TOP);
	value_.resize(temps);
	for (TempId t = 0; t < body_.parameter_count; ++t)
		lattice_[t] = L_BOTTOM;
	executable_.assign(cfg_.block_count, 0);
	edge_.assign(cfg_.preds.size(), 0);
	block_of_ = &c_.entry_block_of(propagate_constants);
	index_phis();
	// Users of each temporary (CSR).
	user_first_.assign(temps + 1, 0);
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			for_each_operand(body_, body_.instructions[i], [&](const Value& v) {
				if (v.is(ValueKind::Temp))
					++user_first_[v.index + 1];
			});
	for (size_t t = 0; t < temps; ++t)
		user_first_[t + 1] += user_first_[t];
	users_.assign(user_first_[temps], 0);
	if (phis_present_)
		user_operand_.assign(user_first_[temps], kNoId);
	std::vector<uint32_t> fill(user_first_.begin(), user_first_.end() - 1);
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			for (const Value& v : inst.ops)
				if (v.is(ValueKind::Temp))
					users_[fill[v.index]++] = i;
			for (uint32_t k = 0; k < inst.extra_count; ++k)
			{
				const Value& v = body_.operands[inst.extra_first + k];
				if (!v.is(ValueKind::Temp))
					continue;
				if (phis_present_)
					user_operand_[fill[v.index]] = inst.op == Opcode::Phi ? inst.extra_first + k : kNoId;
				users_[fill[v.index]++] = i;
			}
		}
	executable_[cfg_.rpo[0]] = 1;
	block_work_.push_back(cfg_.rpo[0]);
	while (!block_work_.empty() || !temp_work_.empty())
	{
		while (!block_work_.empty())
		{
			const BlockId b = block_work_.back();
			block_work_.pop_back();
			visit_block(b);
		}
		while (!temp_work_.empty())
		{
			const TempId t = temp_work_.back();
			temp_work_.pop_back();
			for (uint32_t k = user_first_[t]; k < user_first_[t + 1]; ++k)
			{
				const InstId user = users_[k];
				if ((*block_of_)[user] == kNoId || !executable_[(*block_of_)[user]])
					continue;
				// A phi meets the input on its executable edge alone. (A user
				// already overdefined, or with no result and no terminator,
				// evaluates to no change.)
				if (!phis_present_ || user_operand_[k] == kNoId)
				{
					const Instruction& inst = body_.instructions[user];
					if (inst.result != kNoId ? lattice_[inst.result] != L_BOTTOM : is_terminator(inst.op))
						evaluate(user);
				}
				else if (label_slot_[user_operand_[k] - 1] != kNoId && edge_[label_slot_[user_operand_[k] - 1]])
					meet_input(body_.instructions[user].result, Value::temp(t));
			}
		}
	}
	return rewrite();
}

} // namespace

bool propagate_constants(FunctionContext& context)
{
	static Propagator::Pool pool;
	Propagator propagator(context);
	propagator.exchange(pool);
	const bool changed = propagator.run();
	propagator.exchange(pool);
	return changed;
}

} // namespace opt
