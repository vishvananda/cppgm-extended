#include "native/select/call_abi.h"

#include "native/backend_error.h"

namespace native {

using lowir::Type;
using lowir::TypeKind;

uint32_t value_bytes(Type type)
{
	switch (type.kind())
	{
	case TypeKind::Void:
		return 0;
	case TypeKind::I1:
	case TypeKind::I8:
	case TypeKind::U8:
		return 1;
	case TypeKind::I16:
	case TypeKind::U16:
		return 2;
	case TypeKind::I32:
	case TypeKind::U32:
	case TypeKind::F32:
		return 4;
	case TypeKind::I64:
	case TypeKind::F64:
	case TypeKind::Ptr:
		return 8;
	case TypeKind::I128:
	case TypeKind::F80:
	case TypeKind::V128:
		return 16;
	case TypeKind::Obj:
		// Objects live in frames and data addressed by 32-bit displacements.
		if (type.bytes > 0x7fff0000u)
			throw BackendError("object larger than the 32-bit displacement range");
		return static_cast<uint32_t>(type.bytes);
	}
	return 8;
}

uint32_t value_align(Type type)
{
	if (type.kind() == TypeKind::Obj)
		return 1u << type.align_log2;
	const uint32_t bytes = value_bytes(type);
	return bytes == 0 ? 1 : bytes;
}

namespace {

void add_part(CallLayout& layout, Reg reg, int32_t stack, uint32_t offset, uint32_t size)
{
	ArgPart part;
	part.reg = reg;
	part.stack = stack;
	part.offset = offset;
	part.size = size;
	layout.parts.push_back(part);
}

uint32_t align_up(uint32_t value, uint32_t align)
{
	return (value + align - 1) & ~(align - 1);
}

// A memory argument at the next offset of its own alignment, at least
// eightbyte (psABI 3.2.3; GCC aligns 32- and 64-byte vectors and alignas
// classes to 32 and 64, and Clang agrees).
void add_stack_value(CallLayout& layout, ArgLayout& arg, uint32_t bytes, uint32_t align)
{
	layout.stack_bytes = align_up(layout.stack_bytes, align < 8 ? 8 : align);
	if (align > layout.stack_align)
		layout.stack_align = align;
	arg.cls = ArgClass::Memory;
	arg.part_count = 1;
	add_part(layout, kNoReg, static_cast<int32_t>(layout.stack_bytes), 0, bytes);
	layout.stack_bytes += align_up(bytes, 8);
}

// An object of at most 16 bytes with stated classes (psABI 3.2.3): each
// eightbyte but padding in the next register of its class (an SSEUP
// eightbyte in its SSE eightbyte's), if all fit; otherwise, or for a
// MEMORY or X87 class, the whole object on the stack.
void classify_object(Type type, uint8_t classes, ArgLayout& arg, CallLayout& layout)
{
	const uint32_t bytes = value_bytes(type);
	const uint32_t eightbytes = (bytes + 7) / 8;
	uint32_t gprs = 0, xmms = 0;
	for (uint32_t k = 0; k < eightbytes; ++k)
	{
		const lowir::EightbyteClass cls = lowir::eightbyte_class(classes, k);
		gprs += cls == lowir::EB_INTEGER ? 1 : 0;
		xmms += cls == lowir::EB_SSE ? 1 : 0;
	}
	if (bytes > 16 || (classes & (lowir::OC_MEMORY | lowir::OC_X87 | lowir::OC_COMPLEX_X87)) ||
		layout.gprs + gprs > 6 || layout.xmms + xmms > 8)
	{
		add_stack_value(layout, arg, bytes, value_align(type));
		return;
	}
	arg.cls = xmms != 0 ? ArgClass::Mixed : (gprs != 0 ? ArgClass::Gpr : ArgClass::None);
	for (uint32_t k = 0; k < eightbytes; ++k)
	{
		const lowir::EightbyteClass cls = lowir::eightbyte_class(classes, k);
		if (cls == lowir::EB_NONE)
			continue;
		if (cls == lowir::EB_SSEUP)
		{
			layout.parts.back().size = bytes;
			continue;
		}
		const Reg reg = cls == lowir::EB_INTEGER ? kArgGprs[layout.gprs++] : static_cast<Reg>(XMM0 + layout.xmms++);
		add_part(layout, reg, 0, 8 * k, bytes - 8 * k < 8 ? bytes - 8 * k : 8);
		++arg.part_count;
	}
}

void classify_one(Type type, uint8_t classes, CallLayout& layout)
{
	ArgLayout arg;
	arg.cls = ArgClass::Gpr;
	arg.first_part = static_cast<uint32_t>(layout.parts.size());
	arg.part_count = 0;
	const TypeKind kind = type.kind();
	if (kind == TypeKind::Obj && classes != 0)
		classify_object(type, classes, arg, layout);
	else if (kind == TypeKind::I128 && classes != 0)
	{
		// A member function pointer: two INTEGER eightbytes, 8-aligned on
		// the stack (an __int128 is 16-aligned).
		classify_object(Type::object(16, 3), classes, arg, layout);
	}
	else if (kind == TypeKind::F32 || kind == TypeKind::F64)
	{
		if (layout.xmms < 8)
		{
			arg.cls = ArgClass::Xmm;
			arg.part_count = 1;
			add_part(layout, static_cast<Reg>(XMM0 + layout.xmms++), 0, 0, value_bytes(type));
		}
		else
			add_stack_value(layout, arg, 8, 8);
	}
	else if (kind == TypeKind::V128)
	{
		if (layout.xmms < 8)
		{
			arg.cls = ArgClass::Xmm;
			arg.part_count = 1;
			add_part(layout, static_cast<Reg>(XMM0 + layout.xmms++), 0, 0, 16);
		}
		else
			add_stack_value(layout, arg, 16, 16);
	}
	else if (kind == TypeKind::F80)
		add_stack_value(layout, arg, 16, 16);
	else if (kind == TypeKind::I128 || kind == TypeKind::Obj)
	{
		const uint32_t bytes = value_bytes(type);
		const uint32_t chunks = (bytes + 7) / 8;
		if (bytes <= 16 && bytes > 0 && layout.gprs + chunks <= 6)
		{
			arg.part_count = chunks;
			for (uint32_t i = 0; i < chunks; ++i)
			{
				const uint32_t size = bytes - 8 * i < 8 ? bytes - 8 * i : 8;
				add_part(layout, kArgGprs[layout.gprs++], 0, 8 * i, size);
			}
		}
		else
			add_stack_value(layout, arg, bytes, value_align(type));
	}
	else if (layout.gprs < 6)
	{
		arg.part_count = 1;
		add_part(layout, kArgGprs[layout.gprs++], 0, 0, value_bytes(type));
	}
	else
		add_stack_value(layout, arg, 8, 8);
	layout.args.push_back(arg);
}

} // namespace

// A classified object result (psABI 3.2.3): COMPLEX_X87 in st0 and st1,
// MEMORY through the hidden destination, X87 in st0, else each eightbyte
// but padding in the next of rax, rdx or xmm0, xmm1 (an SSEUP eightbyte in
// its SSE eightbyte's).
void classify_object_result(Type result, uint8_t classes, CallLayout& layout)
{
	const uint32_t bytes = value_bytes(result);
	if (classes & lowir::OC_COMPLEX_X87)
	{
		layout.result = ArgClass::X87Pair;
		return;
	}
	if (bytes > 16 || bytes == 0 || (classes & lowir::OC_MEMORY))
	{
		layout.result = ArgClass::Memory;
		layout.sret = true;
		layout.gprs = 1;
		return;
	}
	if (classes & lowir::OC_X87)
	{
		layout.result = ArgClass::X87;
		return;
	}
	layout.result = ArgClass::Mixed;
	uint32_t gprs = 0, xmms = 0;
	for (uint32_t k = 0; k < (bytes + 7) / 8; ++k)
	{
		const lowir::EightbyteClass cls = lowir::eightbyte_class(classes, k);
		if (cls == lowir::EB_NONE)
			continue;
		if (cls == lowir::EB_SSEUP)
		{
			layout.result_part[layout.result_parts - 1].size = bytes;
			continue;
		}
		ArgPart& part = layout.result_part[layout.result_parts++];
		part.reg = cls == lowir::EB_INTEGER ? (gprs++ == 0 ? RAX : RDX) : static_cast<Reg>(XMM0 + xmms++);
		part.stack = 0;
		part.offset = 8 * k;
		part.size = bytes - 8 * k < 8 ? bytes - 8 * k : 8;
	}
}

void classify_call(
	const Type* params, const uint8_t* classes, uint32_t count, Type result, uint8_t result_classes, CallLayout& layout)
{
	layout.sret = false;
	layout.args.clear();
	layout.parts.clear();
	layout.stack_bytes = 0;
	layout.stack_align = 16;
	layout.gprs = 0;
	layout.xmms = 0;
	layout.result_parts = 0;
	if (result.kind() == TypeKind::Obj && result_classes != 0)
	{
		classify_object_result(result, result_classes, layout);
		for (uint32_t i = 0; i < count; ++i)
			classify_one(params[i], classes != nullptr ? classes[i] : 0, layout);
		return;
	}
	switch (result.kind())
	{
	case TypeKind::Void:
		layout.result = ArgClass::None;
		break;
	case TypeKind::F32:
	case TypeKind::F64:
	case TypeKind::V128:
		layout.result = ArgClass::Xmm;
		break;
	case TypeKind::F80:
		layout.result = ArgClass::X87;
		break;
	case TypeKind::I128:
		layout.result = ArgClass::Gpr;
		layout.result_parts = 2;
		break;
	case TypeKind::Obj:
		if (result.bytes > 16 || result.bytes == 0)
		{
			layout.result = ArgClass::Memory;
			layout.sret = true;
			layout.gprs = 1;
		}
		else
		{
			layout.result = ArgClass::Gpr;
			layout.result_parts = result.bytes > 8 ? 2 : 1;
		}
		break;
	default:
		layout.result = ArgClass::Gpr;
		layout.result_parts = 1;
		break;
	}
	for (uint32_t k = 0; k < layout.result_parts; ++k)
	{
		layout.result_part[k].reg = k == 0 ? RAX : RDX;
		layout.result_part[k].stack = 0;
		layout.result_part[k].offset = 8 * k;
		layout.result_part[k].size = 8;
	}
	for (uint32_t i = 0; i < count; ++i)
		classify_one(params[i], classes != nullptr ? classes[i] : 0, layout);
}

void call_argument_types(const lowir::FunctionBody& body, const lowir::Instruction& inst, const lowir::Signature& sig,
	const lowir::Parameter* params, std::vector<lowir::Type>& out)
{
	out.clear();
	for (uint32_t a = 0; a < inst.extra_count; ++a)
	{
		const lowir::Value v = body.operands[inst.extra_first + a];
		lowir::Type t = v.kind == lowir::ValueKind::Temp ? body.temps[v.index].type
														 : lowir::Type::scalar(lowir::TypeKind::Ptr);
		if (a < sig.parameter_count)
			t = params[a].type;
		else if (v.kind == lowir::ValueKind::Float)
			t = lowir::Type::scalar(v.float_kind == lowir::TypeKind::F80 ? lowir::TypeKind::F80 : lowir::TypeKind::F64);
		else if (v.kind == lowir::ValueKind::Integer)
			t = lowir::Type::scalar(lowir::TypeKind::I64);
		else if (v.kind == lowir::ValueKind::Slot)
			t = body.slots[v.index].type;
		out.push_back(t);
	}
}

} // namespace native
