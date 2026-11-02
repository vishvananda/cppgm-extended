#include <algorithm>

#include "lowir/literals.h"
#include "opt/function_ir.h"
#include "opt/interprocedural.h"
#include "opt/scalar_fold.h"

namespace opt {

using namespace lowir;

namespace {

// What the interpretation knows of a value: a literal, or a symbol's
// address at a byte offset.
struct Known
{
	enum Kind : uint8_t
	{
		Unknown,
		Literal,
		Address
	} kind;
	Constant c;
	SymbolId symbol;
	int64_t offset;
};

// A store of a known value at a known place of a candidate global, made
// before `epoch`'s next unknown write (a call, a store through an unknown
// address): a later load reads it while the epoch lasts.
struct Stored
{
	SymbolId symbol;
	int64_t offset;
	Type type;
	Known value;
	InstId inst;
	uint32_t epoch;
};

const uint32_t kMaxItems = 4096;

class StaticInit
{
public:
	StaticInit(Program& program, SymbolId function)
		: program_(program), function_(function), body_(*program.functions[program.symbols[function].detail].body)
	{}
	bool run(std::vector<uint8_t>& unwritten, std::vector<uint8_t>* moved);

private:
	bool candidate(SymbolId s) const;
	Known value(Value v) const;
	Known load(const Instruction& inst) const;
	Known fold(const Instruction& inst) const;
	void store(InstId id, const Instruction& inst);
	bool storable(Type type, Known& v) const;
	bool convert(SymbolId s, std::vector<const Stored*>& mine);
	uint64_t span(const Global& g) const;

	Program& program_;
	SymbolId function_;
	FunctionBody& body_;
	std::vector<Known> temps_;
	std::vector<Stored> stores_;
	uint32_t epoch_ = 0;
};

// An internal, zero-initialized, writable global no other definition can
// replace: its zero bytes are what the dynamic initialization starts from.
bool StaticInit::candidate(SymbolId s) const
{
	const Symbol& symbol = program_.symbols[s];
	if (symbol.kind != SymbolKind::GlobalDefinition || symbol.metadata.binding != ir_model::SBM_INTERNAL ||
		symbol.metadata.storage != ir_model::GSM_DEFAULT || (symbol.metadata.flags & SF_REPLACEABLE) != 0 ||
		symbol.metadata.section != 0)
		return false;
	const Global& g = program_.globals[symbol.detail];
	if (g.init == GlobalInit::Zero)
		return g.typed;
	if (g.init != GlobalInit::Data || g.item_count > kMaxItems)
		return false;
	for (uint32_t k = 0; k < g.item_count; ++k)
		if (program_.data_items[g.first_item + k].kind != DataItemKind::Zero)
			return false;
	return true;
}

uint64_t StaticInit::span(const Global& g) const
{
	if (g.typed)
		return storage_bytes(g.type);
	uint64_t bytes = 0;
	for (uint32_t k = 0; k < g.item_count; ++k)
		bytes += static_cast<uint64_t>(program_.data_items[g.first_item + k].amount);
	return bytes;
}

Known StaticInit::value(Value v) const
{
	Known k = Known();
	k.kind = Known::Unknown;
	if (literal_constant(body_, v, k.c))
		k.kind = Known::Literal;
	else if (v.is(ValueKind::Symbol))
	{
		k.kind = Known::Address;
		k.symbol = v.index;
		k.offset = 0;
	}
	else if (v.is(ValueKind::Temp) && v.index < temps_.size())
		k = temps_[v.index];
	return k;
}

// A load of bytes this run stored (the same place and width, in the
// current epoch), else of readonly data.
Known StaticInit::load(const Instruction& inst) const
{
	Known out = Known();
	out.kind = Known::Unknown;
	const Known at = value(inst.ops[0]);
	if ((inst.flags & IF_VOLATILE) != 0 || at.kind != Known::Address)
		return out;
	const uint64_t size = storage_bytes(inst.type);
	for (size_t k = stores_.size(); k-- > 0;)
	{
		const Stored& s = stores_[k];
		const uint64_t bytes = storage_bytes(s.type);
		if (s.symbol != at.symbol || s.offset + static_cast<int64_t>(bytes) <= at.offset ||
			at.offset + static_cast<int64_t>(size) <= s.offset)
			continue;
		if (s.epoch == epoch_ && s.offset == at.offset && s.type == inst.type)
			out = s.value;
		return out;
	}
	if (inst.type.is_pointer())
	{
		SymbolId target = kNoId;
		int64_t addend = 0;
		if (read_readonly_address(program_, at.symbol, at.offset, nullptr, target, &addend))
		{
			out.kind = Known::Address;
			out.symbol = target;
			out.offset = addend;
		}
	}
	else if (read_readonly_scalar(program_, at.symbol, at.offset, inst.type, out.c))
		out.kind = Known::Literal;
	return out;
}

Known StaticInit::fold(const Instruction& inst) const
{
	Known out = Known();
	out.kind = Known::Unknown;
	const Known a = value(inst.ops[0]);
	switch (inst.op)
	{
	case Opcode::Addr:
		if (a.kind == Known::Address)
			out = a;
		return out;
	case Opcode::Index:
	{
		const Known b = value(inst.ops[1]);
		if (a.kind == Known::Address && b.kind == Known::Literal && b.c.kind == ValueKind::Integer)
		{
			out = a;
			out.offset += static_cast<int64_t>(b.c.bits.low) * static_cast<int64_t>(storage_bytes(inst.type));
		}
		return out;
	}
	case Opcode::Copy:
		if (a.kind == Known::Address && inst.type.is_pointer())
			out = a;
		else if (a.kind == Known::Literal && fold_copy(inst.type, a.c, out.c))
			out.kind = Known::Literal;
		return out;
	case Opcode::Load:
		return load(inst);
	default:
		break;
	}
	if (a.kind != Known::Literal)
		return out;
	const Known b = value(inst.ops[1]);
	bool folded = false;
	if (inst.op == Opcode::Unary)
		folded = fold_unary(static_cast<UnaryOp>(inst.sub), inst.type, a.c, out.c);
	else if (inst.op == Opcode::Convert)
		folded = fold_convert(static_cast<ConvertOp>(inst.sub), inst.type, convert_source_type(inst), a.c, out.c);
	else if (inst.op == Opcode::Binary && b.kind == Known::Literal)
		folded = fold_binary(static_cast<BinaryOp>(inst.sub), inst.type, a.c, b.c, out.c);
	else if (inst.op == Opcode::Compare && b.kind == Known::Literal)
		folded = fold_compare(static_cast<ComparePredicate>(inst.sub), inst.type, a.c, b.c, out.c);
	if (folded)
		out.kind = Known::Literal;
	return out;
}

// The value as data of `type`: an integer of its width, a float of its
// own format (an integer literal's value converted), or an address.
bool StaticInit::storable(Type type, Known& v) const
{
	const TypeKind kind = type.kind();
	if (v.kind == Known::Address)
		return type.is_pointer();
	if (v.kind != Known::Literal)
		return false;
	if (kind == TypeKind::F32 || kind == TypeKind::F64)
	{
		if (v.c.kind == ValueKind::Integer)
		{
			v.c.bits = integer_as_float(kind, v.c.bits);
			v.c.kind = ValueKind::Float;
			v.c.float_kind = kind;
		}
		return v.c.kind == ValueKind::Float && v.c.float_kind == kind;
	}
	if (v.c.kind != ValueKind::Integer || !(type.is_integer() || type.is_pointer()) || kind == TypeKind::I1 ||
		kind == TypeKind::I128)
		return false;
	v.c.bits = normalize_integer(type, v.c.bits);
	return true;
}

void StaticInit::store(InstId id, const Instruction& inst)
{
	const Known at = value(inst.ops[1]);
	if (at.kind != Known::Address)
	{
		++epoch_; // any memory may have changed
		return;
	}
	Known v = value(inst.ops[0]);
	if ((inst.flags & IF_VOLATILE) == 0 && candidate(at.symbol) && storable(inst.type, v))
		stores_.push_back(Stored{ at.symbol, at.offset, inst.type, v, id, epoch_ });
	else
	{
		// Bytes of unknown value: later loads there read nothing known,
		// and the global stays dynamic.
		Stored unknown = Stored{ at.symbol, at.offset, inst.type, Known(), kNoId, epoch_ };
		unknown.value.kind = Known::Unknown;
		stores_.push_back(unknown);
	}
}

// Writes `mine` (one global's stores, in program order) as its data: the
// last store of each place wins; partial overlaps, misaligned places and
// places past the object leave it dynamic.
bool StaticInit::convert(SymbolId s, std::vector<const Stored*>& mine)
{
	Global& g = program_.globals[program_.symbols[s].detail];
	const uint64_t bytes = span(g);
	std::stable_sort(mine.begin(), mine.end(), [](const Stored* a, const Stored* b) { return a->offset < b->offset; });
	std::vector<const Stored*> last;
	for (const Stored* st : mine)
	{
		const uint64_t size = storage_bytes(st->type);
		const uint64_t align = size >= 16 ? 16 : size;
		if (st->inst == kNoId || st->offset < 0 || size == 0 || static_cast<uint64_t>(st->offset) % align != 0 ||
			static_cast<uint64_t>(st->offset) + size > bytes)
			return false;
		if (!last.empty() && last.back()->offset == st->offset && storage_bytes(last.back()->type) == size)
			last.back() = st; // a later store of the same place
		else if (!last.empty() &&
			static_cast<uint64_t>(last.back()->offset) + storage_bytes(last.back()->type) >
				static_cast<uint64_t>(st->offset))
			return false;
		else
			last.push_back(st);
	}
	auto zero_value = [](const Stored* st) {
		return st->value.kind == Known::Literal && st->value.c.bits.low == 0 && st->value.c.bits.high == 0;
	};
	if (!g.typed || g.type.is_object())
	{
		const uint32_t first = static_cast<uint32_t>(program_.data_items.size());
		uint64_t at = 0;
		auto zero = [&](uint64_t to) {
			if (to <= at)
				return;
			DataItem item = DataItem();
			item.kind = DataItemKind::Zero;
			item.type = Type::scalar(TypeKind::Void);
			item.address = kNoId;
			item.amount = static_cast<int64_t>(to - at);
			program_.data_items.push_back(item);
			at = to;
		};
		for (const Stored* st : last)
		{
			if (zero_value(st))
				continue;
			zero(static_cast<uint64_t>(st->offset));
			DataItem item = DataItem();
			item.type = st->type;
			item.address = kNoId;
			if (st->value.kind == Known::Address)
			{
				item.kind = DataItemKind::Address;
				item.address = st->value.symbol;
				item.amount = st->value.offset;
			}
			else
			{
				item.kind = DataItemKind::Scalar;
				item.constant = st->value.c;
			}
			program_.data_items.push_back(item);
			at += storage_bytes(st->type);
		}
		zero(bytes);
		g.init = GlobalInit::Data;
		g.first_item = first;
		g.item_count = static_cast<uint32_t>(program_.data_items.size()) - first;
		return true;
	}
	// A scalar global: one store of its whole width and type.
	if (last.size() != 1 || last[0]->offset != 0 || storage_bytes(last[0]->type) != bytes ||
		(last[0]->type != g.type && !(last[0]->type.is_pointer() && g.type.is_pointer())))
		return false;
	const Stored* st = last[0];
	if (zero_value(st))
		return true;
	if (st->value.kind == Known::Address)
	{
		g.init = GlobalInit::Address;
		g.address = st->value.symbol;
		g.addend = st->value.offset;
	}
	else
	{
		g.init = GlobalInit::Constant;
		g.constant = st->value.c;
	}
	return true;
}

bool StaticInit::run(std::vector<uint8_t>& unwritten, std::vector<uint8_t>* moved)
{
	if (body_.layout.empty())
		return false;
	temps_.assign(body_.temps.size(), Known());
	const BlockId entry = body_.layout[0];
	for (InstId i = body_.blocks[entry].first; i != kNoId; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		if (inst.op == Opcode::Store)
			store(i, inst);
		else if (may_write_memory(program_, inst))
			++epoch_;
		if (inst.result != kNoId && inst.result < temps_.size())
			temps_[inst.result] = fold(inst);
	}
	if (stores_.empty())
		return false;
	// Each global's stores; one this run could not record, or any other
	// write of the global anywhere (find_unwritten_globals, the recorded
	// stores left out), leaves it dynamic. So does a recorded store of its
	// address, which the scan does not see: a pointer loaded from where it
	// went may write it.
	std::vector<uint8_t> skip(body_.instructions.size(), 0), bad(program_.symbols.size(), 0);
	for (const Stored& st : stores_)
	{
		if (st.inst == kNoId)
			bad[st.symbol] = 1;
		else
			skip[st.inst] = 1;
	}
	find_unwritten_globals(program_, unwritten, function_, &skip);
	for (const Stored& st : stores_)
		if (st.inst != kNoId && st.value.kind == Known::Address)
			unwritten[st.value.symbol] = 0;
	std::vector<const Stored*> order;
	for (const Stored& st : stores_)
		if (bad[st.symbol] == 0 && unwritten[st.symbol] != 0)
			order.push_back(&st);
	std::stable_sort(
		order.begin(), order.end(), [](const Stored* a, const Stored* b) { return a->symbol < b->symbol; });
	std::vector<const Stored*> mine;
	std::vector<uint8_t> removed(body_.instructions.size(), 0);
	bool changed = false;
	for (size_t k = 0; k < order.size();)
	{
		size_t end = k;
		while (end < order.size() && order[end]->symbol == order[k]->symbol)
			++end;
		mine.assign(order.begin() + static_cast<std::ptrdiff_t>(k), order.begin() + static_cast<std::ptrdiff_t>(end));
		const SymbolId symbol = order[k]->symbol;
		k = end;
		if (mine.size() > kMaxItems || !convert(symbol, mine))
		{
			unwritten[symbol] = 0; // its stores stay
			continue;
		}
		for (const Stored* m : mine)
			removed[m->inst] = 1;
		if (moved != nullptr)
			(*moved)[symbol] = 1;
		changed = true;
	}
	if (!changed)
		return false;
	InstId prev = kNoId;
	for (InstId i = body_.blocks[entry].first; i != kNoId;)
	{
		const InstId next = body_.instructions[i].next;
		if (removed[i] != 0)
			unlink_after(body_, entry, prev, i);
		else
			prev = i;
		i = next;
	}
	return true;
}

} // namespace

bool initialize_statically(Program& program, std::vector<uint8_t>& unwritten, std::vector<uint8_t>* moved)
{
	if (moved != nullptr)
		moved->assign(program.symbols.size(), 0);
	bool changed = false;
	for (SymbolId s = 0; s < program.symbols.size(); ++s)
	{
		const Symbol& symbol = program.symbols[s];
		if (symbol.kind == SymbolKind::FunctionDefinition && symbol.metadata.role == ir_model::SR_INIT &&
			program.functions[symbol.detail].body != nullptr)
		{
			StaticInit pass(program, s);
			changed = pass.run(unwritten, moved);
			break;
		}
	}
	// No store moved left `unwritten` as it was, or (on the first call) empty.
	if (unwritten.size() != program.symbols.size())
		find_unwritten_globals(program, unwritten);
	return changed;
}

} // namespace opt
