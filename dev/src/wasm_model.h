#pragma once

#include <cstdint>

// Core Wasm instruction vocabulary. Text is a diagnostic view; the numeric
// opcode and operand form drive binary encoding of the same instruction.
namespace wasm_model {

enum class ValueType : std::uint8_t
{
	I32 = 0x7f,
	I64 = 0x7e,
	F32 = 0x7d,
	F64 = 0x7c,
	V128 = 0x7b,
	Void = 0x40
};

enum class Immediate : std::uint8_t
{
	None,
	Index,
	Signed,
	Float32,
	Float64,
	BlockType,
	Memory,
	Indices,
	Table
};

// name, binary opcode, immediate form, diagnostic spelling
#define CPPGM_WASM_OPERATIONS(X) \
	X(Unreachable, 0x00, None, "unreachable") \
	X(Block, 0x02, BlockType, "block") \
	X(Loop, 0x03, BlockType, "loop") X(If, 0x04, BlockType, "if") X(Else, 0x05, None, "else") X( \
		End, 0x0b, None, "end") X(Br, 0x0c, Index, "br") X(BrIf, 0x0d, Index, "br_if") X(BrTable, 0x0e, Table, \
		"br_table") X(Return, 0x0f, None, "return") X(Call, 0x10, Index, "call") X(CallIndirect, 0x11, Indices, \
		"call_indirect") X(Drop, 0x1a, None, "drop") X(Select, 0x1b, None, "select") X(LocalGet, 0x20, Index, \
		"local.get") X(LocalSet, 0x21, Index, "local.set") X(LocalTee, 0x22, Index, "local.tee") X(GlobalGet, 0x23, \
		Index, "global.get") X(GlobalSet, 0x24, Index, "global.set") X(I32Load, 0x28, Memory, "i32.load") X(I64Load, \
		0x29, Memory, "i64.load") X(F32Load, 0x2a, Memory, "f32.load") X(F64Load, 0x2b, Memory, \
		"f64.load") X(I32Load8S, 0x2c, Memory, "i32.load8_s") X(I32Load8U, 0x2d, Memory, "i32.load8_u") X(I32Load16S, \
		0x2e, Memory, "i32.load16_s") X(I32Load16U, 0x2f, Memory, \
		"i32.load16_u") X(I32Store, 0x36, Memory, \
		"i32.store") X(I64Store, 0x37, Memory, "i64.store") X(F32Store, 0x38, Memory, \
		"f32.store") X(F64Store, 0x39, Memory, "f64.store") X(I32Store8, 0x3a, Memory, "i32.store8") X(I32Store16, \
		0x3b, Memory, "i32.store16") X(I32Const, 0x41, Signed, "i32.const") X(I64Const, \
		0x42, Signed, "i64.const") X(F32Const, 0x43, Float32, "f32.const") X(F64Const, 0x44, Float64, \
		"f64.const") X(I32Eqz, 0x45, None, "i32.eqz") X(I32Eq, 0x46, \
		None, "i32.eq") X(I32Ne, 0x47, None, "i32.ne") X(I32LtS, 0x48, None, "i32.lt_s") X(I32LtU, 0x49, None, \
		"i32.lt_u") X(I32GtS, 0x4a, None, \
		"i32.gt_s") X(I32GtU, 0x4b, None, \
		"i32.gt_u") X(I32LeS, 0x4c, None, \
		"i32.le_s") X(I32LeU, 0x4d, None, \
		"i32.le_u") X(I32GeS, 0x4e, None, "i32.ge_s") \
		X(I32GeU, 0x4f, None, "i32.ge_u") X(I64Eqz, 0x50, None, "i64.eqz") X(I64Eq, 0x51, None, "i64.eq") X( \
			I64Ne, 0x52, None, "i64.ne") X(I64LtS, 0x53, None, "i64.lt_s") X(I64LtU, 0x54, None, "i64.lt_u") X(I64GtS, \
			0x55, None, \
			"i64.gt_s") X(I64GtU, 0x56, None, \
			"i64.gt_u") X(I64LeS, 0x57, None, \
			"i64.le_s") X(I64LeU, 0x58, None, \
			"i64.le_u") X(I64GeS, 0x59, None, \
			"i64.ge_s") X(I64GeU, 0x5a, None, \
			"i64.ge_u") X(F32Eq, 0x5b, None, "f32.eq") X(F32Ne, 0x5c, \
			None, "f32.ne") X(F32Lt, 0x5d, None, "f32.lt") X(F32Gt, 0x5e, \
			None, "f32.gt") X(F32Le, 0x5f, None, "f32.le") X(F32Ge, 0x60, \
			None, "f32.ge") X(F64Eq, 0x61, None, "f64.eq") X(F64Ne, 0x62, \
			None, "f64.ne") X(F64Lt, 0x63, None, "f64.lt") X(F64Gt, 0x64, \
			None, "f64.gt") X(F64Le, 0x65, None, "f64.le") X(F64Ge, 0x66, None, \
			"f64.ge") X(I32Clz, 0x67, None, \
			"i32.clz") X(I32Ctz, 0x68, None, "i32.ctz") X(I32Popcnt, 0x69, None, "i32.popcnt") X(I32Add, 0x6a, None, \
			"i32.add") X(I32Sub, 0x6b, None, \
			"i32.sub") X(I32Mul, 0x6c, None, "i32.mul") X(I32DivS, 0x6d, None, "i32.div_s") X(I32DivU, 0x6e, None, \
			"i32.div_u") X(I32RemS, 0x6f, None, \
			"i32.rem_s") X(I32RemU, 0x70, \
			None, "i32.rem_u") X(I32And, 0x71, None, "i32.and") X(I32Or, 0x72, None, "i32.or") X(I32Xor, 0x73, None, \
			"i32.xor") X(I32Shl, 0x74, None, "i32.shl") X(I32ShrS, 0x75, None, "i32.shr_s") X(I32ShrU, 0x76, None, \
			"i32.shr_u") X(I32Rotl, 0x77, None, \
			"i32.rotl") X(I32Rotr, 0x78, None, "i32.rotr") X(I64Clz, 0x79, None, \
			"i64.clz") X(I64Ctz, 0x7a, None, "i64.ctz") X(I64Popcnt, 0x7b, None, "i64.popcnt") X(I64Add, 0x7c, None, \
			"i64.add") X(I64Sub, 0x7d, None, \
			"i64.sub") X(I64Mul, 0x7e, None, "i64.mul") X(I64DivS, 0x7f, None, "i64.div_s") X(I64DivU, 0x80, None, \
			"i64.div_u") X(I64RemS, 0x81, None, \
			"i64.rem_s") X(I64RemU, 0x82, \
			None, "i64.rem_u") X(I64And, 0x83, None, "i64.and") X(I64Or, 0x84, None, "i64.or") X(I64Xor, 0x85, None, \
			"i64.xor") X(I64Shl, 0x86, None, "i64.shl") X(I64ShrS, 0x87, None, "i64.shr_s") X(I64ShrU, 0x88, None, \
			"i64.shr_u") X(I64Rotl, 0x89, None, \
			"i64.rotl") X(I64Rotr, 0x8a, None, "i64.rotr") X(F32Abs, 0x8b, None, \
			"f32.abs") X(F32Neg, 0x8c, None, "f32.neg") X(F32Ceil, 0x8d, None, "f32.ceil") X(F32Floor, 0x8e, None, \
			"f32.floor") X(F32Trunc, 0x8f, \
			None, "f32.trunc") X(F32Nearest, 0x90, None, "f32.nearest") X(F32Sqrt, 0x91, None, "f32.sqrt") X(F32Add, \
			0x92, None, \
			"f32.add") X(F32Sub, 0x93, None, "f32.sub") X(F32Mul, 0x94, None, \
			"f32.mul") X(F32Div, 0x95, None, "f32.div") X(F32Min, 0x96, None, "f32.min") \
			X(F32Max, 0x97, None, "f32.max") X(F64Abs, 0x99, None, "f64.abs") X(F64Neg, 0x9a, None, "f64.neg") X( \
				F64Ceil, 0x9b, None, "f64.ceil") X(F64Floor, 0x9c, None, "f64.floor") X(F64Trunc, 0x9d, None, \
				"f64.trunc") X(F64Nearest, 0x9e, None, "f64.nearest") X(F64Sqrt, 0x9f, None, "f64.sqrt") \
				X(F64Add, 0xa0, None, "f64.add") X(F64Sub, 0xa1, None, "f64.sub") X(F64Mul, 0xa2, None, "f64.mul") \
					X(F64Div, 0xa3, None, "f64.div") X(F64Min, 0xa4, None, "f64.min") X(F64Max, 0xa5, None, \
						"f64.max") X(I32WrapI64, 0xa7, None, "i32.wrap_i64") X(I64ExtendI32S, 0xac, None, \
						"i64.extend_i32_s") X(I64ExtendI32U, 0xad, None, "i64.extend_i32_u") X(F32ConvertI32S, 0xb2, \
						None, "f32.convert_i32_s") X(F32ConvertI32U, 0xb3, None, "f32.convert_i32_u") \
						X(F32ConvertI64S, 0xb4, None, "f32.convert_i64_s") X(F32ConvertI64U, 0xb5, None, \
							"f32.convert_i64_u") X(F32DemoteF64, 0xb6, None, "f32.demote_f64") X(F64ConvertI32S, 0xb7, \
							None, "f64.convert_i32_s") X(F64ConvertI32U, 0xb8, None, "f64.convert_i32_u") \
							X(F64ConvertI64S, 0xb9, None, "f64.convert_i64_s") X(F64ConvertI64U, 0xba, None, \
								"f64.convert_i64_u") X(F64PromoteF32, 0xbb, None, "f64.promote_f32") \
								X(I32ReinterpretF32, 0xbc, None, "i32.reinterpret_f32") X(I64ReinterpretF64, 0xbd, \
									None, "i64.reinterpret_f64") X(F32ReinterpretI32, 0xbe, None, \
									"f32.reinterpret_i32") X(F64ReinterpretI64, 0xbf, None, "f64.reinterpret_i64") \
									X(I32Extend8S, 0xc0, None, "i32.extend8_s") X(I32Extend16S, 0xc1, None, \
										"i32.extend16_s") X(I32TruncSatF32S, 0xfc00, None, "i32.trunc_sat_f32_s") \
										X(I32TruncSatF32U, 0xfc01, None, "i32.trunc_sat_f32_u") \
											X(I32TruncSatF64S, 0xfc02, None, "i32.trunc_sat_f64_s") \
												X(I32TruncSatF64U, 0xfc03, None, "i32.trunc_sat_f64_u") \
													X(I64TruncSatF32S, 0xfc04, None, "i64.trunc_sat_f32_s") \
														X(I64TruncSatF32U, 0xfc05, None, "i64.trunc_sat_f32_u") \
															X(I64TruncSatF64S, 0xfc06, None, "i64.trunc_sat_f64_s") X( \
																I64TruncSatF64U, 0xfc07, None, "i64.trunc_sat_f64_u") \
																X(MemoryCopy, 0xfc0a, Indices, "memory.copy") \
																	X(MemoryFill, 0xfc0b, Index, "memory.fill")

enum class Op : std::uint16_t
{
#define CPPGM_WASM_ENUM(name, code, immediate, text) name = code,
	CPPGM_WASM_OPERATIONS(CPPGM_WASM_ENUM)
#undef CPPGM_WASM_ENUM
};

inline Immediate immediate_form(Op op)
{
	switch (op)
	{
#define CPPGM_WASM_FORM(name, code, immediate, text) \
	case Op::name: \
		return Immediate::immediate;
		CPPGM_WASM_OPERATIONS(CPPGM_WASM_FORM)
#undef CPPGM_WASM_FORM
	}
	return Immediate::None;
}

inline const char* spelling(Op op)
{
	switch (op)
	{
#define CPPGM_WASM_TEXT(name, code, immediate, text) \
	case Op::name: \
		return text;
		CPPGM_WASM_OPERATIONS(CPPGM_WASM_TEXT)
#undef CPPGM_WASM_TEXT
	}
	return "";
}

#undef CPPGM_WASM_OPERATIONS

inline const char* spelling(ValueType type)
{
	switch (type)
	{
	case ValueType::I32:
		return "i32";
	case ValueType::I64:
		return "i64";
	case ValueType::F32:
		return "f32";
	case ValueType::F64:
		return "f64";
	case ValueType::V128:
		return "v128";
	case ValueType::Void:
		return "void";
	}
	return "";
}

} // namespace wasm_model
