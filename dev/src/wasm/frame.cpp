#include "wasm/selector.h"

#include <algorithm>

namespace wasm {

using namespace lowir;

void Selector::prepare_locals()
{
	if (boundary_.result.passing == Passing::Indirect)
	{
		hidden_result_ = add_local(ValueType::I32);
	}
	for (const AbiValue& parameter : boundary_.parameters)
	{
		if (parameter.passing != Passing::Ignore)
			add_local(value_type(parameter.scalar));
		if (parameter.passing == Passing::Expand)
			add_local(value_type(parameter.scalar));
	}
	out_.parameter_count = static_cast<uint32_t>(out_.locals.size());
	locals_.resize(body_.temps.size(), kNoId);
	incoming_.resize(body_.parameter_count, kNoId);
	uint32_t next = hidden_result_ == kNoId ? 0 : 1;
	for (uint32_t i = 0; i < body_.temps.size(); ++i)
	{
		if (i < body_.parameter_count)
		{
			const Passing passing = boundary_.parameters[i].passing;
			if (passing == Passing::Direct || passing == Passing::Indirect)
			{
				locals_[i] = next++;
				continue;
			}
			if (passing == Passing::Coerce)
				incoming_[i] = next++;
			else if (passing == Passing::Expand)
			{
				incoming_[i] = next;
				next += 2;
			}
		}
		locals_[i] = add_local(body_.temps[i].type.is_object() ? ValueType::I32 : value_type(body_.temps[i].type));
	}
}

void Selector::frame_address(uint32_t offset)
{
	out_.emit(Op::LocalGet, { frame_ });
	if (offset != 0)
	{
		out_.emit(Op::I32Const, { offset });
		out_.emit(Op::I32Add);
	}
}

void Selector::prepare_frame()
{
	uint64_t bytes = 0;
	uint32_t alignment = 16;
	auto reserve = [&](Type type) {
		const machine_model::FrameObject object{ storage_bytes(type), data_alignment(type), 0 };
		const uint32_t align = object.alignment;
		alignment = std::max(alignment, align);
		bytes = (bytes + align - 1) & ~uint64_t(align - 1);
		const uint64_t offset = bytes;
		bytes += object.bytes;
		if (bytes + alignment > 1024 * 1024)
			throw WasmError("Wasm frame exceeds the configured stack reserve");
		return static_cast<uint32_t>(offset);
	};
	slots_.resize(body_.slots.size());
	for (uint32_t i = 0; i < body_.slots.size(); ++i)
		slots_[i] = reserve(body_.slots[i].type);
	objects_.resize(body_.temps.size(), kNoId);
	for (uint32_t i = 0; i < body_.temps.size(); ++i)
		if (body_.temps[i].type.is_object() &&
			!(i < body_.parameter_count && boundary_.parameters[i].passing == Passing::Indirect))
			objects_[i] = reserve(body_.temps[i].type);
	call_areas_.resize(body_.operands.size(), kNoId);
	bool dynamic = false;
	for (BlockId block : body_.layout)
		for (InstId i = body_.blocks[block].first; i != kNoId; i = body_.instructions[i].next)
		{
			const auto& inst = body_.instructions[i];
			dynamic = dynamic || inst.op == Opcode::StackAlloc;
			if (inst.op != Opcode::Call)
				continue;
			const auto* signature = program_.call_boundary(inst);
			if (signature == nullptr)
				throw WasmError("Wasm call requires a typed boundary");
			const Boundary& boundary = module_.boundaries[signature - program_.signatures.data()];
			const Parameter* parameters = program_.parameters_of(*signature);
			for (uint32_t p = 0; p < signature->parameter_count; ++p)
				if (boundary.parameters[p].passing == Passing::Indirect)
					call_areas_[inst.extra_first + p] = reserve(parameters[p].type);
		}
	out_.frame_bytes = static_cast<uint32_t>((bytes + 15) & ~uint64_t(15));
	if (out_.frame_bytes == 0 && !dynamic)
		return;
	saved_stack_ = add_local(ValueType::I32);
	frame_ = add_local(ValueType::I32);
	out_.emit(Op::GlobalGet, { 0 });
	out_.relocate(RelocationKind::GlobalIndex, module_.stack_symbol);
	out_.emit(Op::LocalTee, { saved_stack_ });
	out_.emit(Op::I32Const, { out_.frame_bytes });
	out_.emit(Op::I32Sub);
	if (alignment > 16)
	{
		out_.emit(Op::I32Const, { static_cast<uint64_t>(-int64_t(alignment)) });
		out_.emit(Op::I32And);
	}
	out_.emit(Op::LocalTee, { frame_ });
	out_.emit(Op::I32Const, { module_.stack_floor });
	out_.relocate(RelocationKind::MemoryAddressSigned, module_.stack_low_symbol);
	out_.emit(Op::I32LtU);
	out_.emit(Op::If, { static_cast<uint8_t>(ValueType::Void) });
	out_.emit(Op::Unreachable);
	out_.emit(Op::End);
	out_.emit(Op::LocalGet, { frame_ });
	out_.emit(Op::LocalGet, { saved_stack_ });
	out_.emit(Op::I32GtU);
	out_.emit(Op::If, { static_cast<uint8_t>(ValueType::Void) });
	out_.emit(Op::Unreachable);
	out_.emit(Op::End);
	out_.emit(Op::LocalGet, { frame_ });
	out_.emit(Op::GlobalSet, { 0 });
	out_.relocate(RelocationKind::GlobalIndex, module_.stack_symbol);
	for (uint32_t i = 0; i < objects_.size(); ++i)
	{
		if (objects_[i] == kNoId)
			continue;
		frame_address(objects_[i]);
		out_.emit(Op::LocalSet, { locals_[i] });
		if (i < incoming_.size() && incoming_[i] != kNoId)
		{
			out_.emit(Op::LocalGet, { locals_[i] });
			out_.emit(Op::LocalGet, { incoming_[i] });
			out_.emit(memory_op(boundary_.parameters[i].scalar, true), { 0, 0 });
			if (boundary_.parameters[i].passing == Passing::Expand)
			{
				out_.emit(Op::LocalGet, { locals_[i] });
				out_.emit(Op::LocalGet, { incoming_[i] + 1 });
				out_.emit(Op::I64Store, { 0, 8 });
			}
		}
	}
}

} // namespace wasm
