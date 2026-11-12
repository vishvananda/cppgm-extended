#include "wasm/selector.h"

#include "lowir/literals.h"
#include "wasm/exceptions.h"
#include "machine/exception_clauses.h"

namespace wasm {

using namespace lowir;

Selector::Selector(
	const Program& program, const Module& module, SymbolId symbol, Function& output, const ExceptionPlan* exceptions)
	: program_(program), module_(module), body_(*program.functions[program.symbols[symbol].detail].body),
	  signature_(program.signatures[program.functions[program.symbols[symbol].detail].signature]),
	  boundary_(module.boundaries[program.functions[program.symbols[symbol].detail].signature]), out_(output),
	  exceptions_(exceptions)
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
	case ValueType::V128:
		vector_constant(bits);
		break;
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
	if (type.is_object())
	{
		address(operand);
		return;
	}
	switch (operand.kind)
	{
	case ValueKind::Temp:
	{
		if (wide_locals_[operand.index] != kNoId)
		{
			out_.emit(Op::LocalGet, { wide_locals_[operand.index] });
			if (value_type(type) == ValueType::I32)
				out_.emit(Op::I32WrapI64);
			break;
		}
		out_.emit(Op::LocalGet, { locals_[operand.index] });
		const Type actual = body_.temps[operand.index].type;
		if (memory_value(actual))
		{
			if (!type.is_pointer() || actual.kind() == TypeKind::I128)
				out_.emit(memory_op(type, false), { 0, 0 });
			break;
		}
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
		const bool function = is_function_kind(program_.symbols[operand.index].kind);
		uint32_t address = function ? module_.function_indices[operand.index] : module_.addresses[operand.index];
		if (address == kNoId && !module_.relocatable)
			throw WasmError(
				"undefined Wasm symbol: " + std::string(program_.spelling(program_.symbols[operand.index].name)));
		if (module_.relocatable)
			address = function ? address + 1 : 0;
		else if (function)
			++address;
		out_.emit(Op::I32Const, { address });
		out_.relocate(function ? RelocationKind::TableIndex : RelocationKind::MemoryAddressSigned, operand.index);
		break;
	}
	default:
		throw WasmError("invalid Wasm value operand");
	}
}

void Selector::address(Value operand)
{
	if (operand.is(ValueKind::Temp) && wide_locals_[operand.index] != kNoId)
		throw WasmError("Wasm wide local requires explicit memory materialization");
	if (operand.is(ValueKind::Temp) && memory_value(body_.temps[operand.index].type))
	{
		out_.emit(Op::LocalGet, { locals_[operand.index] });
		return;
	}
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
	if (type.kind() == TypeKind::I128)
	{
		wide_part(operand, 0);
		wide_part(operand, 1);
		out_.emit(Op::I64Or);
		out_.emit(Op::I64Eqz);
		out_.emit(Op::I32Eqz);
		return;
	}
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
	case TypeKind::V128:
		return store ? Op::V128Store : Op::V128Load;
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
		out_.relocate(RelocationKind::GlobalIndex, module_.stack_symbol);
	}
}

void Selector::run()
{
	phi_inputs_.reset(body_.instructions.size());
	prepare_locals();
	if (exceptions_ != nullptr && exceptions_->active)
	{
		const bool cxx = module_.exception_abi == ExceptionAbi::Cxx;
		exception_pointer_ = add_local(cxx ? ValueType::I32 : ValueType::I64);
		if (!cxx)
			exception_high_ = add_local(ValueType::I64);
		exception_reference_ = add_local(ValueType::ExnRef);
		exception_filter_ = add_local(ValueType::I32);
		exception_selector_ = add_local(ValueType::I32);
		exception_stack_ = add_local(ValueType::I32);
		for (uint32_t i = 0; i < exceptions_->tables.landings.size(); ++i)
		{
			exception_contexts_.push_back(add_local(ValueType::ExnRef));
			add_local(cxx ? ValueType::I32 : ValueType::I64);
			add_local(ValueType::I32);
			add_local(ValueType::I32);
			if (!cxx)
				add_local(ValueType::I64);
		}
	}
	prepare_frame();
	const auto last = body_.instructions[body_.blocks[body_.layout[0]].last].op;
	if ((exceptions_ == nullptr || !exceptions_->active) && body_.layout.size() == 1 &&
		(last == Opcode::Return || last == Opcode::Unreachable))
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
	out_.emit(Op::Loop, { block_type(ValueType::Void) });
	// A dense table keeps each fallback edge constant-time even after
	// inlining creates a large control-flow graph.
	std::vector<BlockId> blocks;
	for (BlockId id : body_.layout)
	{
		if (exceptions_ != nullptr && exceptions_->active &&
			exceptions_->analysis.eh_live_[exceptions_->layout_of[id]] == 0)
			continue;
		blocks.push_back(id);
	}
	const uint32_t count = static_cast<uint32_t>(blocks.size());
	std::vector<uint32_t> depths(body_.blocks.size() + 1, count);
	for (uint32_t i = 0; i < count; ++i)
		depths[blocks[i]] = i;
	for (uint32_t i = 0; i <= count; ++i)
		out_.emit(Op::Block, { block_type(ValueType::Void) });
	nesting_ = count + 2;
	out_.emit(Op::LocalGet, { pc_ });
	out_.emit(Op::BrTable);
	for (uint32_t depth : depths)
		out_.operands.push_back(depth);
	out_.instructions.back().operands.count = static_cast<uint32_t>(depths.size());
	for (BlockId id : blocks)
	{
		out_.emit(Op::End);
		--nesting_;
		block(id);
	}
	out_.emit(Op::End);
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
		current_inst_ = i;
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
		const Value incoming = phi_inputs_.incoming(body_, i, from);
		if (phi.type.kind() == TypeKind::I128)
		{
			wide_part(incoming, 0);
			wide_part(incoming, 1);
		}
		else
			value(incoming, phi.type);
		destinations.push_back(phi.result);
	}
	// All readers precede every write, including cycles and overlapping moves.
	for (auto i = destinations.rbegin(); i != destinations.rend(); ++i)
		if (body_.temps[*i].type.kind() == TypeKind::I128)
			store_wide(Value::temp(*i));
		else
			out_.emit(Op::LocalSet, { locals_[*i] });
}

void Selector::edge(BlockId from, BlockId to)
{
	if (exceptions_ != nullptr && exceptions_->active)
	{
		const int64_t selector = exceptions_->entry_selectors[to];
		if (selector != 0)
		{
			out_.emit(Op::I32Const, { static_cast<uint64_t>(selector) });
			out_.emit(Op::LocalGet, { exception_selector_ });
			out_.emit(Op::LocalGet, { exception_selector_ });
			out_.emit(Op::I32Eqz);
			out_.emit(Op::Select);
			out_.emit(Op::LocalSet, { exception_selector_ });
		}
	}
	phi_copies(from, to);
	if (pc_ == kNoId)
	{
		// A natural loop inside a dispatch case keeps its direct backedge.
		if (branch_label(to))
			return;
		const uint32_t region = control_.dispatch_region.empty() ? kNoId : control_.dispatch_region[to];
		if (region != kNoId)
		{
			out_.emit(Op::I32Const, { control_.dispatch_case[to] });
			out_.emit(Op::LocalSet, { region_locals_[region] });
			for (auto label = dispatch_labels_.rbegin(); label != dispatch_labels_.rend(); ++label)
				if (label->target == region)
				{
					out_.emit(Op::Br, { nesting_ - 1 - label->depth });
					return;
				}
			if (branch_label(control_.block_count + region - 1))
				return;
		}
		throw WasmError("Wasm structured branch has no active destination");
	}
	out_.emit(Op::I32Const, { to });
	out_.emit(Op::LocalSet, { pc_ });
	out_.emit(Op::Br, { nesting_ - 1 });
}

} // namespace wasm
