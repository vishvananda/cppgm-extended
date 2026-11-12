#include <algorithm>
#include <string>

#include "lowir/builder.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

namespace opt {

using namespace lowir;

namespace {

// A bounded dense result table replaces dispatch, not an arbitrary CFG.
// Data growth is linear in the switch operands, with a small absolute cap.
const uint32_t kMinCases = 4, kMaxEntries = 512;

struct Result
{
	Type type;
	Value base = Value::none(); // pointer results share this dominating value
	uint64_t bits = 0;
};

struct Case
{
	uint64_t key;
	uint64_t order;
	Result result;
};

struct PhiIncoming
{
	uint32_t position;
	BlockId from;
	uint64_t key, bits;
};

struct PhiTable
{
	BlockId join;
	InstId phi;
	Type selector, result;
	const std::vector<PhiIncoming>& inputs;
	const std::vector<uint64_t>& values;
	bool powers, byte;
};

class ReturnTables
{
public:
	explicit ReturnTables(FunctionContext& context) : c_(context), p_(context.program), b_(context.body) {}
	bool run();
	bool run_phi();

private:
	bool returned(BlockId block, Result& result) const;
	bool available(Value value, BlockId block);
	bool candidate(BlockId block);
	bool phi_candidate(BlockId block);
	void rewrite_phi_table(const Instruction& term, const PhiTable& facts);
	SymbolId table(const std::vector<uint64_t>& values, Type type);
	void rewrite(BlockId block, const Instruction& term, Type selector, uint64_t first,
		const std::vector<uint64_t>& values, const Result& result);
	Value emit(BlockId block, InstId& after, Instruction inst, Type result);
	Value number(uint64_t bits) { return integer_value(b_, integer_literal(static_cast<int64_t>(bits))); }

	FunctionContext& c_;
	Program& p_;
	FunctionBody& b_;
};

bool ReturnTables::returned(BlockId block, Result& result) const
{
	const InstId last = b_.blocks[block].last;
	if (last == kNoId || b_.instructions[last].op != Opcode::Return)
		return false;
	const Instruction& ret = b_.instructions[last];
	if ((!ret.type.is_integer() || integer_bits(ret.type.kind()) > 64) && !ret.type.is_pointer())
		return false;
	for (InstId i = b_.blocks[block].first; i != last; i = b_.instructions[i].next)
	{
		const Opcode op = b_.instructions[i].op;
		if (op != Opcode::Const && op != Opcode::Copy && op != Opcode::Addr && op != Opcode::Index)
			return false;
	}
	result.type = ret.type;
	Value value = ret.ops[0];
	if (ret.type.is_integer())
	{
		if (!value.is(ValueKind::Integer))
			return false;
		const unsigned width = integer_bits(ret.type.kind());
		const uint64_t raw = normalize_integer(ret.type, b_.literals[value.index]).low;
		// Read a smaller signed element, then restore the return type's
		// bits. Unsigned results may share that compact representation.
		const uint64_t sign = uint64_t(1) << (width - 1);
		result.bits = width == 64 ? raw : ((raw & ((sign << 1) - 1)) ^ sign) - sign;
		return true;
	}
	for (unsigned steps = 0; steps < 8 && value.is(ValueKind::Temp) && value.index >= b_.parameter_count; ++steps)
	{
		const InstId id = b_.temps[value.index].def;
		if (id == kNoId)
			break;
		const Instruction& inst = b_.instructions[id];
		if (inst.op == Opcode::Addr || (inst.op == Opcode::Copy && inst.type.is_pointer()))
			value = inst.ops[0];
		else if (inst.op == Opcode::Index && inst.ops[1].is(ValueKind::Integer))
		{
			// LowIR pointer arithmetic has the target's wrapping word size.
			result.bits += b_.literals[inst.ops[1].index].low * storage_bytes(inst.type);
			value = inst.ops[0];
		}
		else
			break;
	}
	result.base = value;
	return value.is(ValueKind::Symbol) ||
		(value.is(ValueKind::Temp) &&
			(b_.temps[value.index].type.is_pointer() || b_.temps[value.index].type.kind() == TypeKind::I64));
}

bool ReturnTables::available(Value value, BlockId block)
{
	if (value.is(ValueKind::None) || value.is(ValueKind::Symbol) || value.index < b_.parameter_count)
		return true;
	const InstId def = b_.temps[value.index].def;
	if (def == kNoId)
		return false;
	c_.entry_block_of(form_return_tables);
	const BlockId origin = c_.block_of[def];
	return origin != kNoId && c_.ensure_cfg().dominates(origin, block);
}

SymbolId ReturnTables::table(const std::vector<uint64_t>& values, Type type)
{
	uint32_t& ordinal = *c_.return_table_names;
	NameId name;
	do
	{
		const std::string spelling = "__switch_results_" + std::to_string(ordinal++);
		name = p_.names.intern(spelling.data(), static_cast<uint32_t>(spelling.size()));
	} while (p_.find_symbol(name) != kNoId);
	const SymbolId symbol = p_.symbol_for(name);
	Global global = Global();
	const uint32_t bytes = static_cast<uint32_t>(storage_bytes(type));
	global.typed = true;
	global.type = Type::object(static_cast<uint32_t>(values.size()) * bytes,
		bytes == 1       ? 0
			: bytes == 2 ? 1
			: bytes == 4 ? 2
						 : 3);
	global.init = GlobalInit::Data;
	global.first_item = static_cast<uint32_t>(p_.data_items.size());
	global.item_count = static_cast<uint32_t>(values.size());
	for (uint64_t value : values)
	{
		DataItem item = DataItem();
		item.kind = DataItemKind::Scalar;
		item.type = type;
		item.constant.kind = ValueKind::Integer;
		item.constant.float_kind = TypeKind::Void;
		item.constant.bits = integer_literal(static_cast<int64_t>(value));
		item.address = kNoId;
		p_.data_items.push_back(item);
	}
	p_.declare_global(symbol, SymbolKind::GlobalDefinition, global);
	p_.symbols[symbol].metadata.binding = ir_model::SBM_INTERNAL;
	p_.symbols[symbol].metadata.storage = ir_model::GSM_READONLY;
	return symbol;
}

Value ReturnTables::emit(BlockId block, InstId& after, Instruction inst, Type result)
{
	const InstId id = append_instruction(b_, inst);
	const TempId temp = append_temp(b_, result, id);
	b_.instructions[id].result = temp;
	insert_after(b_, block, after, id);
	after = id;
	return Value::temp(temp);
}

void ReturnTables::rewrite(BlockId block, const Instruction& term, Type selector, uint64_t first,
	const std::vector<uint64_t>& values, const Result& result)
{
	TypeKind kind = TypeKind::I8;
	for (uint64_t bits : values)
	{
		const int64_t value = static_cast<int64_t>(bits);
		const TypeKind need = value >= -128 && value <= 127 ? TypeKind::I8
			: value >= -32768 && value <= 32767             ? TypeKind::I16
			: value >= INT32_MIN && value <= INT32_MAX      ? TypeKind::I32
															: TypeKind::I64;
		if (storage_bytes(Type::scalar(need)) > storage_bytes(Type::scalar(kind)))
			kind = need;
	}
	const Type element = Type::scalar(kind), word = Type::scalar(TypeKind::I64);
	const SymbolId data = table(values, element);
	const InstId terminal = b_.blocks[block].last;
	InstId after = previous_in_block(b_, block, terminal);
	Value index = term.ops[0];
	if (selector != word)
	{
		Instruction convert = make_instruction(
			integer_bits(selector.kind()) < 64 ? Opcode::Convert : Opcode::Copy, word);
		convert.ops[0] = index;
		if (convert.op == Opcode::Convert)
		{
			convert.sub = static_cast<uint8_t>(is_unsigned(selector.kind()) ? ConvertOp::ZExt : ConvertOp::SExt);
			set_convert_source_type(convert, selector);
		}
		convert.location = term.location;
		index = emit(block, after, convert, word);
	}
	if (first != 0)
	{
		Instruction sub = make_instruction(Opcode::Binary, word);
		sub.sub = static_cast<uint8_t>(BinaryOp::Sub);
		sub.ops[0] = index;
		sub.ops[1] = number(first);
		sub.location = term.location;
		index = emit(block, after, sub, word);
	}
	Instruction test = make_instruction(Opcode::Compare, word);
	test.sub = static_cast<uint8_t>(ComparePredicate::Ult);
	test.ops[0] = index;
	test.ops[1] = number(values.size());
	test.location = term.location;
	const Value inside = emit(block, after, test, word);
	const BlockId lookup = append_block(b_, BlockKind::SwitchCase);
	b_.layout.push_back(lookup);
	Instruction branch = make_instruction(Opcode::Branch, Type::scalar(TypeKind::Void));
	branch.ops[0] = inside;
	branch.ops[1] = Value::block(lookup);
	branch.ops[2] = term.ops[1];
	branch.location = term.location;
	b_.instructions[terminal] = branch;
	after = kNoId;
	Instruction address = make_instruction(Opcode::Index, element);
	address.ops[0] = Value::symbol(data);
	address.ops[1] = index;
	address.location = term.location;
	const Value at = emit(lookup, after, address, Type::scalar(TypeKind::Ptr));
	Instruction load = make_instruction(Opcode::Load, element);
	load.ops[0] = at;
	load.location = term.location;
	Value value = emit(lookup, after, load, element);
	const Type target = result.type.is_pointer() ? word : result.type;
	if (element != target)
	{
		Instruction convert = make_instruction(Opcode::Convert, target);
		const unsigned from = integer_bits(element.kind()), to = integer_bits(target.kind());
		convert.op = from == to ? Opcode::Copy : Opcode::Convert;
		convert.ops[0] = value;
		if (convert.op == Opcode::Convert)
		{
			convert.sub = static_cast<uint8_t>(from < to ? ConvertOp::SExt : ConvertOp::Trunc);
			set_convert_source_type(convert, element);
		}
		convert.location = term.location;
		value = emit(lookup, after, convert, target);
	}
	if (result.type.is_pointer())
	{
		Value base = result.base;
		if (base.is(ValueKind::Temp) && !b_.temps[base.index].type.is_pointer())
		{
			Instruction cast = make_instruction(Opcode::Copy, result.type);
			cast.ops[0] = base;
			cast.location = term.location;
			base = emit(lookup, after, cast, result.type);
		}
		Instruction step = make_instruction(Opcode::Index, Type::scalar(TypeKind::I8));
		step.ops[0] = base;
		step.ops[1] = value;
		step.location = term.location;
		value = emit(lookup, after, step, result.type);
	}
	Instruction ret = make_instruction(Opcode::Return, result.type);
	ret.ops[0] = value;
	ret.location = term.location;
	insert_after(b_, lookup, after, append_instruction(b_, ret));
	c_.invalidate_cfg();
	c_.invalidate_block_of();
}

bool ReturnTables::candidate(BlockId block)
{
	const Instruction term = b_.instructions[b_.blocks[block].last];
	if (term.op != Opcode::Switch || term.extra_count < 2 * kMinCases || term.extra_count > 2 * kMaxEntries)
		return false;
	const Type selector = term.type.is_void() ? Type::scalar(TypeKind::I64) : term.type;
	if (!selector.is_integer() || integer_bits(selector.kind()) > 64)
		return false;
	std::vector<Case> cases;
	cases.reserve(term.extra_count / 2);
	bool varies = false;
	const uint64_t bias = is_unsigned(selector.kind()) ? 0 : uint64_t(1) << 63;
	for (uint32_t i = 0; i < term.extra_count; i += 2)
	{
		Result result;
		if (!returned(b_.operands[term.extra_first + i + 1].index, result))
			return false;
		if (!cases.empty() && (result.type != cases[0].result.type || !(result.base == cases[0].result.base)))
			return false;
		varies = varies || (!cases.empty() && result.bits != cases[0].result.bits);
		const uint64_t key = normalize_integer(selector, b_.literals[b_.operands[term.extra_first + i].index]).low;
		cases.push_back(Case{ key, key ^ bias, result });
	}
	// One common result needs a membership/range decision, not a data load.
	if (!varies)
		return false;
	std::sort(cases.begin(), cases.end(), [](const Case& a, const Case& b) { return a.order < b.order; });
	const uint64_t width = cases.back().key - cases.front().key;
	if (width >= kMaxEntries || width + 1 > cases.size() * 4 || !available(cases[0].result.base, block))
		return false;
	for (size_t i = 1; i < cases.size(); ++i)
		if (cases[i - 1].key == cases[i].key)
			return false;
	Result fallback;
	uint64_t fill = cases[0].result.bits;
	if (width + 1 != cases.size())
	{
		const BlockId otherwise = term.ops[1].index;
		if (returned(otherwise, fallback) && fallback.type == cases[0].result.type &&
			fallback.base == cases[0].result.base)
			fill = fallback.bits;
		else if (b_.blocks[otherwise].first != b_.blocks[otherwise].last ||
			b_.instructions[b_.blocks[otherwise].last].op != Opcode::Unreachable)
			return false;
	}
	std::vector<uint64_t> values(static_cast<size_t>(width + 1), fill);
	for (const Case& entry : cases)
		values[static_cast<size_t>(entry.key - cases.front().key)] = entry.result.bits;
	rewrite(block, term, selector, cases.front().key, values, cases[0].result);
	++c_.stats.folded;
	return true;
}

bool ReturnTables::phi_candidate(BlockId block)
{
	const InstId terminal = b_.blocks[block].last;
	if (terminal == kNoId)
		return false;
	const Instruction term = b_.instructions[terminal];
	if (term.op != Opcode::Switch || term.extra_count < 8 || term.extra_count > 1024)
		return false;
	const Type selector = term.type.is_void() ? Type::scalar(TypeKind::I64) : term.type;
	if (!selector.is_integer() || integer_bits(selector.kind()) > 64)
		return false;
	c_.ensure_cfg_light();
	BlockId join = kNoId;
	InstId phi = kNoId;
	Type result;
	std::vector<PhiIncoming> inputs;
	const uint64_t bias = is_unsigned(selector.kind()) ? 0 : uint64_t(1) << 63;
	for (uint32_t k = 0; k < term.extra_count; k += 2)
	{
		const BlockId target = b_.operands[term.extra_first + k + 1].index;
		if (target == block || target == term.ops[1].index || c_.cfg.pred_count(target) != 1 ||
			c_.cfg.preds[c_.cfg.pred_first[target]] != block ||
			(c_.cfg.flags[target] & (BF_ADDRESS | BF_HANDLER | BF_EH)))
			continue;
		const Block& arm = b_.blocks[target];
		if (arm.first == kNoId || arm.first != arm.last || b_.instructions[arm.last].op != Opcode::Jump)
			continue;
		const BlockId merge = b_.instructions[arm.last].ops[0].index;
		const InstId first = b_.blocks[merge].first;
		if (merge == block || merge == target || first == kNoId || b_.instructions[first].op != Opcode::Phi ||
			(b_.instructions[first].next != kNoId && b_.instructions[b_.instructions[first].next].op == Opcode::Phi) ||
			(join != kNoId && merge != join))
			continue;
		const Instruction& incoming = b_.instructions[first];
		if (!incoming.type.is_integer() || incoming.type.kind() == TypeKind::I128)
			continue;
		const Value value = phi_incoming(b_, incoming, target);
		if (!value.is(ValueKind::Integer) || !b_.operands[term.extra_first + k].is(ValueKind::Integer))
			continue;
		join = merge;
		phi = first;
		result = incoming.type;
		inputs.push_back(PhiIncoming{ k, target,
			normalize_integer(selector, b_.literals[b_.operands[term.extra_first + k].index]).low,
			normalize_integer(result, b_.literals[value.index]).low });
	}
	if (inputs.size() < 4)
		return false;
	std::sort(inputs.begin(), inputs.end(),
		[&](const PhiIncoming& a, const PhiIncoming& b) { return (a.key ^ bias) < (b.key ^ bias); });
	const uint64_t first = inputs.front().key, span = inputs.back().key - first;
	if (span >= 512 || span + 1 > inputs.size() * 2)
		return false;
	// LowIR switches choose their first matching arm. Reject repeated keys
	// in the candidate window, including arms that are not table candidates.
	// The accepted span has at most 512 entries, so the bitmap is bounded.
	uint64_t seen[8] = {};
	for (uint32_t k = 0; k < term.extra_count; k += 2)
	{
		const Value key = b_.operands[term.extra_first + k];
		if (!key.is(ValueKind::Integer))
			return false;
		const uint64_t delta = normalize_integer(selector, b_.literals[key.index]).low - first;
		if (delta > span)
			continue;
		const uint64_t bit = uint64_t(1) << (delta & 63);
		if (seen[delta / 64] & bit)
			return false;
		seen[delta / 64] |= bit;
	}
	bool varies = false, byte = true, powers = true;
	for (const PhiIncoming& in : inputs)
	{
		varies = varies || in.bits != inputs.front().bits;
		byte = byte && in.bits <= 127;
		powers = powers && in.bits != 0 && (in.bits & (in.bits - 1)) == 0;
	}
	if (!varies)
		return false;
	std::vector<uint64_t> values(static_cast<size_t>(span + 1), 0);
	for (const PhiIncoming& in : inputs)
		values[static_cast<size_t>(in.key - first)] = powers ? static_cast<uint64_t>(__builtin_ctzll(in.bits))
															 : in.bits;
	const PhiTable facts{ join, phi, selector, result, inputs, values, powers, byte };
	rewrite_phi_table(term, facts);
	++c_.stats.folded;
	return true;
}

void ReturnTables::rewrite_phi_table(const Instruction& term, const PhiTable& facts)
{
	const BlockId join = facts.join;
	const InstId phi = facts.phi;
	const Type selector = facts.selector, result = facts.result;
	const std::vector<PhiIncoming>& inputs = facts.inputs;
	const std::vector<uint64_t>& values = facts.values;
	const bool powers = facts.powers, byte = facts.byte;
	const uint64_t first = inputs.front().key;
	const Type element = byte || powers ? Type::scalar(TypeKind::I8) : result;
	const SymbolId data = table(values, element);
	const BlockId lookup = append_block(b_, BlockKind::SwitchCase);
	b_.layout.push_back(lookup);
	InstId after = kNoId;
	const Type word = Type::scalar(TypeKind::I64);
	Value index = term.ops[0];
	if (selector != word)
	{
		Instruction cast = make_instruction(integer_bits(selector.kind()) == 64 ? Opcode::Copy : Opcode::Convert, word);
		cast.ops[0] = index;
		if (cast.op == Opcode::Convert)
		{
			cast.sub = static_cast<uint8_t>(is_unsigned(selector.kind()) ? ConvertOp::ZExt : ConvertOp::SExt);
			set_convert_source_type(cast, selector);
		}
		index = emit(lookup, after, cast, word);
	}
	if (first != 0)
	{
		Instruction sub = make_instruction(Opcode::Binary, word);
		sub.sub = static_cast<uint8_t>(BinaryOp::Sub);
		sub.ops[0] = index;
		sub.ops[1] = number(first);
		index = emit(lookup, after, sub, word);
	}
	Instruction address = make_instruction(Opcode::Index, element);
	address.ops[0] = Value::symbol(data);
	address.ops[1] = index;
	const Value at = emit(lookup, after, address, Type::scalar(TypeKind::Ptr));
	Instruction load = make_instruction(Opcode::Load, element);
	load.ops[0] = at;
	Value value = emit(lookup, after, load, element);
	if (element != result)
	{
		Instruction cast = make_instruction(Opcode::Convert, result);
		cast.ops[0] = value;
		cast.sub = static_cast<uint8_t>(integer_bits(result.kind()) > 8 ? ConvertOp::SExt : ConvertOp::Trunc);
		set_convert_source_type(cast, element);
		value = emit(lookup, after, cast, result);
	}
	if (powers)
	{
		Instruction shift = make_instruction(Opcode::Binary, result);
		shift.sub = static_cast<uint8_t>(BinaryOp::Shl);
		shift.ops[0] = number(1);
		shift.ops[1] = value;
		value = emit(lookup, after, shift, result);
	}
	Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
	jump.ops[0] = Value::block(join);
	insert_after(b_, lookup, after, append_instruction(b_, jump));
	// Each arm the table replaces ends unreachable (the cleanup drops it): the
	// phi no longer names it, so it must not still jump to the join.
	for (const PhiIncoming& in : inputs)
	{
		b_.operands[term.extra_first + in.position + 1] = Value::block(lookup);
		remove_phi_incoming(b_, join, in.from);
		Instruction& end = b_.instructions[b_.blocks[in.from].last];
		end.op = Opcode::Unreachable;
		end.ops[0] = Value::none();
	}
	const Instruction old_phi = b_.instructions[phi];
	std::vector<Value> pairs(
		b_.operands.begin() + old_phi.extra_first, b_.operands.begin() + old_phi.extra_first + old_phi.extra_count);
	pairs.push_back(Value::block(lookup));
	pairs.push_back(value);
	b_.instructions[phi].extra_first = append_operands(b_, pairs.data(), static_cast<uint32_t>(pairs.size()));
	b_.instructions[phi].extra_count = static_cast<uint32_t>(pairs.size());
	c_.invalidate_cfg();
	c_.invalidate_block_of();
}

bool ReturnTables::run_phi()
{
	bool changed = false;
	const size_t count = b_.layout.size();
	for (size_t i = 0; i < count; ++i)
		changed = phi_candidate(b_.layout[i]) || changed;
	return changed;
}

bool ReturnTables::run()
{
	bool changed = false;
	const size_t count = b_.layout.size();
	for (size_t i = 0; i < count; ++i)
	{
		const BlockId block = b_.layout[i];
		const InstId last = b_.blocks[block].last;
		if (last != kNoId && b_.instructions[last].op == Opcode::Switch)
			changed = candidate(block) || changed;
	}
	return changed;
}

} // namespace

bool form_phi_result_tables(FunctionContext& context)
{
	if (context.return_table_names == nullptr)
		return false;
	ReturnTables pass(context);
	return pass.run_phi();
}

bool form_return_tables(FunctionContext& context)
{
	if (context.return_table_names == nullptr)
		return false;
	ReturnTables pass(context);
	return pass.run();
}

} // namespace opt
