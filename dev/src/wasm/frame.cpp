#include "wasm/selector.h"

#include <algorithm>
#include "machine/call_arguments.h"
#include "machine/frame_lifetimes.h"
#include "wasm/exceptions.h"

namespace wasm {

using namespace lowir;

void Selector::prepare_locals()
{
	plan_wide_locals();
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
	if (signature_.boundary.arity == ir_model::CAM_VARIADIC)
		variadic_parameter_ = add_local(ValueType::I32);
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
			if (wide_locals_[i] != kNoId)
			{
				wide_locals_[i] = next;
				next += 2;
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
		if (wide_locals_[i] != kNoId)
		{
			wide_locals_[i] = add_local(ValueType::I64);
			add_local(ValueType::I64);
			continue;
		}
		locals_[i] = add_local(memory_value(body_.temps[i].type) ? ValueType::I32 : value_type(body_.temps[i].type));
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

// A frame from this size keeps the two unsigned tests: the one signed test
// reads the frame's size as a signed word.
const uint32_t kOneTestFrame = uint32_t(1) << 30;

void Selector::prepare_frame()
{
	uint64_t bytes = 0;
	uint32_t alignment = 16;
	bool outgoing = false;
	auto reserve = [&](Type type) {
		const machine_model::FrameObject object{ storage_bytes(type), data_alignment(type),
			static_cast<uint32_t>(out_.frame_objects.size()) };
		const uint32_t align = object.alignment;
		alignment = std::max(alignment, align);
		bytes = (bytes + align - 1) & ~uint64_t(align - 1);
		const uint64_t offset = bytes;
		bytes += object.bytes;
		if (bytes + alignment > 1024 * 1024)
			throw WasmError("Wasm frame exceeds the configured stack reserve");
		if (!outgoing)
			out_.frame_objects.push_back(FrameObject{ object, static_cast<uint32_t>(offset) });
		return static_cast<uint32_t>(offset);
	};
	slots_.resize(body_.slots.size());
	slot_locals_.assign(body_.slots.size(), kNoId);
	places_ = plan_object_places(program_, body_, signature_, boundary_,
		exceptions_ != nullptr && exceptions_->active ? &exceptions_->analysis.eh_sites_ : nullptr);
	const auto owners = machine::sequential_copy_slots(program_, body_);
	for (uint32_t i = 0; i < body_.slots.size(); ++i)
	{
		// The storage of an incoming object or of the result object is the
		// caller's: the slot is at the pointer that the caller passed.
		if (places_.parameter_of[i] != kNoId || i == places_.result)
		{
			slot_locals_[i] = i == places_.result ? hidden_result_ : locals_[places_.parameter_of[i]];
			slots_[i] = 0;
			continue;
		}
		const bool follows = !owners.empty() && owners[i] != kNoId && owners[i] != i && !places_.shared(i) &&
			!places_.shared(owners[i]);
		slots_[i] = follows ? slots_[owners[i]] : reserve(body_.slots[i].type);
	}
	objects_.resize(body_.temps.size(), kNoId);
	for (uint32_t i = 0; i < body_.temps.size(); ++i)
		if (memory_value(body_.temps[i].type) && wide_locals_[i] == kNoId && places_.slot_of[i] == kNoId &&
			!(i < body_.parameter_count && boundary_.parameters[i].passing == Passing::Indirect))
			objects_[i] = reserve(body_.temps[i].type);
	const uint64_t outgoing_base = bytes;
	uint64_t outgoing_peak = bytes;
	const bool reuse_calls = exceptions_ == nullptr || !exceptions_->active;
	call_areas_.resize(body_.operands.size(), kNoId);
	bool dynamic = false, reads_stack = exceptions_ != nullptr && exceptions_->active;
	for (BlockId block : body_.layout)
		for (InstId i = body_.blocks[block].first; i != kNoId; i = body_.instructions[i].next)
		{
			const auto& inst = body_.instructions[i];
			dynamic = dynamic || inst.op == Opcode::StackAlloc;
			reads_stack = reads_stack || inst.op == Opcode::Call || inst.op == Opcode::StackAlloc ||
				inst.op == Opcode::StackSave || inst.op == Opcode::StackRestore;
			if (inst.op != Opcode::Call)
				continue;
			if (reuse_calls)
			{
				outgoing_peak = std::max(outgoing_peak, bytes);
				bytes = outgoing_base;
				outgoing = true;
			}
			const auto* signature = program_.call_boundary(inst);
			if (signature == nullptr)
				throw WasmError("Wasm call requires a typed boundary");
			const Boundary& boundary = module_.boundaries[signature - program_.signatures.data()];
			const Parameter* parameters = program_.parameters_of(*signature);
			for (uint32_t p = 0; p < signature->parameter_count; ++p)
				if (boundary.parameters[p].passing == Passing::Indirect &&
					argument_slot(body_.operands[inst.extra_first + p]) == kNoId)
					call_areas_[inst.extra_first + p] = reserve(parameters[p].type);
			if (signature->boundary.arity == ir_model::CAM_VARIADIC && inst.extra_count > signature->parameter_count)
			{
				if (variadic_areas_.empty())
				{
					variadic_areas_.resize(body_.instructions.size(), kNoId);
					variadic_copies_.resize(body_.operands.size(), kNoId);
				}
				const auto& plan = variadic_plan(inst, *signature);
				for (uint32_t p = signature->parameter_count; p < inst.extra_count; ++p)
				{
					const auto& argument = plan.arguments[p - signature->parameter_count];
					if (argument.passing.passing == Passing::Indirect)
						variadic_copies_[inst.extra_first + p] = reserve(argument.type);
				}
				for (uint32_t p = signature->parameter_count; p < inst.extra_count; ++p)
				{
					const auto& argument = plan.arguments[p - signature->parameter_count];
					if (argument.passing.passing == Passing::Ignore)
						continue;
					const Type storage = argument.storage;
					const bool first = variadic_areas_[i] == kNoId;
					const uint32_t offset = reserve(first ? Type::object(storage.bytes, plan.alignment_log) : storage);
					call_areas_[inst.extra_first + p] = offset;
					if (first)
						variadic_areas_[i] = offset;
				}
			}
		}
	if (reuse_calls && outgoing)
	{
		bytes = std::max(outgoing_peak, bytes);
		if (bytes > outgoing_base)
			out_.frame_objects.push_back(FrameObject{ machine_model::FrameObject{ bytes - outgoing_base, 1,
														  static_cast<uint32_t>(out_.frame_objects.size()) },
				static_cast<uint32_t>(outgoing_base) });
	}
	out_.frame_bytes = static_cast<uint32_t>((bytes + 15) & ~uint64_t(15));
	if (out_.frame_bytes == 0 && !dynamic)
	{
		reserve_objects();
		return;
	}
	saved_stack_ = add_local(ValueType::I32);
	frame_ = add_local(ValueType::I32);
	// A function that calls nothing and moves the stack pointer nowhere else
	// keeps its frame below the pointer and leaves the pointer as it found
	// it: nothing reads the pointer before the function returns.
	keeps_stack_ = !reads_stack;
	out_.emit(Op::GlobalGet, { 0 });
	out_.relocate(RelocationKind::GlobalIndex, module_.stack_symbol);
	out_.emit(Op::LocalTee, { saved_stack_ });
	if (alignment <= 16 && out_.frame_bytes < kOneTestFrame)
	{
		// The room above the stack's low end, as a signed word, holds the
		// frame or the function traps: one test covers a frame larger than
		// the room and a stack pointer already below the low end.
		out_.emit(Op::I32Const, { module_.stack_floor });
		out_.relocate(RelocationKind::MemoryAddressSigned, module_.stack_low_symbol);
		out_.emit(Op::I32Sub);
		out_.emit(Op::I32Const, { out_.frame_bytes });
		out_.emit(Op::I32LtS);
		out_.emit(Op::If, { block_type(ValueType::Void) });
		out_.emit(Op::Unreachable);
		out_.emit(Op::End);
		out_.emit(Op::LocalGet, { saved_stack_ });
		out_.emit(Op::I32Const, { out_.frame_bytes });
		out_.emit(Op::I32Sub);
	}
	else
	{
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
		out_.emit(Op::If, { block_type(ValueType::Void) });
		out_.emit(Op::Unreachable);
		out_.emit(Op::End);
		out_.emit(Op::LocalGet, { frame_ });
		out_.emit(Op::LocalGet, { saved_stack_ });
		out_.emit(Op::I32GtU);
		out_.emit(Op::If, { block_type(ValueType::Void) });
		out_.emit(Op::Unreachable);
		out_.emit(Op::End);
		out_.emit(Op::LocalGet, { frame_ });
	}
	if (keeps_stack_)
		out_.emit(Op::LocalSet, { frame_ });
	else
	{
		out_.emit(Op::LocalTee, { frame_ });
		out_.emit(Op::GlobalSet, { 0 });
		out_.relocate(RelocationKind::GlobalIndex, module_.stack_symbol);
	}
	reserve_objects();
}

// The local of a value in memory holds the address of its object.
void Selector::reserve_objects()
{
	for (uint32_t i = 0; i < objects_.size(); ++i)
	{
		if (places_.slot_of[i] != kNoId)
		{
			place_address(slot_place(places_.slot_of[i]));
			out_.emit(Op::LocalSet, { locals_[i] });
			continue;
		}
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
