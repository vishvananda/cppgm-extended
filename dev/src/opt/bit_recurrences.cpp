#include "opt/bit_recurrences.h"

#include <algorithm>
#include <string>

#include "lowir/builder.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

bool BitTableKey::operator==(const BitTableKey& other) const
{
	return polynomial == other.polynomial && width == other.width && steps == other.steps && inverted == other.inverted;
}

size_t BitTableHash::operator()(const BitTableKey& key) const
{
	uint64_t hash = key.polynomial ^ (static_cast<uint64_t>(key.width) << 48) ^
		(static_cast<uint64_t>(key.steps) << 40) ^ (static_cast<uint64_t>(key.inverted) << 32);
	hash ^= hash >> 30;
	hash *= 0xbf58476d1ce4e5b9ULL;
	hash ^= hash >> 27;
	return static_cast<size_t>(hash ^ (hash >> 31));
}

SymbolId BitTablePool::table(Program& program, const BitTableKey& key, OptimizerStats& stats)
{
	if (const SymbolId* known = tables.find(key))
		return *known;
	const uint32_t entries = 1u << key.steps;
	const uint32_t bytes = entries * (key.width / 8);
	if (bytes > remaining)
		return kNoId;
	NameId name;
	do
	{
		const std::string spelling = "__bit_table_" + std::to_string(names++);
		name = program.names.intern(spelling.data(), static_cast<uint32_t>(spelling.size()));
	} while (program.find_symbol(name) != kNoId);
	const SymbolId symbol = program.symbol_for(name);
	const TypeKind kind = key.width == 8 ? TypeKind::U8
		: key.width == 16                ? TypeKind::U16
		: key.width == 32                ? TypeKind::U32
										 : TypeKind::I64;
	Global global = Global();
	global.typed = true;
	global.type = Type::object(bytes, key.width == 8 ? 0 : key.width == 16 ? 1 : key.width == 32 ? 2 : 3);
	global.init = GlobalInit::Data;
	global.first_item = static_cast<uint32_t>(program.data_items.size());
	global.item_count = entries;
	for (uint32_t i = 0; i < entries; ++i)
	{
		uint64_t value = i;
		for (uint32_t step = 0; step < key.steps; ++step)
			value = (value >> 1) ^ (((value & 1) ^ ((key.inverted >> step) & 1)) ? key.polynomial : 0);
		DataItem item = DataItem();
		item.kind = DataItemKind::Scalar;
		item.type = Type::scalar(kind);
		item.constant.kind = ValueKind::Integer;
		item.constant.float_kind = TypeKind::Void;
		item.constant.bits = integer_literal(static_cast<int64_t>(value));
		item.address = kNoId;
		program.data_items.push_back(item);
	}
	program.declare_global(symbol, SymbolKind::GlobalDefinition, global);
	program.symbols[symbol].metadata.binding = ir_model::SBM_INTERNAL;
	program.symbols[symbol].metadata.storage = ir_model::GSM_READONLY;
	tables.insert(key, symbol);
	remaining -= bytes;
	stats.bit_table_bytes += bytes;
	return symbol;
}

namespace {

const unsigned kMinSteps = 4, kMaxSteps = 8;

class BitRecurrences
{
public:
	BitRecurrences(FunctionContext& c, BitTablePool& pool) : c_(c), body_(c.body), pool_(pool) {}
	bool run();

private:
	const Instruction* definition(Value value) const;
	bool literal(Value value, uint64_t& bits) const;
	bool binary(const Instruction* inst, BinaryOp op) const;
	const std::vector<BlockId>& blocks();
	void own(InstId id);
	void own(Value value);
	bool condition(Value value, Value input, bool& true_bit);
	bool phi_arms(InstId id, Value& condition, Value& yes, Value& no, BlockId& left, BlockId& right);
	bool step(InstId id, Value& input, uint64_t& polynomial, bool& inverted);
	bool closed(TempId root);
	bool fold(InstId root);
	Value emit(BlockId block, InstId& after, Instruction inst, uint32_t location);
	Value number(uint64_t value);
	void rewrite(InstId root, Value input, const BitTableKey& key, SymbolId table);

	FunctionContext& c_;
	FunctionBody& body_;
	BitTablePool& pool_;
	bool prepared_ = false, cleaned_ = false;
	const std::vector<BlockId>* blocks_ = nullptr;
	const std::vector<uint32_t>* uses_ = nullptr;
};

const Instruction* BitRecurrences::definition(Value value) const
{
	if (!value.is(ValueKind::Temp) || body_.temps[value.index].def == kNoId)
		return nullptr;
	return &body_.instructions[body_.temps[value.index].def];
}

bool BitRecurrences::literal(Value value, uint64_t& bits) const
{
	if (!value.is(ValueKind::Integer))
		return false;
	bits = body_.literals[value.index].low;
	const uint64_t high = body_.literals[value.index].high;
	return high == 0 || (high == ~uint64_t(0) && (bits >> 63) != 0);
}

bool BitRecurrences::binary(const Instruction* inst, BinaryOp op) const
{
	return inst != nullptr && inst->op == Opcode::Binary && inst->sub == static_cast<uint8_t>(op);
}

const std::vector<BlockId>& BitRecurrences::blocks()
{
	if (blocks_ == nullptr)
		blocks_ = &c_.entry_block_of(fold_bit_recurrences);
	return *blocks_;
}

void BitRecurrences::own(InstId id)
{
	if (std::find(pool_.owned.begin(), pool_.owned.end(), id) == pool_.owned.end())
		pool_.owned.push_back(id);
}

void BitRecurrences::own(Value value)
{
	if (value.is(ValueKind::Temp) && body_.temps[value.index].def != kNoId)
		own(body_.temps[value.index].def);
}

bool BitRecurrences::condition(Value value, Value input, bool& true_bit)
{
	true_bit = true;
	const Instruction* inst = definition(value);
	if (inst != nullptr && inst->op == Opcode::Compare)
	{
		uint64_t constant;
		Value test = inst->ops[0];
		if (!literal(inst->ops[1], constant))
		{
			test = inst->ops[1];
			if (!literal(inst->ops[0], constant))
				return false;
		}
		if (constant > 1 ||
			(inst->sub != static_cast<uint8_t>(ComparePredicate::Eq) &&
				inst->sub != static_cast<uint8_t>(ComparePredicate::Ne)))
			return false;
		true_bit = (inst->sub == static_cast<uint8_t>(ComparePredicate::Eq)) == (constant == 1);
		own(value);
		value = test;
		inst = definition(value);
	}
	if (!binary(inst, BinaryOp::And))
		return false;
	uint64_t mask;
	if (!((inst->ops[0] == input && literal(inst->ops[1], mask) && mask == 1) ||
			(inst->ops[1] == input && literal(inst->ops[0], mask) && mask == 1)))
		return false;
	own(value);
	return true;
}

bool BitRecurrences::phi_arms(InstId id, Value& cond, Value& yes, Value& no, BlockId& left, BlockId& right)
{
	const Instruction& phi = body_.instructions[id];
	if (phi.extra_count != 4)
		return false;
	const BlockId merge = blocks()[id];
	for (InstId p = body_.blocks[merge].first; p != kNoId && body_.instructions[p].op == Opcode::Phi;
		p = body_.instructions[p].next)
		if (p != id)
			return false;
	left = body_.operands[phi.extra_first].index;
	right = body_.operands[phi.extra_first + 2].index;
	if (left == right || left == merge || right == merge)
		return false;
	Cfg& cfg = c_.ensure_cfg_light();
	if (cfg.pred_count(left) != 1 || cfg.pred_count(right) != 1)
		return false;
	const BlockId head = cfg.preds[cfg.pred_first[left]];
	if (cfg.preds[cfg.pred_first[right]] != head)
		return false;
	const Instruction& branch = body_.instructions[body_.blocks[head].last];
	if (branch.op != Opcode::Branch ||
		!((branch.ops[1].index == left && branch.ops[2].index == right) ||
			(branch.ops[1].index == right && branch.ops[2].index == left)))
		return false;
	for (BlockId b : { left, right })
	{
		const Instruction& jump = body_.instructions[body_.blocks[b].last];
		if (jump.op != Opcode::Jump || jump.ops[0].index != merge)
			return false;
	}
	const bool forward = branch.ops[1].index == left;
	yes = body_.operands[phi.extra_first + (forward ? 1 : 3)];
	no = body_.operands[phi.extra_first + (forward ? 3 : 1)];
	cond = branch.ops[0];
	own(body_.blocks[head].last);
	return true;
}

bool BitRecurrences::step(InstId id, Value& input, uint64_t& polynomial, bool& inverted)
{
	const Instruction& root = body_.instructions[id];
	Value cond, yes, no;
	BlockId left = kNoId, right = kNoId;
	if (root.op == Opcode::Select)
	{
		cond = root.ops[0];
		yes = root.ops[1];
		no = root.ops[2];
	}
	else if (root.op != Opcode::Phi || !phi_arms(id, cond, yes, no, left, right))
		return false;
	const bool xor_true = binary(definition(yes), BinaryOp::Xor);
	const Value mixed = xor_true ? yes : no, plain = xor_true ? no : yes;
	const Instruction* x = definition(mixed);
	if (!binary(x, BinaryOp::Xor) || x->type != root.type)
		return false;
	Value shifted = x->ops[0];
	if (!literal(x->ops[1], polynomial))
	{
		shifted = x->ops[1];
		if (!literal(x->ops[0], polynomial))
			return false;
	}
	const Instruction* a = definition(shifted);
	const Instruction* b = definition(plain);
	uint64_t first, second;
	if (!binary(a, BinaryOp::UShr) || !binary(b, BinaryOp::UShr) || a->type != root.type || b->type != root.type ||
		!(a->ops[0] == b->ops[0]) || !literal(a->ops[1], first) || first != 1 || !literal(b->ops[1], second) ||
		second != 1)
		return false;
	input = a->ops[0];
	bool true_bit;
	if (!condition(cond, input, true_bit))
		return false;
	inverted = xor_true != true_bit;
	own(id);
	own(mixed);
	own(plain);
	own(shifted);
	if (left != kNoId)
		for (BlockId block : { left, right })
			for (InstId i = body_.blocks[block].first; i != body_.blocks[block].last; i = body_.instructions[i].next)
				if (std::find(pool_.owned.begin(), pool_.owned.end(), i) == pool_.owned.end())
					return false;
	return true;
}

bool BitRecurrences::closed(TempId root)
{
	if (uses_ == nullptr)
		uses_ = &c_.entry_uses(fold_bit_recurrences);
	const auto& uses = *uses_;
	pool_.internal.assign(pool_.owned.size(), 0);
	for (InstId id : pool_.owned)
		for_each_operand(body_, body_.instructions[id], [&](const Value& value) {
			if (!value.is(ValueKind::Temp))
				return;
			for (size_t n = 0; n < pool_.owned.size(); ++n)
				if (body_.instructions[pool_.owned[n]].result == value.index)
					++pool_.internal[n];
		});
	for (size_t n = 0; n < pool_.owned.size(); ++n)
	{
		const TempId temp = body_.instructions[pool_.owned[n]].result;
		if (temp != kNoId && temp != root && uses[temp] != pool_.internal[n])
			return false;
	}
	return true;
}

Value BitRecurrences::number(uint64_t value)
{
	Literal bits = {};
	bits.low = value;
	return integer_value(body_, bits);
}

Value BitRecurrences::emit(BlockId block, InstId& after, Instruction inst, uint32_t location)
{
	inst.location = location;
	const InstId id = append_instruction(body_, inst);
	body_.instructions[id].result = append_temp(body_, inst.type, id);
	insert_after(body_, block, after, id);
	after = id;
	return Value::temp(body_.instructions[id].result);
}

void BitRecurrences::rewrite(InstId root, Value input, const BitTableKey& key, SymbolId table)
{
	const Type type = body_.instructions[root].type;
	const TempId result = body_.instructions[root].result;
	const uint32_t location = body_.instructions[root].location;
	const BlockId block = blocks()[root];
	// Matching permits only one phi in a merge. Reverse candidate order
	// leaves each original predecessor valid until its rewrite is inserted.
	InstId after = body_.instructions[root].op == Opcode::Phi ? root : pool_.previous[root];
	auto operation = [&](BinaryOp op, Value a, Value b) {
		Instruction inst = make_instruction(Opcode::Binary, type);
		inst.sub = static_cast<uint8_t>(op);
		inst.ops[0] = a;
		inst.ops[1] = b;
		return emit(block, after, inst, location);
	};
	Value index = operation(BinaryOp::And, input, number((1u << key.steps) - 1));
	if (key.width < 64)
	{
		Instruction convert = make_instruction(Opcode::Convert, Type::scalar(TypeKind::I64));
		convert.sub = static_cast<uint8_t>(ConvertOp::ZExt);
		set_convert_source_type(convert, type);
		convert.ops[0] = index;
		index = emit(block, after, convert, location);
	}
	Instruction base = make_instruction(Opcode::Addr, Type::scalar(TypeKind::Ptr));
	base.ops[0] = Value::symbol(table);
	const Value pointer = emit(block, after, base, location);
	Instruction address = make_instruction(Opcode::Index, type);
	address.ops[0] = pointer;
	address.ops[1] = index;
	// index's type is its element type; its result is a pointer.
	const InstId a = append_instruction(body_, address);
	body_.instructions[a].location = location;
	body_.instructions[a].result = append_temp(body_, Type::scalar(TypeKind::Ptr), a);
	insert_after(body_, block, after, a);
	after = a;
	Instruction load = make_instruction(Opcode::Load, type);
	load.ops[0] = Value::temp(body_.instructions[a].result);
	const Value low = emit(block, after, load, location);
	const Value high = key.steps == key.width ? number(0) : operation(BinaryOp::UShr, input, number(key.steps));
	const Value folded = operation(BinaryOp::Xor, high, low);
	pool_.replacements[result] = folded.index;
}

bool BitRecurrences::fold(InstId root)
{
	const Instruction& inst = body_.instructions[root];
	if (!inst.type.is_integer())
		return false;
	const uint32_t width = integer_bits(inst.type.kind());
	if (width < 8 || width > 64)
		return false;
	pool_.owned.clear();
	BitTableKey key = {};
	key.width = width;
	InstId next = root;
	Value input;
	uint32_t reverse = 0;
	while (key.steps < kMaxSteps && key.steps < width)
	{
		if (!pool_.covered.empty() && pool_.covered[next])
			break;
		const size_t before = pool_.owned.size();
		uint64_t polynomial;
		bool inverted;
		Value from;
		const bool matched = body_.instructions[next].type == inst.type && step(next, from, polynomial, inverted);
		if (matched && width < 64)
			polynomial &= (uint64_t(1) << width) - 1;
		if (!matched || (key.steps != 0 && polynomial != key.polynomial))
		{
			pool_.owned.resize(before);
			break;
		}
		key.polynomial = polynomial;
		reverse |= static_cast<uint32_t>(inverted) << key.steps;
		++key.steps;
		input = from;
		if (!input.is(ValueKind::Temp) || body_.temps[input.index].def == kNoId)
			break;
		next = body_.temps[input.index].def;
		if (std::find(pool_.owned.begin(), pool_.owned.end(), next) != pool_.owned.end())
			return false;
	}
	if (key.steps < kMinSteps)
		return false;
	if (!prepared_)
	{
		// Earlier compare placement can leave dead copies reading the same
		// bit test. Clean them only once a long recurrence warrants analysis.
		prepared_ = true;
		cleaned_ = remove_dead_code(c_);
		if (cleaned_)
		{
			c_.invalidate_uses();
			c_.invalidate_block_of();
			blocks_ = nullptr;
		}
	}
	if (blocks()[root] == kNoId || !closed(inst.result))
		return false;
	for (uint32_t i = 0; i < key.steps; ++i)
		key.inverted |= ((reverse >> i) & 1) << (key.steps - i - 1);
	if (width < 64)
		key.polynomial &= (uint64_t(1) << width) - 1;
	const SymbolId table = pool_.table(c_.program, key, c_.stats);
	if (table == kNoId)
		return false;
	if (pool_.covered.empty())
	{
		pool_.covered.assign(body_.instructions.size(), 0);
		pool_.replacements.assign(body_.temps.size(), kNoId);
		pool_.previous.assign(body_.instructions.size(), kNoId);
		for (BlockId b : body_.layout)
			for (InstId prev = kNoId, i = body_.blocks[b].first; i != kNoId; prev = i, i = body_.instructions[i].next)
				pool_.previous[i] = prev;
	}
	for (InstId id : pool_.owned)
		pool_.covered[id] = 1;
	rewrite(root, input, key, table);
	++c_.stats.bit_recurrences_folded;
	return true;
}

bool BitRecurrences::run()
{
	pool_.candidates.clear();
	pool_.covered.clear();
	pool_.replacements.clear();
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			if (body_.instructions[i].op == Opcode::Select || body_.instructions[i].op == Opcode::Phi)
				pool_.candidates.push_back(i);
	bool changed = false;
	for (size_t n = pool_.candidates.size(); n-- > 0;)
	{
		const InstId id = pool_.candidates[n];
		if ((pool_.covered.empty() || !pool_.covered[id]) && fold(id))
			changed = true;
	}
	// Targets are fresh temporaries, so one substitution walk suffices.
	// Keep the original graph/use counts intact throughout matching.
	if (changed)
		for (BlockId b : body_.layout)
			for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
				for_each_operand(body_, body_.instructions[i], [&](Value& value) {
					if (value.is(ValueKind::Temp) && value.index < pool_.replacements.size() &&
						pool_.replacements[value.index] != kNoId)
						value = Value::temp(pool_.replacements[value.index]);
				});
	return changed || cleaned_;
}

} // namespace

bool fold_bit_recurrences(FunctionContext& context)
{
	// For each step F(x)=(x>>1)^((lowbit(x)^flip)?P:0), its linear
	// part maps a word whose low k bits are zero to x>>k after k steps.
	// Thus the composition is (x>>k)^table[x&mask], with the table obtained
	// by evaluating those steps on every low-bit input (affine flips included).
	// No assumption about a particular polynomial or source construct is used.
	if (context.bit_tables == nullptr)
		return false;
	BitRecurrences pass(context, *context.bit_tables);
	return pass.run();
}

} // namespace opt
