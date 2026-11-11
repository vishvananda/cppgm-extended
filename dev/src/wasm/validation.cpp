#include "wasm/validation.h"

namespace wasm {

namespace {

struct Control
{
	Control(Op kind, size_t height, ValueType result) : kind(kind), height(height), result(result) {}
	Op kind;
	size_t height;
	ValueType result;
	bool unreachable = false;
	bool alternate = false;
};

class Validator
{
public:
	Validator(const Function& function, const Signature& signature, const Module& module)
		: function_(function), signature_(signature), module_(module)
	{}
	void run();

private:
	void pop(ValueType expected);
	void push(ValueType type);
	void unreachable();
	void finish_arm();
	void call(const Signature& signature);
	void instruction(const Instruction& inst);
	bool numeric(Op op);
	ValueType branch_type(uint64_t depth) const;
	uint64_t operand(const Instruction& inst, uint32_t index = 0) const;
	const Function& function_;
	const Signature& signature_;
	const Module& module_;
	std::vector<ValueType> stack_;
	std::vector<Control> controls_;
};

uint32_t operand_count(Immediate form)
{
	if (form == Immediate::None)
		return 0;
	if (form == Immediate::Memory || form == Immediate::Indices)
		return 2;
	return 1;
}

void Validator::pop(ValueType expected)
{
	const Control& control = controls_.back();
	if (stack_.size() == control.height && control.unreachable)
		return;
	if (stack_.size() <= control.height || stack_.back() != expected)
		throw WasmError("Wasm operand stack type mismatch");
	stack_.pop_back();
}

void Validator::push(ValueType type)
{
	if (type != ValueType::Void)
		stack_.push_back(type);
}

void Validator::unreachable()
{
	stack_.resize(controls_.back().height);
	controls_.back().unreachable = true;
}

void Validator::finish_arm()
{
	const Control& control = controls_.back();
	if (control.result != ValueType::Void)
		pop(control.result);
	if (stack_.size() != control.height)
		throw WasmError("Wasm control region leaves excess operands");
}

ValueType Validator::branch_type(uint64_t depth) const
{
	if (depth >= controls_.size())
		throw WasmError("Wasm branch depth exceeds the control stack");
	const Control& control = controls_[controls_.size() - 1 - depth];
	return control.kind == Op::Loop ? ValueType::Void : control.result;
}

uint64_t Validator::operand(const Instruction& inst, uint32_t index) const
{
	return function_.operands[inst.operands.first + index];
}

void Validator::call(const Signature& signature)
{
	for (auto type = signature.parameters.rbegin(); type != signature.parameters.rend(); ++type)
		pop(*type);
	push(signature.result);
}

bool Validator::numeric(Op op)
{
	const unsigned code = static_cast<unsigned>(op);
	ValueType input = ValueType::Void, output = ValueType::Void;
	unsigned count = 1;
	if (code >= 0x45 && code <= 0x66)
	{
		input = code <= 0x4f ? ValueType::I32
			: code <= 0x5a   ? ValueType::I64
			: code <= 0x60   ? ValueType::F32
							 : ValueType::F64;
		output = ValueType::I32;
		count = code == 0x45 || code == 0x50 ? 1 : 2;
	}
	else if (code >= 0x67 && code <= 0xa6)
	{
		input = output = code <= 0x78 ? ValueType::I32
			: code <= 0x8a            ? ValueType::I64
			: code <= 0x98            ? ValueType::F32
									  : ValueType::F64;
		count = (code >= 0x6a && code <= 0x78) || (code >= 0x7c && code <= 0x8a) || (code >= 0x92 && code <= 0x98) ||
				code >= 0xa0
			? 2
			: 1;
	}
	else if ((code >= 0xa8 && code <= 0xab) || (code >= 0xae && code <= 0xb1))
	{
		const unsigned conversion = code - (code <= 0xab ? 0xa8 : 0xae);
		input = conversion < 2 ? ValueType::F32 : ValueType::F64;
		output = code <= 0xab ? ValueType::I32 : ValueType::I64;
	}
	else if (code >= 0xfc00 && code <= 0xfc07)
	{
		input = (code & 2) != 0 ? ValueType::F64 : ValueType::F32;
		output = (code & 4) != 0 ? ValueType::I64 : ValueType::I32;
	}
	else
	{
		switch (op)
		{
		case Op::I32WrapI64:
			input = ValueType::I64;
			output = ValueType::I32;
			break;
		case Op::I64ExtendI32S:
		case Op::I64ExtendI32U:
			input = ValueType::I32;
			output = ValueType::I64;
			break;
		case Op::F32ConvertI32S:
		case Op::F32ConvertI32U:
		case Op::F32ReinterpretI32:
			input = ValueType::I32;
			output = ValueType::F32;
			break;
		case Op::F32ConvertI64S:
		case Op::F32ConvertI64U:
			input = ValueType::I64;
			output = ValueType::F32;
			break;
		case Op::F64ConvertI32S:
		case Op::F64ConvertI32U:
			input = ValueType::I32;
			output = ValueType::F64;
			break;
		case Op::F64ConvertI64S:
		case Op::F64ConvertI64U:
		case Op::F64ReinterpretI64:
			input = ValueType::I64;
			output = ValueType::F64;
			break;
		case Op::F32DemoteF64:
			input = ValueType::F64;
			output = ValueType::F32;
			break;
		case Op::F64PromoteF32:
			input = ValueType::F32;
			output = ValueType::F64;
			break;
		case Op::I32ReinterpretF32:
			input = ValueType::F32;
			output = ValueType::I32;
			break;
		case Op::I64ReinterpretF64:
			input = ValueType::F64;
			output = ValueType::I64;
			break;
		case Op::I64Extend8S:
		case Op::I64Extend16S:
		case Op::I64Extend32S:
			input = output = ValueType::I64;
			break;
		case Op::I32Extend8S:
		case Op::I32Extend16S:
			input = output = ValueType::I32;
			break;
		default:
			return false;
		}
	}
	while (count-- != 0)
		pop(input);
	push(output);
	return true;
}

void Validator::instruction(const Instruction& inst)
{
	if (numeric(inst.op))
		return;
	switch (inst.op)
	{
	case Op::Nop:
		return;
	case Op::MemorySize:
	case Op::MemoryGrow:
		if (operand(inst) != 0)
			throw WasmError("invalid Wasm memory index");
		if (inst.op == Op::MemoryGrow)
			pop(ValueType::I32);
		push(ValueType::I32);
		return;
	case Op::Unreachable:
		unreachable();
		return;
	case Op::I32Const:
		push(ValueType::I32);
		return;
	case Op::I64Const:
		push(ValueType::I64);
		return;
	case Op::F32Const:
		push(ValueType::F32);
		return;
	case Op::F64Const:
		push(ValueType::F64);
		return;
	case Op::LocalGet:
	case Op::LocalSet:
	case Op::LocalTee:
	{
		const uint64_t index = operand(inst);
		if (index >= function_.locals.size())
			throw WasmError("invalid Wasm local index");
		if (inst.op != Op::LocalGet)
			pop(function_.locals[index]);
		if (inst.op != Op::LocalSet)
			push(function_.locals[index]);
		return;
	}
	case Op::GlobalGet:
	case Op::GlobalSet:
		if (operand(inst) != 0)
			throw WasmError("invalid Wasm stack global index");
		if (inst.op == Op::GlobalGet)
			push(ValueType::I32);
		else
			pop(ValueType::I32);
		return;
	case Op::Block:
	case Op::Loop:
	case Op::If:
		if (inst.op == Op::If)
			pop(ValueType::I32);
		if (operand(inst) != static_cast<uint8_t>(ValueType::Void))
			throw WasmError("unsupported Wasm control result");
		controls_.push_back(Control{ inst.op, stack_.size(), ValueType::Void });
		return;
	case Op::Else:
		if (controls_.back().kind != Op::If || controls_.back().alternate)
			throw WasmError("unmatched Wasm else");
		finish_arm();
		controls_.back().alternate = true;
		controls_.back().unreachable = false;
		return;
	case Op::End:
	{
		finish_arm();
		const ValueType result = controls_.back().result;
		controls_.pop_back();
		push(result);
		return;
	}
	case Op::Br:
	case Op::BrIf:
	{
		if (inst.op == Op::BrIf)
			pop(ValueType::I32);
		const ValueType type = branch_type(operand(inst));
		if (type != ValueType::Void)
			pop(type);
		if (inst.op == Op::Br)
			unreachable();
		else
			push(type);
		return;
	}
	case Op::BrTable:
	{
		pop(ValueType::I32);
		const ValueType type = branch_type(operand(inst));
		for (uint32_t i = 1; i < inst.operands.count; ++i)
			if (branch_type(operand(inst, i)) != type)
				throw WasmError("Wasm branch table type mismatch");
		if (type != ValueType::Void)
			pop(type);
		unreachable();
		return;
	}
	case Op::Return:
		if (signature_.result != ValueType::Void)
			pop(signature_.result);
		unreachable();
		return;
	case Op::Call:
	{
		const uint64_t index = operand(inst);
		if (index >= module_.definition_types.size())
			throw WasmError("invalid Wasm function index");
		call(module_.signatures[module_.definition_types[index]]);
		return;
	}
	case Op::CallIndirect:
		if (operand(inst) >= module_.signatures.size() || operand(inst, 1) != 0)
			throw WasmError("invalid Wasm indirect call type/table");
		pop(ValueType::I32);
		call(module_.signatures[operand(inst)]);
		return;
	case Op::Drop:
		if (stack_.size() == controls_.back().height && controls_.back().unreachable)
			return;
		if (stack_.size() <= controls_.back().height)
			throw WasmError("empty Wasm drop");
		stack_.pop_back();
		return;
	case Op::Select:
	{
		pop(ValueType::I32);
		if (stack_.size() <= controls_.back().height)
			throw WasmError("empty Wasm select");
		const ValueType type = stack_.back();
		pop(type);
		pop(type);
		push(type);
		return;
	}
	case Op::MemoryCopy:
	case Op::MemoryFill:
		for (uint32_t i = 0; i < inst.operands.count; ++i)
			if (operand(inst, i) != 0)
				throw WasmError("invalid Wasm memory index");
		pop(ValueType::I32);
		pop(ValueType::I32);
		pop(ValueType::I32);
		return;
	default:
		break;
	}
	const unsigned code = static_cast<unsigned>(inst.op);
	if (code >= 0x28 && code <= 0x3e)
	{
		const bool store = code >= 0x36;
		const ValueType type = code == 0x29 || (code >= 0x30 && code <= 0x35) || code == 0x37 || code >= 0x3c
			? ValueType::I64
			: code == 0x2a || code == 0x38 ? ValueType::F32
			: code == 0x2b || code == 0x39 ? ValueType::F64
										   : ValueType::I32;
		const unsigned alignment = code == 0x2c || code == 0x2d || code == 0x30 || code == 0x31 || code == 0x3a ||
				code == 0x3c
			? 0
			: code == 0x2e || code == 0x2f || code == 0x32 || code == 0x33 || code == 0x3b || code == 0x3d ? 1
			: code == 0x34 || code == 0x35 || code == 0x3e                                                 ? 2
			: type == ValueType::I64 || type == ValueType::F64                                             ? 3
																										   : 2;
		if (operand(inst) > alignment || operand(inst, 1) > UINT32_MAX)
			throw WasmError("invalid Wasm memory immediate");
		if (store)
			pop(type);
		pop(ValueType::I32);
		if (!store)
			push(type);
		return;
	}
	throw WasmError("unknown Wasm instruction");
}

void Validator::run()
{
	if (function_.parameter_count != signature_.parameters.size() ||
		function_.locals.size() < function_.parameter_count)
		throw WasmError("Wasm parameter count mismatch");
	for (uint32_t i = 0; i < function_.locals.size(); ++i)
		if (function_.locals[i] == ValueType::Void ||
			(i < function_.parameter_count && function_.locals[i] != signature_.parameters[i]))
			throw WasmError("invalid Wasm local/parameter type");
	controls_.push_back(Control{ Op::Return, 0, signature_.result });
	for (const Instruction& inst : function_.instructions)
	{
		if (controls_.empty())
			throw WasmError("Wasm instruction after function end");
		if (*spelling(inst.op) == '\0' ||
			uint64_t(inst.operands.first) + inst.operands.count > function_.operands.size())
			throw WasmError("invalid Wasm instruction operands");
		const Immediate form = immediate_form(inst.op);
		if (form == Immediate::Table ? inst.operands.count == 0 : inst.operands.count != operand_count(form))
			throw WasmError("invalid Wasm immediate count");
		instruction(inst);
	}
	if (!controls_.empty())
		throw WasmError("unterminated Wasm function/control region");
}

} // namespace

void validate_function(const Function& function, const Signature& signature, const Module& module)
{
	Validator(function, signature, module).run();
}

} // namespace wasm
