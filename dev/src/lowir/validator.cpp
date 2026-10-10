#include "lowir/validator.h"

#include <cstring>

#include "lowir/metadata_spellings.h"

namespace lowir {

namespace {

const uint32_t kPlacedBlock = 1;
const uint32_t kHandlerBlock = 2;

// Phi and its incoming values have a directly representable scalar type.
bool is_phi_type(Type type)
{
	switch (type.kind())
	{
	case TypeKind::I1:
	case TypeKind::I8:
	case TypeKind::U8:
	case TypeKind::I16:
	case TypeKind::U16:
	case TypeKind::I32:
	case TypeKind::U32:
	case TypeKind::I64:
	case TypeKind::F32:
	case TypeKind::F64:
	case TypeKind::Ptr:
		return true;
	default:
		return false;
	}
}

// The integer kinds a bit scan (ctz, clz) reads.
bool is_bit_scan_type(Type type)
{
	switch (type.kind())
	{
	case TypeKind::I32:
	case TypeKind::U32:
	case TypeKind::I64:
		return true;
	default:
		return false;
	}
}

// The integer kinds a byte swap reverses.
bool is_bswap_type(Type type)
{
	switch (type.kind())
	{
	case TypeKind::I16:
	case TypeKind::U16:
	case TypeKind::I32:
	case TypeKind::U32:
	case TypeKind::I64:
		return true;
	default:
		return false;
	}
}

// Where the instruction facts may stand (lowir.md "Instruction Facts"): nowrap on
// an integer add, sub, mul or shl; exact on an integer div, udiv, shr or
// ushr; typed on a load or store of a pointer, a floating type or an
// integer wider than a byte (a byte access may be a character type's).
// Other instructions never spell them, so their flag bits are not facts.
const char* misplaced_fact(const Instruction& inst)
{
	if (inst.op == Opcode::Binary)
	{
		const BinaryOp op = static_cast<BinaryOp>(inst.sub);
		const bool integer = inst.type.is_integer() && inst.type.kind() != TypeKind::I1;
		if ((inst.flags & IF_NO_WRAP) != 0 &&
			!(integer && (op == BinaryOp::Add || op == BinaryOp::Sub || op == BinaryOp::Mul || op == BinaryOp::Shl)))
			return "nowrap requires an integer add, sub, mul or shl";
		if ((inst.flags & IF_EXACT) != 0 &&
			!(integer && (op == BinaryOp::Div || op == BinaryOp::UDiv || op == BinaryOp::Shr || op == BinaryOp::UShr)))
			return "exact requires an integer div, udiv, shr or ushr";
	}
	if ((inst.op == Opcode::Load || inst.op == Opcode::Store) && (inst.flags & IF_TYPED) != 0)
	{
		const TypeKind k = inst.type.kind();
		if (!(inst.type.is_pointer() ||
				((inst.type.is_integer() || inst.type.is_float()) && k != TypeKind::I1 && k != TypeKind::I8 &&
					k != TypeKind::U8)))
			return "typed requires a pointer, floating or wider-than-byte integer access";
	}
	return nullptr;
}

// The argument kinds va_arg reads from the variadic area.
bool is_va_arg_type(Type type)
{
	switch (type.kind())
	{
	case TypeKind::I8:
	case TypeKind::U8:
	case TypeKind::I16:
	case TypeKind::U16:
	case TypeKind::I32:
	case TypeKind::U32:
	case TypeKind::I64:
	case TypeKind::F64:
	case TypeKind::Ptr:
		return true;
	default:
		return false;
	}
}

bool is_valid_conversion(ConvertOp op, Type to, Type from)
{
	if (to == from)
		return true;
	const unsigned to_bits = integer_bits(to.kind());
	const unsigned from_bits = integer_bits(from.kind());
	const unsigned to_rank = float_rank(to.kind());
	const unsigned from_rank = float_rank(from.kind());
	switch (op)
	{
	case ConvertOp::SExt:
	case ConvertOp::ZExt:
		return to_bits != 0 && from_bits != 0 && to_bits > from_bits;
	case ConvertOp::Trunc:
		return to_bits != 0 && from_bits != 0 && to_bits < from_bits;
	case ConvertOp::SIToFP:
	case ConvertOp::UIToFP:
		return to_rank != 0 && from_bits != 0;
	case ConvertOp::FPToSI:
	case ConvertOp::FPToUI:
		return to_bits != 0 && from_rank != 0;
	case ConvertOp::FPExt:
		return to_rank != 0 && from_rank != 0 && to_rank > from_rank;
	case ConvertOp::FPTrunc:
		return to_rank != 0 && from_rank != 0 && to_rank < from_rank;
	}
	return false;
}

// Operand kinds as bit sets (1 << ValueKind).
const uint8_t kNoOperand = 1u << static_cast<unsigned>(ValueKind::None);
const uint8_t kTempOperand = 1u << static_cast<unsigned>(ValueKind::Temp);
const uint8_t kSlotOperand = 1u << static_cast<unsigned>(ValueKind::Slot);
const uint8_t kSymbolOperand = 1u << static_cast<unsigned>(ValueKind::Symbol);
const uint8_t kBlockOperand = 1u << static_cast<unsigned>(ValueKind::Block);
const uint8_t kLiteralOperand = 1u << static_cast<unsigned>(ValueKind::Integer) |
	1u << static_cast<unsigned>(ValueKind::Float);
const uint8_t kValueOperand = kTempOperand | kSlotOperand | kSymbolOperand | kLiteralOperand;
// Storage operands: a name, or an integer address.
const uint8_t kStorageOperand = kTempOperand | kSlotOperand | kSymbolOperand |
	1u << static_cast<unsigned>(ValueKind::Integer);

enum class ExtraOperands : uint8_t
{
	None,
	Values,
	Symbols,
	PhiPairs, // (Block, value); kinds are check_phi's
	CasePairs // (value, Block)
};

struct OperandRule
{
	uint8_t ops[3];
	ExtraOperands extra;
	// Instruction::sub is below this: the operator, projection, failure
	// order or alignment log2 (1: unused, 0).
	uint8_t sub_limit;
};

// The operand kinds of each opcode (lowir/instruction.h), in Opcode order.
const OperandRule kOperandRules[] = {
	{ { kLiteralOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                        // Const
	{ { kValueOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                          // Copy
	{ { kNoOperand, kNoOperand, kNoOperand }, ExtraOperands::PhiPairs, 1 },                         // Phi
	{ { kSlotOperand | kSymbolOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },          // Addr
	{ { kStorageOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                        // Load
	{ { kValueOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                          // AtomicLoad
	{ { kValueOperand, kValueOperand, kNoOperand }, ExtraOperands::None, ir_model::IPK_FIELD + 1 }, // Index
	{ { kValueOperand, kNoOperand, kNoOperand }, ExtraOperands::None, kUnaryOpCount },              // Unary
	{ { kValueOperand, kValueOperand, kNoOperand }, ExtraOperands::None, kBinaryOpCount },          // Binary
	{ { kValueOperand, kValueOperand, kNoOperand }, ExtraOperands::None, kComparePredicateCount },  // Compare
	{ { kValueOperand, kValueOperand, kValueOperand }, ExtraOperands::None, 1 },                    // Select
	{ { kValueOperand, kNoOperand, kNoOperand }, ExtraOperands::None, kConvertOpCount },            // Convert
	{ { kValueOperand, kValueOperand, kNoOperand }, ExtraOperands::None, 1 },                       // AtomicAddFetch
	{ { kValueOperand, kValueOperand, kNoOperand }, ExtraOperands::None, 1 },                       // AtomicExchange
	{ { kValueOperand, kValueOperand, kValueOperand }, ExtraOperands::None,
		kMemoryOrderCount },                                                                 // AtomicCompareExchange
	{ { kValueOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                   // VaArg
	{ { kValueOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                   // StackAlloc
	{ { kTempOperand | kSymbolOperand, kNoOperand, kNoOperand }, ExtraOperands::Values, 1 }, // Call
	{ { kNoOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                      // Exception
	{ { kNoOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                      // ExceptionSelector
	{ { kValueOperand, kStorageOperand, kNoOperand }, ExtraOperands::None, 1 },              // Store
	{ { kValueOperand, kValueOperand, kNoOperand }, ExtraOperands::None, 1 },                // AtomicStore
	{ { kNoOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                      // AtomicThreadFence
	{ { kNoOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                      // AtomicSignalFence
	{ { kValueOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                   // VaStart
	{ { kValueOperand, kValueOperand, kNoOperand }, ExtraOperands::None, 64 },               // CopyObject
	{ { kValueOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 64 },                  // ZeroInit
	{ { kBlockOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                   // EhTry
	{ { kBlockOperand | kNoOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },      // EhCleanup
	{ { kSymbolOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                  // EhCatch
	{ { kNoOperand, kNoOperand, kNoOperand }, ExtraOperands::Symbols, 1 },                   // EhFilter
	{ { kNoOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                      // EhCatchAll
	{ { kNoOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                      // EhEnd
	{ { kValueOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                   // Throw
	{ { kNoOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                      // Resume
	{ { kBlockOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                   // Jump
	{ { kValueOperand, kBlockOperand, kBlockOperand }, ExtraOperands::None, 1 },             // Branch
	{ { kValueOperand, kBlockOperand, kNoOperand }, ExtraOperands::CasePairs, 1 },           // Switch
	{ { kValueOperand | kNoOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },      // Return
	{ { kNoOperand, kNoOperand, kNoOperand }, ExtraOperands::None, 1 },                      // Unreachable
};

static_assert(sizeof kOperandRules / sizeof kOperandRules[0] == static_cast<unsigned>(Opcode::Unreachable) + 1,
	"one operand rule per opcode");

// The type of an instruction's result temporary: a comparison's truth
// value is i64, an address is ptr, otherwise the instruction's type.
Type result_type(const Instruction& inst)
{
	switch (inst.op)
	{
	case Opcode::Compare:
	case Opcode::AtomicCompareExchange:
	case Opcode::ExceptionSelector:
		return Type::scalar(TypeKind::I64);
	case Opcode::Addr:
	case Opcode::Index:
	case Opcode::StackAlloc:
		return Type::scalar(TypeKind::Ptr);
	default:
		return inst.type;
	}
}

class Validator
{
public:
	Validator(const Program& program, Diagnostic& diagnostic) : program_(program), diagnostic_(diagnostic) {}

	bool run();
	// What the text reader's grammar guarantees, for constructed programs:
	// cross-link ids, operand kinds and ids, instruction results.
	bool check_construction(bool spellings);

private:
	bool fail(const char* message, NameId detail = 0);
	bool check_links();
	// Named entries print as their names and unnamed ones as their index
	// (the writer's fallback); fails with `message` if two of the `count`
	// entries named by name_of(i) would print alike.
	template<typename NameOf>
	bool check_spellings(uint32_t count, NameOf name_of, const char* message);
	bool check_symbols();
	bool check_function(SymbolId symbol);
	bool check_layout();
	bool check_operands(InstId id, const Instruction& inst);
	bool is_operand(Value value, uint8_t kinds) const
	{
		// Branch free: the kind is allowed and its id below the body's count.
		const unsigned kind = static_cast<unsigned>(value.kind);
		return ((kinds >> kind) & 1u) != 0 && value.index < id_limit_[kind] &&
			(value.kind != ValueKind::Float || float_rank(value.float_kind) != 0);
	}
	bool check_targets();
	bool check_instruction(InstId id, const Instruction& inst, BlockId block);
	bool check_uses(InstId id, const Instruction& inst);
	bool check_phi(const Instruction& inst, BlockId block);
	bool check_call(const Instruction& inst);
	bool check_return(const Instruction& inst);
	// The type of a temporary, slot or symbol operand; false for others.
	bool value_type(Value value, Type& type) const;
	bool add_edge(BlockId from, Value target);

	const Program& program_;
	Diagnostic& diagnostic_;
	// The function being checked.
	SymbolId function_ = kNoId;
	InstId inst_ = kNoId;
	const FunctionBody* body_ = nullptr;
	const Signature* signature_ = nullptr;
	// Layout position of each instruction.
	std::vector<uint32_t> position_;
	std::vector<uint32_t> block_flags_;
	// Distinct ordinary control-flow edges (to, from), and each block's
	// predecessors: preds_[pred_first_[b], pred_first_[b + 1]).
	std::vector<std::pair<BlockId, BlockId>> edges_;
	std::vector<uint32_t> pred_first_;
	std::vector<BlockId> preds_;
	std::vector<uint32_t> fill_;
	std::vector<uint32_t> marks_;
	uint32_t mark_ = 0;
	// Operand ids of each ValueKind are below id_limit_[kind] (None has id 0).
	uint32_t id_limit_[8] = {};
	// Names seen in the current spelling check, by NameId.
	std::vector<uint32_t> name_marks_;
	uint32_t name_epoch_ = 0;
};

bool Validator::fail(const char* message, NameId detail)
{
	diagnostic_.message = message;
	diagnostic_.detail = detail;
	diagnostic_.function = function_;
	diagnostic_.inst = inst_;
	return false;
}

bool Validator::run()
{
	if (!check_symbols())
		return false;
	for (SymbolId symbol = 0; symbol < program_.symbols.size(); ++symbol)
	{
		if (program_.symbols[symbol].kind == SymbolKind::FunctionDefinition && !check_function(symbol))
			return false;
	}
	return true;
}

bool Validator::check_symbols()
{
	std::vector<SymbolId> role_owner(ir_model::SR_RTTI_POINTER + 1, kNoId);
	std::vector<bool> wrapped(program_.symbols.size(), false);
	for (SymbolId id = 0; id < program_.symbols.size(); ++id)
	{
		const Symbol& symbol = program_.symbols[id];
		const SymbolMetadata& metadata = symbol.metadata;
		if (symbol.kind == SymbolKind::Unresolved)
			return fail("undefined top-level symbol", symbol.name);
		const bool function = is_function_kind(symbol.kind);
		if (metadata.role != ir_model::SR_NONE)
		{
			if (is_global_role(metadata.role) == function)
				return fail(function ? "invalid symbol role" : "function role on global", symbol.name);
			if (is_singleton_role(metadata.role) && role_owner[metadata.role] != kNoId)
				return fail("duplicate singleton role", symbol.name);
			role_owner[metadata.role] = id;
			if (metadata.role == ir_model::SR_ENTRY && symbol.kind != SymbolKind::FunctionDefinition)
				return fail("entry role requires a function definition", symbol.name);
		}
		if (function && (metadata.storage != ir_model::GSM_DEFAULT || metadata.section != 0))
			return fail("storage metadata on non-global", symbol.name);
		if (metadata.tls_for != kNoId)
		{
			const Symbol& target = program_.symbols[metadata.tls_for];
			if (!function)
				return fail("tls_for metadata requires a function", symbol.name);
			if (!is_global_kind(target.kind) || target.metadata.storage != ir_model::GSM_THREAD_LOCAL)
				return fail("tls_for target is not thread-local global", target.name);
			if (wrapped[metadata.tls_for])
				return fail("duplicate tls wrapper", target.name);
			wrapped[metadata.tls_for] = true;
		}
	}
	std::vector<bool> alias_seen(program_.names.size(), false);
	for (const ObjectAlias& alias : program_.aliases)
	{
		if (alias_seen[alias.object])
			return fail("duplicate object alias", alias.object);
		alias_seen[alias.object] = true;
	}
	return true;
}

bool Validator::check_function(SymbolId symbol)
{
	function_ = symbol;
	inst_ = kNoId;
	const Function& function = program_.functions[program_.symbols[symbol].detail];
	body_ = function.body.get();
	signature_ = &program_.signatures[function.signature];
	if (!check_layout() || !check_targets())
		return false;
	for (TempId temp = body_->parameter_count; temp < body_->temps.size(); ++temp)
	{
		if (body_->temps[temp].def == kNoId)
			return fail("undefined temporary", body_->temps[temp].name);
	}
	for (BlockId block : body_->layout)
	{
		// Phis precede the block's ordinary instructions.
		bool ordinary = false;
		for (InstId id = body_->blocks[block].first; id != kNoId; id = body_->instructions[id].next)
		{
			inst_ = id;
			const Instruction& inst = body_->instructions[id];
			if (inst.op == Opcode::Phi && ordinary)
				return fail("phi instructions must precede ordinary instructions");
			ordinary = ordinary || inst.op != Opcode::Phi;
			if (!check_instruction(id, inst, block))
				return false;
		}
	}
	inst_ = kNoId;
	return true;
}

bool Validator::check_layout()
{
	if (body_->layout.empty())
		return fail("function has no blocks");
	block_flags_.assign(body_->blocks.size(), 0);
	position_.assign(body_->instructions.size(), kNoId);
	uint32_t position = 0;
	for (BlockId id : body_->layout)
	{
		const Block& block = body_->blocks[id];
		if ((block_flags_[id] & kPlacedBlock) != 0)
			return fail("duplicate block", block.name);
		block_flags_[id] |= kPlacedBlock;
		if (block.last == kNoId || !program_.may_end_block(body_->instructions[block.last]))
			return fail("block has no terminator", block.name);
		for (InstId inst = block.first; inst != block.last; inst = body_->instructions[inst].next)
		{
			position_[inst] = position++;
			if (is_terminator(body_->instructions[inst].op))
			{
				inst_ = body_->instructions[inst].next;
				return fail("instruction after terminator");
			}
		}
		position_[block.last] = position++;
	}
	return true;
}

bool Validator::check_links()
{
	// Ids stored outside function bodies.
	const uint64_t symbols = program_.symbols.size();
	for (const Signature& signature : program_.signatures)
	{
		const BoundaryMetadata& boundary = signature.boundary;
		if (static_cast<uint64_t>(signature.first_parameter) + signature.parameter_count > program_.parameters.size() ||
			boundary.arity >= kAritySpellings.count || boundary.effects >= kEffectsSpellings.count ||
			boundary.unwind >= kUnwindSpellings.count || boundary.returns >= kReturnSpellings.count ||
			boundary.query >= kQuerySpellings.count)
			return fail("invalid signature");
		const Parameter* parameters = program_.parameters_of(signature);
		for (uint32_t i = 0; i < signature.parameter_count; ++i)
		{
			if (parameters[i].pass >= kPassSpellings.count || parameters[i].alias >= kAliasSpellings.count)
				return fail("invalid parameter metadata", parameters[i].name);
		}
	}
	for (const Symbol& symbol : program_.symbols)
	{
		const SymbolMetadata& metadata = symbol.metadata;
		if (symbol.name == 0 || symbol.name >= program_.names.size())
			return fail("invalid symbol name");
		if (metadata.role >= kRoleSpellings.count || metadata.linkage >= kLinkageSpellings.count ||
			metadata.binding >= kBindingSpellings.count || metadata.storage >= kStorageSpellings.count ||
			(metadata.flags & ~SF_REPLACEABLE) >= 1u << (static_cast<unsigned>(MetadataKey::NoInline) -
													  static_cast<unsigned>(MetadataKey::KeepAlias) + 1))
			return fail("invalid symbol metadata", symbol.name);
		if (metadata.tls_for != kNoId && metadata.tls_for >= symbols)
			return fail("invalid tls_for target", symbol.name);
		if ((metadata.flags & SF_REPLACEABLE) != 0 && metadata.binding != ir_model::SBM_WEAK)
			return fail("replaceable metadata requires binding=weak", symbol.name);
		if (is_function_kind(symbol.kind) && program_.functions[symbol.detail].signature >= program_.signatures.size())
			return fail("invalid function signature", symbol.name);
		if (!is_global_kind(symbol.kind))
			continue;
		const Global& global = program_.globals[symbol.detail];
		if ((global.init == GlobalInit::Address && global.address >= symbols) ||
			(global.init == GlobalInit::Data &&
				static_cast<uint64_t>(global.first_item) + global.item_count > program_.data_items.size()))
			return fail("invalid global initializer", symbol.name);
		if (global.init == GlobalInit::Data && global.typed && !global.type.is_object())
			return fail("a structured global's storage type must be an object type", symbol.name);
		for (uint32_t i = 0; global.init == GlobalInit::Data && i < global.item_count; ++i)
		{
			const DataItem& item = program_.data_items[global.first_item + i];
			if (item.kind == DataItemKind::Address && item.address >= symbols)
				return fail("invalid global initializer", symbol.name);
		}
	}
	for (const ObjectAlias& alias : program_.aliases)
	{
		if (alias.object == 0 || alias.object >= program_.names.size() || alias.target >= symbols)
			return fail("invalid object alias");
	}
	return true;
}

template<typename NameOf>
bool Validator::check_spellings(uint32_t count, NameOf name_of, const char* message)
{
	++name_epoch_;
	for (uint32_t i = 0; i < count; ++i)
	{
		const NameId name = name_of(i);
		if (name >= name_marks_.size())
			return fail("invalid name");
		if (name != 0 && name_marks_[name] == name_epoch_)
			return fail(message, name);
		name_marks_[name] = name_epoch_;
	}
	// (Unnamed entities print free of these names: lowir/local_names.h.)
	return true;
}

bool Validator::check_construction(bool spellings)
{
	if (!check_links())
		return false;
	name_marks_.assign(program_.names.size(), 0);
	name_epoch_ = 0;
	for (const Signature& signature : program_.signatures)
	{
		const Parameter* parameters = program_.parameters_of(signature);
		if (!check_spellings(
				signature.parameter_count, [&](uint32_t i) { return parameters[i].name; }, "duplicate parameter"))
			return false;
	}
	for (SymbolId symbol = 0; symbol < program_.symbols.size(); ++symbol)
	{
		if (program_.symbols[symbol].kind != SymbolKind::FunctionDefinition)
			continue;
		function_ = symbol;
		body_ = program_.functions[program_.symbols[symbol].detail].body.get();
		if (spellings &&
			(!check_spellings(
				 static_cast<uint32_t>(body_->temps.size()),
				 [&](uint32_t i) { return body_->temps[i].kind == TempKind::Plain ? body_->temps[i].name : 0; },
				 "duplicate temporary name") ||
				!check_spellings(
					static_cast<uint32_t>(body_->slots.size()),
					[&](uint32_t i) { return body_->slots[i].kind == SlotKind::Plain ? body_->slots[i].name : 0; },
					"duplicate slot") ||
				!check_spellings(
					static_cast<uint32_t>(body_->blocks.size()),
					[&](uint32_t i) { return body_->blocks[i].kind == BlockKind::Plain ? body_->blocks[i].name : 0; },
					"duplicate block")))
			return false;
		const uint32_t literals = static_cast<uint32_t>(body_->literals.size());
		const uint32_t limits[] = { 1, static_cast<uint32_t>(body_->temps.size()),
			static_cast<uint32_t>(body_->slots.size()), static_cast<uint32_t>(program_.symbols.size()),
			static_cast<uint32_t>(body_->blocks.size()), literals, literals };
		static_assert(
			sizeof limits / sizeof limits[0] == static_cast<unsigned>(ValueKind::Float) + 1, "one limit per kind");
		std::memcpy(id_limit_, limits, sizeof limits);
		for (BlockId block : body_->layout)
		{
			if (block >= body_->blocks.size())
				return fail("undefined or invalid block target");
			for (InstId id = body_->blocks[block].first; id != kNoId; id = body_->instructions[id].next)
			{
				inst_ = id;
				if (!check_operands(id, body_->instructions[id]))
					return false;
			}
		}
		inst_ = kNoId;
	}
	function_ = kNoId;
	return true;
}

bool Validator::check_operands(InstId id, const Instruction& inst)
{
	const OperandRule& rule = kOperandRules[static_cast<unsigned>(inst.op)];
	for (int i = 0; i < 3; ++i)
	{
		if (!is_operand(inst.ops[i], rule.ops[i]))
			return fail("invalid operand for the instruction");
	}
	if (static_cast<uint64_t>(inst.extra_first) + inst.extra_count > body_->operands.size() ||
		(rule.extra == ExtraOperands::None && inst.extra_count != 0) ||
		(rule.extra == ExtraOperands::CasePairs && inst.extra_count % 2 != 0))
		return fail("invalid operand for the instruction");
	const Value* extra = body_->operands.data() + inst.extra_first;
	for (uint32_t i = 0; i < inst.extra_count; ++i)
	{
		const uint8_t kinds = rule.extra == ExtraOperands::Values ? kValueOperand
			: rule.extra == ExtraOperands::Symbols                ? kSymbolOperand
			: rule.extra == ExtraOperands::CasePairs              ? (i % 2 == 0 ? kValueOperand : kBlockOperand)
																  : 0xff;
		if (!is_operand(extra[i], kinds))
			return fail("invalid operand for the instruction");
	}
	if (inst.sub >= rule.sub_limit || static_cast<unsigned>(inst.order) >= kMemoryOrderCount ||
		inst.location > body_->locations.size() ||
		((inst.op == Opcode::CopyObject || inst.op == Opcode::ZeroInit) && inst.immediate == 0))
		return fail("invalid instruction field");
	if ((inst.flags & IF_SIGNATURE) != 0 &&
		(inst.op != Opcode::Call || static_cast<uint64_t>(inst.immediate) >= program_.signatures.size()))
		return fail("invalid call signature");
	// Value instructions define one temporary of their result type; a void
	// call and the effects define none.
	const bool defines = inst.op <= Opcode::ExceptionSelector && (inst.op != Opcode::Call || !inst.type.is_void());
	if (!defines)
		return inst.result == kNoId || fail("a void instruction has no result");
	if (inst.result >= body_->temps.size() || body_->temps[inst.result].def != id)
		return fail("value instruction without its result temporary");
	const Type type = result_type(inst);
	if (type.is_void())
		return fail("a void instruction has no result");
	if (body_->temps[inst.result].type != type)
		return fail("result temporary type mismatch", body_->temps[inst.result].name);
	return true;
}

bool Validator::add_edge(BlockId from, Value target)
{
	if (!target.is(ValueKind::Block) || target.index >= body_->blocks.size() ||
		(block_flags_[target.index] & kPlacedBlock) == 0)
		return fail("undefined or invalid block target");
	if (marks_[target.index] != mark_)
	{
		marks_[target.index] = mark_;
		edges_.push_back(std::make_pair(target.index, from));
	}
	return true;
}

bool Validator::check_targets()
{
	// Ordinary edges come from terminators; eh_try and eh_cleanup name
	// handler blocks, which are not ordinary successors.
	edges_.clear();
	marks_.assign(body_->blocks.size(), 0);
	mark_ = 0;
	for (BlockId block : body_->layout)
	{
		++mark_;
		for (InstId id = body_->blocks[block].first; id != kNoId; id = body_->instructions[id].next)
		{
			inst_ = id;
			const Instruction& inst = body_->instructions[id];
			if ((inst.op == Opcode::EhTry || (inst.op == Opcode::EhCleanup && !inst.ops[0].is(ValueKind::None))))
			{
				if (!inst.ops[0].is(ValueKind::Block) || (block_flags_[inst.ops[0].index] & kPlacedBlock) == 0)
					return fail("undefined or invalid block target");
				block_flags_[inst.ops[0].index] |= kHandlerBlock;
			}
			else if (inst.op == Opcode::Jump && !add_edge(block, inst.ops[0]))
				return false;
			else if (inst.op == Opcode::Branch && (!add_edge(block, inst.ops[1]) || !add_edge(block, inst.ops[2])))
				return false;
			else if (inst.op == Opcode::Switch)
			{
				if (!add_edge(block, inst.ops[1]))
					return false;
				for (uint32_t i = 1; i < inst.extra_count; i += 2)
				{
					if (!add_edge(block, body_->operands[inst.extra_first + i]))
						return false;
				}
			}
		}
	}
	inst_ = kNoId;
	pred_first_.assign(body_->blocks.size() + 1, 0);
	for (const std::pair<BlockId, BlockId>& edge : edges_)
		++pred_first_[edge.first + 1];
	for (size_t i = 1; i < pred_first_.size(); ++i)
		pred_first_[i] += pred_first_[i - 1];
	preds_.assign(edges_.size(), 0);
	fill_.assign(pred_first_.begin(), pred_first_.end() - 1);
	for (const std::pair<BlockId, BlockId>& edge : edges_)
		preds_[fill_[edge.first]++] = edge.second;
	return true;
}

bool Validator::value_type(Value value, Type& type) const
{
	switch (value.kind)
	{
	case ValueKind::Temp:
		type = body_->temps[value.index].type;
		return true;
	case ValueKind::Slot:
		type = body_->slots[value.index].type;
		return true;
	case ValueKind::Symbol:
		type = Type::scalar(TypeKind::Ptr);
		return true;
	default:
		return false;
	}
}

bool Validator::check_uses(InstId id, const Instruction& inst)
{
	// A temporary is defined before its ordinary uses in layout order;
	// phi uses name values of predecessors and may come first.
	const Value* extra = inst.extra_count != 0 ? &body_->operands[inst.extra_first] : nullptr;
	const uint32_t count = 3 + (inst.op == Opcode::Phi ? 0 : inst.extra_count);
	for (uint32_t i = 0; i < count; ++i)
	{
		const Value value = i < 3 ? inst.ops[i] : extra[i - 3];
		if (!value.is(ValueKind::Temp) || value.index < body_->parameter_count)
			continue;
		const Temp& temp = body_->temps[value.index];
		if (position_[temp.def] >= position_[id])
			return fail("undefined temporary", temp.name);
	}
	return true;
}

bool Validator::check_instruction(InstId id, const Instruction& inst, BlockId block)
{
	if (!check_uses(id, inst))
		return false;
	if (const char* fact = misplaced_fact(inst))
		return fail(fact);
	Type type = Type::scalar(TypeKind::Void);
	switch (inst.op)
	{
	case Opcode::Phi:
		return check_phi(inst, block);
	case Opcode::Unary:
		if (static_cast<UnaryOp>(inst.sub) == UnaryOp::ByteSwap && !is_bswap_type(inst.type))
			return fail("invalid bswap type");
		if ((static_cast<UnaryOp>(inst.sub) == UnaryOp::CountTrailingZeros ||
				static_cast<UnaryOp>(inst.sub) == UnaryOp::CountLeadingZeros) &&
			!is_bit_scan_type(inst.type))
			return fail("invalid ctz/clz type");
		return true;
	case Opcode::Compare:
		// Register-width truth and selector comparisons may read a value of
		// another width; a wide (i128) one, a member pointer's carrier say,
		// is compared at its own type (its target word extracted first).
		for (unsigned i = 0; i < 2; ++i)
		{
			if (value_type(inst.ops[i], type) && type.kind() == TypeKind::I128 && type != inst.type)
				return fail("comparison operand type mismatch");
		}
		return true;
	case Opcode::Select:
		if (!(inst.type.is_integer() || inst.type.is_pointer()) || inst.type.kind() == TypeKind::I128)
			return fail("invalid select type");
		return true;
	case Opcode::Convert:
		if (!is_valid_conversion(static_cast<ConvertOp>(inst.sub), inst.type, convert_source_type(inst)))
			return fail("invalid conversion widths or categories");
		return true;
	case Opcode::VaStart:
		if (signature_->boundary.arity != ir_model::CAM_VARIADIC)
			return fail("va_start requires a variadic function");
		if (!value_type(inst.ops[0], type) || !type.is_pointer())
			return fail("va_start requires a pointer value");
		return true;
	case Opcode::VaArg:
		if (!value_type(inst.ops[0], type) || !type.is_pointer())
			return fail("va_arg requires a pointer value");
		// A host object's program also reads an object, a long double or an
		// i128 (psABI 3.5.7; the toolchain's own LowIR, see
		// Program::object_facts).
		if (!is_va_arg_type(inst.type) &&
			!(program_.object_facts &&
				(inst.type.is_object() || inst.type.kind() == TypeKind::F80 || inst.type.kind() == TypeKind::I128)))
			return fail("va_arg scalar type is not supported");
		return true;
	case Opcode::StackAlloc:
		if (!inst.ops[0].is(ValueKind::Integer) && (!value_type(inst.ops[0], type) || !type.is_integer()))
			return fail("stack_alloc requires an integer size");
		return true;
	case Opcode::Call:
		return check_call(inst);
	case Opcode::Return:
		return check_return(inst);
	default:
		return true;
	}
}

bool Validator::check_phi(const Instruction& inst, BlockId block)
{
	if ((block_flags_[block] & kHandlerBlock) != 0)
		return fail("phi is not permitted in an exception handler block");
	if (!is_phi_type(inst.type))
		return fail("phi requires a directly representable scalar type");
	if (inst.extra_count == 0 || inst.extra_count % 2 != 0)
		return fail("phi requires predecessor/value pairs");
	const uint32_t predecessors = mark_ + 1;
	const uint32_t seen = mark_ + 2;
	mark_ += 2;
	for (uint32_t i = pred_first_[block]; i < pred_first_[block + 1]; ++i)
		marks_[preds_[i]] = predecessors;
	const Value* pairs = &body_->operands[inst.extra_first];
	for (uint32_t i = 0; i < inst.extra_count; i += 2)
	{
		if (!pairs[i].is(ValueKind::Block) || pairs[i].index >= body_->blocks.size())
			return fail("phi requires predecessor/value pairs");
		const BlockId from = pairs[i].index;
		if (marks_[from] == seen)
			return fail("duplicate phi predecessor", body_->blocks[from].name);
		if (marks_[from] != predecessors)
			return fail("phi label is not a predecessor of its block", body_->blocks[from].name);
		marks_[from] = seen;
		const Value value = pairs[i + 1];
		switch (value.kind)
		{
		case ValueKind::Temp:
			if (body_->temps[value.index].type != inst.type)
				return fail("phi incoming temporary type mismatch", body_->temps[value.index].name);
			break;
		case ValueKind::Integer:
			if (!inst.type.is_integer() && !inst.type.is_pointer())
				return fail("phi integer literal type mismatch");
			break;
		case ValueKind::Float:
			if (value.float_kind != inst.type.kind())
				return fail("phi floating literal type mismatch");
			break;
		case ValueKind::Symbol:
			if (!inst.type.is_pointer())
				return fail("phi global operand requires ptr type");
			break;
		default:
			return fail("phi incoming operand is not a scalar value");
		}
	}
	if (inst.extra_count / 2 != pred_first_[block + 1] - pred_first_[block])
		return fail("phi must name every ordinary predecessor exactly once");
	return true;
}

bool Validator::check_call(const Instruction& inst)
{
	const Value callee = inst.ops[0];
	const bool direct = callee.is(ValueKind::Symbol) && is_function_kind(program_.symbols[callee.index].kind);
	const Signature* signature = program_.call_boundary(inst);
	if ((inst.flags & IF_SIGNATURE) != 0)
	{
		if (inst.type != signature->result)
			return fail("call signature return mismatch");
	}
	else if (!direct)
		return fail("indirect call requires signature");
	else if (inst.type != signature->result)
		return fail("direct call return mismatch");
	const uint32_t count = inst.extra_count;
	const uint32_t parameters = signature->parameter_count;
	if (signature->boundary.arity == ir_model::CAM_FIXED ? count != parameters : count < parameters)
		return fail("call arity mismatch");
	// A slot argument passes the slot's value, of the slot's type, except
	// to an indirect_result or by_address parameter, which takes its storage.
	const Parameter* parameter = program_.parameters_of(*signature);
	for (uint32_t i = 0; i < parameters; ++i)
	{
		const Value argument = body_->operands[inst.extra_first + i];
		if (argument.is(ValueKind::Slot) && parameter[i].pass == ir_model::PPM_DIRECT &&
			body_->slots[argument.index].type != parameter[i].type)
			return fail("call argument slot type does not match the parameter type", body_->slots[argument.index].name);
	}
	if ((inst.flags & IF_COPY_ELISION) != 0)
	{
		if (!direct)
			return fail("copy elision requires a direct call");
		if (!inst.type.is_void() || count < 2)
			return fail("copy elision requires a direct void call with destination and source");
	}
	return true;
}

bool Validator::check_return(const Instruction& inst)
{
	const bool has_value = !inst.ops[0].is(ValueKind::None);
	if (inst.type != signature_->result || has_value == inst.type.is_void())
		return fail("return type does not match function");
	return true;
}

} // namespace

bool validate_program(const Program& program, Diagnostic& diagnostic)
{
	Validator validator(program, diagnostic);
	return validator.run();
}

bool validate_constructed_program(const Program& program, Diagnostic& diagnostic, bool spellings)
{
	Validator validator(program, diagnostic);
	return validator.check_construction(spellings) && validator.run();
}

std::string describe(const Program& program, const Diagnostic& diagnostic)
{
	std::string text = diagnostic.message;
	if (diagnostic.detail != 0)
		text += std::string(": ") + program.spelling(diagnostic.detail);
	if (diagnostic.function != kNoId)
		text += std::string(" in function @") + program.spelling(program.symbols[diagnostic.function].name);
	return text;
}

void apply_legacy_roles(Program& program)
{
	struct Legacy
	{
		uint8_t role;
		const char* name;
	};
	const Legacy legacy[] = { { ir_model::SR_ENTRY, "main" }, { ir_model::SR_INIT, "__cppgm_init" },
		{ ir_model::SR_FINI, "__cppgm_fini" } };
	uint32_t taken_roles = 0;
	for (const Symbol& symbol : program.symbols)
	{
		if (symbol.metadata.role <= ir_model::SR_FINI)
			taken_roles |= 1u << symbol.metadata.role;
	}
	// A program with an explicit entry follows the role convention; the
	// legacy names are ordinary names there.
	if ((taken_roles & (1u << ir_model::SR_ENTRY)) != 0)
		return;
	for (const Legacy& entry : legacy)
	{
		const bool taken = (taken_roles & (1u << entry.role)) != 0;
		const SymbolId id = program.find_symbol(program.names.find(entry.name, std::strlen(entry.name)));
		if (taken || id == kNoId)
			continue;
		Symbol& symbol = program.symbols[id];
		if (symbol.kind == SymbolKind::FunctionDefinition && symbol.metadata.role == ir_model::SR_NONE)
			symbol.metadata.role = entry.role;
	}
}

} // namespace lowir
