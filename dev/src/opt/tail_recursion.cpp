#include "lowir/builder.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

// Tail recursion: a call of the function itself whose result is all it then
// returns becomes a jump back to the entry, its arguments the parameters' new
// values. The caller's frame is reused, so no address of it may reach anything
// but this frame's own loads and stores. A call whose result an integer add
// then returns (`return a + f(x)`) does the same with an accumulator: a phi
// of the entry, zero on entering, that the call's site adds `a` to and every
// other return adds to what it returns (integer addition reassociates, so no
// add keeps `nowrap`).
class TailRecursion
{
public:
	explicit TailRecursion(FunctionContext& context) : c_(context), body_(context.body) {}

	bool run();

private:
	struct Site
	{
		BlockId block;
		InstId call;
		InstId before_call;
		BlockId merge; // the block of the phi that returns the result, if any
		InstId add;    // the add of an accumulating site, else kNoId
		Value addend;  // and what it adds to the call's result
	};

	bool frame_private();
	bool self_call(BlockId block, Site& site, Value& returned) const;
	void find_sites();
	void rewrite(BlockId header);

	FunctionContext& c_;
	FunctionBody& body_;
	std::vector<Site> sites_;
};

// No address of a slot leaves this frame's own loads and stores.
bool TailRecursion::frame_private()
{
	if (body_.slots.empty())
		return true;
	std::vector<uint8_t> derived(body_.temps.size(), 0);
	for (unsigned walk = 0; walk < 2; ++walk)
		for (BlockId b : body_.layout)
			for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			{
				const Instruction& inst = body_.instructions[i];
				if (inst.op == Opcode::Addr)
				{
					derived[inst.result] = 1;
					continue;
				}
				if (inst.op == Opcode::Index && inst.ops[0].is(ValueKind::Temp) && derived[inst.ops[0].index])
				{
					derived[inst.result] = 1;
					continue;
				}
				uint32_t position = 0;
				bool leaves = false;
				for_each_operand(body_, inst, [&](const Value& v) {
					const uint32_t at = position++;
					const bool address = (inst.op == Opcode::Load && at == 0) || (inst.op == Opcode::Store && at == 1);
					leaves = leaves || (v.is(ValueKind::Temp) && derived[v.index] && !address);
				});
				if (leaves)
					return false;
			}
	return true;
}

// The call of this function that ends `block`, when all that follows is the
// return of its result. `returned` is what the return gives.
bool TailRecursion::self_call(BlockId block, Site& site, Value& returned) const
{
	const InstId last = body_.blocks[block].last;
	if (last == kNoId)
		return false;
	const Instruction& term = body_.instructions[last];
	site.merge = kNoId;
	if (term.op == Opcode::Return)
		returned = term.ops[0];
	else if (term.op == Opcode::Jump)
	{
		// To a block of nothing but the return of a phi.
		const BlockId merge = term.ops[0].index;
		const InstId first = body_.blocks[merge].first;
		const InstId end = body_.blocks[merge].last;
		if (first == kNoId || body_.instructions[end].op != Opcode::Return)
			return false;
		const Instruction& head = body_.instructions[first];
		if (first == end)
			returned = head.ops[0];
		else if (head.op == Opcode::Phi && head.next == end &&
			body_.instructions[end].ops[0] == Value::temp(head.result))
			returned = phi_incoming(body_, head, block);
		else
			return false;
		site.merge = merge;
	}
	else
		return false;
	site.block = block;
	site.call = previous_in_block(body_, block, last);
	site.add = kNoId;
	site.addend = Value::none();
	if (site.call == kNoId)
		return false;
	const Instruction& add = body_.instructions[site.call];
	if (add.op == Opcode::Binary && add.sub == static_cast<uint8_t>(BinaryOp::Add) && add.type.is_integer() &&
		add.type.kind() != TypeKind::I1 && returned == Value::temp(add.result))
	{
		const InstId at = previous_in_block(body_, block, site.call);
		if (at == kNoId || body_.instructions[at].op != Opcode::Call || body_.instructions[at].type.is_void())
			return false;
		const Value result = Value::temp(body_.instructions[at].result);
		if (add.ops[0] == result && !(add.ops[1] == result))
			site.addend = add.ops[1];
		else if (add.ops[1] == result && !(add.ops[0] == result))
			site.addend = add.ops[0];
		else
			return false;
		site.add = site.call;
		site.call = at;
		returned = result;
	}
	const Instruction& call = body_.instructions[site.call];
	if (call.op != Opcode::Call || !call.ops[0].is(ValueKind::Symbol) || call.ops[0].index != c_.symbol ||
		call.extra_count != body_.parameter_count)
		return false;
	if (call.type.is_void())
		return true;
	return returned == Value::temp(call.result);
}

void TailRecursion::find_sites()
{
	sites_.clear();
	for (BlockId b : body_.layout)
	{
		Site site;
		Value returned = Value::none();
		if (self_call(b, site, returned))
		{
			site.before_call = previous_in_block(body_, b, site.call);
			sites_.push_back(site);
		}
	}
}

void TailRecursion::rewrite(BlockId header)
{
	// The parameters that change, each a phi of the header.
	const uint32_t count = body_.parameter_count;
	std::vector<uint8_t> changes(count, 0);
	for (const Site& site : sites_)
		for (uint32_t k = 0; k < count; ++k)
			changes[k] = changes[k] ||
				!(body_.operands[body_.instructions[site.call].extra_first + k] == Value::temp(k));
	std::vector<TempId> phis(count, 0);
	for (uint32_t k = 0; k < count; ++k)
		if (changes[k])
		{
			phis[k] = append_temp(body_, body_.temps[k].type, kNoId);
			replace_all_uses(body_, k, Value::temp(phis[k]));
		}
	// (An addend read from the instruction holds what it was.)
	for (Site& site : sites_)
		if (site.addend.is(ValueKind::Temp) && site.addend.index < count && changes[site.addend.index])
			site.addend = Value::temp(phis[site.addend.index]);

	const BlockId entry = append_block(body_, BlockKind::Plain);
	Instruction enter = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
	enter.ops[0] = Value::block(header);
	insert_after(body_, entry, kNoId, append_instruction(body_, enter));
	body_.layout.insert(body_.layout.begin(), entry);

	std::vector<Value> pairs;
	for (uint32_t k = 0; k < count; ++k)
	{
		if (!changes[k])
			continue;
		pairs.clear();
		pairs.push_back(Value::block(entry));
		pairs.push_back(Value::temp(k));
		for (const Site& site : sites_)
		{
			pairs.push_back(Value::block(site.block));
			pairs.push_back(body_.operands[body_.instructions[site.call].extra_first + k]);
		}
		Instruction phi = make_instruction(Opcode::Phi, body_.temps[k].type);
		phi.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
		phi.extra_count = static_cast<uint32_t>(pairs.size());
		phi.result = phis[k];
		const InstId id = append_instruction(body_, phi);
		body_.temps[phis[k]].def = id;
		insert_after(body_, header, kNoId, id);
	}

	// The accumulator, when a site adds to the call's result.
	bool accumulates = false;
	for (const Site& site : sites_)
		accumulates = accumulates || site.add != kNoId;
	TempId sum = kNoId;
	std::vector<Value> sums(sites_.size(), Value::none());
	const auto add = [&](Value left, Value right, Type type) {
		Instruction inst = make_instruction(Opcode::Binary, type);
		inst.sub = static_cast<uint8_t>(BinaryOp::Add);
		inst.ops[0] = left;
		inst.ops[1] = right;
		inst.result = append_temp(body_, type, kNoId);
		const InstId id = append_instruction(body_, inst);
		body_.temps[body_.instructions[id].result].def = id;
		return id;
	};
	Type type = Type::scalar(TypeKind::Void);
	if (accumulates)
	{
		for (const Site& site : sites_)
			if (site.add != kNoId)
				type = body_.instructions[site.add].type;
		sum = append_temp(body_, type, kNoId);
		// Every return adds the accumulator (the sites' returns go below).
		for (BlockId b : body_.layout)
		{
			const InstId last = body_.blocks[b].last;
			if (last == kNoId || body_.instructions[last].op != Opcode::Return)
				continue;
			bool site_block = false;
			for (const Site& site : sites_)
				site_block = site_block || (site.block == b && site.merge == kNoId);
			if (site_block)
				continue;
			const InstId id = add(Value::temp(sum), body_.instructions[last].ops[0], type);
			insert_after(body_, b, previous_in_block(body_, b, last), id);
			body_.instructions[last].ops[0] = Value::temp(body_.instructions[id].result);
		}
	}

	for (size_t k = 0; k < sites_.size(); ++k)
	{
		const Site& site = sites_[k];
		if (site.merge != kNoId)
			remove_phi_incoming(body_, site.merge, site.block);
		if (site.add != kNoId)
		{
			unlink_after(body_, site.block, site.add, body_.blocks[site.block].last);
			unlink_after(body_, site.block, site.call, site.add);
		}
		else
			unlink_after(body_, site.block, site.call, body_.blocks[site.block].last);
		unlink_after(body_, site.block, site.before_call, site.call);
		InstId after = site.before_call;
		sums[k] = Value::temp(sum);
		if (site.add != kNoId)
		{
			const InstId id = add(Value::temp(sum), site.addend, type);
			insert_after(body_, site.block, after, id);
			after = id;
			sums[k] = Value::temp(body_.instructions[id].result);
		}
		insert_after(body_, site.block, after, append_instruction(body_, enter));
		++c_.stats.tail_calls_removed;
	}
	if (!accumulates)
		return;
	pairs.clear();
	pairs.push_back(Value::block(entry));
	pairs.push_back(integer_value(body_, Literal{ 0, 0 }));
	for (size_t k = 0; k < sites_.size(); ++k)
	{
		pairs.push_back(Value::block(sites_[k].block));
		pairs.push_back(sums[k]);
	}
	Instruction phi = make_instruction(Opcode::Phi, type);
	phi.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
	phi.extra_count = static_cast<uint32_t>(pairs.size());
	phi.result = sum;
	const InstId id = append_instruction(body_, phi);
	body_.temps[sum].def = id;
	insert_after(body_, header, kNoId, id);
}

bool TailRecursion::run()
{
	const Symbol& symbol = c_.program.symbols[c_.symbol];
	const Signature& signature = c_.program.signatures[c_.program.functions[symbol.detail].signature];
	if (signature.boundary.arity == ir_model::CAM_VARIADIC || (symbol.metadata.flags & SF_REPLACEABLE) != 0 ||
		body_.layout.empty())
		return false;
	find_sites();
	if (sites_.empty())
		return false;
	for (uint32_t k = 0; k < body_.parameter_count; ++k)
	{
		const TypeKind kind = body_.temps[k].type.kind();
		if (kind == TypeKind::Obj || kind == TypeKind::F80 || kind == TypeKind::V128)
			return false;
	}
	const Cfg& cfg = c_.ensure_cfg();
	const BlockId header = body_.layout[0];
	if (cfg.pred_first[header] != cfg.pred_first[header + 1])
		return false;
	for (BlockId b : body_.layout)
		if ((cfg.flags[b] & (BF_EH | BF_HANDLER)) != 0)
			return false;
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			if (is_stack_instruction(body_.instructions[i].op))
				return false;
	if (!frame_private())
		return false;
	rewrite(header);
	c_.invalidate_cfg();
	c_.invalidate_block_of();
	return true;
}

} // namespace

bool eliminate_tail_recursion(FunctionContext& context)
{
	TailRecursion pass(context);
	return pass.run();
}

} // namespace opt
