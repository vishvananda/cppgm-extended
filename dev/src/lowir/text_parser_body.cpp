#include "lowir/text_parser.h"

#include <cstring>

namespace lowir {
namespace text {

namespace {

// block_uses_ value of a block whose label has been read.
const uint32_t kPlaced = 0xffffffffu;

const Type kVoid = Type::scalar(TypeKind::Void);
const Type kI64 = Type::scalar(TypeKind::I64);
const Type kPtr = Type::scalar(TypeKind::Ptr);

} // namespace

// ---------------------------------------------------------------------------
// Local names

Parser::LocalName& Parser::local(NameId name)
{
	if (name >= locals_.size())
	{
		const uint32_t size = program_.names.size() > name ? program_.names.size() : name + 1;
		locals_.resize(size, LocalName{ 0, kNoId, kNoId, kNoId });
	}
	LocalName& entry = locals_[name];
	if (entry.epoch != epoch_)
		entry = LocalName{ epoch_, kNoId, kNoId, kNoId };
	return entry;
}

TempId Parser::use_temp(FunctionBuilder& builder, NameId name)
{
	LocalName& entry = local(name);
	if (entry.temp == kNoId)
	{
		entry.temp = builder.add_temp(name);
		temp_uses_.push_back(token().line);
	}
	return entry.temp;
}

TempId Parser::define_temp(FunctionBuilder& builder, NameId name)
{
	LocalName& entry = local(name);
	if (entry.temp == kNoId)
	{
		entry.temp = builder.add_temp(name);
		temp_uses_.push_back(token().line);
		return entry.temp;
	}
	const FunctionBody& body = builder.body();
	if (entry.temp < body.parameter_count || body.temps[entry.temp].def != kNoId)
		fail("duplicate temporary definition");
	return entry.temp;
}

SlotId Parser::use_slot(NameId name)
{
	const SlotId slot = local(name).slot;
	if (slot == kNoId)
		fail(std::string("undefined slot: ") + program_.spelling(name));
	return slot;
}

BlockId Parser::use_block(FunctionBuilder& builder, NameId name)
{
	LocalName& entry = local(name);
	if (entry.block == kNoId)
	{
		entry.block = builder.add_block(name);
		block_uses_.push_back(token().line);
	}
	return entry.block;
}

BlockId Parser::expect_block(FunctionBuilder& builder)
{
	if (!at(TokenKind::Block))
		fail("undefined or invalid block target");
	const BlockId block = use_block(builder, token().name);
	advance();
	return block;
}

// ---------------------------------------------------------------------------
// Bodies

void Parser::parse_body(SymbolId function)
{
	FunctionBuilder builder(program_, function);
	FunctionBody& body = builder.body();
	++epoch_;
	temp_uses_.assign(body.temps.size(), 0);
	block_uses_.clear();
	terminated_ = false;
	body_lines_.push_back(BodyLines{ function, source_, static_cast<uint32_t>(inst_lines_.size()) });
	for (TempId temp = 0; temp < body.parameter_count; ++temp)
		local(body.temps[temp].name).temp = temp;
	while (!accept(TokenKind::RBrace))
	{
		if (at(TokenKind::End))
			fail("unexpected end of LowIR");
		if (accept_keyword(Keyword::Slot))
		{
			if (!body.layout.empty())
				fail("slot declaration after first block");
			parse_slot(builder);
		}
		else if (accept_keyword(Keyword::Block))
			parse_block_label(builder);
		else
		{
			if (body.layout.empty())
				fail("instruction outside block");
			if (terminated_)
				fail("instruction after terminator");
			parse_instruction(builder);
		}
	}
	finish_body(builder, function);
	builder.finish();
}

void Parser::parse_slot(FunctionBuilder& builder)
{
	const NameId name = expect_name(TokenKind::Slot, "slot name");
	expect(TokenKind::Colon, ":");
	const Type type = parse_type();
	LocalName& entry = local(name);
	if (entry.slot != kNoId)
		fail("duplicate slot");
	entry.slot = builder.add_slot(name, type);
}

void Parser::parse_block_label(FunctionBuilder& builder)
{
	FunctionBody& body = builder.body();
	if (!body.layout.empty() && lacks_end(body))
		fail(std::string("block has no terminator: ") + program_.spelling(body.blocks[body.layout.back()].name));
	const NameId name = expect_name(TokenKind::Block, "block name");
	const BlockId block = use_block(builder, name);
	if (block_uses_[block] == kPlaced)
		fail("duplicate block");
	block_uses_[block] = kPlaced;
	expect(TokenKind::Colon, ":");
	builder.place_block(block);
	builder.set_insertion_block(block);
	terminated_ = false;
}

bool Parser::lacks_end(const FunctionBody& body) const
{
	// A block ends in a terminator or in a call to a return=noreturn
	// boundary, whose callee may be declared later: the validator decides
	// for calls.
	const Block& block = body.blocks[body.layout.back()];
	return !terminated_ && (block.last == kNoId || body.instructions[block.last].op != Opcode::Call);
}

void Parser::finish_body(FunctionBuilder& builder, SymbolId function)
{
	(void)function;
	const FunctionBody& body = builder.body();
	if (body.layout.empty())
		fail("function has no blocks");
	if (lacks_end(body))
		fail(std::string("block has no terminator: ") + program_.spelling(body.blocks[body.layout.back()].name));
	for (BlockId block = 0; block < body.blocks.size(); ++block)
	{
		if (block_uses_[block] != kPlaced)
			fail_at(Position{ source_, block_uses_[block] }, "undefined or invalid block target");
	}
	for (TempId temp = body.parameter_count; temp < body.temps.size(); ++temp)
	{
		if (body.temps[temp].def == kNoId)
			fail_at(Position{ source_, temp_uses_[temp] },
				std::string("undefined temporary: ") + program_.spelling(body.temps[temp].name));
	}
}

// ---------------------------------------------------------------------------
// Operands

bool Parser::at_value_start() const
{
	switch (token().kind)
	{
	case TokenKind::Temp:
	case TokenKind::Slot:
	case TokenKind::Global:
	case TokenKind::Number:
	case TokenKind::Minus:
		return true;
	default:
		return word(token()).word_class == WordClass::NamedLiteral;
	}
}

Value Parser::parse_value(FunctionBuilder& builder, TypeKind context)
{
	Value value = Value::none();
	switch (token().kind)
	{
	case TokenKind::Temp:
		value = Value::temp(use_temp(builder, token().name));
		break;
	case TokenKind::Slot:
		value = Value::slot(use_slot(token().name));
		break;
	case TokenKind::Global:
		value = Value::symbol(reference_symbol(token().name));
		break;
	case TokenKind::Block:
		fail("block label used as value");
	default:
		if (!at_value_start())
			fail("expected value, got '" + spelling(token()) + "'");
		const Constant constant = parse_constant(context);
		return builder.literal(constant.kind, constant.float_kind, constant.bits);
	}
	advance();
	return value;
}

Value Parser::parse_storage(FunctionBuilder& builder)
{
	// A temporary, slot or global; producers also write a literal address
	// (`load ptr 0` on a path that cannot execute).
	return parse_value(builder, TypeKind::Ptr);
}

Value Parser::parse_callee(FunctionBuilder& builder)
{
	if (!at(TokenKind::Temp) && !at(TokenKind::Global))
		fail("expected a function, global or temporary callee, got '" + spelling(token()) + "'");
	return parse_value(builder, TypeKind::Void);
}

MemoryOrder Parser::parse_order()
{
	const int64_t order = parse_small_integer("memory order");
	if (order < 0 || order >= static_cast<int64_t>(kMemoryOrderCount))
		fail("invalid memory order");
	return static_cast<MemoryOrder>(order);
}

// ---------------------------------------------------------------------------
// Instructions

void Parser::parse_instruction(FunctionBuilder& builder)
{
	const uint32_t line = token().line;
	if (at(TokenKind::Temp))
	{
		const NameId name = token().name;
		advance();
		expect(TokenKind::Equals, "=");
		parse_rvalue(builder, define_temp(builder, name));
	}
	else
		parse_effect(builder);
	inst_lines_.push_back(line);
	if (at(TokenKind::Debug) || (at(TokenKind::Invalid) && *token().text == '!'))
		builder.set_location(static_cast<InstId>(builder.body().instructions.size() - 1), parse_debug_location());
}

void Parser::parse_rvalue(FunctionBuilder& builder, TempId result)
{
	const Token head = token();
	const WordInfo info = word(head);
	if (info.word_class != WordClass::Opcode)
		fail("unknown rvalue instruction: " + spelling(head));
	advance();
	const Opcode keyword = static_cast<Opcode>(info.value);
	Instruction inst = make_instruction(Opcode::Const, kVoid);
	Type type = kVoid;
	if (!parse_value_instruction(builder, keyword, inst, type) &&
		!parse_memory_instruction(builder, keyword, inst, type))
		fail_at(Position{ source_, head.line }, "unknown rvalue instruction: " + spelling(head));
	if (type.is_void())
		fail_at(Position{ source_, head.line }, "a void instruction has no result");
	const InstId id = builder.append(inst);
	builder.bind(result, type, id);
}

// vector <op> [operands], by the operation's shape (lowir_model::VectorShape).
void Parser::parse_vector(FunctionBuilder& builder, Instruction& inst, Type& result)
{
	const WordInfo info = word(token());
	if (info.word_class != WordClass::Vector)
		fail("unknown vector operation: " + spelling(token()));
	advance();
	const VectorOp op = static_cast<VectorOp>(info.value);
	const VectorShape shape = shape_of(op);
	inst.op = Opcode::Vector;
	inst.sub = info.value;
	inst.type = result = vector_result_type(op);
	const TypeKind v128 = TypeKind::V128;
	auto next = [&](TypeKind kind) {
		expect(TokenKind::Comma, ",");
		return parse_value(builder, kind);
	};
	switch (shape)
	{
	case VectorShape::Zero:
		break;
	case VectorShape::FromScalar:
		inst.ops[0] = parse_value(builder, vector_scalar_kind(op));
		break;
	case VectorShape::Unary:
	case VectorShape::Mask:
	case VectorShape::ToScalar:
		inst.ops[0] = parse_value(builder, v128);
		break;
	case VectorShape::UnaryImmediate:
		inst.ops[0] = parse_value(builder, v128);
		inst.ops[2] = next(TypeKind::I32);
		break;
	case VectorShape::Binary:
	case VectorShape::Test:
		inst.ops[0] = parse_value(builder, v128);
		inst.ops[1] = next(v128);
		break;
	case VectorShape::BinaryImmediate:
		inst.ops[0] = parse_value(builder, v128);
		inst.ops[1] = next(v128);
		inst.ops[2] = next(TypeKind::I32);
		break;
	case VectorShape::Blend:
		inst.ops[0] = parse_value(builder, v128);
		inst.ops[1] = next(v128);
		inst.ops[2] = next(v128);
		break;
	}
}

bool Parser::parse_value_instruction(FunctionBuilder& builder, Opcode keyword, Instruction& inst, Type& result)
{
	switch (keyword)
	{
	case Opcode::Const:
	{
		inst.op = Opcode::Const;
		inst.type = result = parse_type();
		if (!at_value_start() || at(TokenKind::Temp) || at(TokenKind::Slot) || at(TokenKind::Global))
			fail("expected literal");
		inst.ops[0] = parse_value(builder, inst.type.kind());
		return true;
	}
	case Opcode::Copy:
		inst.op = Opcode::Copy;
		inst.type = result = parse_type();
		inst.ops[0] = parse_value(builder, inst.type.kind());
		return true;
	case Opcode::Phi:
		parse_phi(builder, inst);
		result = inst.type;
		return true;
	case Opcode::Unary:
	case Opcode::Binary:
	case Opcode::Compare:
	{
		const WordInfo info = word(token());
		const WordClass expected = keyword == Opcode::Unary ? WordClass::Unary
			: keyword == Opcode::Binary                     ? WordClass::Binary
															: WordClass::Compare;
		if (info.word_class != expected)
			fail("unknown LowIR operation: " + spelling(token()));
		advance();
		inst.op = keyword == Opcode::Unary ? Opcode::Unary
			: keyword == Opcode::Binary    ? Opcode::Binary
										   : Opcode::Compare;
		inst.sub = info.value;
		if (keyword == Opcode::Binary)
		{
			// Instruction facts, in this order (the validator checks where).
			if (accept_word(WordClass::Fact, IF_NO_WRAP))
				inst.flags |= IF_NO_WRAP;
			if (accept_word(WordClass::Fact, IF_EXACT))
				inst.flags |= IF_EXACT;
		}
		inst.type = parse_type();
		inst.ops[0] = parse_value(builder, inst.type.kind());
		if (keyword != Opcode::Unary)
		{
			expect(TokenKind::Comma, ",");
			inst.ops[1] = parse_value(builder, inst.type.kind());
		}
		result = keyword == Opcode::Compare ? kI64 : inst.type;
		return true;
	}
	case Opcode::Select:
	{
		// select <type> <condition>, <if nonzero>, <else>: the condition at
		// its own temporary's type (a literal reads as i64).
		inst.op = Opcode::Select;
		inst.type = result = parse_type();
		inst.ops[0] = parse_value(builder, TypeKind::I64);
		expect(TokenKind::Comma, ",");
		inst.ops[1] = parse_value(builder, inst.type.kind());
		expect(TokenKind::Comma, ",");
		inst.ops[2] = parse_value(builder, inst.type.kind());
		return true;
	}
	case Opcode::Vector:
		parse_vector(builder, inst, result);
		return true;
	case Opcode::Convert:
	{
		const WordInfo info = word(token());
		if (info.word_class != WordClass::Convert)
			fail("unknown LowIR operation: " + spelling(token()));
		advance();
		inst.op = Opcode::Convert;
		inst.sub = info.value;
		inst.type = result = parse_type();
		const Type source = parse_type();
		set_convert_source_type(inst, source);
		inst.ops[0] = parse_value(builder, source.kind());
		return true;
	}
	case Opcode::Call:
		parse_call(builder, inst);
		result = inst.type;
		return true;
	case Opcode::Exception:
	case Opcode::ExceptionSelector:
		inst.op = keyword == Opcode::Exception ? Opcode::Exception : Opcode::ExceptionSelector;
		inst.type = parse_type();
		result = keyword == Opcode::Exception ? inst.type : kI64;
		return true;
	case Opcode::StackAlloc:
		inst.op = Opcode::StackAlloc;
		inst.type = result = kPtr;
		inst.ops[0] = parse_value(builder, TypeKind::Void);
		return true;
	case Opcode::VaArg:
		inst.op = Opcode::VaArg;
		inst.type = result = parse_type();
		inst.ops[0] = parse_value(builder, TypeKind::Void);
		if (accept(TokenKind::LBracket))
		{
			// Host objects: [classes=<classes>] of the object read.
			if (!program_.object_facts || !at_word(ir_model::spelling(MetadataKey::Classes)))
				fail("invalid va_arg metadata");
			advance();
			expect(TokenKind::Equals, "=");
			const InstId id = static_cast<InstId>(builder.body().instructions.size());
			builder.body().argument_classes.push_back(
				ArgumentClasses{ id, 0, parse_object_classes(Type::object(16, 3)) });
			expect(TokenKind::RBracket, "]");
		}
		return true;
	default:
		return false;
	}
}

bool Parser::parse_memory_instruction(FunctionBuilder& builder, Opcode keyword, Instruction& inst, Type& result)
{
	switch (keyword)
	{
	case Opcode::Addr:
		inst.op = Opcode::Addr;
		inst.type = result = kPtr;
		if (!at(TokenKind::Slot) && !at(TokenKind::Global))
			fail(at(TokenKind::Block) ? "block label used as value" : "expected a slot, global or function to address");
		inst.ops[0] = parse_value(builder, TypeKind::Void);
		return true;
	case Opcode::Load:
		inst.op = Opcode::Load;
		if (accept_word(WordClass::Fact, IF_VOLATILE))
			inst.flags |= IF_VOLATILE;
		if (accept_word(WordClass::Fact, IF_TYPED))
			inst.flags |= IF_TYPED;
		inst.type = result = parse_type();
		inst.ops[0] = parse_storage(builder);
		return true;
	case Opcode::AtomicLoad:
		inst.op = Opcode::AtomicLoad;
		inst.type = result = parse_type();
		inst.ops[0] = parse_value(builder, TypeKind::Ptr);
		expect(TokenKind::Comma, ",");
		inst.order = parse_order();
		return true;
	case Opcode::Index:
		inst.op = Opcode::Index;
		inst.type = parse_type();
		result = kPtr;
		if (at(TokenKind::LBracket))
			parse_index_metadata(inst);
		inst.ops[0] = parse_value(builder, TypeKind::Ptr);
		expect(TokenKind::Comma, ",");
		inst.ops[1] = parse_value(builder, TypeKind::Void);
		return true;
	case Opcode::AtomicAddFetch:
	case Opcode::AtomicExchange:
		inst.op = keyword == Opcode::AtomicAddFetch ? Opcode::AtomicAddFetch : Opcode::AtomicExchange;
		inst.type = result = parse_type();
		inst.ops[0] = parse_value(builder, TypeKind::Ptr);
		expect(TokenKind::Comma, ",");
		inst.ops[1] = parse_value(builder, inst.type.kind());
		expect(TokenKind::Comma, ",");
		inst.order = parse_order();
		return true;
	case Opcode::AtomicCompareExchange:
		inst.op = Opcode::AtomicCompareExchange;
		inst.type = parse_type();
		result = kI64;
		inst.ops[0] = parse_value(builder, TypeKind::Ptr);
		expect(TokenKind::Comma, ",");
		inst.ops[1] = parse_value(builder, TypeKind::Ptr);
		expect(TokenKind::Comma, ",");
		inst.ops[2] = parse_value(builder, inst.type.kind());
		expect(TokenKind::Comma, ",");
		inst.order = parse_order();
		expect(TokenKind::Comma, ",");
		inst.sub = static_cast<uint8_t>(parse_order());
		return true;
	default:
		return false;
	}
}

void Parser::parse_effect(FunctionBuilder& builder)
{
	const Token head = token();
	const WordInfo info = word(head);
	if (info.word_class != WordClass::Opcode)
		fail("unknown instruction: " + spelling(head));
	advance();
	const Opcode keyword = static_cast<Opcode>(info.value);
	Instruction inst = make_instruction(Opcode::Store, kVoid);
	switch (keyword)
	{
	case Opcode::Store:
		if (accept_word(WordClass::Fact, IF_VOLATILE))
			inst.flags |= IF_VOLATILE;
		if (accept_word(WordClass::Fact, IF_TYPED))
			inst.flags |= IF_TYPED;
		inst.type = parse_type();
		inst.ops[0] = parse_value(builder, inst.type.kind());
		expect(TokenKind::Comma, ",");
		inst.ops[1] = parse_storage(builder);
		break;
	case Opcode::AtomicStore:
		inst.op = Opcode::AtomicStore;
		inst.type = parse_type();
		inst.ops[0] = parse_value(builder, inst.type.kind());
		expect(TokenKind::Comma, ",");
		inst.ops[1] = parse_value(builder, TypeKind::Ptr);
		expect(TokenKind::Comma, ",");
		inst.order = parse_order();
		break;
	case Opcode::AtomicThreadFence:
	case Opcode::AtomicSignalFence:
		inst.op = keyword == Opcode::AtomicThreadFence ? Opcode::AtomicThreadFence : Opcode::AtomicSignalFence;
		inst.order = parse_order();
		break;
	case Opcode::VaStart:
		inst.op = Opcode::VaStart;
		inst.ops[0] = parse_value(builder, TypeKind::Ptr);
		break;
	case Opcode::Call:
		parse_call(builder, inst);
		if (!inst.type.is_void())
			fail_at(Position{ source_, head.line }, "a call without a result must be void");
		break;
	case Opcode::CopyObject:
	case Opcode::ZeroInit:
	{
		inst.op = keyword == Opcode::CopyObject ? Opcode::CopyObject : Opcode::ZeroInit;
		uint64_t bytes = 0;
		unsigned align_log2 = 0;
		parse_span(bytes, align_log2);
		inst.immediate = static_cast<int64_t>(bytes);
		inst.sub = static_cast<uint8_t>(align_log2);
		inst.ops[0] = parse_value(builder, TypeKind::Ptr);
		if (keyword == Opcode::CopyObject)
		{
			expect(TokenKind::Comma, ",");
			inst.ops[1] = parse_value(builder, TypeKind::Ptr);
		}
		break;
	}
	case Opcode::EhTry:
	case Opcode::EhCleanup:
	case Opcode::EhCatch:
	case Opcode::EhFilter:
	case Opcode::EhCatchAll:
	case Opcode::EhEnd:
		parse_eh_instruction(builder, keyword, inst);
		break;
	default:
		if (!parse_control_instruction(builder, keyword, inst))
			fail_at(Position{ source_, head.line }, "unknown instruction: " + spelling(head));
		terminated_ = true;
		break;
	}
	builder.append(inst);
}

bool Parser::parse_control_instruction(FunctionBuilder& builder, Opcode keyword, Instruction& inst)
{
	switch (keyword)
	{
	case Opcode::Throw:
		inst.op = Opcode::Throw;
		inst.type = parse_type();
		inst.ops[0] = parse_value(builder, inst.type.kind());
		return true;
	case Opcode::Resume:
		inst.op = Opcode::Resume;
		return true;
	case Opcode::Unreachable:
		inst.op = Opcode::Unreachable;
		return true;
	case Opcode::Jump:
		inst.op = Opcode::Jump;
		inst.ops[0] = Value::block(expect_block(builder));
		return true;
	case Opcode::Branch:
		inst.op = Opcode::Branch;
		inst.ops[0] = parse_value(builder, TypeKind::I64);
		expect(TokenKind::Comma, ",");
		inst.ops[1] = Value::block(expect_block(builder));
		expect(TokenKind::Comma, ",");
		inst.ops[2] = Value::block(expect_block(builder));
		return true;
	case Opcode::Switch:
		parse_switch(builder, inst);
		return true;
	case Opcode::Return:
		inst.op = Opcode::Return;
		inst.type = parse_type();
		if (!inst.type.is_void())
			inst.ops[0] = parse_value(builder, inst.type.kind());
		return true;
	default:
		return false;
	}
}

void Parser::parse_eh_instruction(FunctionBuilder& builder, Opcode keyword, Instruction& inst)
{
	switch (keyword)
	{
	case Opcode::EhTry:
		inst.op = Opcode::EhTry;
		inst.ops[0] = Value::block(expect_block(builder));
		break;
	case Opcode::EhCleanup:
		inst.op = Opcode::EhCleanup;
		if (at(TokenKind::Block))
			inst.ops[0] = Value::block(expect_block(builder));
		break;
	case Opcode::EhCatch:
		inst.op = Opcode::EhCatch;
		inst.ops[0] = Value::symbol(reference_symbol(expect_name(TokenKind::Global, "catch type symbol")));
		if (accept(TokenKind::Comma))
		{
			inst.immediate = parse_small_integer("catch selector");
			inst.flags |= IF_SELECTOR;
		}
		break;
	case Opcode::EhFilter:
	{
		inst.op = Opcode::EhFilter;
		operand_scratch_.clear();
		// Without type-info symbols the grammar writes the selector after a
		// comma (`eh_filter , 3`); the reference writer omits the comma.
		bool selector = at(TokenKind::Number) || at(TokenKind::Minus) || accept(TokenKind::Comma);
		while (!selector && at(TokenKind::Global))
		{
			operand_scratch_.push_back(Value::symbol(reference_symbol(token().name)));
			advance();
			if (!accept(TokenKind::Comma))
				break;
			selector = !at(TokenKind::Global);
		}
		if (selector)
		{
			inst.immediate = parse_small_integer("filter selector");
			inst.flags |= IF_SELECTOR;
		}
		inst.extra_count = static_cast<uint32_t>(operand_scratch_.size());
		inst.extra_first = builder.add_operands(operand_scratch_.data(), inst.extra_count);
		break;
	}
	case Opcode::EhCatchAll:
		inst.op = Opcode::EhCatchAll;
		if (accept(TokenKind::Comma))
		{
			inst.immediate = parse_small_integer("catch-all selector");
			inst.flags |= IF_SELECTOR;
		}
		break;
	default:
		inst.op = Opcode::EhEnd;
		break;
	}
}

void Parser::parse_call(FunctionBuilder& builder, Instruction& inst)
{
	inst.op = Opcode::Call;
	inst.type = parse_type();
	inst.ops[0] = parse_callee(builder);
	expect(TokenKind::LParen, "(");
	operand_scratch_.clear();
	if (!at(TokenKind::RParen))
	{
		do
			operand_scratch_.push_back(parse_value(builder, TypeKind::Void));
		while (accept(TokenKind::Comma));
	}
	expect(TokenKind::RParen, ")");
	inst.extra_count = static_cast<uint32_t>(operand_scratch_.size());
	inst.extra_first = builder.add_operands(operand_scratch_.data(), inst.extra_count);
	if (accept(TokenKind::LBracket))
	{
		// [elision=copy], and in host objects the classes of object
		// arguments to `...`, `classes=<argument>:<classes>` each.
		const InstId id = static_cast<InstId>(builder.body().instructions.size());
		do
		{
			if (at_word(ir_model::spelling(MetadataKey::Elision)) && (inst.flags & IF_COPY_ELISION) == 0)
			{
				advance();
				expect(TokenKind::Equals, "=");
				if (!at_word(elision_copy_spelling()))
					fail("invalid call metadata");
				advance();
				inst.flags |= IF_COPY_ELISION;
				continue;
			}
			if (!program_.object_facts || !at_word(ir_model::spelling(MetadataKey::Classes)))
				fail("invalid call metadata");
			advance();
			expect(TokenKind::Equals, "=");
			if (!at(TokenKind::Number))
				fail("expected argument index");
			const uint64_t argument = parse_natural(token().text, token().length);
			advance();
			expect(TokenKind::Colon, ":");
			if (argument >= inst.extra_count)
				fail("classes metadata names no argument");
			const uint8_t classes = parse_object_classes(Type::object(16, 3));
			builder.body().argument_classes.push_back(ArgumentClasses{ id, static_cast<uint32_t>(argument), classes });
		} while (accept(TokenKind::Comma));
		expect(TokenKind::RBracket, "]");
	}
	if (accept_keyword(Keyword::As))
	{
		inst.immediate = parse_signature(MetadataContext::CallSignature, nullptr);
		inst.flags |= IF_SIGNATURE;
	}
}

void Parser::parse_phi(FunctionBuilder& builder, Instruction& inst)
{
	inst.op = Opcode::Phi;
	inst.type = parse_type();
	expect(TokenKind::LBracket, "[");
	if (!at(TokenKind::Block))
		fail("phi requires predecessor/value pairs");
	operand_scratch_.clear();
	do
	{
		const BlockId block = expect_block(builder);
		expect(TokenKind::Colon, ":");
		operand_scratch_.push_back(Value::block(block));
		operand_scratch_.push_back(parse_value(builder, inst.type.kind()));
	} while (accept(TokenKind::Comma));
	expect(TokenKind::RBracket, "]");
	inst.extra_count = static_cast<uint32_t>(operand_scratch_.size());
	inst.extra_first = builder.add_operands(operand_scratch_.data(), inst.extra_count);
}

void Parser::parse_switch(FunctionBuilder& builder, Instruction& inst)
{
	inst.op = Opcode::Switch;
	inst.ops[0] = parse_value(builder, TypeKind::Void);
	expect(TokenKind::Comma, ",");
	inst.ops[1] = Value::block(expect_block(builder));
	operand_scratch_.clear();
	while (accept(TokenKind::Comma))
	{
		const Value value = parse_value(builder, TypeKind::Void);
		expect(TokenKind::Colon, ":");
		operand_scratch_.push_back(value);
		operand_scratch_.push_back(Value::block(expect_block(builder)));
	}
	inst.extra_count = static_cast<uint32_t>(operand_scratch_.size());
	inst.extra_first = builder.add_operands(operand_scratch_.data(), inst.extra_count);
}

void Parser::parse_index_metadata(Instruction& inst)
{
	advance();
	if (!at(TokenKind::Word) || !at_word(ir_model::spelling(MetadataKey::Projection)))
		fail("invalid index metadata");
	advance();
	expect(TokenKind::Equals, "=");
	IndexProjectionKind projection = IPK_NONE;
	if (!at(TokenKind::Word) || !ir_model::from_spelling(token().text, token().length, &projection))
		fail("invalid index projection");
	advance();
	inst.sub = static_cast<uint8_t>(projection);
	if (!at(TokenKind::RBracket))
		fail("invalid index metadata");
	advance();
}

} // namespace text
} // namespace lowir
