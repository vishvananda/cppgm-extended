#include "wasm/selector.h"

#include "lowir/literals.h"

namespace wasm {

using namespace lowir;

Selector::Selector(const Program& program, const Module& module, SymbolId symbol, Function& output)
	: program_(program), module_(module), body_(*program.functions[program.symbols[symbol].detail].body), out_(output)
{}

uint32_t Selector::add_local(ValueType type)
{
	const uint32_t id = static_cast<uint32_t>(out_.locals.size());
	out_.locals.push_back(type);
	return id;
}

Type Selector::operand_type(Value operand, Type fallback) const
{
	if (operand.is(ValueKind::Slot))
		return body_.slots[operand.index].type;
	return operand.is(ValueKind::Temp) ? body_.temps[operand.index].type : fallback;
}

void Selector::constant(Literal bits, Type type)
{
	switch (value_type(type))
	{
	case ValueType::I32:
		out_.emit(Op::I32Const, { static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(bits.low))) });
		break;
	case ValueType::I64:
		out_.emit(Op::I64Const, { bits.low });
		break;
	case ValueType::F32:
		out_.emit(Op::F32Const, { bits.low });
		break;
	case ValueType::F64:
		out_.emit(Op::F64Const, { bits.low });
		break;
	default:
		throw WasmError("unsupported Wasm scalar literal");
	}
}

void Selector::value(Value operand, Type type)
{
	switch (operand.kind)
	{
	case ValueKind::Temp:
	{
		out_.emit(Op::LocalGet, { operand.index });
		const Type actual = body_.temps[operand.index].type;
		const ValueType from = value_type(actual), to = value_type(type);
		if (from == ValueType::I32 && to == ValueType::I64)
			out_.emit(actual.is_pointer() || is_unsigned(actual.kind()) ? Op::I64ExtendI32U : Op::I64ExtendI32S);
		else if (from == ValueType::I64 && to == ValueType::I32)
			out_.emit(Op::I32WrapI64);
		break;
	}
	case ValueKind::Integer:
		constant(type.is_float() ? integer_as_float(type.kind(), body_.literals[operand.index])
								 : body_.literals[operand.index],
			type);
		normalize(type);
		break;
	case ValueKind::Float:
		constant(body_.literals[operand.index], type);
		break;
	case ValueKind::Slot:
		address(operand);
		out_.emit(memory_op(type, false), { 0, 0 });
		break;
	case ValueKind::Symbol:
	{
		uint32_t address = module_.addresses[operand.index];
		if (is_function_kind(program_.symbols[operand.index].kind))
			address = module_.function_indices[operand.index] == kNoId ? kNoId
																	   : module_.function_indices[operand.index] + 1;
		if (address == kNoId)
			throw WasmError(
				"undefined Wasm symbol: " + std::string(program_.spelling(program_.symbols[operand.index].name)));
		out_.emit(Op::I32Const, { address });
		break;
	}
	default:
		throw WasmError("invalid Wasm value operand");
	}
}

void Selector::address(Value operand)
{
	if (!operand.is(ValueKind::Slot))
	{
		value(operand, program_.pointer_type());
		return;
	}
	out_.emit(Op::LocalGet, { frame_ });
	out_.emit(Op::I32Const, { slots_[operand.index] });
	out_.emit(Op::I32Add);
}

void Selector::normalize(Type type)
{
	switch (type.kind())
	{
	case TypeKind::I8:
		out_.emit(Op::I32Extend8S);
		break;
	case TypeKind::I16:
		out_.emit(Op::I32Extend16S);
		break;
	case TypeKind::I1:
	case TypeKind::U8:
	case TypeKind::U16:
		out_.emit(Op::I32Const, { (uint64_t(1) << integer_bits(type.kind())) - 1 });
		out_.emit(Op::I32And);
		break;
	default:
		break;
	}
}

void Selector::condition(Value operand)
{
	const Type type = operand_type(operand, Type::scalar(TypeKind::I64));
	value(operand, type);
	if (value_type(type) == ValueType::I64)
	{
		out_.emit(Op::I64Eqz);
		out_.emit(Op::I32Eqz);
	}
}

Op Selector::memory_op(Type type, bool store) const
{
	switch (type.kind())
	{
	case TypeKind::I1:
	case TypeKind::U8:
		return store ? Op::I32Store8 : Op::I32Load8U;
	case TypeKind::I8:
		return store ? Op::I32Store8 : Op::I32Load8S;
	case TypeKind::U16:
		return store ? Op::I32Store16 : Op::I32Load16U;
	case TypeKind::I16:
		return store ? Op::I32Store16 : Op::I32Load16S;
	case TypeKind::I32:
	case TypeKind::U32:
	case TypeKind::Ptr:
		return store ? Op::I32Store : Op::I32Load;
	case TypeKind::I64:
		return store ? Op::I64Store : Op::I64Load;
	case TypeKind::F32:
		return store ? Op::F32Store : Op::F32Load;
	case TypeKind::F64:
		return store ? Op::F64Store : Op::F64Load;
	default:
		throw WasmError("Wasm memory value requires legalization");
	}
}

void Selector::restore_stack()
{
	if (saved_stack_ != kNoId)
	{
		out_.emit(Op::LocalGet, { saved_stack_ });
		out_.emit(Op::GlobalSet, { 0 });
	}
}

void Selector::run()
{
	out_.parameter_count = body_.parameter_count;
	for (const Temp& temp : body_.temps)
		add_local(value_type(temp.type));
	slots_.resize(body_.slots.size());
	uint64_t frame_bytes = 0;
	for (uint32_t i = 0; i < body_.slots.size(); ++i)
	{
		const machine_model::FrameObject object{ storage_bytes(body_.slots[i].type),
			data_alignment(body_.slots[i].type), i };
		if (object.alignment > 16)
			throw WasmError("Wasm over-aligned frame requires dynamic alignment");
		frame_bytes = (frame_bytes + object.alignment - 1) & ~uint64_t(object.alignment - 1);
		slots_[i] = static_cast<uint32_t>(frame_bytes);
		frame_bytes += object.bytes;
	}
	if (frame_bytes > 1024 * 1024)
		throw WasmError("Wasm static frame exceeds the configured stack reserve");
	out_.frame_bytes = static_cast<uint32_t>((frame_bytes + 15) & ~uint64_t(15));
	bool dynamic = false;
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			dynamic = dynamic || body_.instructions[i].op == Opcode::StackAlloc;
	if (out_.frame_bytes != 0 || dynamic)
	{
		saved_stack_ = add_local(ValueType::I32);
		frame_ = add_local(ValueType::I32);
		out_.emit(Op::GlobalGet, { 0 });
		out_.emit(Op::LocalTee, { saved_stack_ });
		out_.emit(Op::I32Const, { out_.frame_bytes });
		out_.emit(Op::I32Sub);
		out_.emit(Op::LocalTee, { frame_ });
		out_.emit(Op::GlobalSet, { 0 });
		out_.emit(Op::LocalGet, { frame_ });
		out_.emit(Op::I32Const, { module_.stack_floor });
		out_.emit(Op::I32LtU);
		out_.emit(Op::If, { static_cast<uint8_t>(ValueType::Void) });
		out_.emit(Op::Unreachable);
		out_.emit(Op::End);
	}
	const auto last = body_.instructions[body_.blocks[body_.layout[0]].last].op;
	if (body_.layout.size() == 1 && (last == Opcode::Return || last == Opcode::Unreachable))
		simple();
	else if (!structured())
		dispatch();
	out_.emit(Op::End);
}

void Selector::simple()
{
	block(body_.layout[0]);
}

void Selector::dispatch()
{
	pc_ = add_local(ValueType::I32);
	out_.emit(Op::I32Const, { body_.layout[0] });
	out_.emit(Op::LocalSet, { pc_ });
	out_.emit(Op::Loop, { static_cast<uint8_t>(ValueType::Void) });
	nesting_ = 1;
	for (BlockId id : body_.layout)
	{
		out_.emit(Op::LocalGet, { pc_ });
		out_.emit(Op::I32Const, { id });
		out_.emit(Op::I32Eq);
		out_.emit(Op::If, { static_cast<uint8_t>(ValueType::Void) });
		++nesting_;
		block(id);
		--nesting_;
		out_.emit(Op::End);
	}
	out_.emit(Op::Unreachable);
	out_.emit(Op::End);
	out_.emit(Op::Unreachable);
	nesting_ = 0;
}

void Selector::block(BlockId id)
{
	current_block_ = id;
	for (InstId i = body_.blocks[id].first; i != kNoId; i = body_.instructions[i].next)
	{
		out_.location = body_.instructions[i].location;
		instruction(body_.instructions[i]);
	}
}

void Selector::phi_copies(BlockId from, BlockId to)
{
	std::vector<TempId> destinations;
	for (InstId i = body_.blocks[to].first; i != kNoId; i = body_.instructions[i].next)
	{
		const auto& phi = body_.instructions[i];
		if (phi.op != Opcode::Phi)
			break;
		value(opt::phi_incoming(body_, phi, from), phi.type);
		destinations.push_back(phi.result);
	}
	// All readers precede every write, including cycles and overlapping moves.
	for (auto i = destinations.rbegin(); i != destinations.rend(); ++i)
		out_.emit(Op::LocalSet, { *i });
}

void Selector::edge(BlockId from, BlockId to)
{
	phi_copies(from, to);
	if (pc_ == kNoId)
	{
		for (auto label = labels_.rbegin(); label != labels_.rend(); ++label)
			if (label->target == to)
			{
				out_.emit(Op::Br, { nesting_ - 1 - label->depth });
				return;
			}
		throw WasmError("Wasm structured branch has no active destination");
	}
	out_.emit(Op::I32Const, { to });
	out_.emit(Op::LocalSet, { pc_ });
	out_.emit(Op::Br, { nesting_ - 1 });
}

void Selector::open_label(Op op, BlockId target)
{
	out_.emit(op, { static_cast<uint8_t>(ValueType::Void) });
	labels_.push_back(Label{ target, nesting_++ });
}

void Selector::close_label()
{
	out_.emit(Op::End);
	labels_.pop_back();
	--nesting_;
}

void Selector::region(uint32_t id)
{
	const auto& children = control_.regions[id].children;
	auto entry = [&](uint32_t child) {
		return child < control_.block_count ? child : control_.regions[child - control_.block_count + 1].header;
	};
	for (uint32_t i = static_cast<uint32_t>(children.size()); i > 1; --i)
		open_label(Op::Block, entry(children[i - 1]));
	for (uint32_t i = 0; i < children.size(); ++i)
	{
		if (i != 0)
			close_label();
		const uint32_t child = children[i];
		if (child < control_.block_count)
			block(child);
		else
		{
			open_label(Op::Loop, entry(child));
			region(child - control_.block_count + 1);
			close_label();
		}
	}
}

bool Selector::structured()
{
	if (!structure_control(program_, body_, control_))
		return false;
	region(0);
	out_.emit(Op::Unreachable);
	return true;
}

} // namespace wasm
