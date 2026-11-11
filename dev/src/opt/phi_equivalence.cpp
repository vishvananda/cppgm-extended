#include "opt/phi_equivalence.h"
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

} // namespace opt
