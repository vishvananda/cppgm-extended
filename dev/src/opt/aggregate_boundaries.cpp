#include "lowir/builder.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

// Integer-class aggregates use scalar values at register boundaries. Their
// storage accesses still use the original extent, including partial words.
bool word_object(Type t)
{
	return t.is_object() && (storage_bytes(t) >= 1 && storage_bytes(t) <= 16) && t.align_log2 <= 3;
}
bool word_boundary(Type t, uint8_t classes)
{
	return word_object(t) &&
		(classes == 0 || classes == (OC_STATED | EB_INTEGER | (storage_bytes(t) > 8 ? (EB_INTEGER << 2) : 0)));
}
Type word_type(Type t)
{
	return Type::scalar(storage_bytes(t) > 8 ? TypeKind::I128 : TypeKind::I64);
}
} // namespace

void scalarize_aggregate_boundaries(Program& program)
{
	const std::vector<Signature> old_signatures = program.signatures;
	const std::vector<Parameter> old_parameters = program.parameters;
	bool changed = false;
	for (Signature& sig : program.signatures)
	{
		if (word_boundary(sig.result, sig.boundary.result_classes))
		{
			sig.result = word_type(sig.result);
			sig.boundary.result_classes = sig.result.kind() == TypeKind::I128
				? (OC_STATED | EB_INTEGER | (EB_INTEGER << 2))
				: 0;
			changed = true;
		}
		for (uint32_t p = 0; p < sig.parameter_count; ++p)
		{
			Parameter& param = program.parameters[sig.first_parameter + p];
			if (param.pass == ir_model::PPM_DIRECT && word_boundary(param.type, param.classes))
			{
				param.type = word_type(param.type);
				param.classes = param.type.kind() == TypeKind::I128 ? (OC_STATED | EB_INTEGER | (EB_INTEGER << 2)) : 0;
				changed = true;
			}
		}
	}
	if (!changed)
		return;
	for (SymbolId symbol : program.order)
	{
		if (program.symbols[symbol].kind != SymbolKind::FunctionDefinition)
			continue;
		Function& function = program.functions[program.symbols[symbol].detail];
		bool wide = false;
		if (function.body == nullptr)
			continue;
		FunctionBody& body = *function.body;
		const Signature& sig = old_signatures[function.signature];
		const uint32_t temps = body.temps.size();
		std::vector<SlotId> homes(temps, kNoId);
		auto home = [&](TempId t) {
			homes[t] = body.slots.size();
			body.slots.push_back(Slot{ 0, SlotKind::Aggregate, 0, 0,
				Type::object(storage_bytes(body.temps[t].type) > 8 ? 16 : 8, body.temps[t].type.align_log2) });
			body.temps[t].type = word_type(body.temps[t].type);
			wide = wide || body.temps[t].type.kind() == TypeKind::I128;
		};
		for (uint32_t p = 0; p < body.parameter_count; ++p)
			if (old_parameters[sig.first_parameter + p].type != program.parameters[sig.first_parameter + p].type)
				home(p);
		// The signatures retain their ids, so explicit and symbol-based calls
		// use the same ABI choice, including calls through function pointers.
		for (BlockId b : body.layout)
			for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			{
				Instruction& inst = body.instructions[i];
				if (inst.op != Opcode::Call || !word_object(inst.type))
					continue;
				const Signature* target = program.call_boundary(inst);
				if (target != nullptr &&
					(target->result.kind() == TypeKind::I64 || target->result.kind() == TypeKind::I128))
				{
					inst.type = target->result;
					if (inst.result != kNoId)
						home(inst.result);
				}
			}
		auto emit = [&](BlockId b, InstId& prev, Instruction inst) {
			const InstId added = append_instruction(body, inst);
			insert_after(body, b, prev, added);
			prev = added;
			return added;
		};
		auto save = [&](BlockId b, InstId& prev, TempId t, uint32_t location) {
			Instruction st = make_instruction(Opcode::Store, body.temps[t].type);
			st.ops[0] = Value::temp(t);
			st.ops[1] = Value::slot(homes[t]);
			st.location = location;
			emit(b, prev, st);
		};
		auto load = [&](BlockId b, InstId& prev, Value storage, Type original, uint32_t location) {
			const Type type = word_type(original);
			wide = wide || type.kind() == TypeKind::I128;
			auto value = [&](Instruction inst, Type result) {
				inst.location = location;
				const InstId id = emit(b, prev, inst);
				const TempId t = append_temp(body, result, id);
				body.instructions[id].result = t;
				return Value::temp(t);
			};
			auto literal = [&](uint64_t bits) {
				const uint32_t id = body.literals.size();
				body.literals.push_back(Literal{ bits, 0 });
				return Value::make(ValueKind::Integer, id);
			};
			const uint64_t size = storage_bytes(original);
			if (storage.is(ValueKind::Temp) && body.temps[storage.index].type.is_object())
			{
				// An object value is a snapshot, not a pointer to its original
				// source. Give it explicit storage before reading its bits.
				const SlotId slot = body.slots.size();
				body.slots.push_back(Slot{ 0, SlotKind::Aggregate, 0, 0, original });
				Instruction copy = make_instruction(Opcode::CopyObject, Type::scalar(TypeKind::Void));
				copy.ops[0] = storage;
				copy.ops[1] = Value::slot(slot);
				copy.immediate = size;
				copy.sub = original.align_log2;
				copy.location = location;
				emit(b, prev, copy);
				storage = Value::slot(slot);
			}
			if (size == 8 || size == 16)
			{
				Instruction ld = make_instruction(Opcode::Load, type);
				ld.ops[0] = storage;
				return value(ld, type);
			}
			Value address = storage;
			if (address.is(ValueKind::Slot))
			{
				Instruction addr = make_instruction(Opcode::Addr, body.pointer_type());
				addr.ops[0] = storage;
				address = value(addr, addr.type);
			}
			Value result = Value::none();
			for (uint64_t offset = 0; offset < size;)
			{
				const uint64_t left = size - offset;
				const uint64_t bytes = left >= 8 ? 8 : left >= 4 ? 4 : left >= 2 ? 2 : 1;
				const Type field = Type::scalar(bytes == 8 ? TypeKind::I64
						: bytes == 4                       ? TypeKind::U32
						: bytes == 2                       ? TypeKind::U16
														   : TypeKind::U8);
				Value at = address;
				if (offset != 0)
				{
					Instruction index = make_instruction(Opcode::Index, Type::scalar(TypeKind::I8));
					index.sub = ir_model::IPK_FIELD;
					index.ops[0] = address;
					index.ops[1] = literal(offset);
					at = value(index, body.pointer_type());
				}
				Instruction ld = make_instruction(Opcode::Load, field);
				ld.ops[0] = at;
				Value part = value(ld, field);
				Instruction extend = make_instruction(Opcode::Convert, type);
				extend.sub = static_cast<uint8_t>(ConvertOp::ZExt);
				extend.ops[0] = part;
				set_convert_source_type(extend, field);
				part = value(extend, type);
				if (offset != 0)
				{
					Instruction shift = make_instruction(Opcode::Binary, type);
					shift.sub = static_cast<uint8_t>(BinaryOp::Shl);
					shift.ops[0] = part;
					shift.ops[1] = literal(offset * 8);
					part = value(shift, type);
				}
				if (result.kind == ValueKind::None)
					result = part;
				else
				{
					Instruction join = make_instruction(Opcode::Binary, type);
					join.sub = static_cast<uint8_t>(BinaryOp::Or);
					join.ops[0] = result;
					join.ops[1] = part;
					result = value(join, type);
				}
				offset += bytes;
			}
			return result;
		};
		for (BlockId b : body.layout)
		{
			InstId prev = kNoId;
			for (InstId i = body.blocks[b].first; i != kNoId;)
			{
				Instruction inst = body.instructions[i];
				const InstId next = inst.next;
				for_each_operand(body, inst, [&](Value& v) {
					if (v.is(ValueKind::Temp) && v.index < temps && homes[v.index] != kNoId)
						v = Value::slot(homes[v.index]);
				});
				if (inst.op == Opcode::Call)
				{
					const Signature* target = program.call_boundary(inst);
					if (target != nullptr)
					{
						const Signature& old = old_signatures[target - program.signatures.data()];
						for (uint32_t p = 0; p < inst.extra_count && p < old.parameter_count; ++p)
							if (old_parameters[old.first_parameter + p].type !=
								program.parameters[target->first_parameter + p].type)
							{
								Value arg = body.operands[inst.extra_first + p];
								body.operands[inst.extra_first + p] = load(
									b, prev, arg, old_parameters[old.first_parameter + p].type, inst.location);
							}
					}
				}
				else if (inst.op == Opcode::Return && word_boundary(sig.result, sig.boundary.result_classes))
				{
					inst.type = word_type(sig.result);
					wide = wide || inst.type.kind() == TypeKind::I128;
					inst.ops[0] = load(b, prev, inst.ops[0], sig.result, inst.location);
				}
				else if (inst.op == Opcode::Store && word_object(inst.type) && inst.ops[0].is(ValueKind::Slot))
				{
					inst.op = Opcode::CopyObject;
					inst.immediate = storage_bytes(inst.type);
					inst.sub = inst.type.align_log2;
					inst.type = Type::scalar(TypeKind::Void);
				}
				else if (inst.op == Opcode::Copy && word_object(inst.type) && inst.ops[0].is(ValueKind::Slot))
					inst.op = Opcode::Load;
				body.instructions[i] = inst;
				prev = i;
				if (inst.result != kNoId && inst.result < temps && homes[inst.result] != kNoId)
					save(b, prev, inst.result, inst.location);
				i = next;
			}
		}
		if (!body.layout.empty())
		{
			InstId prev = kNoId;
			for (uint32_t p = 0; p < body.parameter_count; ++p)
				if (homes[p] != kNoId)
					save(body.layout.front(), prev, p, 0);
		}
		if (wide)
		{
			OptimizerStats stats;
			FunctionContext context(program, symbol, stats, 2);
			expand_wide_integers(context);
		}
	}
}

} // namespace opt
