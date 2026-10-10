#include "lowir/exercises.h"

#include <cstring>

#include "lowir/builder.h"

namespace lowir {

namespace {

const Type kI64 = Type::scalar(TypeKind::I64);
const Type kPtr = Type::scalar(TypeKind::Ptr);
const Type kVoid = Type::scalar(TypeKind::Void);

struct ParameterSpec
{
	const char* name;
	Type type;
};

NameId intern(Program& program, const char* text)
{
	return program.names.intern(text, static_cast<uint32_t>(std::strlen(text)));
}

SignatureId add_signature(Program& program, const ParameterSpec* parameters, uint32_t count, Type result)
{
	Signature signature;
	signature.first_parameter = static_cast<uint32_t>(program.parameters.size());
	signature.parameter_count = count;
	signature.result = result;
	for (uint32_t i = 0; i < count; ++i)
	{
		Parameter parameter = Parameter();
		parameter.name = intern(program, parameters[i].name);
		parameter.type = parameters[i].type;
		program.parameters.push_back(parameter);
	}
	return program.add_signature(signature);
}

// A function definition with one placed, current block named `entry`.
SymbolId define_function(
	Program& program, const char* name, const ParameterSpec* parameters, uint32_t count, Type result)
{
	const SymbolId symbol = program.symbol_for(intern(program, name));
	program.declare_function(symbol, SymbolKind::FunctionDefinition, add_signature(program, parameters, count, result));
	return symbol;
}

BlockId begin_entry(FunctionBuilder& builder)
{
	const BlockId entry = builder.add_block(intern(builder.program(), "entry"));
	builder.place_block(entry);
	builder.set_insertion_block(entry);
	return entry;
}

// sum_to(n) = n (n + 1) / 2: for n <= 1,000,000 the product stays far
// below 2^63, so the closed form needs no loop.
void build_sum(Program& program)
{
	const ParameterSpec parameters[] = { { "n", kI64 } };
	FunctionBuilder builder(program, define_function(program, "sum_to", parameters, 1, kI64));
	begin_entry(builder);
	const Value n = Value::temp(builder.parameter(0));
	const TempId next = builder.binary(BinaryOp::Add, kI64, n, builder.integer(1), intern(program, "n_plus_one"));
	const TempId product = builder.binary(BinaryOp::Mul, kI64, n, Value::temp(next), intern(program, "product"));
	const TempId sum = builder.binary(
		BinaryOp::UDiv, kI64, Value::temp(product), builder.integer(2), intern(program, "sum"));
	builder.ret(kI64, Value::temp(sum));
}

// Both values are loaded before either store, so a pointer pair naming the
// same object stores back the value it held.
void build_swap(Program& program)
{
	const ParameterSpec parameters[] = { { "left", kPtr }, { "right", kPtr } };
	FunctionBuilder builder(program, define_function(program, "swap_values", parameters, 2, kVoid));
	begin_entry(builder);
	const Value left = Value::temp(builder.parameter(0));
	const Value right = Value::temp(builder.parameter(1));
	const TempId left_value = builder.load(kI64, left, intern(program, "left_value"));
	const TempId right_value = builder.load(kI64, right, intern(program, "right_value"));
	builder.store(kI64, Value::temp(right_value), left);
	builder.store(kI64, Value::temp(left_value), right);
	builder.ret(kVoid, Value::none());
}

// Two calls through the i64(i64) pointer, the second on the first's result.
void build_call(Program& program)
{
	const ParameterSpec parameters[] = { { "fn", kPtr }, { "x", kI64 } };
	FunctionBuilder builder(program, define_function(program, "call_twice", parameters, 2, kI64));
	begin_entry(builder);
	const ParameterSpec callee_parameters[] = { { "value", kI64 } };
	const SignatureId callee = add_signature(program, callee_parameters, 1, kI64);
	const Value function = Value::temp(builder.parameter(0));
	const Value x = Value::temp(builder.parameter(1));
	const TempId first = builder.call(kI64, function, &x, 1, callee, intern(program, "first"));
	const Value first_value = Value::temp(first);
	const TempId second = builder.call(kI64, function, &first_value, 1, callee, intern(program, "second"));
	builder.ret(kI64, Value::temp(second));
}

} // namespace

bool build_exercise(Program& program, const std::string& exercise)
{
	if (exercise == "sum")
		build_sum(program);
	else if (exercise == "swap")
		build_swap(program);
	else if (exercise == "call")
		build_call(program);
	else
		return false;
	return true;
}

} // namespace lowir
