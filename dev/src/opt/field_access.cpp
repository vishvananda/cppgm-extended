#include <map>
#include <vector>

#include "lowir/builder.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

namespace opt {

using namespace lowir;

namespace {

// Producers followed for the bits a value may have set.
const unsigned kFieldDepth = 6;

// O1+, once memory forwarding is done: a field of a loaded or stored word
// (a bit-field's storage unit, a flags word) is read, tested and written
// through its own bytes where it has whole bytes.
class FieldAccess
{
public:
	explicit FieldAccess(FunctionContext& c) : c_(c), body_(c.body) {}
	bool run(bool loads);

private:
	FunctionContext& c_;
	FunctionBody& body_;
	std::vector<BlockId> block_of_;
	std::vector<uint32_t> uses_; // per temp, as the pass began
	// The field loads made, by the word's load, the field's bit and its width.
	std::map<uint64_t, Value> fields_;

	const Instruction* producer(Value v) const
	{
		return v.is(ValueKind::Temp) && v.index >= body_.parameter_count && body_.temps[v.index].def != kNoId
			? &body_.instructions[body_.temps[v.index].def]
			: nullptr;
	}
	// The integer type's width when a field can be cut from it.
	static unsigned word_bits(Type type)
	{
		const unsigned bits = type.is_integer() ? lowir::integer_bits(type.kind()) : 0;
		return bits == 16 || bits == 32 || bits == 64 ? bits : 0;
	}
	static uint64_t ones(unsigned bits) { return bits >= 64 ? ~uint64_t(0) : (uint64_t(1) << bits) - 1; }
	bool literal(Value v, unsigned bits, uint64_t& out) const
	{
		if (!v.is(ValueKind::Integer) || body_.literals[v.index].high != 0)
			return false;
		out = body_.literals[v.index].low & ones(bits);
		return true;
	}
	// `binary op x, k` at `type`, k a literal.
	const Instruction* binary(Value v, BinaryOp op, Type type, uint64_t& k) const
	{
		const Instruction* d = producer(v);
		return d != nullptr && d->op == Opcode::Binary && static_cast<BinaryOp>(d->sub) == op && d->type == type &&
				literal(d->ops[1], lowir::integer_bits(type.kind()), k)
			? d
			: nullptr;
	}
	// A right shift by a literal, and whether it copies the sign in.
	const Instruction* right_shift(Value v, Type type, uint64_t& count, bool& arithmetic) const
	{
		const Instruction* d = binary(v, BinaryOp::UShr, type, count);
		arithmetic = false;
		if (d == nullptr)
		{
			d = binary(v, BinaryOp::Shr, type, count);
			arithmetic = d != nullptr && !lowir::is_unsigned(type.kind());
		}
		return d != nullptr && count < lowir::integer_bits(type.kind()) ? d : nullptr;
	}
	// The plain load of a word through a pointer value.
	const Instruction* word_load(Value v, unsigned bits) const
	{
		const Instruction* d = producer(v);
		return d != nullptr && d->op == Opcode::Load && (d->flags & IF_VOLATILE) == 0 && word_bits(d->type) == bits &&
				d->ops[0].is(ValueKind::Temp)
			? d
			: nullptr;
	}
	// (An index's type is its element's; its result is a pointer.)
	InstId emit(BlockId block, InstId& prev, const Instruction& inst)
	{
		const InstId id = append_instruction(body_, inst);
		const Type result = inst.op == Opcode::Index ? body_.pointer_type() : inst.type;
		body_.instructions[id].result = append_temp(body_, result, id);
		insert_after(body_, block, prev, id);
		prev = id;
		block_of_.resize(body_.instructions.size(), block);
		block_of_[id] = block;
		return id;
	}
	// The address `bytes` past `address`.
	Value displaced(BlockId block, InstId& prev, Value address, uint64_t bytes, uint32_t location)
	{
		if (bytes == 0)
			return address;
		Instruction index = make_instruction(Opcode::Index, Type::scalar(TypeKind::I8));
		index.ops[0] = address;
		index.ops[1] = integer_value(body_, Literal{ bytes, 0 });
		index.location = location;
		return Value::temp(body_.instructions[emit(block, prev, index)].result);
	}
	static Type field_type(unsigned bits)
	{
		return Type::scalar(bits == 8 ? TypeKind::U8 : bits == 16 ? TypeKind::U16 : TypeKind::U32);
	}
	uint64_t possible_ones(Value v, Type type, unsigned depth) const;
	bool field_load(InstId id);
	bool shifted_test(BlockId block, InstId& prev, Value& tested, uint32_t location);
	bool shifted_compare(BlockId block, InstId& prev, InstId id);
	bool whole_shift(InstId id);
	bool field_store(BlockId block, InstId& prev, InstId id);
};

// The bits of `v` that may be set.
uint64_t FieldAccess::possible_ones(Value v, Type type, unsigned depth) const
{
	const unsigned bits = lowir::integer_bits(type.kind());
	const uint64_t all = ones(bits);
	uint64_t k;
	if (literal(v, bits, k))
		return k;
	const Instruction* d = producer(v);
	if (d == nullptr || depth == 0)
		return all;
	if (d->op == Opcode::Compare)
		return 1;
	if (d->op == Opcode::Copy && d->ops[0].is(ValueKind::Temp) && body_.temps[d->ops[0].index].type.is_integer() &&
		lowir::integer_bits(body_.temps[d->ops[0].index].type.kind()) == bits)
		return possible_ones(d->ops[0], body_.temps[d->ops[0].index].type, depth - 1);
	if (d->op == Opcode::Convert && static_cast<ConvertOp>(d->sub) == ConvertOp::ZExt &&
		convert_source_type(*d).is_integer())
	{
		const Type from = convert_source_type(*d);
		return possible_ones(d->ops[0], from, depth - 1) & ones(lowir::integer_bits(from.kind()));
	}
	if (d->op != Opcode::Binary || d->type != type)
		return all;
	const BinaryOp op = static_cast<BinaryOp>(d->sub);
	if (op == BinaryOp::Or || op == BinaryOp::Xor)
		return possible_ones(d->ops[0], type, depth - 1) | possible_ones(d->ops[1], type, depth - 1);
	if (op == BinaryOp::And)
		return possible_ones(d->ops[0], type, depth - 1) & possible_ones(d->ops[1], type, depth - 1);
	if (!literal(d->ops[1], bits, k) || k >= bits)
		return all;
	if (op == BinaryOp::Shl)
		return (possible_ones(d->ops[0], type, depth - 1) << k) & all;
	if (op == BinaryOp::UShr || (op == BinaryOp::Shr && !!lowir::is_unsigned(type.kind())))
		return possible_ones(d->ops[0], type, depth - 1) >> k;
	return all;
}

// `and (shr (load word p), 8 * b), ones(8 | 16 | 32)`, the shift optional:
// the field's own load, zero-extended. The load goes where the word's is,
// which reads the same memory, and every reader of the field takes it.
bool FieldAccess::field_load(InstId id)
{
	const Instruction conj = body_.instructions[id];
	const unsigned bits = word_bits(conj.type);
	uint64_t mask, count = 0;
	if (conj.op != Opcode::Binary || static_cast<BinaryOp>(conj.sub) != BinaryOp::And || bits == 0 ||
		!literal(conj.ops[1], bits, mask) || (mask != 0xff && mask != 0xffff && mask != 0xffffffffull))
		return false;
	const unsigned width = mask == 0xff ? 8 : mask == 0xffff ? 16 : 32;
	Value word = conj.ops[0];
	bool arithmetic = false;
	if (const Instruction* shift = right_shift(word, conj.type, count, arithmetic))
		word = shift->ops[0];
	const Instruction* loaded = word_load(word, bits);
	if (loaded == nullptr || count % 8 != 0 || count + width > bits || width >= bits)
		return false;
	const InstId at = body_.temps[word.index].def;
	const uint64_t key = (uint64_t(at) << 16) | (count << 8) | width;
	auto made = fields_.find(key);
	if (made == fields_.end())
	{
		const Value address = loaded->ops[0];
		const uint32_t location = loaded->location;
		const BlockId block = block_of_[at];
		InstId prev = at;
		Instruction load = make_instruction(Opcode::Load, field_type(width));
		load.ops[0] = displaced(block, prev, address, count / 8, location);
		load.location = location;
		const Value narrow = Value::temp(body_.instructions[emit(block, prev, load)].result);
		made = fields_.emplace(key, narrow).first;
	}
	const Value narrow = made->second;
	Instruction& out = body_.instructions[id];
	out.op = Opcode::Convert;
	out.sub = static_cast<uint8_t>(ConvertOp::ZExt);
	out.flags = 0;
	out.ops[0] = narrow;
	out.ops[1] = Value::none();
	set_convert_source_type(out, field_type(width));
	return true;
}

// A value only tested against zero, `and (shr x, c), m`: `and x, m << c`
// is zero when it is (the mask in the shifted-in bits is none of x's).
bool FieldAccess::shifted_test(BlockId block, InstId& prev, Value& tested, uint32_t location)
{
	const Instruction* conj = producer(tested);
	if (conj == nullptr || conj->op != Opcode::Binary || static_cast<BinaryOp>(conj->sub) != BinaryOp::And)
		return false;
	const Type type = conj->type;
	const unsigned bits = type.is_integer() ? lowir::integer_bits(type.kind()) : 0;
	uint64_t mask, count;
	bool arithmetic;
	if (bits == 0 || bits > 64 || !literal(conj->ops[1], bits, mask))
		return false;
	const Instruction* shift = right_shift(conj->ops[0], type, count, arithmetic);
	// (A test's immediate is 32 bits, sign-extended.)
	if (shift == nullptr || count == 0 || mask == 0 || mask > (ones(bits) >> count) || (mask << count) > 0x7fffffffull)
		return false;
	Instruction moved = make_instruction(Opcode::Binary, type);
	moved.sub = static_cast<uint8_t>(BinaryOp::And);
	moved.ops[0] = shift->ops[0];
	moved.ops[1] = integer_value(body_, Literal{ mask << count, 0 });
	moved.location = location;
	tested = Value::temp(body_.instructions[emit(block, prev, moved)].result);
	return true;
}

// `cmp eq (and (shr x, c), m), k` (or `ne`), the shift and the mask read by
// nothing else: `cmp eq (and x, m << c), k << c`, the field compared where
// it lies in its word. (Late: the passes that gather a field's tests read
// the field's own value.)
bool FieldAccess::shifted_compare(BlockId block, InstId& prev, InstId id)
{
	const Instruction seen = body_.instructions[id];
	const Instruction* conj = producer(seen.ops[0]);
	if (conj == nullptr || conj->op != Opcode::Binary || static_cast<BinaryOp>(conj->sub) != BinaryOp::And ||
		conj->type != seen.type || conj->result >= uses_.size() || uses_[conj->result] != 1)
		return false;
	const Type type = conj->type;
	const unsigned bits = type.is_integer() ? lowir::integer_bits(type.kind()) : 0;
	uint64_t mask, count, k;
	bool arithmetic;
	if (bits == 0 || bits > 64 || !literal(conj->ops[1], bits, mask) || !literal(seen.ops[1], bits, k))
		return false;
	const Instruction* shift = right_shift(conj->ops[0], type, count, arithmetic);
	// (An immediate is 32 bits, sign-extended.)
	if (shift == nullptr || shift->result >= uses_.size() || uses_[shift->result] != 1 || count == 0 || mask == 0 ||
		mask > (ones(bits) >> count) || (mask << count) > 0x7fffffffull || (k & ~mask) != 0)
		return false;
	Instruction moved = make_instruction(Opcode::Binary, type);
	moved.sub = static_cast<uint8_t>(BinaryOp::And);
	moved.ops[0] = shift->ops[0];
	moved.ops[1] = integer_value(body_, normalize_integer(type, Literal{ mask << count, 0 }));
	moved.location = seen.location;
	const Value field = Value::temp(body_.instructions[emit(block, prev, moved)].result);
	Instruction& out = body_.instructions[id];
	out.ops[0] = field;
	out.ops[1] = integer_value(body_, normalize_integer(type, Literal{ k << count, 0 }));
	return true;
}

// `and (shr x, c), ones(bits - c)` of a signed x: the logical shift alone.
bool FieldAccess::whole_shift(InstId id)
{
	Instruction& conj = body_.instructions[id];
	if (conj.op != Opcode::Binary || static_cast<BinaryOp>(conj.sub) != BinaryOp::And || !conj.type.is_integer())
		return false;
	const unsigned bits = lowir::integer_bits(conj.type.kind());
	uint64_t mask, count;
	bool arithmetic;
	if (bits == 0 || bits > 64 || !literal(conj.ops[1], bits, mask))
		return false;
	const Instruction* shift = right_shift(conj.ops[0], conj.type, count, arithmetic);
	if (shift == nullptr || count == 0 || mask != (ones(bits) >> count))
		return false;
	conj.sub = static_cast<uint8_t>(BinaryOp::UShr);
	conj.flags &= static_cast<uint8_t>(~(IF_NO_WRAP | IF_EXACT));
	conj.ops[1] = shift->ops[1];
	conj.ops[0] = shift->ops[0];
	return true;
}

// `store word (or (and (load word p), keep), v), p` (or the `and` alone)
// with nothing writing memory between the load and the store, `keep`
// clear on exactly the bytes [lo, hi) and v zero outside them: the store
// of v's bytes there. The other bytes keep what they hold.
bool FieldAccess::field_store(BlockId block, InstId& prev, InstId id)
{
	const Instruction store = body_.instructions[id];
	const unsigned bits = store.op == Opcode::Store && (store.flags & IF_VOLATILE) == 0 ? word_bits(store.type) : 0;
	if (bits == 0 || !store.ops[1].is(ValueKind::Temp))
		return false;
	const Type type = store.type;
	const Instruction* merged = producer(store.ops[0]);
	if (merged == nullptr || merged->op != Opcode::Binary || merged->type != type)
		return false;
	Value inserted = Value::none();
	Value kept = store.ops[0];
	if (static_cast<BinaryOp>(merged->sub) == BinaryOp::Or)
	{
		uint64_t k;
		const unsigned side = binary(merged->ops[0], BinaryOp::And, type, k) != nullptr &&
				word_load(producer(merged->ops[0])->ops[0], bits) != nullptr
			? 0
			: 1;
		kept = merged->ops[side];
		inserted = merged->ops[1 - side];
	}
	uint64_t keep;
	const Instruction* conj = binary(kept, BinaryOp::And, type, keep);
	const Instruction* loaded = conj != nullptr ? word_load(conj->ops[0], bits) : nullptr;
	if (loaded == nullptr || !(loaded->ops[0] == store.ops[1]) ||
		block_of_[body_.temps[conj->ops[0].index].def] != block)
		return false;
	const uint64_t written = ~keep & ones(bits);
	if (written == 0)
		return false;
	const unsigned lo = static_cast<unsigned>(__builtin_ctzll(written)) / 8;
	const unsigned hi = (64 - static_cast<unsigned>(__builtin_clzll(written)) + 7) / 8;
	const unsigned width = 8 * (hi - lo);
	if ((width != 8 && width != 16 && width != 32) || width >= bits || written != (ones(width) << (8 * lo)) ||
		(!inserted.is(ValueKind::None) && (possible_ones(inserted, type, kFieldDepth) & ~written) != 0))
		return false;
	// (The load's memory is the store's: nothing between them writes any.)
	InstId i = body_.instructions[body_.temps[conj->ops[0].index].def].next;
	for (; i != kNoId && i != id; i = body_.instructions[i].next)
		if (may_write_memory(c_.program, body_.instructions[i]))
			return false;
	if (i != id)
		return false;
	// v's bytes [lo, hi): through `shl x, 8 * lo` they are x's low ones,
	// and a mask keeping all of them changes none.
	Value value = inserted;
	unsigned shift = 8 * lo;
	uint64_t k;
	if (value.is(ValueKind::None))
		value = integer_value(body_, Literal{ 0, 0 });
	else if (literal(value, bits, k))
		value = integer_value(body_, Literal{ (k >> shift) & ones(width), 0 });
	else
	{
		const Instruction* left = binary(value, BinaryOp::Shl, type, k);
		if (left != nullptr && k == shift)
		{
			value = left->ops[0];
			shift = 0;
		}
		if (shift != 0)
		{
			Instruction down = make_instruction(Opcode::Binary, type);
			down.sub = static_cast<uint8_t>(BinaryOp::UShr);
			down.ops[0] = value;
			down.ops[1] = integer_value(body_, Literal{ shift, 0 });
			down.location = store.location;
			value = Value::temp(body_.instructions[emit(block, prev, down)].result);
		}
		for (const Instruction* m = binary(value, BinaryOp::And, type, k);
			m != nullptr && (k & ones(width)) == ones(width); m = binary(value, BinaryOp::And, type, k))
			value = m->ops[0];
		Instruction cut = make_instruction(Opcode::Convert, field_type(width));
		cut.sub = static_cast<uint8_t>(ConvertOp::Trunc);
		cut.ops[0] = value;
		cut.location = store.location;
		set_convert_source_type(cut, type);
		value = Value::temp(body_.instructions[emit(block, prev, cut)].result);
	}
	const Value address = displaced(block, prev, store.ops[1], lo, store.location);
	Instruction& out = body_.instructions[id];
	out.type = field_type(width);
	out.flags &= static_cast<uint8_t>(~IF_TYPED);
	out.ops[0] = value;
	out.ops[1] = address;
	return true;
}

// The stores and the tests first (`loads` false): a narrowed store leaves
// the word's load, its mask and the merge without a use.
bool FieldAccess::run(bool loads)
{
	map_instruction_blocks(body_, block_of_);
	bool changed = false;
	if (!loads)
	{
		uses_.assign(body_.temps.size(), 0);
		for (BlockId b : body_.layout)
			for (InstId id = body_.blocks[b].first; id != kNoId; id = body_.instructions[id].next)
				for_each_operand(body_, body_.instructions[id], [&](const Value& v) {
					if (v.is(ValueKind::Temp))
						++uses_[v.index];
				});
	}
	for (BlockId b : body_.layout)
	{
		InstId prev = kNoId;
		for (InstId id = body_.blocks[b].first; id != kNoId; prev = id, id = body_.instructions[id].next)
		{
			const Instruction seen = body_.instructions[id];
			uint64_t zero;
			if (loads)
			{
				if (seen.op == Opcode::Binary)
					changed = field_load(id) || whole_shift(id) || changed;
				continue;
			}
			const bool equality = seen.op == Opcode::Compare && seen.type.is_integer() &&
				(static_cast<ComparePredicate>(seen.sub) == ComparePredicate::Eq ||
					static_cast<ComparePredicate>(seen.sub) == ComparePredicate::Ne) &&
				literal(seen.ops[1], 64, zero);
			const bool test = seen.op == Opcode::Branch || (equality && zero == 0);
			Value tested = seen.ops[0];
			if (test && tested.is(ValueKind::Temp) && shifted_test(b, prev, tested, seen.location))
			{
				body_.instructions[id].ops[0] = tested;
				changed = true;
			}
			else if (equality && zero != 0 && shifted_compare(b, prev, id))
				changed = true;
			else if (seen.op == Opcode::Store)
				changed = field_store(b, prev, id) || changed;
		}
	}
	return changed;
}

} // namespace

bool narrow_field_stores(FunctionContext& context)
{
	return FieldAccess(context).run(false);
}

bool narrow_field_loads(FunctionContext& context)
{
	return FieldAccess(context).run(true);
}

} // namespace opt
