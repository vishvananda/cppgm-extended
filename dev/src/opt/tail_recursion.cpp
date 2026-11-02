#include "lowir/builder.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

// Tail recursion: a call of the function itself whose result is all it then
// returns becomes a jump back to the entry, its arguments the parameters' new
// values. The caller's frame is reused, so no address of it may reach anything
// but this frame's own loads and stores.
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
	if (site.call == kNoId)
		return false;
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

	for (const Site& site : sites_)
	{
		if (site.merge != kNoId)
			remove_phi_incoming(body_, site.merge, site.block);
		unlink_after(body_, site.block, site.call, body_.blocks[site.block].last);
		unlink_after(body_, site.block, site.before_call, site.call);
		insert_after(body_, site.block, site.before_call, append_instruction(body_, enter));
		++c_.stats.tail_calls_removed;
	}
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
			if (body_.instructions[i].op == Opcode::StackAlloc)
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
