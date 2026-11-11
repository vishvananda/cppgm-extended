#include "opt/phi_equivalence.h"
#include "lowir/builder.h"
#include "opt/scalar_fold.h"
#include <cstring>
namespace opt {
using namespace lowir;
// Match each predecessor, including mutually dependent phi pairs. A pair
// already visited is an induction hypothesis; every distinct external input
// must still match. Bound the proof work and retain the ordinary exact path.
bool PhiComparison::equal(
	FunctionContext& c, const Instruction& a, const Instruction& b, const std::vector<Value>& replacements)
{
	const FunctionBody& body_ = c.body;
	const auto resolve = [&](Value v) {
		while (v.is(ValueKind::Temp) && !replacements[v.index].is(ValueKind::None))
			v = replacements[v.index];
		return v;
	};
	const auto producer = [&](Value v) -> const Instruction* {
		return v.is(ValueKind::Temp) && v.index >= body_.parameter_count && body_.temps[v.index].def != kNoId
			? &body_.instructions[body_.temps[v.index].def]
			: nullptr;
	};
	if (pred_input_.size() < body_.blocks.size())
	{
		pred_input_.resize(body_.blocks.size(), Value::none());
		pred_stamp_.resize(body_.blocks.size(), 0);
	}
	std::pair<InstId, InstId> pairs[16];
	pairs[0] = std::make_pair(body_.temps[a.result].def, body_.temps[b.result].def);
	unsigned count = 1, work = 0;
	for (unsigned at = 0; at < count; ++at)
	{
		const Instruction& left = body_.instructions[pairs[at].first];
		const Instruction& right = body_.instructions[pairs[at].second];
		if (left.type != right.type || left.extra_count != right.extra_count ||
			c.block_of[pairs[at].first] != c.block_of[pairs[at].second])
			return false;
		work += left.extra_count;
		if (at != 0 && work > 256)
			return false;
		++stamp_;
		for (uint32_t k = 0; k < right.extra_count; k += 2)
		{
			const BlockId pred = body_.operands[right.extra_first + k].index;
			if (pred_stamp_[pred] != stamp_)
			{
				pred_stamp_[pred] = stamp_;
				pred_input_[pred] = resolve(body_.operands[right.extra_first + k + 1]);
			}
		}
		for (uint32_t k = 0; k < left.extra_count; k += 2)
		{
			const BlockId pred = body_.operands[left.extra_first + k].index;
			const Value in = pred_stamp_[pred] == stamp_ ? pred_input_[pred] : Value::none();
			const Value mine = resolve(body_.operands[left.extra_first + k + 1]);
			if (in == mine)
				continue;
			Constant lhs, rhs;
			if (literal_constant(body_, in, lhs) && literal_constant(body_, mine, rhs) &&
				same_constant(left.type, lhs, rhs))
				continue;
			const Instruction* p = producer(in);
			const Instruction* q = producer(mine);
			if (p == nullptr || q == nullptr || p->op != Opcode::Phi || q->op != Opcode::Phi)
				return false;
			const auto pair = std::make_pair(body_.temps[p->result].def, body_.temps[q->result].def);
			bool seen = false;
			for (unsigned j = 0; j < count; ++j)
				seen = seen || pairs[j] == pair || (pairs[j].first == pair.second && pairs[j].second == pair.first);
			if (seen)
				continue;
			if (count == 16)
				return false;
			pairs[count++] = pair;
		}
	}
	return true;
}

// Equal for phis same_phi may match: the type, the input count and the
// sum of each (block, input) pair's hash. Phi inputs use their block identity
// so equal cyclic phi graphs reach the bounded structural proof above.
uint64_t phi_signature(const FunctionBody& body_, const std::vector<BlockId>& block_of, const Instruction& phi)
{
	const auto producer = [&](Value v) -> const Instruction* {
		return v.is(ValueKind::Temp) && v.index >= body_.parameter_count && body_.temps[v.index].def != kNoId
			? &body_.instructions[body_.temps[v.index].def]
			: nullptr;
	};
	uint64_t bits;
	std::memcpy(&bits, &phi.type, sizeof bits);
	uint64_t sum = 0;
	for (uint32_t k = 0; k < phi.extra_count; k += 2)
	{
		const Value in = body_.operands[phi.extra_first + k + 1];
		const Instruction* incoming = producer(in);
		const bool from_phi = incoming != nullptr && incoming->op == Opcode::Phi;
		uint64_t h = (static_cast<uint64_t>(body_.operands[phi.extra_first + k].index) + 1) * 0x9e3779b97f4a7c15ull;
		Constant constant;
		if (literal_constant(body_, in, constant))
		{
			const Literal bits = constant.kind == ValueKind::Integer ? normalize_integer(phi.type, constant.bits)
																	 : constant.bits;
			h ^= bits.low * 0xc2b2ae3d27d4eb4full ^ bits.high * 0x165667b19e3779f9ull;
		}
		else
			h ^= from_phi ? (static_cast<uint64_t>(block_of[body_.temps[incoming->result].def]) + 1) * 0x5bd1e995ull
						  : (static_cast<uint64_t>(in.kind) << 32 | in.index) * 0xc2b2ae3d27d4eb4full;
		sum += h ^ (h >> 31);
	}
	return sum ^ (bits * 0x165667b19e3779f9ull) ^ phi.extra_count;
}

// Aggregate promotion leaves a phi of copies beside the phi of the sources
// themselves (a pointer member read whole as i64): the two then merge, the
// copy folds with a copy back, and a test of one decides a test of the other.
// A block copied for a predecessor leaves each copy's field address to a phi
// where one address of the phi of the objects serves. The copies and the phi
// name one register, and a constant step joins its uses' addresses.
bool retype_copy_phis(FunctionContext& c, BlockId block, std::vector<Value>& replacements, std::vector<uint32_t>& uses)
{
	FunctionBody& body = c.body;
	const auto resolve = [&](Value v) {
		while (v.is(ValueKind::Temp) && !replacements[v.index].is(ValueKind::None))
			v = replacements[v.index];
		return v;
	};
	const auto retyped = [](Type t) {
		return t.is_pointer() || (t.is_integer() && t.kind() != TypeKind::I1 && t.kind() != TypeKind::I128);
	};
	const auto zero = [&](Value v) {
		Constant k;
		return literal_constant(body, v, k) && k.kind == ValueKind::Integer && (k.bits.low | k.bits.high) == 0;
	};
	// Two steps by one constant, stated alike.
	const auto same_step = [&](const Instruction& a, const Instruction& b) {
		Constant x, y;
		return a.type == b.type && a.sub == b.sub && a.flags == b.flags && a.order == b.order &&
			literal_constant(body, a.ops[1], x) && literal_constant(body, b.ops[1], y) &&
			same_constant(Type::scalar(TypeKind::I64), x, y);
	};
	// (A loop's pointer keeps its own phi: the loop passes read its step.)
	bool header = false;
	for (uint32_t k = c.cfg.pred_first[block]; k < c.cfg.pred_first[block + 1]; ++k)
		header = header || c.cfg.dominates(block, c.cfg.preds[k]);
	std::vector<Value> pairs;
	bool changed = false;
	InstId next = kNoId;
	for (InstId id = body.blocks[block].first; id != kNoId && body.instructions[id].op == Opcode::Phi; id = next)
	{
		const Instruction phi = body.instructions[id];
		next = phi.next;
		if (!retyped(phi.type) || phi.extra_count == 0)
			continue;
		// The sources' one type and each edge's source; or the one step.
		Type from = phi.type;
		InstId step = kNoId;
		bool all = true;
		pairs.clear();
		for (uint32_t k = 0; k < phi.extra_count && all; k += 2)
		{
			Value in = resolve(body.operands[phi.extra_first + k + 1]);
			const Instruction* def = in.is(ValueKind::Temp) && in.index >= body.parameter_count &&
					body.temps[in.index].def != kNoId
				? &body.instructions[body.temps[in.index].def]
				: nullptr;
			if (in.is(ValueKind::Temp) && in.index == phi.result)
				in = Value::none(); // the new phi itself, named below
			else if (def != nullptr && def->op == Opcode::Copy && (def->flags & IF_NONNULL) == 0 && step == kNoId)
			{
				in = resolve(def->ops[0]);
				const Type type = in.is(ValueKind::Temp) ? body.temps[in.index].type : phi.type;
				all = in.is(ValueKind::Temp) ? type != phi.type && retyped(type) &&
						storage_bytes(type) == storage_bytes(phi.type) && (from == phi.type || from == type)
											 : zero(in);
				if (all && in.is(ValueKind::Temp))
					from = type;
			}
			else if (def != nullptr && def->op == Opcode::Index && def->ops[1].is_literal() && !header &&
				from == phi.type && k == (step == kNoId ? 0 : k) &&
				(step == kNoId || same_step(body.instructions[step], *def)))
			{
				step = body.temps[in.index].def;
				in = resolve(def->ops[0]);
				all = in.is(ValueKind::Temp);
			}
			else
				all = step == kNoId && zero(in);
			pairs.push_back(body.operands[phi.extra_first + k]);
			pairs.push_back(in);
		}
		if (!all || (from == phi.type && step == kNoId))
			continue;
		Instruction merged = make_instruction(Opcode::Phi, from);
		merged.location = phi.location;
		const InstId at = append_instruction(body, merged);
		const TempId result = append_temp(body, from, at);
		replacements.resize(body.temps.size(), Value::none());
		uses.resize(body.temps.size(), 0);
		uses[result] = 1;
		for (uint32_t k = 1; k < pairs.size(); k += 2)
		{
			const Value old = resolve(body.operands[phi.extra_first + k]);
			if (old.is(ValueKind::Temp) && old.index != phi.result && uses[old.index] != 0xffffffffu &&
				uses[old.index] != 0)
				--uses[old.index];
			if (pairs[k].is(ValueKind::None))
				pairs[k] = Value::temp(result);
			if (pairs[k].is(ValueKind::Temp) && uses[pairs[k].index] != 0xffffffffu)
				++uses[pairs[k].index];
		}
		const uint32_t first = append_operands(body, pairs.data(), static_cast<uint32_t>(pairs.size()));
		body.instructions[at].result = result;
		body.instructions[at].extra_first = first;
		body.instructions[at].extra_count = static_cast<uint32_t>(pairs.size());
		// The phi's instruction becomes the copy or the step, after the
		// block's phis.
		InstId prev = kNoId, last = kNoId;
		for (InstId i = body.blocks[block].first; i != kNoId && body.instructions[i].op == Opcode::Phi;
			i = body.instructions[i].next)
		{
			prev = i == id ? last : prev;
			last = i;
		}
		unlink_after(body, block, prev, id);
		Instruction after = step == kNoId ? make_instruction(Opcode::Copy, phi.type) : body.instructions[step];
		after.result = phi.result;
		after.location = phi.location;
		after.ops[0] = Value::temp(result);
		body.instructions[id] = after;
		insert_after(body, block, last == id ? prev : last, id);
		insert_after(body, block, kNoId, at);
		c.block_of.resize(body.instructions.size(), kNoId);
		c.block_of[at] = block;
		changed = true;
	}
	return changed;
}

} // namespace opt
