#include <cstring>

#include "lowir/builder.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

// `sin(x)` and `cos(x)` of one value share their work: `sincos` gives both for
// about the price of one. The first call becomes `sincos`, and the calls it
// dominates read what that wrote.
class SinCos
{
public:
	explicit SinCos(FunctionContext& context) : c_(context), body_(context.body) {}

	bool run();

private:
	struct Call
	{
		InstId id;
		BlockId block;
		Value x;
		MathFunction function;
		bool done;
	};

	SymbolId sincos_symbol(TypeKind kind);
	void combine(size_t first, const std::vector<size_t>& later, Value sine, Value cosine);
	Value emit(BlockId block, InstId& at, Instruction inst, Type type, uint32_t location);

	FunctionContext& c_;
	FunctionBody& body_;
	std::vector<Call> calls_;
};

// `sincos` or `sincosf`, declared on first use (kNoId when the unit gives that
// name to something else).
SymbolId SinCos::sincos_symbol(TypeKind kind)
{
	Program& program = c_.program;
	const char* name = kind == TypeKind::F64 ? "sincos" : "sincosf";
	const NameId id = program.names.intern(name, static_cast<uint32_t>(std::strlen(name)));
	const SymbolId symbol = program.symbol_for(id);
	const Symbol& known = program.symbols[symbol];
	if (known.kind != SymbolKind::Unresolved)
		return known.kind == SymbolKind::FunctionDeclaration && known.metadata.linkage == ir_model::LLM_C ? symbol
																										  : kNoId;
	Signature sig;
	sig.first_parameter = static_cast<uint32_t>(program.parameters.size());
	sig.parameter_count = 3;
	sig.result = Type::scalar(TypeKind::Void);
	sig.boundary.unwind = ir_model::CUM_NO;
	const Type types[3] = { Type::scalar(kind), Type::scalar(TypeKind::Ptr), Type::scalar(TypeKind::Ptr) };
	for (const Type& type : types)
	{
		Parameter p = Parameter();
		p.name = 0;
		p.type = type;
		p.pass = ir_model::PPM_DIRECT;
		p.object_bytes = 0;
		program.parameters.push_back(p);
	}
	program.declare_function(symbol, SymbolKind::FunctionDeclaration, program.add_signature(sig));
	SymbolMetadata& m = program.symbols[symbol].metadata;
	m.binding = ir_model::SBM_STRONG;
	m.linkage = ir_model::LLM_C;
	m.object = id;
	return symbol;
}

Value SinCos::emit(BlockId block, InstId& at, Instruction inst, Type type, uint32_t location)
{
	inst.location = location;
	const InstId id = append_instruction(body_, inst);
	if (!type.is_void())
		body_.instructions[id].result = append_temp(body_, type, id);
	insert_after(body_, block, at, id);
	at = id;
	return type.is_void() ? Value::none() : Value::temp(body_.instructions[id].result);
}

bool SinCos::run()
{
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op != Opcode::Call || !inst.ops[0].is(ValueKind::Symbol) || inst.extra_count != 1 ||
				inst.result == kNoId)
				continue;
			const MathFunction function = c_.program.math_function(inst.ops[0].index);
			if ((function != MathFunction::Sin && function != MathFunction::Cos) ||
				(inst.type.kind() != TypeKind::F32 && inst.type.kind() != TypeKind::F64))
				continue;
			const Value x = body_.operands[inst.extra_first];
			if (x.is(ValueKind::Temp) && body_.temps[x.index].type == inst.type)
				calls_.push_back(Call{ i, b, x, function, false });
		}
	if (calls_.size() < 2)
		return false;
	const Cfg& cfg = c_.ensure_cfg();
	bool changed = false;
	std::vector<size_t> later;
	for (size_t first = 0; first < calls_.size(); ++first)
	{
		if (calls_[first].done)
			continue;
		later.clear();
		bool other = false;
		const Type type = body_.instructions[calls_[first].id].type;
		for (size_t k = first + 1; k < calls_.size(); ++k)
		{
			const Call& call = calls_[k];
			if (call.done || !(call.x == calls_[first].x) || !(body_.instructions[call.id].type == type) ||
				!cfg.dominates(calls_[first].block, call.block))
				continue;
			later.push_back(k);
			other = other || call.function != calls_[first].function;
		}
		if (!other)
			continue;
		const SymbolId symbol = sincos_symbol(type.kind());
		if (symbol == kNoId)
			return changed;
		const Type pointer = Type::scalar(TypeKind::Ptr);
		const uint32_t slot = static_cast<uint32_t>(body_.slots.size());
		body_.slots.push_back(Slot{ 0, SlotKind::Aggregate, 0, 0, type });
		body_.slots.push_back(Slot{ 0, SlotKind::Aggregate, 0, 0, type });
		const Call& head = calls_[first];
		const uint32_t location = body_.instructions[head.id].location;
		InstId at = previous_in_block(body_, head.block, head.id);
		Instruction addr = make_instruction(Opcode::Addr, pointer);
		addr.ops[0] = Value::slot(slot);
		const Value sine_at = emit(head.block, at, addr, pointer, location);
		addr.ops[0] = Value::slot(slot + 1);
		const Value cosine_at = emit(head.block, at, addr, pointer, location);
		Instruction call = make_instruction(Opcode::Call, Type::scalar(TypeKind::Void));
		call.ops[0] = Value::symbol(symbol);
		const Value args[3] = { head.x, sine_at, cosine_at };
		call.extra_first = append_operands(body_, args, 3);
		call.extra_count = 3;
		emit(head.block, at, call, Type::scalar(TypeKind::Void), location);
		Instruction load = make_instruction(Opcode::Load, type);
		load.ops[0] = sine_at;
		const Value sine = emit(head.block, at, load, type, location);
		load.ops[0] = cosine_at;
		const Value cosine = emit(head.block, at, load, type, location);
		combine(first, later, sine, cosine);
		changed = true;
	}
	if (changed)
	{
		c_.invalidate_block_of();
		c_.invalidate_cfg();
	}
	return changed;
}

// The calls give way to the two values.
void SinCos::combine(size_t first, const std::vector<size_t>& later, Value sine, Value cosine)
{
	auto drop = [&](size_t k) {
		Call& call = calls_[k];
		const TempId result = body_.instructions[call.id].result;
		replace_all_uses(body_, result, call.function == MathFunction::Sin ? sine : cosine);
		unlink_after(body_, call.block, previous_in_block(body_, call.block, call.id), call.id);
		call.done = true;
		++c_.stats.math_calls_shared;
	};
	drop(first);
	for (size_t k : later)
		drop(k);
}

} // namespace

bool combine_sin_cos(FunctionContext& context)
{
	SinCos pass(context);
	return pass.run();
}

} // namespace opt
