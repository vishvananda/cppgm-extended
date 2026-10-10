#include "native/link/routine_builder.h"

#include <cstring>

#include "lowir/literals.h"

namespace native {

using lowir::Instruction;
using lowir::Opcode;
using lowir::Type;
using lowir::Value;

lowir::NameId RuntimeProgram::name(const char* spelling)
{
	return program_.names.intern(spelling, static_cast<uint32_t>(std::strlen(spelling)));
}

bool RuntimeProgram::defined(const char* spelling)
{
	const lowir::SymbolId symbol = program_.find_symbol(name(spelling));
	if (symbol == lowir::kNoId)
		return false;
	const lowir::SymbolKind kind = program_.symbols[symbol].kind;
	return kind == lowir::SymbolKind::FunctionDefinition || kind == lowir::SymbolKind::GlobalDefinition;
}

lowir::SignatureId RuntimeProgram::signature(Kind result, std::initializer_list<Kind> parameters)
{
	lowir::Signature signature;
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	signature.parameter_count = static_cast<uint32_t>(parameters.size());
	signature.result = Type::scalar(result);
	for (const Kind kind : parameters)
	{
		lowir::Parameter parameter;
		parameter.name = 0;
		parameter.type = Type::scalar(kind);
		parameter.pass = ir_model::PPM_DIRECT;
		parameter.alias = ir_model::PALM_DEFAULT;
		parameter.object_bytes = 0;
		program_.parameters.push_back(parameter);
	}
	return program_.add_signature(signature);
}

lowir::SymbolId RuntimeProgram::function(const char* spelling, lowir::SignatureId signature, bool define)
{
	const lowir::SymbolId symbol = program_.symbol_for(name(spelling));
	if (program_.symbols[symbol].kind == lowir::SymbolKind::Unresolved)
	{
		program_.declare_function(
			symbol, define ? lowir::SymbolKind::FunctionDefinition : lowir::SymbolKind::FunctionDeclaration, signature);
		program_.symbols[symbol].metadata.binding = ir_model::SBM_STRONG;
		program_.symbols[symbol].metadata.linkage = ir_model::LLM_C;
	}
	return symbol;
}

lowir::SymbolId RuntimeProgram::global(const char* spelling, uint64_t bytes)
{
	const lowir::SymbolId symbol = program_.symbol_for(name(spelling));
	if (program_.symbols[symbol].kind != lowir::SymbolKind::Unresolved)
		return symbol;
	lowir::Global global = lowir::Global();
	if (bytes == 0)
	{
		global.init = lowir::GlobalInit::None;
		program_.declare_global(symbol, lowir::SymbolKind::GlobalDeclaration, global);
	}
	else
	{
		// `zero n` after an 8-byte item keeps the storage 8-aligned.
		lowir::DataItem first = lowir::DataItem();
		first.kind = lowir::DataItemKind::Scalar;
		first.type = Type::scalar(lowir::TypeKind::I64);
		first.constant.kind = lowir::ValueKind::Integer;
		first.constant.bits = lowir::integer_literal(0);
		lowir::DataItem rest = lowir::DataItem();
		rest.kind = lowir::DataItemKind::Zero;
		rest.amount = static_cast<int64_t>(bytes > 8 ? bytes - 8 : 0);
		global.init = lowir::GlobalInit::Data;
		global.first_item = static_cast<uint32_t>(program_.data_items.size());
		program_.data_items.push_back(first);
		global.item_count = 1;
		if (rest.amount != 0)
		{
			program_.data_items.push_back(rest);
			global.item_count = 2;
		}
		program_.declare_global(symbol, lowir::SymbolKind::GlobalDefinition, global);
	}
	program_.symbols[symbol].metadata.binding = ir_model::SBM_STRONG;
	return symbol;
}

lowir::SymbolId RuntimeProgram::words(const char* spelling, std::initializer_list<DataWord> items)
{
	const lowir::SymbolId symbol = program_.symbol_for(name(spelling));
	if (program_.symbols[symbol].kind != lowir::SymbolKind::Unresolved)
		return symbol;
	lowir::Global global = lowir::Global();
	global.init = lowir::GlobalInit::Data;
	global.first_item = static_cast<uint32_t>(program_.data_items.size());
	global.item_count = static_cast<uint32_t>(items.size());
	for (const DataWord& word : items)
	{
		lowir::DataItem item = lowir::DataItem();
		if (word.symbol != lowir::kNoId)
		{
			item.kind = lowir::DataItemKind::Address;
			item.address = word.symbol;
			item.amount = word.value;
		}
		else
		{
			item.kind = lowir::DataItemKind::Scalar;
			item.type = Type::scalar(lowir::TypeKind::I64);
			item.constant.kind = lowir::ValueKind::Integer;
			item.constant.bits = lowir::integer_literal(word.value);
		}
		program_.data_items.push_back(item);
	}
	program_.declare_global(symbol, lowir::SymbolKind::GlobalDefinition, global);
	program_.symbols[symbol].metadata.binding = ir_model::SBM_STRONG;
	program_.symbols[symbol].metadata.storage = ir_model::GSM_READONLY;
	return symbol;
}

lowir::SymbolId RuntimeProgram::string(const char* spelling, const char* text)
{
	const lowir::SymbolId symbol = program_.symbol_for(name(spelling));
	if (program_.symbols[symbol].kind != lowir::SymbolKind::Unresolved)
		return symbol;
	lowir::Global global = lowir::Global();
	global.init = lowir::GlobalInit::Data;
	global.first_item = static_cast<uint32_t>(program_.data_items.size());
	const size_t length = std::strlen(text) + 1;
	global.item_count = static_cast<uint32_t>(length);
	for (size_t k = 0; k < length; ++k)
	{
		lowir::DataItem item = lowir::DataItem();
		item.kind = lowir::DataItemKind::Scalar;
		item.type = Type::scalar(lowir::TypeKind::I8);
		item.constant.kind = lowir::ValueKind::Integer;
		item.constant.bits = lowir::integer_literal(static_cast<unsigned char>(text[k]));
		program_.data_items.push_back(item);
	}
	program_.declare_global(symbol, lowir::SymbolKind::GlobalDefinition, global);
	program_.symbols[symbol].metadata.binding = ir_model::SBM_STRONG;
	program_.symbols[symbol].metadata.storage = ir_model::GSM_READONLY;
	return symbol;
}

void RuntimeProgram::alias(const char* spelling, lowir::SymbolId target)
{
	program_.aliases.push_back(lowir::ObjectAlias{ name(spelling), target });
}

RoutineBuilder::RoutineBuilder(lowir::Program& program, lowir::SymbolId function)
	: program_(program), builder_(program, function)
{
	enter(block());
}

Value RoutineBuilder::value_of(Instruction inst, Kind result)
{
	return Value::temp(builder_.append_value(inst, Type::scalar(result), 0));
}

Value RoutineBuilder::at(Value base, int64_t offset)
{
	return offset == 0 ? base : at(base, integer(offset));
}

Value RoutineBuilder::at(Value base, Value offset)
{
	Instruction inst = lowir::make_instruction(Opcode::Index, Type::scalar(Kind::I8));
	inst.ops[0] = base;
	inst.ops[1] = offset;
	return value_of(inst, Kind::Ptr);
}

Value RoutineBuilder::load(Kind kind, Value address, int64_t offset)
{
	return Value::temp(builder_.load(Type::scalar(kind), at(address, offset), 0));
}

void RoutineBuilder::store(Kind kind, Value value, Value address, int64_t offset)
{
	builder_.store(Type::scalar(kind), value, at(address, offset));
}

Value RoutineBuilder::address(lowir::SymbolId symbol)
{
	Instruction inst = lowir::make_instruction(Opcode::Addr, Type::scalar(Kind::Ptr));
	inst.ops[0] = Value::symbol(symbol);
	return value_of(inst, Kind::Ptr);
}

Value RoutineBuilder::slot_address(lowir::SlotId slot)
{
	Instruction inst = lowir::make_instruction(Opcode::Addr, Type::scalar(Kind::Ptr));
	inst.ops[0] = Value::slot(slot);
	return value_of(inst, Kind::Ptr);
}

Value RoutineBuilder::binary(lowir::BinaryOp op, Kind kind, Value a, Value b)
{
	return Value::temp(builder_.binary(op, Type::scalar(kind), a, b, 0));
}

Value RoutineBuilder::compare(lowir::ComparePredicate predicate, Kind kind, Value a, Value b)
{
	Instruction inst = lowir::make_instruction(Opcode::Compare, Type::scalar(kind));
	inst.sub = static_cast<uint8_t>(predicate);
	inst.ops[0] = a;
	inst.ops[1] = b;
	return value_of(inst, Kind::I64);
}

Value RoutineBuilder::convert(lowir::ConvertOp op, Kind to, Kind from, Value value)
{
	Instruction inst = lowir::make_instruction(Opcode::Convert, Type::scalar(to));
	inst.sub = static_cast<uint8_t>(op);
	inst.ops[0] = value;
	lowir::set_convert_source_type(inst, Type::scalar(from));
	return value_of(inst, to);
}

Value RoutineBuilder::call(Kind result, lowir::SymbolId callee, std::initializer_list<Value> arguments)
{
	return Value::temp(builder_.call(Type::scalar(result), Value::symbol(callee), arguments.begin(),
		static_cast<uint32_t>(arguments.size()), lowir::kNoId, 0));
}

Value RoutineBuilder::call_indirect(
	Kind result, Value callee, lowir::SignatureId signature, std::initializer_list<Value> arguments)
{
	return Value::temp(builder_.call(
		Type::scalar(result), callee, arguments.begin(), static_cast<uint32_t>(arguments.size()), signature, 0));
}

Value RoutineBuilder::floating(Kind kind, long double value)
{
	return builder_.literal(lowir::ValueKind::Float, kind, lowir::float_literal(kind, value));
}

lowir::SlotId RoutineBuilder::local(Kind kind)
{
	return builder_.add_slot(0, Type::scalar(kind));
}

Value RoutineBuilder::read(Kind kind, lowir::SlotId slot)
{
	return Value::temp(builder_.load(Type::scalar(kind), Value::slot(slot), 0));
}

void RoutineBuilder::write(Kind kind, lowir::SlotId slot, Value value)
{
	builder_.store(Type::scalar(kind), value, Value::slot(slot));
}

lowir::BlockId RoutineBuilder::block()
{
	return builder_.add_block(0);
}

void RoutineBuilder::enter(lowir::BlockId block)
{
	builder_.place_block(block);
	builder_.set_insertion_block(block);
}

void RoutineBuilder::ret(Kind kind, Value value)
{
	builder_.ret(Type::scalar(kind), value);
}

void RoutineBuilder::ret()
{
	builder_.ret(Type::scalar(Kind::Void), Value::none());
}

void RoutineBuilder::unreachable()
{
	builder_.append(lowir::make_instruction(Opcode::Unreachable, Type::scalar(Kind::Void)));
}

} // namespace native
