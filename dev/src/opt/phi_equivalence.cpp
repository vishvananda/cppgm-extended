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

// A loop rotated around a narrow counter that may wrap leaves the extended
// counter a phi of its own: the two hold one count in two registers, and the
// back edge copies one to the other.
bool convert_matching_phis(
	FunctionContext& c, BlockId block, std::vector<Value>& replacements, std::vector<uint32_t>& uses)
{
	FunctionBody& body = c.body;
	const auto resolve = [&](Value v) {
		while (v.is(ValueKind::Temp) && !replacements[v.index].is(ValueKind::None))
			v = replacements[v.index];
		return v;
	};
	const auto producer = [&](Value v) -> const Instruction* {
		return v.is(ValueKind::Temp) && v.index >= body.parameter_count && body.temps[v.index].def != kNoId
			? &body.instructions[body.temps[v.index].def]
			: nullptr;
	};
	const auto whole = [](Type t) { return t.is_integer() && t.kind() != TypeKind::I1 && t.kind() != TypeKind::I128; };
	bool changed = false;
	InstId next = kNoId;
	for (InstId id = body.blocks[block].first; id != kNoId && body.instructions[id].op == Opcode::Phi; id = next)
	{
		const Instruction phi = body.instructions[id];
		next = phi.next;
		if (!whole(phi.type) || phi.extra_count == 0 || phi.extra_count > 16)
			continue;
		// The one conversion of every input that is not a constant.
		const Instruction* conversion = nullptr;
		bool all = true;
		for (uint32_t k = 1; k < phi.extra_count && all; k += 2)
		{
			const Value in = resolve(body.operands[phi.extra_first + k]);
			const Instruction* def = producer(in);
			Constant value;
			if (def != nullptr && def->op == Opcode::Convert && whole(convert_source_type(*def)))
			{
				// (A counter that never wraps keeps its extended copy a phi:
				// the copy steps with it, and counter widening and the
				// vectorizer read it so.)
				const Instruction* step = producer(resolve(def->ops[0]));
				all = static_cast<ConvertOp>(def->sub) <= ConvertOp::Trunc &&
					(step == nullptr || step->op != Opcode::Binary || (step->flags & IF_NO_WRAP) == 0) &&
					(conversion == nullptr ||
						(conversion->sub == def->sub && convert_source_type(*conversion) == convert_source_type(*def)));
				conversion = def;
			}
			else
				all = literal_constant(body, in, value) && value.kind == ValueKind::Integer;
		}
		if (!all || conversion == nullptr)
			continue;
		const ConvertOp op = static_cast<ConvertOp>(conversion->sub);
		const Type from = convert_source_type(*conversion);
		// A phi of the sources: each predecessor's input is the conversion's
		// operand, or a constant the conversion takes to this phi's.
		const auto converts = [&](const Instruction& source) {
			for (uint32_t k = 0; k < phi.extra_count; k += 2)
			{
				Value narrow = Value::none();
				for (uint32_t j = 0; j < source.extra_count; j += 2)
					if (body.operands[source.extra_first + j].index == body.operands[phi.extra_first + k].index)
						narrow = resolve(body.operands[source.extra_first + j + 1]);
				const Value in = resolve(body.operands[phi.extra_first + k + 1]);
				const Instruction* def = producer(in);
				Constant x, y, r;
				if (def != nullptr && def->op == Opcode::Convert
						? !(resolve(def->ops[0]) == narrow)
						: !literal_constant(body, narrow, x) || !literal_constant(body, in, y) ||
							!fold_convert(op, phi.type, from, x, r) || !same_constant(phi.type, r, y))
					return false;
			}
			return true;
		};
		InstId source = kNoId, last = kNoId, prev = kNoId;
		for (InstId i = body.blocks[block].first; i != kNoId && body.instructions[i].op == Opcode::Phi;
			i = body.instructions[i].next)
		{
			const Instruction& other = body.instructions[i];
			if (source == kNoId && i != id && other.type == from && other.extra_count == phi.extra_count &&
				replacements[other.result].is(ValueKind::None) && converts(other))
				source = i;
			prev = i == id ? last : prev;
			last = i;
		}
		if (source == kNoId)
			continue;
		for (uint32_t k = 1; k < phi.extra_count; k += 2)
		{
			const Value old = resolve(body.operands[phi.extra_first + k]);
			if (old.is(ValueKind::Temp) && uses[old.index] != 0xffffffffu && uses[old.index] != 0)
				--uses[old.index];
		}
		const TempId narrow = body.instructions[source].result;
		if (uses[narrow] != 0xffffffffu)
			++uses[narrow];
		// The phi's instruction becomes the conversion, after the block's
		// phis.
		unlink_after(body, block, prev, id);
		Instruction after = make_instruction(Opcode::Convert, phi.type);
		after.sub = static_cast<uint8_t>(op);
		set_convert_source_type(after, from);
		after.result = phi.result;
		after.location = phi.location;
		after.ops[0] = Value::temp(narrow);
		body.instructions[id] = after;
		insert_after(body, block, last == id ? prev : last, id);
		changed = true;
	}
	return changed;
}

namespace {

// The most steps of a derived phi's chain, and the most phis of a block read for one.
constexpr size_t kDerivedSteps = 3;
constexpr unsigned kDerivedPhis = 16;

} // namespace

bool derive_matching_phis(
	FunctionContext& c, BlockId block, std::vector<Value>& replacements, std::vector<uint32_t>& uses)
{
	FunctionBody& body = c.body;
	const auto resolve = [&](Value v) {
		while (v.is(ValueKind::Temp) && !replacements[v.index].is(ValueKind::None))
			v = replacements[v.index];
		return v;
	};
	const auto producer = [&](Value v) -> const Instruction* {
		return v.is(ValueKind::Temp) && v.index >= body.parameter_count && body.temps[v.index].def != kNoId
			? &body.instructions[body.temps[v.index].def]
			: nullptr;
	};
	struct Step
	{
		BinaryOp op;
		Value literal;
	};
	// A step of a chain: an operation of `type` on a value and a literal that
	// runs anywhere (no division that could trap).
	const auto step_of = [&](const Instruction* def, Type type, Step& step) {
		if (def == nullptr || def->op != Opcode::Binary || !(def->type == type))
			return false;
		const Value k = resolve(def->ops[1]);
		if (!k.is(ValueKind::Integer))
			return false;
		step = Step{ static_cast<BinaryOp>(def->sub), k };
		const uint64_t low = normalize_integer(type, body.literals[k.index]).low;
		switch (step.op)
		{
		case BinaryOp::Add:
		case BinaryOp::Sub:
		case BinaryOp::Mul:
		case BinaryOp::And:
		case BinaryOp::Or:
		case BinaryOp::Xor:
			return true;
		case BinaryOp::Shl:
		case BinaryOp::Shr:
		case BinaryOp::UShr:
			return low < lowir::integer_bits(type.kind());
		case BinaryOp::Div:
		case BinaryOp::Mod:
			return static_cast<int64_t>(low) > 0;
		case BinaryOp::UDiv:
		case BinaryOp::UMod:
			return low != 0;
		default:
			return false;
		}
	};
	const auto sums = [](const Step& step) { return step.op == BinaryOp::Add || step.op == BinaryOp::Sub; };
	bool changed = false;
	InstId next = kNoId;
	for (InstId id = body.blocks[block].first; id != kNoId && body.instructions[id].op == Opcode::Phi; id = next)
	{
		const Instruction phi = body.instructions[id];
		next = phi.next;
		const Type type = phi.type;
		if (!type.is_integer() || type.kind() == TypeKind::I1 || type.kind() == TypeKind::I128 || phi.extra_count < 4 ||
			phi.extra_count > 16)
			continue;
		const unsigned bits = lowir::integer_bits(type.kind());
		const uint64_t mask = bits >= 64 ? ~uint64_t(0) : (uint64_t(1) << bits) - 1;
		// `v` as a base plus an offset, through sums with literals (a literal: no base).
		const auto affine = [&](Value v, uint64_t& offset) {
			offset = 0;
			for (unsigned depth = 0; depth < 8; ++depth)
			{
				const Instruction* def = producer(v);
				if (def == nullptr || def->op != Opcode::Binary || !(def->type == type))
					break;
				const BinaryOp op = static_cast<BinaryOp>(def->sub);
				const Value a = resolve(def->ops[0]), b = resolve(def->ops[1]);
				if ((op == BinaryOp::Add || op == BinaryOp::Sub) && b.is(ValueKind::Integer))
				{
					const uint64_t k = normalize_integer(type, body.literals[b.index]).low;
					offset += op == BinaryOp::Add ? k : 0 - k;
					v = a;
				}
				else if (op == BinaryOp::Add && a.is(ValueKind::Integer))
				{
					offset += normalize_integer(type, body.literals[a.index]).low;
					v = b;
				}
				else
					break;
			}
			if (v.is(ValueKind::Integer))
			{
				offset += normalize_integer(type, body.literals[v.index]).low;
				v = Value::none();
			}
			return v;
		};
		std::vector<Step> steps; // the outermost first
		// Whether `v` is the chain on `x`.
		const auto derives = [&](Value v, Value x) {
			Constant cv, cx;
			if (literal_constant(body, v, cv) && literal_constant(body, x, cx))
			{
				for (size_t k = steps.size(); k-- != 0;)
				{
					Constant by, out;
					if (!literal_constant(body, steps[k].literal, by) || !fold_binary(steps[k].op, type, cx, by, out))
						return false;
					cx = out;
				}
				return cv.kind == cx.kind && same_constant(type, cv, cx);
			}
			size_t k = 0;
			for (; k < steps.size(); ++k)
			{
				bool tail = true;
				for (size_t j = k; j < steps.size() && tail; ++j)
					tail = sums(steps[j]);
				if (tail)
					break;
				Step step;
				const Instruction* def = producer(v);
				if (!step_of(def, type, step) || step.op != steps[k].op ||
					normalize_integer(type, body.literals[step.literal.index]).low !=
						normalize_integer(type, body.literals[steps[k].literal.index]).low)
					return false;
				v = resolve(def->ops[0]);
			}
			uint64_t delta = 0, from_v, from_x;
			for (; k < steps.size(); ++k)
			{
				const uint64_t by = normalize_integer(type, body.literals[steps[k].literal.index]).low;
				delta += steps[k].op == BinaryOp::Add ? by : 0 - by;
			}
			const Value base_v = affine(v, from_v), base_x = affine(x, from_x);
			return base_v == base_x && ((from_v - from_x - delta) & mask) == 0;
		};
		// The other phi's input from the predecessor of this phi's pair `k`.
		const auto input_of = [&](const Instruction& other, uint32_t k) {
			for (uint32_t j = 0; j < other.extra_count; j += 2)
				if (body.operands[other.extra_first + j].index == body.operands[phi.extra_first + k].index)
					return resolve(body.operands[other.extra_first + j + 1]);
			return Value::none();
		};
		// Each input read by this phi alone; the chain from the first that
		// reaches, a step at a time, what another phi takes there.
		bool sole = true;
		for (uint32_t k = 1; k < phi.extra_count && sole; k += 2)
		{
			const Value in = resolve(body.operands[phi.extra_first + k]);
			sole = !in.is(ValueKind::Temp) || (in.index < uses.size() && uses[in.index] == 1);
		}
		InstId source = kNoId, last = kNoId, prev = kNoId;
		for (uint32_t k = 0; k < phi.extra_count && sole && source == kNoId; k += 2)
		{
			Value v = resolve(body.operands[phi.extra_first + k + 1]);
			steps.clear();
			Step step;
			while (source == kNoId && steps.size() < kDerivedSteps && step_of(producer(v), type, step))
			{
				steps.push_back(step);
				v = resolve(producer(v)->ops[0]);
				unsigned seen = 0;
				for (InstId i = body.blocks[block].first;
					i != kNoId && body.instructions[i].op == Opcode::Phi && seen < kDerivedPhis && source == kNoId;
					i = body.instructions[i].next, ++seen)
				{
					const Instruction& other = body.instructions[i];
					if (i == id || !(other.type == type) || other.extra_count != phi.extra_count ||
						!replacements[other.result].is(ValueKind::None) || !(input_of(other, k) == v))
						continue;
					bool all = true;
					for (uint32_t j = 0; j < phi.extra_count && all; j += 2)
						all = derives(resolve(body.operands[phi.extra_first + j + 1]), input_of(other, j));
					if (all)
						source = i;
				}
			}
		}
		if (source == kNoId)
			continue;
		for (InstId i = body.blocks[block].first; i != kNoId && body.instructions[i].op == Opcode::Phi;
			i = body.instructions[i].next)
		{
			prev = i == id ? last : prev;
			last = i;
		}
		for (uint32_t k = 1; k < phi.extra_count; k += 2)
		{
			const Value old = resolve(body.operands[phi.extra_first + k]);
			if (old.is(ValueKind::Temp) && uses[old.index] != 0xffffffffu && uses[old.index] != 0)
				--uses[old.index];
		}
		const TempId from = body.instructions[source].result;
		if (uses[from] != 0xffffffffu)
			++uses[from];
		// The phi's instruction becomes the chain's last step, after the
		// block's phis and the steps before it.
		unlink_after(body, block, prev, id);
		InstId after = last == id ? prev : last;
		Value value = Value::temp(from);
		for (size_t k = steps.size(); k-- != 0;)
		{
			Instruction inst = make_instruction(Opcode::Binary, type);
			inst.sub = static_cast<uint8_t>(steps[k].op);
			inst.ops[0] = value;
			inst.ops[1] = steps[k].literal;
			inst.location = phi.location;
			if (k == 0)
			{
				inst.result = phi.result;
				body.instructions[id] = inst;
				insert_after(body, block, after, id);
				break;
			}
			const InstId at = append_instruction(body, inst);
			const TempId t = append_temp(body, type, at);
			body.instructions[at].result = t;
			insert_after(body, block, after, at);
			after = at;
			replacements.resize(body.temps.size(), Value::none());
			uses.resize(body.temps.size(), 0);
			uses[t] = 1;
			value = Value::temp(t);
		}
		changed = true;
	}
	return changed;
}

bool diamond_at(const FunctionBody& body, const Cfg& cfg, BlockId head, Diamond& d)
{
	const InstId t = body.blocks[head].last;
	if (t == kNoId || body.instructions[t].op != Opcode::Branch)
		return false;
	const Instruction& term = body.instructions[t];
	d.head = head;
	d.condition = term.ops[0];
	d.on_true = term.ops[1].index;
	d.on_false = term.ops[2].index;
	if (d.on_true == d.on_false)
		return false;
	BlockId merges[2];
	const BlockId arms[2] = { d.on_true, d.on_false };
	for (unsigned k = 0; k < 2; ++k)
	{
		const InstId last = body.blocks[arms[k]].last;
		if (cfg.pred_count(arms[k]) != 1 || last == kNoId || body.instructions[last].op != Opcode::Jump ||
			(cfg.flags[arms[k]] & BF_HANDLER) != 0)
			return false;
		merges[k] = body.instructions[last].ops[0].index;
	}
	d.merge = merges[0];
	return merges[0] == merges[1] && cfg.pred_count(d.merge) == 2;
}

// No instruction of the block writes memory.
bool write_free(const Program& program, const FunctionBody& body, BlockId block)
{
	for (InstId i = body.blocks[block].first; i != kNoId; i = body.instructions[i].next)
		if (may_write_memory(program, body.instructions[i]) || body.instructions[i].op == Opcode::Store ||
			(body.instructions[i].flags & IF_VOLATILE) != 0)
			return false;
	return true;
}

bool same_computation(const FunctionBody& body, Value a, Value b, unsigned depth, const QuietBlocks& quiet)
{
	if (a == b)
		return true;
	Constant x, y;
	if (literal_constant(body, a, x) && literal_constant(body, b, y))
		return x.kind == y.kind && x.bits.low == y.bits.low && x.bits.high == y.bits.high;
	// Equal symbols were handled above. Distinct symbol IDs cannot be
	// interpreted as temporary IDs when comparing defining instructions.
	if (!a.is(ValueKind::Temp) || !b.is(ValueKind::Temp) || depth == 0 || a.index < body.parameter_count ||
		b.index < body.parameter_count || body.temps[a.index].def == kNoId || body.temps[b.index].def == kNoId)
		return false;
	const Instruction& p = body.instructions[body.temps[a.index].def];
	const Instruction& q = body.instructions[body.temps[b.index].def];
	const bool load = p.op == Opcode::Load && (p.flags & IF_VOLATILE) == 0 && quiet.holds(body.temps[a.index].def) &&
		quiet.holds(body.temps[b.index].def);
	if (p.op != q.op || p.sub != q.sub || p.type != q.type || p.immediate != q.immediate || p.flags != q.flags ||
		!(is_pure(p) || load) || p.op == Opcode::Phi || p.extra_count != 0 || q.extra_count != 0)
		return false;
	for (unsigned k = 0; k < 3; ++k)
		if (!same_computation(body, p.ops[k], q.ops[k], depth - 1, quiet))
			return false;
	return true;
}

uint32_t phi_count(const FunctionBody& body, BlockId block)
{
	uint32_t count = 0;
	for (InstId i = body.blocks[block].first; i != kNoId && body.instructions[i].op == Opcode::Phi;
		i = body.instructions[i].next)
		++count;
	return count;
}
} // namespace opt
