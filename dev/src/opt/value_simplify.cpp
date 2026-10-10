#include "lowir/builder.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"
#include "support/flat_map.h"

namespace opt {

using namespace lowir;

namespace {

// An operand of a hashed expression: a temporary, symbol or slot by id, a
// literal by value.
struct OperandKey
{
	uint8_t kind;
	uint8_t float_kind;
	uint64_t low;
	uint64_t high;

	bool operator==(const OperandKey& o) const
	{
		return kind == o.kind && float_kind == o.float_kind && low == o.low && high == o.high;
	}
};

struct ExprKey
{
	uint8_t op;
	uint8_t sub;
	uint64_t type;
	uint64_t immediate;
	OperandKey a;
	OperandKey b;
	OperandKey c; // a select's third operand

	bool operator==(const ExprKey& o) const
	{
		return op == o.op && sub == o.sub && type == o.type && immediate == o.immediate && a == o.a && b == o.b &&
			c == o.c;
	}
};

struct ExprHash
{
	size_t operator()(const ExprKey& k) const
	{
		// (Independent products: the map's finalizer mixes the sum.)
		const uint64_t kinds = static_cast<uint64_t>(k.op) | static_cast<uint64_t>(k.sub) << 8 |
			static_cast<uint64_t>(k.a.kind) << 16 | static_cast<uint64_t>(k.b.kind) << 24 |
			static_cast<uint64_t>(k.c.kind) << 32;
		uint64_t h = kinds * 0x9e3779b97f4a7c15ull;
		h ^= k.type * 0xc2b2ae3d27d4eb4full;
		h ^= (k.a.low + k.immediate * 31) * 0x165667b19e3779f9ull;
		h ^= k.b.low * 0xd6e8feb86659fd93ull;
		h ^= (k.a.high ^ (k.b.high << 1) ^ (k.c.low << 2)) * 0xff51afd7ed558ccdull;
		return static_cast<size_t>(h ^ (h >> 32));
	}
};

uint64_t type_bits(Type type)
{
	uint64_t bits;
	static_assert(sizeof bits == sizeof type, "Type is 8 bytes");
	__builtin_memcpy(&bits, &type, sizeof bits);
	return bits;
}

bool is_commutative(const Instruction& inst)
{
	if (inst.op == Opcode::Binary && inst.type.is_integer())
	{
		switch (static_cast<BinaryOp>(inst.sub))
		{
		case BinaryOp::Add:
		case BinaryOp::Mul:
		case BinaryOp::And:
		case BinaryOp::Or:
		case BinaryOp::Xor:
			return true;
		default:
			return false;
		}
	}
	return inst.op == Opcode::Compare &&
		(static_cast<ComparePredicate>(inst.sub) == ComparePredicate::Eq ||
			static_cast<ComparePredicate>(inst.sub) == ComparePredicate::Ne);
}

// The operand type of a scalar instruction's operands.
Type operand_type(const Instruction& inst)
{
	if (inst.op == Opcode::Convert)
		return convert_source_type(inst);
	if (inst.op == Opcode::Index)
		return Type::scalar(TypeKind::I64);
	return inst.type;
}

class Simplifier
{
public:
	explicit Simplifier(FunctionContext& context) : c_(context), body_(context.body), changed_(false) {}

	bool run();
	bool edited() const { return edited_; } // substitute changed an operand
	// The containers, kept across runs for their capacity (run() clears or
	// assigns each; same_phi's stamps keep rising across runs).
	struct Pool;
	void exchange(Pool& pool);

private:
	Value resolve(Value v);
	bool constant(Value v, Type type, Constant& out);
	Value literal(const Constant& c) { return literal_value(body_, c.kind, c.float_kind, c.bits); }
	OperandKey key_of(Value v, Type type);
	bool boolean(Value v, unsigned depth = 0);
	const Instruction* producer(Value v) const
	{
		return v.is(ValueKind::Temp) && v.index >= body_.parameter_count && body_.temps[v.index].def != kNoId
			? &body_.instructions[body_.temps[v.index].def]
			: nullptr;
	}
	void substitute(Instruction& inst);
	bool fold(const Instruction& inst, Value& out);
	bool identity(Instruction& inst, Value& out);
	bool readonly_load(const Instruction& inst, Value& out);
	bool known_strlen(const Instruction& inst, Value& out);
	bool merge_index(Instruction& inst);
	bool readonly_table_address(Instruction& inst);
	bool readonly_base(Value address, Value& base, int64_t& offset);
	bool binary_identity(Instruction& inst, Value& out);
	bool compare_identity(Instruction& inst, Value& out);
	bool convert_identity(Instruction& inst, Value& out);
	bool reassociate(Instruction& inst);
	uint64_t possible_bits(Value v, Type type, unsigned depth = 0);
	bool mask_or(Instruction& inst);
	void canonicalize(Instruction& inst);
	bool narrow_compare(Instruction& inst);
	bool bulk_call(Instruction& inst);
	bool equality_memcmp(BlockId block, InstId& prev, InstId id);
	void simplify_branch(Instruction& term);
	bool phi_value(const Instruction& phi, Value& out);
	void process_block(BlockId block);
	uint64_t phi_signature(const Instruction& phi) const;
	bool same_phi(const Instruction& a, const Instruction& b);
	std::vector<Value> pred_input_; // same_phi's scratch, by block
	std::vector<uint32_t> pred_stamp_;
	uint32_t stamp_ = 0;
	void finish(bool full);

	FunctionContext& c_;
	FunctionBody& body_;
	bool changed_;
	BlockId current_block_ = kNoId;
	std::vector<Value> repl_;
	std::vector<uint32_t> uses_;  // per temp at the pass start (a retyped load: none)
	bool edited_ = false;         // substitute changed an operand (reported or not)
	std::vector<uint8_t> walked_; // per block: process_block ran on it
	// Expression -> (temporary, its block): an entry is reusable only where
	// its block dominates, so a sibling subtree's entries need no removal
	// (they are replaced on the next insertion).
	struct Available
	{
		TempId temp;
		BlockId block;
		uint32_t ordinal; // its instruction's position in the block
	};
	FlatMap<ExprKey, Available, ExprHash> table_;
	// One block's phis by signature: (first, last) chain node, and per
	// node (phi, next node).
	struct SignatureHash
	{
		size_t operator()(uint64_t k) const { return static_cast<size_t>(k ^ (k >> 29)); }
	};
	struct Chain
	{
		uint32_t first, last;
	};
	FlatMap<uint64_t, Chain, SignatureHash> phi_heads_;
	std::vector<std::pair<InstId, uint32_t>> phi_chain_;
	// Handler entry: a handler block's dominator is its opener's block, but
	// the handler starts at a throw point after the opener, so a block a
	// handler reaches (its code, and what follows a catch) may not have run
	// what an opener's block computes after its first opener. There an
	// entry of such a block is available only from before that opener.
	std::vector<uint32_t> ordinal_;        // per instruction
	std::vector<uint32_t> first_opener_;   // per block: its first opener's ordinal (kNoId: none)
	std::vector<uint8_t> handler_reached_; // per block
	void index_openers();
	// Whether the value `temp` (defined in a block dominating `block`) has
	// run on every path into `block`.
	bool available_in(TempId temp, BlockId block) const;
	// Per temporary: proved nonzero on every path into the current block
	// (by a branch edge into a block whose only predecessor tested it).
	std::vector<uint8_t> nonzero_;
	std::vector<TempId> nonzero_undo_;
	// Per temporary: the literal a branch edge proved it equals on every
	// path into the current block (none outside such a region).
	std::vector<Value> equal_;
	std::vector<TempId> equal_undo_;
	void edge_facts(BlockId block);
	bool edge_equality(BlockId from, BlockId to, Value& x, Value& y) const;
	bool select_identity(const Instruction& phi, BlockId block, Value& out) const;
	bool merge_diamonds();
	bool known_nonzero(Value v) const
	{
		return v.is(ValueKind::Temp) && v.index < nonzero_.size() && nonzero_[v.index] != 0;
	}
};

struct Simplifier::Pool
{
	std::vector<Value> pred_input, repl, equal;
	std::vector<uint32_t> pred_stamp, uses, ordinal, first_opener;
	uint32_t stamp = 0;
	FlatMap<ExprKey, Available, ExprHash> table;
	FlatMap<uint64_t, Chain, SignatureHash> phi_heads;
	std::vector<std::pair<InstId, uint32_t>> phi_chain;
	std::vector<uint8_t> handler_reached, nonzero, walked;
	std::vector<TempId> nonzero_undo, equal_undo;
};

void Simplifier::exchange(Pool& pool)
{
	pred_input_.swap(pool.pred_input);
	repl_.swap(pool.repl);
	equal_.swap(pool.equal);
	pred_stamp_.swap(pool.pred_stamp);
	uses_.swap(pool.uses);
	ordinal_.swap(pool.ordinal);
	first_opener_.swap(pool.first_opener);
	std::swap(stamp_, pool.stamp);
	table_.swap(pool.table);
	phi_heads_.swap(pool.phi_heads);
	phi_chain_.swap(pool.phi_chain);
	handler_reached_.swap(pool.handler_reached);
	nonzero_.swap(pool.nonzero);
	walked_.swap(pool.walked);
	nonzero_undo_.swap(pool.nonzero_undo);
	equal_undo_.swap(pool.equal_undo);
}

// Instruction ordinals within their blocks, each block's first opener,
// and the blocks handlers reach (ordinary and handler edges).
// O(instructions + edges).
void Simplifier::index_openers()
{
	const Cfg& cfg = c_.cfg;
	ordinal_.assign(body_.instructions.size(), 0);
	first_opener_.assign(cfg.block_count, kNoId);
	handler_reached_.assign(cfg.block_count, 0);
	std::vector<BlockId> work;
	for (BlockId b : body_.layout)
	{
		// (Ordinals are compared only in a block with an opener, which the
		// CFG flags BF_EH.)
		uint32_t ordinal = 0;
		if ((cfg.flags[b] & BF_EH) != 0)
			for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			{
				ordinal_[i] = ordinal++;
				const Instruction& inst = body_.instructions[i];
				if ((inst.op == Opcode::EhTry || inst.op == Opcode::EhCleanup) && first_opener_[b] == kNoId)
					first_opener_[b] = ordinal_[i];
			}
		if ((cfg.flags[b] & BF_HANDLER) != 0)
		{
			handler_reached_[b] = 1;
			work.push_back(b);
		}
	}
	while (!work.empty())
	{
		const BlockId b = work.back();
		work.pop_back();
		auto visit = [&](BlockId s) {
			if (!handler_reached_[s])
			{
				handler_reached_[s] = 1;
				work.push_back(s);
			}
		};
		for (uint32_t k = cfg.succ_first[b]; k < cfg.succ_first[b + 1]; ++k)
			visit(cfg.succs[k]);
		for (uint32_t k = cfg.handler_first[b]; k < cfg.handler_first[b + 1]; ++k)
			visit(cfg.handlers[k]);
	}
}

bool Simplifier::available_in(TempId temp, BlockId block) const
{
	if (!handler_reached_[block] || temp < body_.parameter_count || body_.temps[temp].def == kNoId)
		return true;
	const InstId def = body_.temps[temp].def;
	const BlockId at = def < c_.block_of.size() ? c_.block_of[def] : kNoId;
	return at != kNoId && (first_opener_[at] == kNoId || ordinal_[def] < first_opener_[at]);
}

Value Simplifier::resolve(Value v)
{
	if (!v.is(ValueKind::Temp))
		return v;
	Value r = v;
	while (r.is(ValueKind::Temp) && !repl_[r.index].is(ValueKind::None))
		r = repl_[r.index];
	// Path compression.
	while (v.is(ValueKind::Temp) && !repl_[v.index].is(ValueKind::None))
	{
		const Value next = repl_[v.index];
		repl_[v.index] = r;
		v = next;
	}
	return r;
}

bool Simplifier::constant(Value v, Type type, Constant& out)
{
	(void)type;
	return literal_constant(body_, v, out);
}

OperandKey Simplifier::key_of(Value v, Type type)
{
	OperandKey key;
	key.kind = static_cast<uint8_t>(v.kind);
	key.float_kind = 0;
	key.low = v.index;
	key.high = 0;
	Constant c;
	if (constant(v, type, c))
	{
		key.float_kind = static_cast<uint8_t>(c.float_kind);
		Literal bits = c.bits;
		if (c.kind == ValueKind::Integer)
			bits = normalize_integer(type, bits);
		key.low = bits.low;
		key.high = bits.high;
	}
	return key;
}

// An i64 truth value (0 or 1): a comparison, a logical not, a 0/1
// literal, a phi of truth values, or a widening/narrowing of one.
bool Simplifier::boolean(Value v, unsigned depth)
{
	Constant c;
	if (literal_constant(body_, v, c))
		return c.kind == ValueKind::Integer && (c.bits.low <= 1 && c.bits.high == 0);
	const Instruction* def = producer(v);
	if (def == nullptr || depth > 3)
		return false;
	switch (def->op)
	{
	case Opcode::Compare:
		return true;
	case Opcode::Unary:
		return static_cast<UnaryOp>(def->sub) == UnaryOp::Not;
	case Opcode::Convert:
		return (static_cast<ConvertOp>(def->sub) == ConvertOp::ZExt ||
				   static_cast<ConvertOp>(def->sub) == ConvertOp::Trunc) &&
			boolean(def->ops[0], depth + 1);
	case Opcode::Phi:
		for (uint32_t k = 1; k < def->extra_count; k += 2)
		{
			const Value in = body_.operands[def->extra_first + k];
			if (!(in.is(ValueKind::Temp) && in.index == def->result) && !boolean(in, depth + 1))
				return false;
		}
		return true;
	default:
		return false;
	}
}

void Simplifier::substitute(Instruction& inst)
{
	// (Only a temporary operand may change.)
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!inst.ops[i].is(ValueKind::Temp))
			continue;
		Value r = resolve(inst.ops[i]);
		if (r.is(ValueKind::Temp) && r.index < equal_.size() && !equal_[r.index].is(ValueKind::None) &&
			!(inst.op == Opcode::Call && i == 0))
			r = equal_[r.index];
		// A callee stays a temporary or symbol.
		if (inst.op == Opcode::Call && i == 0 && r.is_literal())
			continue;
		// O2+: a call through a function's address is a direct call (its
		// stated signature kept; O1 keeps it, pa32 pins the address-taken
		// callee's survival there).
		if (inst.op == Opcode::Call && i == 0 && c_.level >= 2 && r.is(ValueKind::Temp) &&
			body_.temps[r.index].def != kNoId)
		{
			const Instruction& def = body_.instructions[body_.temps[r.index].def];
			if (def.op == Opcode::Addr && def.ops[0].is(ValueKind::Symbol) &&
				is_function_kind(c_.program.symbols[def.ops[0].index].kind) && (inst.flags & IF_SIGNATURE) != 0)
				r = def.ops[0];
		}
		edited_ = edited_ || !(inst.ops[i] == r);
		inst.ops[i] = r;
	}
	for (uint32_t k = 0; k < inst.extra_count; ++k)
	{
		Value& v = body_.operands[inst.extra_first + k];
		if (!v.is(ValueKind::Temp))
			continue;
		const Value was = v;
		v = resolve(v);
		if (v.is(ValueKind::Temp) && v.index < equal_.size() && !equal_[v.index].is(ValueKind::None))
			v = equal_[v.index];
		edited_ = edited_ || !(was == v);
	}
}

bool Simplifier::fold(const Instruction& inst, Value& out)
{
	Constant a, b, r;
	const Type type = operand_type(inst);
	switch (inst.op)
	{
	case Opcode::Const:
		if (!constant(inst.ops[0], type, a) || !fold_copy(inst.type, a, r))
			return false;
		break;
	case Opcode::Copy:
		if (!constant(inst.ops[0], type, a) || !fold_copy(inst.type, a, r))
			return false;
		break;
	case Opcode::Unary:
		if (!constant(inst.ops[0], type, a) || !fold_unary(static_cast<UnaryOp>(inst.sub), inst.type, a, r))
			return false;
		break;
	case Opcode::Binary:
		if (!constant(inst.ops[0], type, a) || !constant(inst.ops[1], type, b) ||
			!fold_binary(static_cast<BinaryOp>(inst.sub), inst.type, a, b, r))
			return false;
		break;
	case Opcode::Compare:
		if (!constant(inst.ops[0], type, a) || !constant(inst.ops[1], type, b) ||
			!fold_compare(static_cast<ComparePredicate>(inst.sub), inst.type, a, b, r))
			return false;
		break;
	case Opcode::Convert:
		if (!constant(inst.ops[0], type, a) ||
			!fold_convert(static_cast<ConvertOp>(inst.sub), inst.type, convert_source_type(inst), a, r))
			return false;
		break;
	default:
		return false;
	}
	out = literal(r);
	return true;
}

bool Simplifier::identity(Instruction& inst, Value& out)
{
	switch (inst.op)
	{
	case Opcode::Select:
	{
		// A known condition picks its operand; equal operands are either.
		Constant c;
		if (literal_constant(body_, inst.ops[0], c) && c.kind == ValueKind::Integer)
		{
			out = (c.bits.low | c.bits.high) != 0 ? inst.ops[1] : inst.ops[2];
			return true;
		}
		if (inst.ops[1] == inst.ops[2])
		{
			out = inst.ops[1];
			return true;
		}
		return false;
	}
	case Opcode::Copy:
	{
		if (inst.ops[0].is(ValueKind::Temp) && body_.temps[inst.ops[0].index].type == inst.type)
		{
			out = inst.ops[0];
			return true;
		}
		// The only reader of a nonvolatile integer load reinterpreting it at
		// the same width (`(unsigned char)c`): the load reads that type.
		const TempId loaded = inst.ops[0].is(ValueKind::Temp) ? inst.ops[0].index : kNoId;
		Instruction* load = loaded != kNoId && loaded < uses_.size() && uses_[loaded] == 1 &&
				producer(inst.ops[0]) != nullptr
			? &body_.instructions[body_.temps[loaded].def]
			: nullptr;
		if (load == nullptr || load->op != Opcode::Load || (load->flags & IF_VOLATILE) != 0 ||
			!inst.type.is_integer() || !load->type.is_integer() ||
			storage_bytes(load->type) != storage_bytes(inst.type) || inst.type.kind() == TypeKind::I128 ||
			load->type.kind() == TypeKind::I128)
			return false;
		load->type = inst.type;
		body_.temps[loaded].type = inst.type;
		uses_[loaded] = 0xffffffffu; // its readers are this copy's now
		out = inst.ops[0];
		return true;
	}
	case Opcode::Index:
	{
		Constant offset;
		// A zero-element step through a pointer temporary.
		if (inst.ops[0].is(ValueKind::Temp) && body_.temps[inst.ops[0].index].type.is_pointer() &&
			constant(inst.ops[1], Type::scalar(TypeKind::I64), offset) &&
			is_integer(Type::scalar(TypeKind::I64), offset, 0))
		{
			out = inst.ops[0];
			return true;
		}
		return false;
	}
	case Opcode::Load:
		return readonly_load(inst, out);
	case Opcode::Call:
		return known_strlen(inst, out);
	case Opcode::Binary:
		return binary_identity(inst, out);
	case Opcode::Compare:
		return compare_identity(inst, out);
	case Opcode::Convert:
		return convert_identity(inst, out);
	case Opcode::Phi:
		return phi_value(inst, out) || select_identity(inst, current_block_, out);
	default:
		return false;
	}
}

// A nonvolatile integer load from a constant offset into readonly data
// (an `addr` of the global, `index` steps by literals, pointer copies).
bool Simplifier::readonly_base(Value address, Value& base, int64_t& offset)
{
	// The global and constant offset an address denotes (through `addr`,
	// `index` steps by literals and pointer copies).
	base = address;
	offset = 0;
	for (unsigned step = 0; step < 8 && base.is(ValueKind::Temp); ++step)
	{
		const Instruction* def = producer(base);
		if (def == nullptr)
			return false;
		if (def->op == Opcode::Addr || (def->op == Opcode::Copy && def->type.is_pointer()))
			base = def->ops[0];
		else if (def->op == Opcode::Index && def->ops[1].is(ValueKind::Integer))
		{
			offset += static_cast<int64_t>(body_.literals[def->ops[1].index].low) *
				static_cast<int64_t>(storage_bytes(def->type));
			base = def->ops[0];
		}
		else
			return false;
	}
	return base.is(ValueKind::Symbol);
}

// A nonvolatile pointer load from a constant offset into readonly data
// holding an address with an addend (a VTT's table address point): the
// address computed, `index i8 @table, addend`, which later loads through
// it read in turn (a base-object constructor inlined with a known VTT).
bool Simplifier::readonly_table_address(Instruction& inst)
{
	if (inst.op != Opcode::Load || (inst.flags & IF_VOLATILE) != 0 || !inst.type.is_pointer())
		return false;
	Value base;
	int64_t offset = 0;
	SymbolId target = kNoId;
	int64_t addend = 0;
	if (!readonly_base(inst.ops[0], base, offset) ||
		!read_readonly_address(c_.program, base.index, offset, c_.unwritten, target, &addend) || addend == 0)
		return false;
	inst.op = Opcode::Index;
	inst.type = Type::scalar(TypeKind::I8);
	inst.sub = ir_model::IPK_NONE;
	inst.flags = 0;
	inst.ops[0] = Value::symbol(target);
	inst.ops[1] = integer_value(body_, Literal{ static_cast<uint64_t>(addend), addend < 0 ? ~0ull : 0 });
	return true;
}

bool Simplifier::readonly_load(const Instruction& inst, Value& out)
{
	if ((inst.flags & IF_VOLATILE) != 0 || !(inst.type.is_integer() || inst.type.is_pointer()))
		return false;
	Value base;
	int64_t offset = 0;
	if (!readonly_base(inst.ops[0], base, offset))
		return false;
	if (inst.type.is_pointer())
	{
		// A table's pointer to another global: that global's address.
		SymbolId target = kNoId;
		if (!read_readonly_address(c_.program, base.index, offset, c_.unwritten, target))
			return false;
		out = Value::symbol(target);
		return true;
	}
	Constant c;
	if (!base.is(ValueKind::Symbol) ||
		!read_readonly_integer(c_.program, base.index, offset, inst.type, c, c_.unwritten))
		return false;
	out = literal(integer_constant(inst.type, c.bits));
	return true;
}

// strlen of the direct address of readonly byte data whose first NUL is
// known: the callee is a strlen (Program::strlen_function).
bool Simplifier::known_strlen(const Instruction& inst, Value& out)
{
	if (!inst.ops[0].is(ValueKind::Symbol) || inst.extra_count != 1 || !inst.type.is_integer() ||
		!c_.program.strlen_function(inst.ops[0].index))
		return false;
	Value v = body_.operands[inst.extra_first];
	for (unsigned step = 0; step < 4 && v.is(ValueKind::Temp); ++step)
	{
		const Instruction* def = producer(v);
		if (def == nullptr || !(def->op == Opcode::Addr || (def->op == Opcode::Copy && def->type.is_pointer())))
			return false;
		v = def->ops[0];
	}
	if (!v.is(ValueKind::Symbol))
		return false;
	const Symbol& data = c_.program.symbols[v.index];
	if (!constant_contents(c_.program, v.index, c_.unwritten))
		return false;
	const Global& g = c_.program.globals[data.detail];
	if (g.init != GlobalInit::Data || g.item_count > 4096)
		return false; // bounded work per call (as readonly loads)
	for (uint32_t k = 0; k < g.item_count; ++k)
	{
		const DataItem& item = c_.program.data_items[g.first_item + k];
		const bool byte = item.kind == DataItemKind::Scalar && item.constant.kind == ValueKind::Integer &&
			(item.type.kind() == TypeKind::I8 || item.type.kind() == TypeKind::U8);
		if (!byte && !(item.kind == DataItemKind::Zero && item.amount > 0))
			return false;
		if (item.kind == DataItemKind::Zero || (item.constant.bits.low & 0xff) == 0)
		{
			out = literal(integer_constant(inst.type, integer_literal(k)));
			return true;
		}
	}
	return false;
}

// index T1 (index T2 base, c2), c1 -> index i8 base, c2*|T2| + c1*|T1|.
bool Simplifier::merge_index(Instruction& inst)
{
	if (inst.op != Opcode::Index || !inst.ops[1].is(ValueKind::Integer))
		return false;
	const Instruction* def = producer(inst.ops[0]);
	if (def == nullptr || def->op != Opcode::Index || !def->ops[1].is(ValueKind::Integer))
		return false;
	const int64_t inner = static_cast<int64_t>(body_.literals[def->ops[1].index].low) *
		static_cast<int64_t>(storage_bytes(def->type));
	const int64_t outer = static_cast<int64_t>(body_.literals[inst.ops[1].index].low) *
		static_cast<int64_t>(storage_bytes(inst.type));
	inst.type = Type::scalar(TypeKind::I8);
	inst.ops[0] = def->ops[0];
	inst.ops[1] = integer_value(body_, Literal{ static_cast<uint64_t>(inner + outer), inner + outer < 0 ? ~0ull : 0 });
	return true;
}

bool Simplifier::binary_identity(Instruction& inst, Value& out)
{
	if (!inst.type.is_integer())
		return false;
	const Type type = inst.type;
	const BinaryOp op = static_cast<BinaryOp>(inst.sub);
	const Value x = inst.ops[0], y = inst.ops[1];
	Constant cy;
	const bool literal_y = constant(y, type, cy);
	if (literal_y)
	{
		// (0 and 1 normalize to themselves at every width.)
		Literal bits;
		const bool integral = integer_bits(type, cy, bits);
		const bool zero = integral && bits.low == 0 && bits.high == 0;
		const bool one = integral && bits.low == 1 && bits.high == 0;
		if (zero &&
			(op == BinaryOp::Add || op == BinaryOp::Sub || op == BinaryOp::Or || op == BinaryOp::Xor ||
				op == BinaryOp::Shl || op == BinaryOp::Shr || op == BinaryOp::UShr))
		{
			out = x;
			return true;
		}
		if (one && (op == BinaryOp::Mul || op == BinaryOp::Div || op == BinaryOp::UDiv))
		{
			out = x;
			return true;
		}
		if (op == BinaryOp::And && is_all_ones(type, cy))
		{
			out = x;
			return true;
		}
		if (zero && (op == BinaryOp::Mul || op == BinaryOp::And))
		{
			out = literal(integer_constant(type, cy.bits));
			return true;
		}
		if (op == BinaryOp::Or && is_all_ones(type, cy))
		{
			out = y;
			return true;
		}
		// A mask that clears only bits x cannot have keeps x; one that
		// keeps none of them leaves zero.
		if (op == BinaryOp::And && integral && type.kind() != TypeKind::I128)
		{
			const uint64_t possible = possible_bits(x, type);
			if ((possible & ~bits.low) == 0)
			{
				out = x;
				return true;
			}
			if ((possible & bits.low) == 0)
			{
				Literal none = { 0, 0 };
				out = literal(integer_constant(type, none));
				return true;
			}
		}
		// A logical shift right of a zero-extended value by at least the
		// width it was extended from leaves zero.
		const Instruction* ext = producer(x);
		if (op == BinaryOp::UShr && ext != nullptr && ext->op == Opcode::Convert &&
			static_cast<ConvertOp>(ext->sub) == ConvertOp::ZExt && cy.bits.high == 0 &&
			cy.bits.low >= 8 * storage_bytes(convert_source_type(*ext)) && cy.bits.low < 8 * storage_bytes(type) &&
			convert_source_type(*ext).is_integer())
		{
			Literal zero = { 0, 0 };
			out = literal(integer_constant(type, zero));
			return true;
		}
	}
	if (x == y && x.is(ValueKind::Temp))
	{
		if (op == BinaryOp::And || op == BinaryOp::Or)
		{
			out = x;
			return true;
		}
		if (op == BinaryOp::Sub || op == BinaryOp::Xor)
		{
			Literal zero = { 0, 0 };
			out = literal(integer_constant(type, zero));
			return true;
		}
	}
	return false;
}

// Entering a block whose only predecessor (its immediate dominator)
// branches on c: on the true edge c is nonzero, as is x for c = `x != 0`;
// on the false edge, x for c = `x == 0`.
void Simplifier::edge_facts(BlockId block)
{
	const Cfg& cfg = c_.cfg;
	if (cfg.pred_count(block) != 1 || cfg.preds[cfg.pred_first[block]] != cfg.idom[block])
		return;
	const BlockId p = cfg.idom[block];
	const InstId t = body_.blocks[p].last;
	if (t == kNoId || body_.instructions[t].op != Opcode::Branch)
		return;
	const Instruction& term = body_.instructions[t];
	if (term.ops[1].index == term.ops[2].index)
		return;
	const bool taken = term.ops[1].index == block;
	auto mark = [&](Value v) {
		if (v.is(ValueKind::Temp) && !nonzero_[v.index])
		{
			nonzero_[v.index] = 1;
			nonzero_undo_.push_back(v.index);
		}
	};
	if (taken)
		mark(term.ops[0]);
	Value x, y;
	if (edge_equality(p, block, x, y) && x.is(ValueKind::Temp) && y.is_literal() && equal_[x.index].is(ValueKind::None))
	{
		equal_[x.index] = y;
		equal_undo_.push_back(x.index);
	}
	const Instruction* def = producer(term.ops[0]);
	Constant zero;
	if (def == nullptr || def->op != Opcode::Compare || !def->type.is_integer() ||
		!literal_constant(body_, def->ops[1], zero) || !is_integer(def->type, zero, 0))
		return;
	const ComparePredicate pred = static_cast<ComparePredicate>(def->sub);
	if ((taken && pred == ComparePredicate::Ne) || (!taken && pred == ComparePredicate::Eq))
		mark(def->ops[0]);
}

// The equality x == y the branch ending `from` proves on its edge to `to`:
// x == y true or x != y false, and the unsigned bounds whose failure pins
// x (x < max false, x > 0 false, x <= 0 true, x < 1 true).
bool Simplifier::edge_equality(BlockId from, BlockId to, Value& x, Value& y) const
{
	const InstId t = body_.blocks[from].last;
	if (t == kNoId || body_.instructions[t].op != Opcode::Branch)
		return false;
	const Instruction& term = body_.instructions[t];
	if (term.ops[1].index == term.ops[2].index || (term.ops[1].index != to && term.ops[2].index != to))
		return false;
	const bool taken = term.ops[1].index == to;
	const Instruction* def = producer(term.ops[0]);
	if (def == nullptr || def->op != Opcode::Compare || def->type.is_float())
		return false;
	const ComparePredicate pred = static_cast<ComparePredicate>(def->sub);
	x = def->ops[0];
	y = def->ops[1];
	if ((taken && pred == ComparePredicate::Eq) || (!taken && pred == ComparePredicate::Ne))
	{
		if (x.is_literal())
			std::swap(x, y);
		return true;
	}
	if (!def->type.is_integer())
		return false;
	Constant c;
	if (!literal_constant(body_, y, c))
		return false;
	Literal bits = { 0, 0 };
	if ((!taken && pred == ComparePredicate::Ult && is_all_ones(def->type, c)))
		bits = normalize_integer(def->type, Literal{ ~0ull, ~0ull });
	else if (!((!taken && pred == ComparePredicate::Ugt && is_integer(def->type, c, 0)) ||
				 (taken && pred == ComparePredicate::Ule && is_integer(def->type, c, 0)) ||
				 (taken && pred == ComparePredicate::Ult && is_integer(def->type, c, 1))))
		return false;
	y = const_cast<Simplifier*>(this)->literal(integer_constant(def->type, bits));
	return true;
}

// A phi whose every input equals one value u on its edge (directly, or by
// the equality its edge proves from a branch through a jump-only arm) is u:
// min(n, max), max(n, 0) and an equality's choice of its own operand.
bool Simplifier::select_identity(const Instruction& phi, BlockId block, Value& out) const
{
	if (phi.extra_count != 4)
		return false;
	auto equal = [&](Value a, Value b, Type type) {
		Constant x, y;
		return a == b || (literal_constant(body_, a, x) && literal_constant(body_, b, y) && same_constant(type, x, y));
	};
	for (uint32_t pick = 1; pick < 4; pick += 2)
	{
		const Value u = body_.operands[phi.extra_first + pick];
		if (u.is(ValueKind::Temp) &&
			(u.index == phi.result ||
				(u.index >= body_.parameter_count && body_.temps[u.index].def != kNoId &&
					c_.block_of.size() > body_.temps[u.index].def &&
					(!c_.cfg.dominates(c_.block_of[body_.temps[u.index].def], block) ||
						!available_in(u.index, block)))))
			continue;
		bool all = true;
		for (uint32_t k = 0; k < 4 && all; k += 2)
		{
			const BlockId pred = body_.operands[phi.extra_first + k].index;
			const Value v = body_.operands[phi.extra_first + k + 1];
			if (equal(v, u, phi.type))
				continue;
			// The conditional edge the input arrives on: from `pred` itself,
			// or into `pred` when it is a jump-only arm with one predecessor.
			const Block& arm = body_.blocks[pred];
			BlockId from = kNoId, to = pred;
			if (arm.last != kNoId && body_.instructions[arm.last].op == Opcode::Branch)
			{
				from = pred;
				to = block;
			}
			else if (arm.first == arm.last && body_.instructions[arm.first].op == Opcode::Jump &&
				c_.cfg.pred_count(pred) == 1)
				from = c_.cfg.preds[c_.cfg.pred_first[pred]];
			Value x, y;
			all = from != kNoId && edge_equality(from, to, x, y) &&
				((equal(v, x, phi.type) && equal(u, y, phi.type)) || (equal(v, y, phi.type) && equal(u, x, phi.type)));
		}
		if (all)
		{
			out = u;
			return true;
		}
	}
	return false;
}

bool Simplifier::compare_identity(Instruction& inst, Value& out)
{
	const Type type = inst.type;
	const ComparePredicate pred = static_cast<ComparePredicate>(inst.sub);
	const Value x = inst.ops[0], y = inst.ops[1];
	// A value proved nonzero: x != 0, x == 0, and x - 1 against x
	// (unsigned: it cannot wrap below zero).
	if (type.is_integer())
	{
		Constant cy;
		if (known_nonzero(x) && constant(y, type, cy) && is_integer(type, cy, 0) &&
			(pred == ComparePredicate::Ne || pred == ComparePredicate::Eq))
		{
			out = literal(truth_constant(pred == ComparePredicate::Ne));
			return true;
		}
		// Either orientation: (x - 1) pred x, or x pred (x - 1).
		for (unsigned side = 0; side < 2; ++side)
		{
			const Value d = side == 0 ? x : y, base = side == 0 ? y : x;
			const ComparePredicate p = side == 0 ? pred : swapped(pred);
			const Instruction* dec = producer(d);
			Constant one;
			if (dec != nullptr && dec->op == Opcode::Binary && static_cast<BinaryOp>(dec->sub) == BinaryOp::Sub &&
				dec->type == type && dec->ops[0] == base && known_nonzero(base) && constant(dec->ops[1], type, one) &&
				is_integer(type, one, 1) &&
				(p == ComparePredicate::Uge || p == ComparePredicate::Ugt || p == ComparePredicate::Ult ||
					p == ComparePredicate::Ule))
			{
				out = literal(truth_constant(p == ComparePredicate::Ult || p == ComparePredicate::Ule));
				return true;
			}
		}
	}
	if (!type.is_float() && x == y && (x.is(ValueKind::Temp) || x.is(ValueKind::Symbol)))
	{
		const bool equal = pred == ComparePredicate::Eq || pred == ComparePredicate::Le ||
			pred == ComparePredicate::Ge || pred == ComparePredicate::Ule || pred == ComparePredicate::Uge;
		out = literal(truth_constant(equal));
		return true;
	}
	if (type.is_float())
		return false;
	Constant cx, cy;
	const bool lx = constant(x, type, cx), ly = constant(y, type, cy);
	// Unsigned comparisons against 0 whose result is fixed.
	if ((ly && is_integer(type, cy, 0) && (pred == ComparePredicate::Ult || pred == ComparePredicate::Uge)) ||
		(lx && is_integer(type, cx, 0) && (pred == ComparePredicate::Ugt || pred == ComparePredicate::Ule)))
	{
		out = literal(truth_constant(pred == ComparePredicate::Uge || pred == ComparePredicate::Ule));
		return true;
	}
	// Truth values compared with 0 or 1.
	if (!ly || !(pred == ComparePredicate::Eq || pred == ComparePredicate::Ne) || !boolean(x))
		return false;
	const bool one = is_integer(type, cy, 1);
	if (!one && !is_integer(type, cy, 0))
		return false;
	// x != 0 and x == 1 are x itself (an i64 truth value).
	if (one == (pred == ComparePredicate::Eq))
	{
		Value v = x;
		// Look through truth-preserving conversions to an i64 truth value.
		for (const Instruction* def = producer(v); def != nullptr && def->op == Opcode::Convert; def = producer(v))
			v = def->ops[0];
		if (v.is(ValueKind::Temp) && body_.temps[v.index].type == Type::scalar(TypeKind::I64) && boolean(v))
		{
			out = v;
			return true;
		}
		return false;
	}
	// x == 0 and x != 1 invert an integer comparison x is (through
	// truth-preserving conversions).
	Value v = x;
	const Instruction* def = producer(v);
	while (def != nullptr && def->op == Opcode::Convert)
	{
		v = def->ops[0];
		def = producer(v);
	}
	if (def == nullptr || def->op != Opcode::Compare || def->type.is_float())
		return false;
	inst.sub = static_cast<uint8_t>(inverted(static_cast<ComparePredicate>(def->sub)));
	inst.type = def->type;
	inst.ops[0] = def->ops[0];
	inst.ops[1] = def->ops[1];
	changed_ = true;
	return false;
}

bool Simplifier::convert_identity(Instruction& inst, Value& out)
{
	const ConvertOp op = static_cast<ConvertOp>(inst.sub);
	const Type from = convert_source_type(inst);
	if ((op == ConvertOp::SExt || op == ConvertOp::ZExt || op == ConvertOp::Trunc || op == ConvertOp::FPExt ||
			op == ConvertOp::FPTrunc) &&
		from == inst.type && inst.ops[0].is(ValueKind::Temp) && body_.temps[inst.ops[0].index].type == inst.type)
	{
		out = inst.ops[0];
		return true;
	}
	// A truncation back to the type an extension widened.
	const Instruction* def = op == ConvertOp::Trunc ? producer(inst.ops[0]) : nullptr;
	if (def != nullptr && def->op == Opcode::Convert &&
		(static_cast<ConvertOp>(def->sub) == ConvertOp::ZExt || static_cast<ConvertOp>(def->sub) == ConvertOp::SExt) &&
		def->ops[0].is(ValueKind::Temp) && body_.temps[def->ops[0].index].type == inst.type)
	{
		out = def->ops[0];
		return true;
	}
	return false;
}

bool Simplifier::phi_value(const Instruction& phi, Value& out)
{
	Value same = Value::none();
	for (uint32_t k = 1; k < phi.extra_count; k += 2)
	{
		const Value in = body_.operands[phi.extra_first + k];
		if (in.is(ValueKind::Temp) && in.index == phi.result)
			continue;
		if (same.is(ValueKind::None))
		{
			same = in;
			continue;
		}
		if (in == same)
			continue;
		Constant a, b;
		if (!literal_constant(body_, in, a) || !literal_constant(body_, same, b) || !same_constant(phi.type, a, b))
			return false;
	}
	if (same.is(ValueKind::None))
		return false;
	out = same;
	return true;
}

// (x op c1) op c2 -> x op (c1 op c2) for add, mul, and, or, xor; also sub
// of a constant as an add.
// The bits of integer value v that may be set: a literal's; a zero
// extension's, its source's; and/or/xor and constant shifts of those;
// every bit of the type otherwise (types up to 64 bits).
uint64_t Simplifier::possible_bits(Value v, Type type, unsigned depth)
{
	const unsigned width = static_cast<unsigned>(8 * storage_bytes(type));
	const uint64_t all = width >= 64 ? ~static_cast<uint64_t>(0) : (static_cast<uint64_t>(1) << width) - 1;
	Constant c;
	Literal bits;
	if (constant(v, type, c))
		return integer_bits(type, c, bits) ? bits.low & all : all;
	const Instruction* def = producer(v);
	if (def == nullptr || depth >= 4 || !type.is_integer() || type.kind() == TypeKind::I128 || def->type != type)
		return all;
	if (def->op == Opcode::Convert && static_cast<ConvertOp>(def->sub) == ConvertOp::ZExt)
	{
		const Type from = convert_source_type(*def);
		return from.is_integer() && from.kind() != TypeKind::I128 && storage_bytes(from) < storage_bytes(type)
			? possible_bits(def->ops[0], from, depth + 1)
			: all;
	}
	if (def->op != Opcode::Binary)
		return all;
	const BinaryOp op = static_cast<BinaryOp>(def->sub);
	if (op == BinaryOp::And)
		return possible_bits(def->ops[0], type, depth + 1) & possible_bits(def->ops[1], type, depth + 1);
	if (op == BinaryOp::Or || op == BinaryOp::Xor)
		return possible_bits(def->ops[0], type, depth + 1) | possible_bits(def->ops[1], type, depth + 1);
	if ((op == BinaryOp::Shl || op == BinaryOp::UShr) && constant(def->ops[1], type, c) &&
		integer_bits(type, c, bits) && bits.high == 0 && bits.low < width)
	{
		const uint64_t from = possible_bits(def->ops[0], type, depth + 1);
		return (op == BinaryOp::Shl ? from << bits.low : from >> bits.low) & all;
	}
	return all;
}

// `and (or a, b), c` where c keeps none of a's possible bits is
// `and b, c` (a packed field's store after another field's: the earlier
// merge's kept bits fall out, and with them the eightbyte's old value).
bool Simplifier::mask_or(Instruction& inst)
{
	if (inst.op != Opcode::Binary || static_cast<BinaryOp>(inst.sub) != BinaryOp::And || !inst.type.is_integer() ||
		inst.type.kind() == TypeKind::I128)
		return false;
	Constant cy;
	Literal bits;
	if (!constant(inst.ops[1], inst.type, cy) || !integer_bits(inst.type, cy, bits))
		return false;
	const Instruction* def = producer(inst.ops[0]);
	if (def == nullptr || def->op != Opcode::Binary || static_cast<BinaryOp>(def->sub) != BinaryOp::Or ||
		def->type != inst.type)
		return false;
	for (unsigned k = 0; k < 2; ++k)
		if ((possible_bits(def->ops[k], inst.type) & bits.low) == 0)
		{
			inst.ops[0] = def->ops[1 - k];
			return true;
		}
	return false;
}

bool Simplifier::reassociate(Instruction& inst)
{
	if (inst.op != Opcode::Binary || !inst.type.is_integer())
		return false;
	BinaryOp op = static_cast<BinaryOp>(inst.sub);
	Constant cy;
	if (!constant(inst.ops[1], inst.type, cy))
		return false;
	const Instruction* def = producer(inst.ops[0]);
	if (def == nullptr || def->op != Opcode::Binary || def->type != inst.type)
		return false;
	BinaryOp inner = static_cast<BinaryOp>(def->sub);
	Constant cx;
	if (!constant(def->ops[1], def->type, cx))
		return false;
	Literal zero = { 0, 0 };
	Constant zero_c = integer_constant(inst.type, zero), r;
	// Subtractions of constants are additions of their negations.
	if (op == BinaryOp::Sub)
	{
		if (!fold_binary(BinaryOp::Sub, inst.type, zero_c, cy, cy))
			return false;
		op = BinaryOp::Add;
	}
	if (inner == BinaryOp::Sub)
	{
		if (!fold_binary(BinaryOp::Sub, inst.type, zero_c, cx, cx))
			return false;
		inner = BinaryOp::Add;
	}
	if (op != inner ||
		!(op == BinaryOp::Add || op == BinaryOp::Mul || op == BinaryOp::And || op == BinaryOp::Or ||
			op == BinaryOp::Xor))
		return false;
	if (!fold_binary(op, inst.type, cx, cy, r))
		return false;
	inst.sub = static_cast<uint8_t>(op);
	inst.ops[0] = def->ops[0];
	inst.ops[1] = literal(r);
	inst.flags &= static_cast<uint8_t>(~(IF_NO_WRAP | IF_EXACT)); // a different computation
	return true;
}

// A literal operand goes right (a comparison's predicate swapped with
// it); two temporaries of a commutative operation go in index order. An
// ordered comparison of two temporaries keeps the source's operand order
// (the course's branch-sense convention): its value-numbering key is
// ordered instead.
void Simplifier::canonicalize(Instruction& inst)
{
	if (inst.op != Opcode::Binary && inst.op != Opcode::Compare)
		return;
	Value& x = inst.ops[0];
	Value& y = inst.ops[1];
	bool swap = false;
	if (x.is_literal() && !y.is_literal())
		swap = true;
	else if (x.is(ValueKind::Temp) && y.is(ValueKind::Temp) && x.index > y.index)
		swap = is_commutative(inst);
	if (!swap)
		return;
	if (is_commutative(inst))
		std::swap(x, y);
	else if (inst.op == Opcode::Compare)
	{
		std::swap(x, y);
		inst.sub = static_cast<uint8_t>(swapped(static_cast<ComparePredicate>(inst.sub)));
	}
}

// A comparison of an extended narrow integer with a literal the narrow
// type holds compares the narrow value (its load then folds into the
// compare): a zero-extension's under unsigned predicates (a signed one of
// the wider type reads a non-negative value), a sign-extension's under
// equality and signed predicates.
bool Simplifier::narrow_compare(Instruction& inst)
{
	if (inst.op != Opcode::Compare || !inst.type.is_integer() || inst.type.kind() == TypeKind::I128)
		return false;
	// Two extensions alike from one narrow type compare at that type
	// (unsigned predicates for zero extensions).
	if (!inst.ops[1].is(ValueKind::Integer))
	{
		const Instruction* a = producer(inst.ops[0]);
		const Instruction* b = producer(inst.ops[1]);
		if (a == nullptr || b == nullptr || a->op != Opcode::Convert || b->op != Opcode::Convert || a->sub != b->sub ||
			!a->ops[0].is(ValueKind::Temp) || !b->ops[0].is(ValueKind::Temp) || a->type != inst.type ||
			b->type != inst.type)
			return false;
		const ConvertOp op = static_cast<ConvertOp>(a->sub);
		const Type narrow = convert_source_type(*a);
		if ((op != ConvertOp::ZExt && op != ConvertOp::SExt) || narrow != convert_source_type(*b) ||
			!narrow.is_integer() || narrow.kind() == TypeKind::I128 ||
			storage_bytes(narrow) >= storage_bytes(inst.type))
			return false;
		ComparePredicate pred = static_cast<ComparePredicate>(inst.sub);
		const bool equality = pred == ComparePredicate::Eq || pred == ComparePredicate::Ne;
		if (op == ConvertOp::ZExt)
		{
			switch (pred)
			{
			case ComparePredicate::Lt:
				pred = ComparePredicate::Ult;
				break;
			case ComparePredicate::Le:
				pred = ComparePredicate::Ule;
				break;
			case ComparePredicate::Gt:
				pred = ComparePredicate::Ugt;
				break;
			case ComparePredicate::Ge:
				pred = ComparePredicate::Uge;
				break;
			default:
				break;
			}
		}
		else if (!equality && pred != ComparePredicate::Lt && pred != ComparePredicate::Le &&
			pred != ComparePredicate::Gt && pred != ComparePredicate::Ge)
			return false;
		inst.ops[0] = a->ops[0];
		inst.ops[1] = b->ops[0];
		inst.type = narrow;
		inst.sub = static_cast<uint8_t>(pred);
		return true;
	}
	const Instruction* ext = producer(inst.ops[0]);
	if (ext == nullptr || ext->op != Opcode::Convert || !ext->ops[0].is(ValueKind::Temp))
		return false;
	const ConvertOp op = static_cast<ConvertOp>(ext->sub);
	const Type narrow = convert_source_type(*ext);
	if ((op != ConvertOp::ZExt && op != ConvertOp::SExt) || !narrow.is_integer() || narrow.kind() == TypeKind::I128 ||
		ext->type != inst.type || storage_bytes(narrow) >= storage_bytes(inst.type))
		return false;
	const unsigned bits = narrow.kind() == TypeKind::I1 ? 1 : 8 * static_cast<unsigned>(storage_bytes(narrow));
	const Literal wide = normalize_integer(inst.type, body_.literals[inst.ops[1].index]);
	const int64_t c = static_cast<int64_t>(wide.low);
	ComparePredicate pred = static_cast<ComparePredicate>(inst.sub);
	const bool equality = pred == ComparePredicate::Eq || pred == ComparePredicate::Ne;
	if (op == ConvertOp::ZExt)
	{
		if (c < 0 || static_cast<uint64_t>(c) > (bits == 64 ? ~0ull : (1ull << bits) - 1))
			return false;
		switch (pred)
		{
		case ComparePredicate::Lt:
			pred = ComparePredicate::Ult;
			break;
		case ComparePredicate::Le:
			pred = ComparePredicate::Ule;
			break;
		case ComparePredicate::Gt:
			pred = ComparePredicate::Ugt;
			break;
		case ComparePredicate::Ge:
			pred = ComparePredicate::Uge;
			break;
		default:
			break;
		}
	}
	else
	{
		const int64_t low = -(int64_t(1) << (bits - 1)), high = (int64_t(1) << (bits - 1)) - 1;
		const bool signed_pred = pred == ComparePredicate::Lt || pred == ComparePredicate::Le ||
			pred == ComparePredicate::Gt || pred == ComparePredicate::Ge;
		if (c < low || c > high || !(equality || signed_pred))
			return false;
	}
	Literal bits_of = { static_cast<uint64_t>(c), c < 0 ? ~0ull : 0 };
	inst.ops[0] = ext->ops[0];
	inst.ops[1] = integer_value(body_, normalize_integer(narrow, bits_of));
	inst.type = narrow;
	inst.sub = static_cast<uint8_t>(pred);
	return true;
}

// memset(p, 0, n) and memcpy(d, s, n) of a literal n up to kBulkCallBytes:
// zeroinit and copyobj (the back end stores directly); the result is the
// destination.
const int64_t kBulkCallBytes = 128;

bool Simplifier::bulk_call(Instruction& inst)
{
	if (inst.op != Opcode::Call || !inst.ops[0].is(ValueKind::Symbol) || inst.extra_count != 3)
		return false;
	const bool zero = c_.program.memset_function(inst.ops[0].index);
	if (!zero && !c_.program.memcpy_function(inst.ops[0].index))
		return false;
	const Value* args = &body_.operands[inst.extra_first];
	Constant n, fill;
	if (!constant(args[2], Type::scalar(TypeKind::I64), n) || n.kind != ValueKind::Integer)
		return false;
	const int64_t bytes = static_cast<int64_t>(n.bits.low);
	if (bytes <= 0 || bytes > kBulkCallBytes || n.bits.high != 0)
		return false;
	if (zero &&
		(!constant(args[1], Type::scalar(TypeKind::I32), fill) || !is_integer(Type::scalar(TypeKind::I32), fill, 0)))
		return false;
	const Value to = args[0], from = args[1];
	if (inst.result != kNoId)
	{
		if (repl_.size() <= inst.result)
			repl_.resize(body_.temps.size(), Value::none());
		repl_[inst.result] = to;
	}
	inst.op = zero ? Opcode::ZeroInit : Opcode::CopyObject;
	inst.type = Type::scalar(TypeKind::Void);
	inst.result = kNoId;
	inst.ops[0] = zero ? to : from;
	inst.ops[1] = zero ? Value::none() : to;
	inst.ops[2] = Value::none();
	inst.extra_first = inst.extra_count = 0;
	inst.immediate = bytes;
	inst.sub = 0;
	inst.flags = 0;
	return true;
}

// O2+: memcmp(a, b, n) of a literal n up to 16 whose result only meets
// zero in eq/ne comparisons: whether one or two overlapping n-covering chunks
// differ (loads of each, xor, or), zero-extended to its i32. The new
// instructions go after `prev` (which ends at the last of them).
bool Simplifier::equality_memcmp(BlockId block, InstId& prev, InstId id)
{
	Instruction& call = body_.instructions[id];
	if (c_.level < 2 || call.op != Opcode::Call || !call.ops[0].is(ValueKind::Symbol) || call.extra_count != 3 ||
		call.result == kNoId || !c_.program.memcmp_function(call.ops[0].index))
		return false;
	const Value a = body_.operands[call.extra_first], b = body_.operands[call.extra_first + 1];
	Constant n;
	if (!(a.is(ValueKind::Temp) || a.is(ValueKind::Symbol)) || !(b.is(ValueKind::Temp) || b.is(ValueKind::Symbol)) ||
		!constant(body_.operands[call.extra_first + 2], Type::scalar(TypeKind::I64), n) ||
		n.kind != ValueKind::Integer || n.bits.high != 0 || n.bits.low == 0 || n.bits.low > 16)
		return false;
	const TempId result = call.result;
	if (result >= uses_.size() || uses_[result] == 0)
		return false;
	uint32_t compares = 0;
	for (BlockId b2 : body_.layout)
		for (InstId i = body_.blocks[b2].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& user = body_.instructions[i];
			bool reads = false;
			for_each_operand(body_, user, [&](const Value& v) { reads = reads || v == Value::temp(result); });
			if (!reads)
				continue;
			Constant zero;
			if (user.op != Opcode::Compare || !(user.ops[0] == Value::temp(result)) ||
				user.ops[1] == Value::temp(result) || !constant(user.ops[1], Type::scalar(TypeKind::I32), zero) ||
				!is_integer(Type::scalar(TypeKind::I32), zero, 0) ||
				(static_cast<ComparePredicate>(user.sub) != ComparePredicate::Eq &&
					static_cast<ComparePredicate>(user.sub) != ComparePredicate::Ne))
				return false;
			++compares;
		}
	if (compares != uses_[result])
		return false;
	const uint64_t bytes = n.bits.low;
	const uint64_t chunk = bytes >= 8 ? 8 : bytes >= 4 ? 4 : bytes >= 2 ? 2 : 1;
	const Type type = Type::scalar(chunk == 8 ? TypeKind::I64
			: chunk == 4                      ? TypeKind::U32
			: chunk == 2                      ? TypeKind::U16
											  : TypeKind::U8);
	const uint32_t location = call.location;
	auto emit = [&](Instruction inst, Type result_type) {
		inst.location = location;
		const InstId at = append_instruction(body_, inst);
		if (!result_type.is_void())
			body_.instructions[at].result = append_temp(body_, result_type, at);
		insert_after(body_, block, prev, at);
		prev = at;
		return Value::temp(body_.instructions[at].result);
	};
	auto load_at = [&](Value base, uint64_t offset) {
		Value p = base;
		if (offset != 0 && base.is(ValueKind::Symbol))
		{
			Instruction addr = make_instruction(Opcode::Addr, Type::scalar(TypeKind::Ptr));
			addr.ops[0] = base;
			base = emit(addr, Type::scalar(TypeKind::Ptr));
		}
		if (offset != 0)
		{
			Instruction index = make_instruction(Opcode::Index, Type::scalar(TypeKind::I8));
			index.ops[0] = base;
			index.ops[1] = integer_value(body_, integer_literal(offset));
			p = emit(index, Type::scalar(TypeKind::Ptr));
		}
		Instruction load = make_instruction(Opcode::Load, type);
		load.ops[0] = p;
		return emit(load, type);
	};
	Value diff = Value::none();
	const uint64_t offsets[2] = { 0, bytes - chunk };
	for (unsigned k = 0; k < (offsets[1] != 0 ? 2u : 1u); ++k)
	{
		Instruction x = make_instruction(Opcode::Binary, type);
		x.sub = static_cast<uint8_t>(BinaryOp::Xor);
		x.ops[0] = load_at(a, offsets[k]);
		x.ops[1] = load_at(b, offsets[k]);
		const Value d = emit(x, type);
		if (diff.is(ValueKind::None))
			diff = d;
		else
		{
			Instruction o = make_instruction(Opcode::Binary, type);
			o.sub = static_cast<uint8_t>(BinaryOp::Or);
			o.ops[0] = diff;
			o.ops[1] = d;
			diff = emit(o, type);
		}
	}
	Instruction ne = make_instruction(Opcode::Compare, type);
	ne.sub = static_cast<uint8_t>(ComparePredicate::Ne);
	ne.ops[0] = diff;
	ne.ops[1] = integer_value(body_, integer_literal(0));
	const Value differs = emit(ne, Type::scalar(TypeKind::I64));
	// The new temporaries in every per-temp table (edge facts may meet
	// them as a branch's operands).
	repl_.resize(body_.temps.size(), Value::none());
	uses_.resize(body_.temps.size(), 0);
	nonzero_.resize(body_.temps.size(), 0);
	equal_.resize(body_.temps.size(), Value::none());
	// The call becomes the i32 the comparisons read.
	Instruction& replaced = body_.instructions[id];
	replaced.op = Opcode::Convert;
	replaced.sub = static_cast<uint8_t>(ConvertOp::Trunc);
	replaced.type = Type::scalar(TypeKind::I32);
	set_convert_source_type(replaced, Type::scalar(TypeKind::I64));
	replaced.ops[0] = differs;
	replaced.ops[1] = replaced.ops[2] = Value::none();
	replaced.extra_first = replaced.extra_count = 0;
	replaced.flags = 0;
	return true;
}

// A branch tests only whether its condition is zero: look through
// zero-extensions, truncations of truth values and `x != 0`.
void Simplifier::simplify_branch(Instruction& term)
{
	for (unsigned step = 0; step < 4; ++step)
	{
		const Instruction* def = producer(term.ops[0]);
		if (def == nullptr)
			return;
		if (def->op == Opcode::Convert && def->ops[0].is(ValueKind::Temp) &&
			(static_cast<ConvertOp>(def->sub) == ConvertOp::ZExt ||
				(static_cast<ConvertOp>(def->sub) == ConvertOp::Trunc && boolean(def->ops[0]))))
		{
			term.ops[0] = def->ops[0];
			changed_ = true;
			continue;
		}
		Constant zero;
		if (def->op == Opcode::Compare && static_cast<ComparePredicate>(def->sub) == ComparePredicate::Ne &&
			def->type.is_integer() && def->ops[0].is(ValueKind::Temp) && constant(def->ops[1], def->type, zero) &&
			is_integer(def->type, zero, 0) && body_.temps[def->ops[0].index].type == def->type)
		{
			term.ops[0] = def->ops[0];
			changed_ = true;
			continue;
		}
		return;
	}
}

// Phis of one block with the same type and the same input for every
// predecessor are one value (twin induction variables). O(inputs): b's
// inputs by predecessor (the first pair of each) in a stamped scratch.
bool Simplifier::same_phi(const Instruction& a, const Instruction& b)
{
	if (a.type != b.type || a.extra_count != b.extra_count)
		return false;
	if (pred_input_.size() < body_.blocks.size())
	{
		pred_input_.resize(body_.blocks.size(), Value::none());
		pred_stamp_.resize(body_.blocks.size(), 0);
	}
	++stamp_;
	for (uint32_t k = 0; k < b.extra_count; k += 2)
	{
		const BlockId pred = body_.operands[b.extra_first + k].index;
		if (pred_stamp_[pred] != stamp_)
		{
			pred_stamp_[pred] = stamp_;
			pred_input_[pred] = body_.operands[b.extra_first + k + 1];
		}
	}
	for (uint32_t k = 0; k < a.extra_count; k += 2)
	{
		const BlockId pred = body_.operands[a.extra_first + k].index;
		const Value in = pred_stamp_[pred] == stamp_ ? pred_input_[pred] : Value::none();
		const Value mine = body_.operands[a.extra_first + k + 1];
		// A self-reference matches the other's self-reference.
		const bool self_a = mine.is(ValueKind::Temp) && mine.index == a.result;
		const bool self_b = in.is(ValueKind::Temp) && in.index == b.result;
		if (self_a != self_b || (!self_a && !(in == mine)))
			return false;
	}
	return true;
}

// Equal for phis same_phi may match: the type, the input count and the
// sum of each (block, input) pair's hash, a self-reference hashed alike.
uint64_t Simplifier::phi_signature(const Instruction& phi) const
{
	uint64_t sum = 0;
	for (uint32_t k = 0; k < phi.extra_count; k += 2)
	{
		const Value in = body_.operands[phi.extra_first + k + 1];
		const bool self = in.is(ValueKind::Temp) && in.index == phi.result;
		uint64_t h = (static_cast<uint64_t>(body_.operands[phi.extra_first + k].index) + 1) * 0x9e3779b97f4a7c15ull;
		h ^= self ? 0x5bd1e995ull : (static_cast<uint64_t>(in.kind) << 32 | in.index) * 0xc2b2ae3d27d4eb4full;
		sum += h ^ (h >> 31);
	}
	return sum ^ (type_bits(phi.type) * 0x165667b19e3779f9ull) ^ phi.extra_count;
}

void Simplifier::process_block(BlockId block)
{
	current_block_ = block;
	walked_[block] = 1;
	// Duplicate phis first (their inputs from back edges are not yet
	// resolved here; resolve() follows the same chains). Earlier phis are
	// found by an order-independent signature of their (block, input)
	// pairs, each signature's phis chained in block order.
	uint32_t phis = 0;
	for (InstId i = body_.blocks[block].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
		++phis;
	if (phis > 1)
	{
		phi_heads_.clear(phis);
		phi_chain_.clear();
	}
	for (InstId i = body_.blocks[block].first; i != kNoId && body_.instructions[i].op == Opcode::Phi && phis > 1;
		i = body_.instructions[i].next)
	{
		Instruction& phi = body_.instructions[i];
		for (uint32_t k = 1; k < phi.extra_count; k += 2)
			body_.operands[phi.extra_first + k] = resolve(body_.operands[phi.extra_first + k]);
		const uint64_t signature = phi_signature(phi);
		Chain* chain = phi_heads_.find(signature);
		bool merged = false;
		for (uint32_t c = chain != nullptr ? chain->first : kNoId; c != kNoId && !merged; c = phi_chain_[c].second)
		{
			const Instruction& earlier = body_.instructions[phi_chain_[c].first];
			if (repl_[earlier.result].is(ValueKind::None) && same_phi(earlier, phi))
			{
				repl_[phi.result] = Value::temp(earlier.result);
				++c_.stats.reused;
				changed_ = true;
				merged = true;
			}
		}
		const uint32_t node = static_cast<uint32_t>(phi_chain_.size());
		phi_chain_.push_back(std::make_pair(i, kNoId));
		if (chain == nullptr)
			phi_heads_.insert(signature, Chain{ node, node });
		else
		{
			phi_chain_[chain->last].second = node;
			chain->last = node;
		}
	}
	if (phis == 1)
	{
		Instruction& phi = body_.instructions[body_.blocks[block].first];
		for (uint32_t k = 1; k < phi.extra_count; k += 2)
			body_.operands[phi.extra_first + k] = resolve(body_.operands[phi.extra_first + k]);
	}
	InstId prev = kNoId;
	for (InstId id = body_.blocks[block].first; id != kNoId;)
	{
		Instruction& inst = body_.instructions[id];
		const InstId next = inst.next;
		if (inst.op == Opcode::Phi && !repl_[inst.result].is(ValueKind::None))
		{
			unlink_after(body_, block, prev, id);
			id = next;
			continue;
		}
		substitute(inst);
		// The rewrite appends instructions (`inst` may move): the call,
		// now a conversion, is taken up again.
		if (inst.op == Opcode::Call && equality_memcmp(block, prev, id))
		{
			changed_ = true;
			continue;
		}
		if (inst.op == Opcode::Branch)
			simplify_branch(inst);
		canonicalize(inst);
		if (reassociate(inst) || mask_or(inst) || merge_index(inst) || narrow_compare(inst) || bulk_call(inst) ||
			readonly_table_address(inst))
			changed_ = true;
		Value value;
		if (inst.result != kNoId && (fold(inst, value) || identity(inst, value)))
		{
			repl_[inst.result] = value;
			unlink_after(body_, block, prev, id);
			++c_.stats.folded;
			changed_ = true;
			id = next;
			continue;
		}
		const bool reusable = inst.result != kNoId && is_pure(inst) && inst.op != Opcode::Phi &&
			inst.op != Opcode::Const;
		if (reusable)
		{
			const Type type = operand_type(inst);
			ExprKey key;
			key.op = static_cast<uint8_t>(inst.op);
			key.sub = inst.sub;
			key.type = type_bits(inst.type);
			key.immediate = static_cast<uint64_t>(inst.immediate);
			key.a = key_of(inst.ops[0], type);
			key.b = key_of(inst.ops[1], type);
			key.c = key_of(inst.ops[2], type);
			if (inst.op == Opcode::Compare && inst.ops[0].is(ValueKind::Temp) && inst.ops[1].is(ValueKind::Temp) &&
				inst.ops[0].index > inst.ops[1].index)
			{
				// `b > a` is `a < b`.
				std::swap(key.a, key.b);
				key.sub = static_cast<uint8_t>(swapped(static_cast<ComparePredicate>(inst.sub)));
			}
			Available* found = table_.find(key);
			if (found != nullptr && c_.cfg.dominates(found->block, block))
			{
				if (handler_reached_[block] && first_opener_[found->block] != kNoId &&
					found->ordinal >= first_opener_[found->block])
				{
					// Computed after an opener, in code a handler reaches:
					// neither reused nor replaced.
					prev = id;
					id = next;
					continue;
				}
				repl_[inst.result] = Value::temp(found->temp);
				unlink_after(body_, block, prev, id);
				++c_.stats.reused;
				changed_ = true;
				id = next;
				continue;
			}
			if (found != nullptr)
				*found = Available{ inst.result, block, ordinal_[id] };
			else
				table_.insert(key, Available{ inst.result, block, ordinal_[id] });
		}
		prev = id;
		id = next;
	}
}

// Every operand resolved, phis included (back edges name later blocks)
// and unreachable blocks too.
// A diamond: `head` branches on c to two arms with it as their one
// predecessor, which jump to one merge they are the only predecessors of.
struct Diamond
{
	BlockId head = kNoId, on_true = kNoId, on_false = kNoId, merge = kNoId;
	Value condition;
};

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

// Whether a and b compute the same value: equal operands, or the same pure
// operation or nonvolatile load over operands that are (depth-bounded). A
// load counts only inside the write-free blocks `quiet` (merge_diamonds'
// arms and first merge), so no write can separate the two; a load before
// the first diamond may follow a write.
struct QuietBlocks
{
	const std::vector<BlockId>& block_of;
	BlockId blocks[5];
	bool holds(InstId inst) const
	{
		const BlockId b = inst < block_of.size() ? block_of[inst] : kNoId;
		for (BlockId q : blocks)
			if (b != kNoId && b == q)
				return true;
		return false;
	}
};

bool same_computation(const FunctionBody& body, Value a, Value b, unsigned depth, const QuietBlocks& quiet)
{
	if (a == b)
		return true;
	Constant x, y;
	if (literal_constant(body, a, x) && literal_constant(body, b, y))
		return x.kind == y.kind && x.bits.low == y.bits.low && x.bits.high == y.bits.high;
	if (!(a.is(ValueKind::Temp) || a.is(ValueKind::Symbol)) || !(b.is(ValueKind::Temp) || b.is(ValueKind::Symbol)) ||
		depth == 0 || a.index < body.parameter_count || b.index < body.parameter_count ||
		body.temps[a.index].def == kNoId || body.temps[b.index].def == kNoId)
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

// Back-to-back diamonds on one condition (the second's head is the first's
// merge, nothing in either writes memory): a phi of the second whose arm
// inputs compute what a phi of the first takes from the same sides is that
// phi. Each pair of diamonds compares at most kDiamondPairs phi pairs.
// Returns whether any phi was replaced.
const uint32_t kDiamondPairs = 1024;

uint32_t phi_count(const FunctionBody& body, BlockId block)
{
	uint32_t count = 0;
	for (InstId i = body.blocks[block].first; i != kNoId && body.instructions[i].op == Opcode::Phi;
		i = body.instructions[i].next)
		++count;
	return count;
}

bool Simplifier::merge_diamonds()
{
	const Cfg& cfg = c_.cfg;
	bool changed = false;
	for (BlockId h1 : body_.layout)
	{
		Diamond first, second;
		if (!cfg.reachable(h1) || !diamond_at(body_, cfg, h1, first) || !diamond_at(body_, cfg, first.merge, second) ||
			!(first.condition == second.condition) || !write_free(c_.program, body_, first.on_true) ||
			!write_free(c_.program, body_, first.on_false) || !write_free(c_.program, body_, first.merge) ||
			!write_free(c_.program, body_, second.on_true) || !write_free(c_.program, body_, second.on_false) ||
			static_cast<uint64_t>(phi_count(body_, first.merge)) * phi_count(body_, second.merge) > kDiamondPairs)
			continue;
		for (InstId i = body_.blocks[second.merge].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
			i = body_.instructions[i].next)
		{
			const Instruction& p2 = body_.instructions[i];
			if (!repl_[p2.result].is(ValueKind::None))
				continue;
			const Value t2 = phi_incoming(body_, p2, second.on_true), f2 = phi_incoming(body_, p2, second.on_false);
			const QuietBlocks quiet = { c_.block_of,
				{ first.on_true, first.on_false, first.merge, second.on_true, second.on_false } };
			for (InstId j = body_.blocks[first.merge].first; j != kNoId && body_.instructions[j].op == Opcode::Phi;
				j = body_.instructions[j].next)
			{
				const Instruction& p1 = body_.instructions[j];
				if (p1.type != p2.type)
					continue;
				if (same_computation(body_, phi_incoming(body_, p1, first.on_true), t2, 4, quiet) &&
					same_computation(body_, phi_incoming(body_, p1, first.on_false), f2, 4, quiet))
				{
					repl_[p2.result] = Value::temp(p1.result);
					changed = true;
					break;
				}
			}
		}
	}
	return changed;
}

void Simplifier::finish(bool full)
{
	// With no replacement and no equality in force, substitution changes
	// only a call through a function's address (in a block the walk did
	// not reach).
	bool any = false;
	for (size_t t = 0; t < repl_.size() && !any; ++t)
		any = !repl_[t].is(ValueKind::None);
	for (size_t t = 0; t < equal_.size() && !any; ++t)
		any = !equal_[t].is(ValueKind::None);
	// In a block the walk processed, an instruction past the phis was
	// substituted then, its operands defined before it in the walk (a
	// replacement is recorded only for the result being processed, an
	// equality's value is a literal): only phi inputs from back edges and
	// later blocks remain (unless `full`: merge_diamonds' replacements).
	for (BlockId b : body_.layout)
	{
		const bool walked = !full && b < walked_.size() && walked_[b];
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			if (walked && body_.instructions[i].op != Opcode::Phi)
				break;
			if (any || body_.instructions[i].op == Opcode::Call)
				substitute(body_.instructions[i]);
		}
	}
}

bool Simplifier::run()
{
	c_.ensure_cfg();
	table_.clear(body_.instructions.size());
	nonzero_undo_.clear();
	equal_undo_.clear();
	repl_.assign(body_.temps.size(), Value::none());
	uses_ = c_.entry_uses(simplify_values);
	const DomChildren& tree = c_.ensure_dom();
	if (c_.cfg.rpo.empty())
		return false;
	// Preorder over the dominator tree.
	std::vector<std::pair<BlockId, uint32_t>> stack;
	nonzero_.assign(body_.temps.size(), 0);
	equal_.assign(body_.temps.size(), Value::none());
	walked_.assign(body_.blocks.size(), 0);
	c_.entry_block_of(simplify_values);
	std::vector<size_t> nonzero_marks, equal_marks;
	index_openers();
	equal_marks.push_back(0);
	stack.push_back(std::make_pair(c_.cfg.rpo[0], 0u));
	nonzero_marks.push_back(nonzero_undo_.size());
	process_block(c_.cfg.rpo[0]);
	while (!stack.empty())
	{
		const BlockId b = stack.back().first;
		const uint32_t k = stack.back().second++;
		if (tree.first[b] + k < tree.first[b + 1])
		{
			const BlockId child = tree.children[tree.first[b] + k];
			stack.push_back(std::make_pair(child, 0u));
			nonzero_marks.push_back(nonzero_undo_.size());
			equal_marks.push_back(equal_undo_.size());
			edge_facts(child);
			process_block(child);
			continue;
		}
		for (size_t u = equal_undo_.size(); u > equal_marks.back(); --u)
			equal_[equal_undo_[u - 1]] = Value::none();
		equal_undo_.resize(equal_marks.back());
		equal_marks.pop_back();
		for (size_t u = nonzero_undo_.size(); u > nonzero_marks.back(); --u)
			nonzero_[nonzero_undo_[u - 1]] = 0;
		nonzero_undo_.resize(nonzero_marks.back());
		nonzero_marks.pop_back();
		stack.pop_back();
	}
	finish(false);
	if (c_.level >= 3 && merge_diamonds())
	{
		changed_ = true;
		finish(true);
	}
	return changed_;
}

} // namespace

bool simplify_values(FunctionContext& context)
{
	static Simplifier::Pool pool;
	Simplifier simplifier(context);
	simplifier.exchange(pool);
	const bool changed = simplifier.run();
	simplifier.exchange(pool);
	// An operand an equality replaced is not reported as a change (the
	// driver's rounds depend on the report): the use counts kept for the
	// next pass are stale then.
	if (!changed && simplifier.edited())
		context.invalidate_uses();
	// A reused expression may be defined later in layout than the use it
	// replaced (it dominates it).
	if (changed)
		restore_definition_order(context);
	return changed;
}

} // namespace opt
