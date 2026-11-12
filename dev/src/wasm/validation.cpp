#include "wasm/validation.h"

namespace wasm {

namespace {

const ValueType kUnknown = static_cast<ValueType>(0);

// A view of either a module-owned type vector or an inline scalar. Ordinary
// empty/single-result control regions need no per-region heap allocation.
struct Types
{
	Types() = default;
	explicit Types(ValueType type) : single(type) {}
	explicit Types(const std::vector<ValueType>& types) : many(&types) {}
	size_t size() const { return many != nullptr ? many->size() : single != ValueType::Void; }
	ValueType operator[](size_t i) const { return many != nullptr ? (*many)[i] : single; }
	bool operator==(Types other) const
	{
		if (size() != other.size())
			return false;
		for (size_t i = 0; i < size(); ++i)
			if ((*this)[i] != other[i])
				return false;
		return true;
	}
	const std::vector<ValueType>* many = nullptr;
	ValueType single = ValueType::Void;
};

struct Control
{
	Control(Op kind, size_t height, Types parameters, Types results)
		: kind(kind), height(height), parameters(parameters), results(results)
	{}
	Op kind;
	size_t height;
	Types parameters;
	Types results;
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
	ValueType pop_any();
	void pop(ValueType expected);
	void pop(Types types);
	void push(ValueType type);
	void push(Types types);
	void open_control(Op op, int64_t type);
	const Signature& tag_type(uint64_t tag) const;
	void try_table(const Instruction& inst);
	void unreachable();
	void finish_arm();
	void call(const Signature& signature);
	void instruction(const Instruction& inst);
	bool numeric(Op op);
	bool simd(const Instruction& inst);
	Types branch_types(uint64_t depth) const;
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
	if (form == Immediate::MemoryLane)
		return 3;
	if (form == Immediate::Bytes16 || form == Immediate::Shuffle)
		return 16;
	return 1;
}

ValueType Validator::pop_any()
{
	const Control& control = controls_.back();
	if (stack_.size() == control.height && control.unreachable)
		return kUnknown;
	if (stack_.size() <= control.height)
		throw WasmError("Wasm operand stack underflow");
	const ValueType type = stack_.back();
	stack_.pop_back();
	return type;
}

void Validator::pop(ValueType expected)
{
	const ValueType actual = pop_any();
	if (actual != kUnknown && actual != expected)
		throw WasmError(std::string("Wasm operand stack type mismatch: expected ") + spelling(expected) + ", got " +
			spelling(actual));
}

void Validator::pop(Types types)
{
	for (size_t i = types.size(); i != 0; --i)
		pop(types[i - 1]);
}

void Validator::push(ValueType type)
{
	if (type != ValueType::Void)
		stack_.push_back(type);
}

void Validator::push(Types types)
{
	for (size_t i = 0; i < types.size(); ++i)
		push(types[i]);
}

void Validator::open_control(Op op, int64_t type)
{
	Types parameters, results;
	if (type >= 0)
	{
		if (static_cast<uint64_t>(type) >= module_.signatures.size())
			throw WasmError("invalid Wasm block signature index");
		const Signature& signature = module_.signatures[static_cast<size_t>(type)];
		parameters = Types(signature.parameters);
		results = Types(signature.results);
	}
	else
	{
		if (type < -64 || *spelling(static_cast<ValueType>(type + 128)) == '\0')
			throw WasmError("invalid Wasm inline block type");
		results = Types(static_cast<ValueType>(type + 128));
	}
	pop(parameters);
	controls_.push_back(Control{ op, stack_.size(), parameters, results });
	push(parameters);
}

const Signature& Validator::tag_type(uint64_t tag) const
{
	if (tag >= module_.tag_types.size() || module_.tag_types[tag] >= module_.signatures.size())
		throw WasmError("invalid Wasm exception tag index");
	const Signature& type = module_.signatures[module_.tag_types[tag]];
	if (!type.results.empty())
		throw WasmError("Wasm exception tag cannot have results");
	return type;
}

void Validator::try_table(const Instruction& inst)
{
	uint32_t at = 2;
	const auto take = [&]() {
		if (at >= inst.operands.count)
			throw WasmError("truncated Wasm try_table clause");
		return operand(inst, at++);
	};
	for (uint64_t i = 0; i < operand(inst, 1); ++i)
	{
		const uint64_t kind = take();
		if (kind > static_cast<unsigned>(CatchKind::AllRef))
			throw WasmError("invalid Wasm catch kind");
		const bool tagged = kind <= static_cast<unsigned>(CatchKind::TagRef);
		const bool reference = kind == static_cast<unsigned>(CatchKind::TagRef) ||
			kind == static_cast<unsigned>(CatchKind::AllRef);
		const Types payload = tagged ? Types(tag_type(take()).parameters) : Types();
		const Types target = branch_types(take());
		if (target.size() != payload.size() + reference)
			throw WasmError("Wasm catch target tuple arity mismatch");
		for (size_t j = 0; j < payload.size(); ++j)
			if (target[j] != payload[j])
				throw WasmError("Wasm catch target payload type mismatch");
		if (reference && target[payload.size()] != ValueType::ExnRef)
			throw WasmError("Wasm catch target requires an exception reference");
	}
	if (at != inst.operands.count)
		throw WasmError("excess Wasm try_table operands");
	open_control(Op::TryTable, static_cast<int64_t>(operand(inst)));
}

void Validator::unreachable()
{
	stack_.resize(controls_.back().height);
	controls_.back().unreachable = true;
}

void Validator::finish_arm()
{
	const Control& control = controls_.back();
	pop(control.results);
	if (stack_.size() != control.height)
		throw WasmError("Wasm control region leaves excess operands");
}

Types Validator::branch_types(uint64_t depth) const
{
	if (depth >= controls_.size())
		throw WasmError("Wasm branch depth exceeds the control stack");
	const Control& control = controls_[controls_.size() - 1 - depth];
	return control.kind == Op::Loop ? control.parameters : control.results;
}

uint64_t Validator::operand(const Instruction& inst, uint32_t index) const
{
	return function_.operands[inst.operands.first + index];
}

void Validator::call(const Signature& signature)
{
	pop(Types(signature.parameters));
	push(Types(signature.results));
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

bool Validator::simd(const Instruction& inst)
{
	const SimdInfo info = simd_info(inst.op);
	if (info.form == SimdForm::None)
		return false;
	const Immediate immediate = immediate_form(inst.op);
	if ((immediate == Immediate::Memory || immediate == Immediate::MemoryLane) &&
		(operand(inst) > info.alignment || operand(inst, 1) > UINT32_MAX))
		throw WasmError("invalid Wasm SIMD memory alignment");
	if ((immediate == Immediate::Lane || immediate == Immediate::MemoryLane) &&
		operand(inst, immediate == Immediate::Lane ? 0 : 2) >= info.lanes)
		throw WasmError("invalid Wasm SIMD lane index");
	if (immediate == Immediate::Shuffle || immediate == Immediate::Bytes16)
		for (uint32_t i = 0; i < inst.operands.count; ++i)
			if (operand(inst, i) >= (immediate == Immediate::Shuffle ? 32u : 256u))
				throw WasmError("invalid Wasm SIMD byte immediate");
	ValueType scalar = ValueType::Void;
	bool extract = false, replace = false, splat = false;
	switch (info.form)
	{
	case SimdForm::ExtractI32:
		scalar = ValueType::I32;
		extract = true;
		break;
	case SimdForm::ReplaceI32:
		scalar = ValueType::I32;
		replace = true;
		break;
	case SimdForm::SplatI32:
		scalar = ValueType::I32;
		splat = true;
		break;
	case SimdForm::ExtractI64:
		scalar = ValueType::I64;
		extract = true;
		break;
	case SimdForm::ReplaceI64:
		scalar = ValueType::I64;
		replace = true;
		break;
	case SimdForm::SplatI64:
		scalar = ValueType::I64;
		splat = true;
		break;
	case SimdForm::ExtractF32:
		scalar = ValueType::F32;
		extract = true;
		break;
	case SimdForm::ReplaceF32:
		scalar = ValueType::F32;
		replace = true;
		break;
	case SimdForm::SplatF32:
		scalar = ValueType::F32;
		splat = true;
		break;
	case SimdForm::ExtractF64:
		scalar = ValueType::F64;
		extract = true;
		break;
	case SimdForm::ReplaceF64:
		scalar = ValueType::F64;
		replace = true;
		break;
	case SimdForm::SplatF64:
		scalar = ValueType::F64;
		splat = true;
		break;
	default:
		break;
	}
	if (extract || replace || splat)
	{
		if (!extract)
			pop(scalar);
		if (!splat)
			pop(ValueType::V128);
		push(extract ? scalar : ValueType::V128);
		return true;
	}
	switch (info.form)
	{
	case SimdForm::Load:
		pop(ValueType::I32);
		break;
	case SimdForm::Store:
	case SimdForm::LoadLane:
		pop(ValueType::V128);
		pop(ValueType::I32);
		break;
	case SimdForm::Shift:
		pop(ValueType::I32);
		pop(ValueType::V128);
		break;
	case SimdForm::Ternary:
		pop(ValueType::V128);
		// fall through
	case SimdForm::Binary:
		pop(ValueType::V128);
		// fall through
	case SimdForm::Unary:
	case SimdForm::ToI32:
		pop(ValueType::V128);
		break;
	case SimdForm::Constant:
		break;
	default:
		throw WasmError("unknown Wasm SIMD stack form");
	}
	if (info.form != SimdForm::Store)
		push(info.form == SimdForm::ToI32 ? ValueType::I32 : ValueType::V128);
	return true;
}

void Validator::instruction(const Instruction& inst)
{
	if (simd(inst) || numeric(inst.op))
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
	case Op::RefNull:
		if (operand(inst) != block_type(ValueType::ExnRef))
			throw WasmError("unsupported Wasm null reference type");
		push(ValueType::ExnRef);
		return;
	case Op::RefIsNull:
		pop(ValueType::ExnRef);
		push(ValueType::I32);
		return;
	case Op::Throw:
		pop(Types(tag_type(operand(inst)).parameters));
		unreachable();
		return;
	case Op::ThrowRef:
		pop(ValueType::ExnRef);
		unreachable();
		return;
	case Op::TryTable:
		try_table(inst);
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
	{
		const uint64_t index = operand(inst);
		if (index >= module_.global_types.size() || index >= module_.mutable_globals.size())
			throw WasmError("invalid Wasm global index");
		if (inst.op == Op::GlobalGet)
			push(module_.global_types[index]);
		else
		{
			if (!module_.mutable_globals[index])
				throw WasmError("Wasm store to an immutable global");
			pop(module_.global_types[index]);
		}
		return;
	}
	case Op::Block:
	case Op::Loop:
	case Op::If:
		if (inst.op == Op::If)
			pop(ValueType::I32);
		open_control(inst.op, static_cast<int64_t>(operand(inst)));
		return;
	case Op::Else:
		if (controls_.back().kind != Op::If || controls_.back().alternate)
			throw WasmError("unmatched Wasm else");
		finish_arm();
		controls_.back().alternate = true;
		controls_.back().unreachable = false;
		push(controls_.back().parameters);
		return;
	case Op::End:
	{
		const Control control = controls_.back();
		if (control.kind == Op::If && !control.alternate && !(control.parameters == control.results))
			throw WasmError("Wasm if without else must preserve its parameter types");
		finish_arm();
		controls_.pop_back();
		push(control.results);
		return;
	}
	case Op::Br:
	case Op::BrIf:
	{
		if (inst.op == Op::BrIf)
			pop(ValueType::I32);
		const Types types = branch_types(operand(inst));
		pop(types);
		if (inst.op == Op::Br)
			unreachable();
		else
			push(types);
		return;
	}
	case Op::BrTable:
	{
		pop(ValueType::I32);
		const Types types = branch_types(operand(inst));
		for (uint32_t i = 1; i < inst.operands.count; ++i)
			if (!(branch_types(operand(inst, i)) == types))
				throw WasmError("Wasm branch table type mismatch");
		pop(types);
		unreachable();
		return;
	}
	case Op::Return:
		pop(Types(signature_.results));
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
		pop_any();
		return;
	case Op::Select:
	{
		pop(ValueType::I32);
		const ValueType second = pop_any(), first = pop_any();
		if (first != kUnknown && second != kUnknown && first != second)
			throw WasmError("Wasm select type mismatch");
		const ValueType type = first != kUnknown ? first : second;
		if (type == ValueType::ExnRef)
			throw WasmError("Wasm reference selection requires a typed select");
		push(type);
		return;
	}
	case Op::SelectTyped:
	{
		const uint64_t code = operand(inst);
		if (code > 255 || static_cast<ValueType>(code) == ValueType::Void ||
			*spelling(static_cast<ValueType>(code)) == '\0')
			throw WasmError("invalid Wasm typed select type");
		const ValueType type = static_cast<ValueType>(code);
		pop(ValueType::I32);
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
		if (function_.locals[i] == ValueType::Void || *spelling(function_.locals[i]) == '\0' ||
			(i < function_.parameter_count && function_.locals[i] != signature_.parameters[i]))
			throw WasmError("invalid Wasm local/parameter type");
	controls_.push_back(Control{ Op::Return, 0, Types(), Types(signature_.results) });
	for (const Instruction& inst : function_.instructions)
	{
		if (controls_.empty())
			throw WasmError("Wasm instruction after function end");
		if (*spelling(inst.op) == '\0' ||
			uint64_t(inst.operands.first) + inst.operands.count > function_.operands.size())
			throw WasmError("invalid Wasm instruction operands");
		const Immediate form = immediate_form(inst.op);
		if (form == Immediate::Table          ? inst.operands.count == 0
				: form == Immediate::TryTable ? inst.operands.count < 2
											  : inst.operands.count != operand_count(form))
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
