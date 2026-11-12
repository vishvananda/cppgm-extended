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
	ExnRef = 0x69,
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
	Table,
	Types,
	TryTable,
	Bytes16,
	Shuffle,
	Lane,
	MemoryLane
};

// A block's signed 33-bit immediate is a nonnegative type index or a
// negative inline value type. ValueType stores that type's seven-bit byte.
constexpr std::uint64_t block_type(ValueType type)
{
	return static_cast<std::uint64_t>(static_cast<std::int64_t>(static_cast<std::uint8_t>(type)) - 128);
}

enum class CatchKind : std::uint8_t
{
	Tag,
	TagRef,
	All,
	AllRef
};

// Standard linking-v2 relocation vocabulary; indices retain symbol/type identity.
enum class RelocationKind : std::uint8_t
{
	FunctionIndex = 0,
	TableIndex = 1,
	TableIndexI32 = 2,
	MemoryAddress = 3,
	MemoryAddressSigned = 4,
	MemoryAddressI32 = 5,
	TypeIndex = 6,
	GlobalIndex = 7,
	TagIndex = 10,
	MemoryAddressRelative = 11,
	TableIndexRelative = 12,
	GlobalIndexI32 = 13,
	TableNumber = 20,
	FunctionIndexI32 = 26
};

// name, binary opcode, immediate form, diagnostic spelling
// clang-format off: clang-format reflows this list differently on every run.
#define CPPGM_WASM_OPERATIONS(X) \
	X(Unreachable, 0x00, None, "unreachable") \
	X(Nop, 0x01, None, "nop") \
	X(Block, 0x02, BlockType, "block") \
	X(Loop, 0x03, BlockType, "loop") \
	X(If, 0x04, BlockType, "if") \
	X(Else, 0x05, None, "else") \
	X(Throw, 0x08, Index, "throw") \
	X(ThrowRef, 0x0a, None, "throw_ref") \
	X(End, 0x0b, None, "end") \
	X(Br, 0x0c, Index, "br") \
	X(BrIf, 0x0d, Index, "br_if") \
	X(BrTable, 0x0e, Table, "br_table") \
	X(Return, 0x0f, None, "return") \
	X(Call, 0x10, Index, "call") \
	X(CallIndirect, 0x11, Indices, "call_indirect") \
	X(Drop, 0x1a, None, "drop") \
	X(Select, 0x1b, None, "select") \
	X(SelectTyped, 0x1c, Types, "select_t") \
	X(TryTable, 0x1f, TryTable, "try_table") \
	X(LocalGet, 0x20, Index, "local.get") \
	X(LocalSet, 0x21, Index, "local.set") \
	X(LocalTee, 0x22, Index, "local.tee") \
	X(GlobalGet, 0x23, Index, "global.get") \
	X(GlobalSet, 0x24, Index, "global.set") \
	X(I32Load, 0x28, Memory, "i32.load") \
	X(I64Load, 0x29, Memory, "i64.load") \
	X(F32Load, 0x2a, Memory, "f32.load") \
	X(F64Load, 0x2b, Memory, "f64.load") \
	X(I32Load8S, 0x2c, Memory, "i32.load8_s") \
	X(I32Load8U, 0x2d, Memory, "i32.load8_u") \
	X(I32Load16S, 0x2e, Memory, "i32.load16_s") \
	X(I32Load16U, 0x2f, Memory, "i32.load16_u") \
	X(I64Load8S, 0x30, Memory, "i64.load8_s") \
	X(I64Load8U, 0x31, Memory, "i64.load8_u") \
	X(I64Load16S, 0x32, Memory, "i64.load16_s") \
	X(I64Load16U, 0x33, Memory, "i64.load16_u") \
	X(I64Load32S, 0x34, Memory, "i64.load32_s") \
	X(I64Load32U, 0x35, Memory, "i64.load32_u") \
	X(I32Store, 0x36, Memory, "i32.store") \
	X(I64Store, 0x37, Memory, "i64.store") \
	X(F32Store, 0x38, Memory, "f32.store") \
	X(F64Store, 0x39, Memory, "f64.store") \
	X(I32Store8, 0x3a, Memory, "i32.store8") \
	X(I32Store16, 0x3b, Memory, "i32.store16") \
	X(I64Store8, 0x3c, Memory, "i64.store8") \
	X(I64Store16, 0x3d, Memory, "i64.store16") \
	X(I64Store32, 0x3e, Memory, "i64.store32") \
	X(MemorySize, 0x3f, Index, "memory.size") \
	X(MemoryGrow, 0x40, Index, "memory.grow") \
	X(I32Const, 0x41, Signed, "i32.const") \
	X(I64Const, 0x42, Signed, "i64.const") \
	X(F32Const, 0x43, Float32, "f32.const") \
	X(F64Const, 0x44, Float64, "f64.const") \
	X(I32Eqz, 0x45, None, "i32.eqz") \
	X(I32Eq, 0x46, None, "i32.eq") \
	X(I32Ne, 0x47, None, "i32.ne") \
	X(I32LtS, 0x48, None, "i32.lt_s") \
	X(I32LtU, 0x49, None, "i32.lt_u") \
	X(I32GtS, 0x4a, None, "i32.gt_s") \
	X(I32GtU, 0x4b, None, "i32.gt_u") \
	X(I32LeS, 0x4c, None, "i32.le_s") \
	X(I32LeU, 0x4d, None, "i32.le_u") \
	X(I32GeS, 0x4e, None, "i32.ge_s") \
	X(I32GeU, 0x4f, None, "i32.ge_u") \
	X(I64Eqz, 0x50, None, "i64.eqz") \
	X(I64Eq, 0x51, None, "i64.eq") \
	X(I64Ne, 0x52, None, "i64.ne") \
	X(I64LtS, 0x53, None, "i64.lt_s") \
	X(I64LtU, 0x54, None, "i64.lt_u") \
	X(I64GtS, 0x55, None, "i64.gt_s") \
	X(I64GtU, 0x56, None, "i64.gt_u") \
	X(I64LeS, 0x57, None, "i64.le_s") \
	X(I64LeU, 0x58, None, "i64.le_u") \
	X(I64GeS, 0x59, None, "i64.ge_s") \
	X(I64GeU, 0x5a, None, "i64.ge_u") \
	X(F32Eq, 0x5b, None, "f32.eq") \
	X(F32Ne, 0x5c, None, "f32.ne") \
	X(F32Lt, 0x5d, None, "f32.lt") \
	X(F32Gt, 0x5e, None, "f32.gt") \
	X(F32Le, 0x5f, None, "f32.le") \
	X(F32Ge, 0x60, None, "f32.ge") \
	X(F64Eq, 0x61, None, "f64.eq") \
	X(F64Ne, 0x62, None, "f64.ne") \
	X(F64Lt, 0x63, None, "f64.lt") \
	X(F64Gt, 0x64, None, "f64.gt") \
	X(F64Le, 0x65, None, "f64.le") \
	X(F64Ge, 0x66, None, "f64.ge") \
	X(I32Clz, 0x67, None, "i32.clz") \
	X(I32Ctz, 0x68, None, "i32.ctz") \
	X(I32Popcnt, 0x69, None, "i32.popcnt") \
	X(I32Add, 0x6a, None, "i32.add") \
	X(I32Sub, 0x6b, None, "i32.sub") \
	X(I32Mul, 0x6c, None, "i32.mul") \
	X(I32DivS, 0x6d, None, "i32.div_s") \
	X(I32DivU, 0x6e, None, "i32.div_u") \
	X(I32RemS, 0x6f, None, "i32.rem_s") \
	X(I32RemU, 0x70, None, "i32.rem_u") \
	X(I32And, 0x71, None, "i32.and") \
	X(I32Or, 0x72, None, "i32.or") \
	X(I32Xor, 0x73, None, "i32.xor") \
	X(I32Shl, 0x74, None, "i32.shl") \
	X(I32ShrS, 0x75, None, "i32.shr_s") \
	X(I32ShrU, 0x76, None, "i32.shr_u") \
	X(I32Rotl, 0x77, None, "i32.rotl") \
	X(I32Rotr, 0x78, None, "i32.rotr") \
	X(I64Clz, 0x79, None, "i64.clz") \
	X(I64Ctz, 0x7a, None, "i64.ctz") \
	X(I64Popcnt, 0x7b, None, "i64.popcnt") \
	X(I64Add, 0x7c, None, "i64.add") \
	X(I64Sub, 0x7d, None, "i64.sub") \
	X(I64Mul, 0x7e, None, "i64.mul") \
	X(I64DivS, 0x7f, None, "i64.div_s") \
	X(I64DivU, 0x80, None, "i64.div_u") \
	X(I64RemS, 0x81, None, "i64.rem_s") \
	X(I64RemU, 0x82, None, "i64.rem_u") \
	X(I64And, 0x83, None, "i64.and") \
	X(I64Or, 0x84, None, "i64.or") \
	X(I64Xor, 0x85, None, "i64.xor") \
	X(I64Shl, 0x86, None, "i64.shl") \
	X(I64ShrS, 0x87, None, "i64.shr_s") \
	X(I64ShrU, 0x88, None, "i64.shr_u") \
	X(I64Rotl, 0x89, None, "i64.rotl") \
	X(I64Rotr, 0x8a, None, "i64.rotr") \
	X(F32Abs, 0x8b, None, "f32.abs") \
	X(F32Neg, 0x8c, None, "f32.neg") \
	X(F32Ceil, 0x8d, None, "f32.ceil") \
	X(F32Floor, 0x8e, None, "f32.floor") \
	X(F32Trunc, 0x8f, None, "f32.trunc") \
	X(F32Nearest, 0x90, None, "f32.nearest") \
	X(F32Sqrt, 0x91, None, "f32.sqrt") \
	X(F32Add, 0x92, None, "f32.add") \
	X(F32Sub, 0x93, None, "f32.sub") \
	X(F32Mul, 0x94, None, "f32.mul") \
	X(F32Div, 0x95, None, "f32.div") \
	X(F32Min, 0x96, None, "f32.min") \
	X(F32Max, 0x97, None, "f32.max") \
	X(F64Abs, 0x99, None, "f64.abs") \
	X(F64Neg, 0x9a, None, "f64.neg") \
	X(F64Ceil, 0x9b, None, "f64.ceil") \
	X(F64Floor, 0x9c, None, "f64.floor") \
	X(F64Trunc, 0x9d, None, "f64.trunc") \
	X(F64Nearest, 0x9e, None, "f64.nearest") \
	X(F64Sqrt, 0x9f, None, "f64.sqrt") \
	X(F64Add, 0xa0, None, "f64.add") \
	X(F64Sub, 0xa1, None, "f64.sub") \
	X(F64Mul, 0xa2, None, "f64.mul") \
	X(F64Div, 0xa3, None, "f64.div") \
	X(F64Min, 0xa4, None, "f64.min") \
	X(F64Max, 0xa5, None, "f64.max") \
	X(F64Copysign, 0xa6, None, "f64.copysign") \
	X(I32WrapI64, 0xa7, None, "i32.wrap_i64") \
	X(I32TruncF32S, 0xa8, None, "i32.trunc_f32_s") \
	X(I32TruncF32U, 0xa9, None, "i32.trunc_f32_u") \
	X(I32TruncF64S, 0xaa, None, "i32.trunc_f64_s") \
	X(I32TruncF64U, 0xab, None, "i32.trunc_f64_u") \
	X(I64ExtendI32S, 0xac, None, "i64.extend_i32_s") \
	X(I64ExtendI32U, 0xad, None, "i64.extend_i32_u") \
	X(I64TruncF32S, 0xae, None, "i64.trunc_f32_s") \
	X(I64TruncF32U, 0xaf, None, "i64.trunc_f32_u") \
	X(I64TruncF64S, 0xb0, None, "i64.trunc_f64_s") \
	X(I64TruncF64U, 0xb1, None, "i64.trunc_f64_u") \
	X(F32ConvertI32S, 0xb2, None, "f32.convert_i32_s") \
	X(F32ConvertI32U, 0xb3, None, "f32.convert_i32_u") \
	X(F32ConvertI64S, 0xb4, None, "f32.convert_i64_s") \
	X(F32ConvertI64U, 0xb5, None, "f32.convert_i64_u") \
	X(F32DemoteF64, 0xb6, None, "f32.demote_f64") \
	X(F64ConvertI32S, 0xb7, None, "f64.convert_i32_s") \
	X(F64ConvertI32U, 0xb8, None, "f64.convert_i32_u") \
	X(F64ConvertI64S, 0xb9, None, "f64.convert_i64_s") \
	X(F64ConvertI64U, 0xba, None, "f64.convert_i64_u") \
	X(F64PromoteF32, 0xbb, None, "f64.promote_f32") \
	X(I32ReinterpretF32, 0xbc, None, "i32.reinterpret_f32") \
	X(I64ReinterpretF64, 0xbd, None, "i64.reinterpret_f64") \
	X(F32ReinterpretI32, 0xbe, None, "f32.reinterpret_i32") \
	X(F64ReinterpretI64, 0xbf, None, "f64.reinterpret_i64") \
	X(I32Extend8S, 0xc0, None, "i32.extend8_s") \
	X(I32Extend16S, 0xc1, None, "i32.extend16_s") \
	X(I64Extend8S, 0xc2, None, "i64.extend8_s") \
	X(I64Extend16S, 0xc3, None, "i64.extend16_s") \
	X(I64Extend32S, 0xc4, None, "i64.extend32_s") \
	X(RefNull, 0xd0, Signed, "ref.null") \
	X(RefIsNull, 0xd1, None, "ref.is_null") \
	X(I32TruncSatF32S, 0xfc00, None, "i32.trunc_sat_f32_s") \
	X(I32TruncSatF32U, 0xfc01, None, "i32.trunc_sat_f32_u") \
	X(I32TruncSatF64S, 0xfc02, None, "i32.trunc_sat_f64_s") \
	X(I32TruncSatF64U, 0xfc03, None, "i32.trunc_sat_f64_u") \
	X(I64TruncSatF32S, 0xfc04, None, "i64.trunc_sat_f32_s") \
	X(I64TruncSatF32U, 0xfc05, None, "i64.trunc_sat_f32_u") \
	X(I64TruncSatF64S, 0xfc06, None, "i64.trunc_sat_f64_s") \
	X(I64TruncSatF64U, 0xfc07, None, "i64.trunc_sat_f64_u") \
	X(MemoryCopy, 0xfc0a, Indices, "memory.copy") \
	X(MemoryFill, 0xfc0b, Index, "memory.fill")
// clang-format on

// SIMD128 facts follow the core specification pinned in doc/wasm/manifest.json.
// The stack form and immediate limits are shared by selection and validation.
enum class SimdForm : std::uint8_t
{
	None,
	Binary,
	Constant,
	ExtractF32,
	ExtractF64,
	ExtractI32,
	ExtractI64,
	Load,
	LoadLane,
	ReplaceF32,
	ReplaceF64,
	ReplaceI32,
	ReplaceI64,
	Shift,
	SplatF32,
	SplatF64,
	SplatI32,
	SplatI64,
	Store,
	Ternary,
	ToI32,
	Unary,
};

struct SimdInfo
{
	SimdForm form;
	std::uint8_t lanes;
	std::uint8_t alignment;
};

// name, opcode, immediates, spelling, stack form, lane bound, maximum alignment
// clang-format off
#define CPPGM_WASM_SIMD_OPERATIONS(X) \
 X(V128Load, 0xfd00, Memory, "v128.load", Load, 0, 4) \
 X(V128Load8x8S, 0xfd01, Memory, "v128.load8x8_s", Load, 0, 3) \
 X(V128Load8x8U, 0xfd02, Memory, "v128.load8x8_u", Load, 0, 3) \
 X(V128Load16x4S, 0xfd03, Memory, "v128.load16x4_s", Load, 0, 3) \
 X(V128Load16x4U, 0xfd04, Memory, "v128.load16x4_u", Load, 0, 3) \
 X(V128Load32x2S, 0xfd05, Memory, "v128.load32x2_s", Load, 0, 3) \
 X(V128Load32x2U, 0xfd06, Memory, "v128.load32x2_u", Load, 0, 3) \
 X(V128Load8Splat, 0xfd07, Memory, "v128.load8_splat", Load, 0, 0) \
 X(V128Load16Splat, 0xfd08, Memory, "v128.load16_splat", Load, 0, 1) \
 X(V128Load32Splat, 0xfd09, Memory, "v128.load32_splat", Load, 0, 2) \
 X(V128Load64Splat, 0xfd0a, Memory, "v128.load64_splat", Load, 0, 3) \
 X(V128Store, 0xfd0b, Memory, "v128.store", Store, 0, 4) \
 X(V128Const, 0xfd0c, Bytes16, "v128.const", Constant, 0, 0) \
 X(I8x16Shuffle, 0xfd0d, Shuffle, "i8x16.shuffle", Binary, 0, 0) \
 X(I8x16Swizzle, 0xfd0e, None, "i8x16.swizzle", Binary, 0, 0) \
 X(I8x16Splat, 0xfd0f, None, "i8x16.splat", SplatI32, 0, 0) \
 X(I16x8Splat, 0xfd10, None, "i16x8.splat", SplatI32, 0, 0) \
 X(I32x4Splat, 0xfd11, None, "i32x4.splat", SplatI32, 0, 0) \
 X(I64x2Splat, 0xfd12, None, "i64x2.splat", SplatI64, 0, 0) \
 X(F32x4Splat, 0xfd13, None, "f32x4.splat", SplatF32, 0, 0) \
 X(F64x2Splat, 0xfd14, None, "f64x2.splat", SplatF64, 0, 0) \
 X(I8x16ExtractLaneS, 0xfd15, Lane, "i8x16.extract_lane_s", ExtractI32, 16, 0) \
 X(I8x16ExtractLaneU, 0xfd16, Lane, "i8x16.extract_lane_u", ExtractI32, 16, 0) \
 X(I8x16ReplaceLane, 0xfd17, Lane, "i8x16.replace_lane", ReplaceI32, 16, 0) \
 X(I16x8ExtractLaneS, 0xfd18, Lane, "i16x8.extract_lane_s", ExtractI32, 8, 0) \
 X(I16x8ExtractLaneU, 0xfd19, Lane, "i16x8.extract_lane_u", ExtractI32, 8, 0) \
 X(I16x8ReplaceLane, 0xfd1a, Lane, "i16x8.replace_lane", ReplaceI32, 8, 0) \
 X(I32x4ExtractLane, 0xfd1b, Lane, "i32x4.extract_lane", ExtractI32, 4, 0) \
 X(I32x4ReplaceLane, 0xfd1c, Lane, "i32x4.replace_lane", ReplaceI32, 4, 0) \
 X(I64x2ExtractLane, 0xfd1d, Lane, "i64x2.extract_lane", ExtractI64, 2, 0) \
 X(I64x2ReplaceLane, 0xfd1e, Lane, "i64x2.replace_lane", ReplaceI64, 2, 0) \
 X(F32x4ExtractLane, 0xfd1f, Lane, "f32x4.extract_lane", ExtractF32, 4, 0) \
 X(F32x4ReplaceLane, 0xfd20, Lane, "f32x4.replace_lane", ReplaceF32, 4, 0) \
 X(F64x2ExtractLane, 0xfd21, Lane, "f64x2.extract_lane", ExtractF64, 2, 0) \
 X(F64x2ReplaceLane, 0xfd22, Lane, "f64x2.replace_lane", ReplaceF64, 2, 0) \
 X(I8x16Eq, 0xfd23, None, "i8x16.eq", Binary, 0, 0) \
 X(I8x16Ne, 0xfd24, None, "i8x16.ne", Binary, 0, 0) \
 X(I8x16LtS, 0xfd25, None, "i8x16.lt_s", Binary, 0, 0) \
 X(I8x16LtU, 0xfd26, None, "i8x16.lt_u", Binary, 0, 0) \
 X(I8x16GtS, 0xfd27, None, "i8x16.gt_s", Binary, 0, 0) \
 X(I8x16GtU, 0xfd28, None, "i8x16.gt_u", Binary, 0, 0) \
 X(I8x16LeS, 0xfd29, None, "i8x16.le_s", Binary, 0, 0) \
 X(I8x16LeU, 0xfd2a, None, "i8x16.le_u", Binary, 0, 0) \
 X(I8x16GeS, 0xfd2b, None, "i8x16.ge_s", Binary, 0, 0) \
 X(I8x16GeU, 0xfd2c, None, "i8x16.ge_u", Binary, 0, 0) \
 X(I16x8Eq, 0xfd2d, None, "i16x8.eq", Binary, 0, 0) \
 X(I16x8Ne, 0xfd2e, None, "i16x8.ne", Binary, 0, 0) \
 X(I16x8LtS, 0xfd2f, None, "i16x8.lt_s", Binary, 0, 0) \
 X(I16x8LtU, 0xfd30, None, "i16x8.lt_u", Binary, 0, 0) \
 X(I16x8GtS, 0xfd31, None, "i16x8.gt_s", Binary, 0, 0) \
 X(I16x8GtU, 0xfd32, None, "i16x8.gt_u", Binary, 0, 0) \
 X(I16x8LeS, 0xfd33, None, "i16x8.le_s", Binary, 0, 0) \
 X(I16x8LeU, 0xfd34, None, "i16x8.le_u", Binary, 0, 0) \
 X(I16x8GeS, 0xfd35, None, "i16x8.ge_s", Binary, 0, 0) \
 X(I16x8GeU, 0xfd36, None, "i16x8.ge_u", Binary, 0, 0) \
 X(I32x4Eq, 0xfd37, None, "i32x4.eq", Binary, 0, 0) \
 X(I32x4Ne, 0xfd38, None, "i32x4.ne", Binary, 0, 0) \
 X(I32x4LtS, 0xfd39, None, "i32x4.lt_s", Binary, 0, 0) \
 X(I32x4LtU, 0xfd3a, None, "i32x4.lt_u", Binary, 0, 0) \
 X(I32x4GtS, 0xfd3b, None, "i32x4.gt_s", Binary, 0, 0) \
 X(I32x4GtU, 0xfd3c, None, "i32x4.gt_u", Binary, 0, 0) \
 X(I32x4LeS, 0xfd3d, None, "i32x4.le_s", Binary, 0, 0) \
 X(I32x4LeU, 0xfd3e, None, "i32x4.le_u", Binary, 0, 0) \
 X(I32x4GeS, 0xfd3f, None, "i32x4.ge_s", Binary, 0, 0) \
 X(I32x4GeU, 0xfd40, None, "i32x4.ge_u", Binary, 0, 0) \
 X(F32x4Eq, 0xfd41, None, "f32x4.eq", Binary, 0, 0) \
 X(F32x4Ne, 0xfd42, None, "f32x4.ne", Binary, 0, 0) \
 X(F32x4Lt, 0xfd43, None, "f32x4.lt", Binary, 0, 0) \
 X(F32x4Gt, 0xfd44, None, "f32x4.gt", Binary, 0, 0) \
 X(F32x4Le, 0xfd45, None, "f32x4.le", Binary, 0, 0) \
 X(F32x4Ge, 0xfd46, None, "f32x4.ge", Binary, 0, 0) \
 X(F64x2Eq, 0xfd47, None, "f64x2.eq", Binary, 0, 0) \
 X(F64x2Ne, 0xfd48, None, "f64x2.ne", Binary, 0, 0) \
 X(F64x2Lt, 0xfd49, None, "f64x2.lt", Binary, 0, 0) \
 X(F64x2Gt, 0xfd4a, None, "f64x2.gt", Binary, 0, 0) \
 X(F64x2Le, 0xfd4b, None, "f64x2.le", Binary, 0, 0) \
 X(F64x2Ge, 0xfd4c, None, "f64x2.ge", Binary, 0, 0) \
 X(V128Not, 0xfd4d, None, "v128.not", Unary, 0, 0) \
 X(V128And, 0xfd4e, None, "v128.and", Binary, 0, 0) \
 X(V128Andnot, 0xfd4f, None, "v128.andnot", Binary, 0, 0) \
 X(V128Or, 0xfd50, None, "v128.or", Binary, 0, 0) \
 X(V128Xor, 0xfd51, None, "v128.xor", Binary, 0, 0) \
 X(V128Bitselect, 0xfd52, None, "v128.bitselect", Ternary, 0, 0) \
 X(V128AnyTrue, 0xfd53, None, "v128.any_true", ToI32, 0, 0) \
 X(V128Load8Lane, 0xfd54, MemoryLane, "v128.load8_lane", LoadLane, 16, 0) \
 X(V128Load16Lane, 0xfd55, MemoryLane, "v128.load16_lane", LoadLane, 8, 1) \
 X(V128Load32Lane, 0xfd56, MemoryLane, "v128.load32_lane", LoadLane, 4, 2) \
 X(V128Load64Lane, 0xfd57, MemoryLane, "v128.load64_lane", LoadLane, 2, 3) \
 X(V128Store8Lane, 0xfd58, MemoryLane, "v128.store8_lane", Store, 16, 0) \
 X(V128Store16Lane, 0xfd59, MemoryLane, "v128.store16_lane", Store, 8, 1) \
 X(V128Store32Lane, 0xfd5a, MemoryLane, "v128.store32_lane", Store, 4, 2) \
 X(V128Store64Lane, 0xfd5b, MemoryLane, "v128.store64_lane", Store, 2, 3) \
 X(V128Load32Zero, 0xfd5c, Memory, "v128.load32_zero", Load, 0, 2) \
 X(V128Load64Zero, 0xfd5d, Memory, "v128.load64_zero", Load, 0, 3) \
 X(F32x4DemoteF64x2Zero, 0xfd5e, None, "f32x4.demote_f64x2_zero", Unary, 0, 0) \
 X(F64x2PromoteLowF32x4, 0xfd5f, None, "f64x2.promote_low_f32x4", Unary, 0, 0) \
 X(I8x16Abs, 0xfd60, None, "i8x16.abs", Unary, 0, 0) \
 X(I8x16Neg, 0xfd61, None, "i8x16.neg", Unary, 0, 0) \
 X(I8x16Popcnt, 0xfd62, None, "i8x16.popcnt", Unary, 0, 0) \
 X(I8x16AllTrue, 0xfd63, None, "i8x16.all_true", ToI32, 0, 0) \
 X(I8x16Bitmask, 0xfd64, None, "i8x16.bitmask", ToI32, 0, 0) \
 X(I8x16NarrowI16x8S, 0xfd65, None, "i8x16.narrow_i16x8_s", Binary, 0, 0) \
 X(I8x16NarrowI16x8U, 0xfd66, None, "i8x16.narrow_i16x8_u", Binary, 0, 0) \
 X(F32x4Ceil, 0xfd67, None, "f32x4.ceil", Unary, 0, 0) \
 X(F32x4Floor, 0xfd68, None, "f32x4.floor", Unary, 0, 0) \
 X(F32x4Trunc, 0xfd69, None, "f32x4.trunc", Unary, 0, 0) \
 X(F32x4Nearest, 0xfd6a, None, "f32x4.nearest", Unary, 0, 0) \
 X(I8x16Shl, 0xfd6b, None, "i8x16.shl", Shift, 0, 0) \
 X(I8x16ShrS, 0xfd6c, None, "i8x16.shr_s", Shift, 0, 0) \
 X(I8x16ShrU, 0xfd6d, None, "i8x16.shr_u", Shift, 0, 0) \
 X(I8x16Add, 0xfd6e, None, "i8x16.add", Binary, 0, 0) \
 X(I8x16AddSatS, 0xfd6f, None, "i8x16.add_sat_s", Binary, 0, 0) \
 X(I8x16AddSatU, 0xfd70, None, "i8x16.add_sat_u", Binary, 0, 0) \
 X(I8x16Sub, 0xfd71, None, "i8x16.sub", Binary, 0, 0) \
 X(I8x16SubSatS, 0xfd72, None, "i8x16.sub_sat_s", Binary, 0, 0) \
 X(I8x16SubSatU, 0xfd73, None, "i8x16.sub_sat_u", Binary, 0, 0) \
 X(F64x2Ceil, 0xfd74, None, "f64x2.ceil", Unary, 0, 0) \
 X(F64x2Floor, 0xfd75, None, "f64x2.floor", Unary, 0, 0) \
 X(I8x16MinS, 0xfd76, None, "i8x16.min_s", Binary, 0, 0) \
 X(I8x16MinU, 0xfd77, None, "i8x16.min_u", Binary, 0, 0) \
 X(I8x16MaxS, 0xfd78, None, "i8x16.max_s", Binary, 0, 0) \
 X(I8x16MaxU, 0xfd79, None, "i8x16.max_u", Binary, 0, 0) \
 X(F64x2Trunc, 0xfd7a, None, "f64x2.trunc", Unary, 0, 0) \
 X(I8x16AvgrU, 0xfd7b, None, "i8x16.avgr_u", Binary, 0, 0) \
 X(I16x8ExtaddPairwiseI8x16S, 0xfd7c, None, "i16x8.extadd_pairwise_i8x16_s", Unary, 0, 0) \
 X(I16x8ExtaddPairwiseI8x16U, 0xfd7d, None, "i16x8.extadd_pairwise_i8x16_u", Unary, 0, 0) \
 X(I32x4ExtaddPairwiseI16x8S, 0xfd7e, None, "i32x4.extadd_pairwise_i16x8_s", Unary, 0, 0) \
 X(I32x4ExtaddPairwiseI16x8U, 0xfd7f, None, "i32x4.extadd_pairwise_i16x8_u", Unary, 0, 0) \
 X(I16x8Abs, 0xfd80, None, "i16x8.abs", Unary, 0, 0) \
 X(I16x8Neg, 0xfd81, None, "i16x8.neg", Unary, 0, 0) \
 X(I16x8Q15mulrSatS, 0xfd82, None, "i16x8.q15mulr_sat_s", Binary, 0, 0) \
 X(I16x8AllTrue, 0xfd83, None, "i16x8.all_true", ToI32, 0, 0) \
 X(I16x8Bitmask, 0xfd84, None, "i16x8.bitmask", ToI32, 0, 0) \
 X(I16x8NarrowI32x4S, 0xfd85, None, "i16x8.narrow_i32x4_s", Binary, 0, 0) \
 X(I16x8NarrowI32x4U, 0xfd86, None, "i16x8.narrow_i32x4_u", Binary, 0, 0) \
 X(I16x8ExtendLowI8x16S, 0xfd87, None, "i16x8.extend_low_i8x16_s", Unary, 0, 0) \
 X(I16x8ExtendHighI8x16S, 0xfd88, None, "i16x8.extend_high_i8x16_s", Unary, 0, 0) \
 X(I16x8ExtendLowI8x16U, 0xfd89, None, "i16x8.extend_low_i8x16_u", Unary, 0, 0) \
 X(I16x8ExtendHighI8x16U, 0xfd8a, None, "i16x8.extend_high_i8x16_u", Unary, 0, 0) \
 X(I16x8Shl, 0xfd8b, None, "i16x8.shl", Shift, 0, 0) \
 X(I16x8ShrS, 0xfd8c, None, "i16x8.shr_s", Shift, 0, 0) \
 X(I16x8ShrU, 0xfd8d, None, "i16x8.shr_u", Shift, 0, 0) \
 X(I16x8Add, 0xfd8e, None, "i16x8.add", Binary, 0, 0) \
 X(I16x8AddSatS, 0xfd8f, None, "i16x8.add_sat_s", Binary, 0, 0) \
 X(I16x8AddSatU, 0xfd90, None, "i16x8.add_sat_u", Binary, 0, 0) \
 X(I16x8Sub, 0xfd91, None, "i16x8.sub", Binary, 0, 0) \
 X(I16x8SubSatS, 0xfd92, None, "i16x8.sub_sat_s", Binary, 0, 0) \
 X(I16x8SubSatU, 0xfd93, None, "i16x8.sub_sat_u", Binary, 0, 0) \
 X(F64x2Nearest, 0xfd94, None, "f64x2.nearest", Unary, 0, 0) \
 X(I16x8Mul, 0xfd95, None, "i16x8.mul", Binary, 0, 0) \
 X(I16x8MinS, 0xfd96, None, "i16x8.min_s", Binary, 0, 0) \
 X(I16x8MinU, 0xfd97, None, "i16x8.min_u", Binary, 0, 0) \
 X(I16x8MaxS, 0xfd98, None, "i16x8.max_s", Binary, 0, 0) \
 X(I16x8MaxU, 0xfd99, None, "i16x8.max_u", Binary, 0, 0) \
 X(I16x8AvgrU, 0xfd9b, None, "i16x8.avgr_u", Binary, 0, 0) \
 X(I16x8ExtmulLowI8x16S, 0xfd9c, None, "i16x8.extmul_low_i8x16_s", Binary, 0, 0) \
 X(I16x8ExtmulHighI8x16S, 0xfd9d, None, "i16x8.extmul_high_i8x16_s", Binary, 0, 0) \
 X(I16x8ExtmulLowI8x16U, 0xfd9e, None, "i16x8.extmul_low_i8x16_u", Binary, 0, 0) \
 X(I16x8ExtmulHighI8x16U, 0xfd9f, None, "i16x8.extmul_high_i8x16_u", Binary, 0, 0) \
 X(I32x4Abs, 0xfda0, None, "i32x4.abs", Unary, 0, 0) \
 X(I32x4Neg, 0xfda1, None, "i32x4.neg", Unary, 0, 0) \
 X(I32x4AllTrue, 0xfda3, None, "i32x4.all_true", ToI32, 0, 0) \
 X(I32x4Bitmask, 0xfda4, None, "i32x4.bitmask", ToI32, 0, 0) \
 X(I32x4ExtendLowI16x8S, 0xfda7, None, "i32x4.extend_low_i16x8_s", Unary, 0, 0) \
 X(I32x4ExtendHighI16x8S, 0xfda8, None, "i32x4.extend_high_i16x8_s", Unary, 0, 0) \
 X(I32x4ExtendLowI16x8U, 0xfda9, None, "i32x4.extend_low_i16x8_u", Unary, 0, 0) \
 X(I32x4ExtendHighI16x8U, 0xfdaa, None, "i32x4.extend_high_i16x8_u", Unary, 0, 0) \
 X(I32x4Shl, 0xfdab, None, "i32x4.shl", Shift, 0, 0) \
 X(I32x4ShrS, 0xfdac, None, "i32x4.shr_s", Shift, 0, 0) \
 X(I32x4ShrU, 0xfdad, None, "i32x4.shr_u", Shift, 0, 0) \
 X(I32x4Add, 0xfdae, None, "i32x4.add", Binary, 0, 0) \
 X(I32x4Sub, 0xfdb1, None, "i32x4.sub", Binary, 0, 0) \
 X(I32x4Mul, 0xfdb5, None, "i32x4.mul", Binary, 0, 0) \
 X(I32x4MinS, 0xfdb6, None, "i32x4.min_s", Binary, 0, 0) \
 X(I32x4MinU, 0xfdb7, None, "i32x4.min_u", Binary, 0, 0) \
 X(I32x4MaxS, 0xfdb8, None, "i32x4.max_s", Binary, 0, 0) \
 X(I32x4MaxU, 0xfdb9, None, "i32x4.max_u", Binary, 0, 0) \
 X(I32x4DotI16x8S, 0xfdba, None, "i32x4.dot_i16x8_s", Binary, 0, 0) \
 X(I32x4ExtmulLowI16x8S, 0xfdbc, None, "i32x4.extmul_low_i16x8_s", Binary, 0, 0) \
 X(I32x4ExtmulHighI16x8S, 0xfdbd, None, "i32x4.extmul_high_i16x8_s", Binary, 0, 0) \
 X(I32x4ExtmulLowI16x8U, 0xfdbe, None, "i32x4.extmul_low_i16x8_u", Binary, 0, 0) \
 X(I32x4ExtmulHighI16x8U, 0xfdbf, None, "i32x4.extmul_high_i16x8_u", Binary, 0, 0) \
 X(I64x2Abs, 0xfdc0, None, "i64x2.abs", Unary, 0, 0) \
 X(I64x2Neg, 0xfdc1, None, "i64x2.neg", Unary, 0, 0) \
 X(I64x2AllTrue, 0xfdc3, None, "i64x2.all_true", ToI32, 0, 0) \
 X(I64x2Bitmask, 0xfdc4, None, "i64x2.bitmask", ToI32, 0, 0) \
 X(I64x2ExtendLowI32x4S, 0xfdc7, None, "i64x2.extend_low_i32x4_s", Unary, 0, 0) \
 X(I64x2ExtendHighI32x4S, 0xfdc8, None, "i64x2.extend_high_i32x4_s", Unary, 0, 0) \
 X(I64x2ExtendLowI32x4U, 0xfdc9, None, "i64x2.extend_low_i32x4_u", Unary, 0, 0) \
 X(I64x2ExtendHighI32x4U, 0xfdca, None, "i64x2.extend_high_i32x4_u", Unary, 0, 0) \
 X(I64x2Shl, 0xfdcb, None, "i64x2.shl", Shift, 0, 0) \
 X(I64x2ShrS, 0xfdcc, None, "i64x2.shr_s", Shift, 0, 0) \
 X(I64x2ShrU, 0xfdcd, None, "i64x2.shr_u", Shift, 0, 0) \
 X(I64x2Add, 0xfdce, None, "i64x2.add", Binary, 0, 0) \
 X(I64x2Sub, 0xfdd1, None, "i64x2.sub", Binary, 0, 0) \
 X(I64x2Mul, 0xfdd5, None, "i64x2.mul", Binary, 0, 0) \
 X(I64x2Eq, 0xfdd6, None, "i64x2.eq", Binary, 0, 0) \
 X(I64x2Ne, 0xfdd7, None, "i64x2.ne", Binary, 0, 0) \
 X(I64x2LtS, 0xfdd8, None, "i64x2.lt_s", Binary, 0, 0) \
 X(I64x2GtS, 0xfdd9, None, "i64x2.gt_s", Binary, 0, 0) \
 X(I64x2LeS, 0xfdda, None, "i64x2.le_s", Binary, 0, 0) \
 X(I64x2GeS, 0xfddb, None, "i64x2.ge_s", Binary, 0, 0) \
 X(I64x2ExtmulLowI32x4S, 0xfddc, None, "i64x2.extmul_low_i32x4_s", Binary, 0, 0) \
 X(I64x2ExtmulHighI32x4S, 0xfddd, None, "i64x2.extmul_high_i32x4_s", Binary, 0, 0) \
 X(I64x2ExtmulLowI32x4U, 0xfdde, None, "i64x2.extmul_low_i32x4_u", Binary, 0, 0) \
 X(I64x2ExtmulHighI32x4U, 0xfddf, None, "i64x2.extmul_high_i32x4_u", Binary, 0, 0) \
 X(F32x4Abs, 0xfde0, None, "f32x4.abs", Unary, 0, 0) \
 X(F32x4Neg, 0xfde1, None, "f32x4.neg", Unary, 0, 0) \
 X(F32x4Sqrt, 0xfde3, None, "f32x4.sqrt", Unary, 0, 0) \
 X(F32x4Add, 0xfde4, None, "f32x4.add", Binary, 0, 0) \
 X(F32x4Sub, 0xfde5, None, "f32x4.sub", Binary, 0, 0) \
 X(F32x4Mul, 0xfde6, None, "f32x4.mul", Binary, 0, 0) \
 X(F32x4Div, 0xfde7, None, "f32x4.div", Binary, 0, 0) \
 X(F32x4Min, 0xfde8, None, "f32x4.min", Binary, 0, 0) \
 X(F32x4Max, 0xfde9, None, "f32x4.max", Binary, 0, 0) \
 X(F32x4Pmin, 0xfdea, None, "f32x4.pmin", Binary, 0, 0) \
 X(F32x4Pmax, 0xfdeb, None, "f32x4.pmax", Binary, 0, 0) \
 X(F64x2Abs, 0xfdec, None, "f64x2.abs", Unary, 0, 0) \
 X(F64x2Neg, 0xfded, None, "f64x2.neg", Unary, 0, 0) \
 X(F64x2Sqrt, 0xfdef, None, "f64x2.sqrt", Unary, 0, 0) \
 X(F64x2Add, 0xfdf0, None, "f64x2.add", Binary, 0, 0) \
 X(F64x2Sub, 0xfdf1, None, "f64x2.sub", Binary, 0, 0) \
 X(F64x2Mul, 0xfdf2, None, "f64x2.mul", Binary, 0, 0) \
 X(F64x2Div, 0xfdf3, None, "f64x2.div", Binary, 0, 0) \
 X(F64x2Min, 0xfdf4, None, "f64x2.min", Binary, 0, 0) \
 X(F64x2Max, 0xfdf5, None, "f64x2.max", Binary, 0, 0) \
 X(F64x2Pmin, 0xfdf6, None, "f64x2.pmin", Binary, 0, 0) \
 X(F64x2Pmax, 0xfdf7, None, "f64x2.pmax", Binary, 0, 0) \
 X(I32x4TruncSatF32x4S, 0xfdf8, None, "i32x4.trunc_sat_f32x4_s", Unary, 0, 0) \
 X(I32x4TruncSatF32x4U, 0xfdf9, None, "i32x4.trunc_sat_f32x4_u", Unary, 0, 0) \
 X(F32x4ConvertI32x4S, 0xfdfa, None, "f32x4.convert_i32x4_s", Unary, 0, 0) \
 X(F32x4ConvertI32x4U, 0xfdfb, None, "f32x4.convert_i32x4_u", Unary, 0, 0) \
 X(I32x4TruncSatF64x2SZero, 0xfdfc, None, "i32x4.trunc_sat_f64x2_s_zero", Unary, 0, 0) \
 X(I32x4TruncSatF64x2UZero, 0xfdfd, None, "i32x4.trunc_sat_f64x2_u_zero", Unary, 0, 0) \
 X(F64x2ConvertLowI32x4S, 0xfdfe, None, "f64x2.convert_low_i32x4_s", Unary, 0, 0) \
 X(F64x2ConvertLowI32x4U, 0xfdff, None, "f64x2.convert_low_i32x4_u", Unary, 0, 0)
// clang-format on

enum class Op : std::uint16_t
{
#define CPPGM_WASM_ENUM(name, code, immediate, text) name = code,
	CPPGM_WASM_OPERATIONS(CPPGM_WASM_ENUM)
#undef CPPGM_WASM_ENUM
#define CPPGM_WASM_SIMD_ENUM(name, code, immediate, text, stack, lanes, align) name = code,
		CPPGM_WASM_SIMD_OPERATIONS(CPPGM_WASM_SIMD_ENUM)
#undef CPPGM_WASM_SIMD_ENUM
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
#define CPPGM_WASM_SIMD_FORM(name, code, immediate, text, stack, lanes, align) \
	case Op::name: \
		return Immediate::immediate;
		CPPGM_WASM_SIMD_OPERATIONS(CPPGM_WASM_SIMD_FORM)
#undef CPPGM_WASM_SIMD_FORM
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
#define CPPGM_WASM_SIMD_TEXT(name, code, immediate, text, stack, lanes, align) \
	case Op::name: \
		return text;
		CPPGM_WASM_SIMD_OPERATIONS(CPPGM_WASM_SIMD_TEXT)
#undef CPPGM_WASM_SIMD_TEXT
	}
	return "";
}

inline SimdInfo simd_info(Op op)
{
	switch (op)
	{
#define CPPGM_WASM_SIMD_INFO(name, code, immediate, text, stack, lanes, align) \
	case Op::name: \
		return SimdInfo{ SimdForm::stack, lanes, align };
		CPPGM_WASM_SIMD_OPERATIONS(CPPGM_WASM_SIMD_INFO)
#undef CPPGM_WASM_SIMD_INFO
	default:
		return SimdInfo{ SimdForm::None, 0, 0 };
	}
}

#undef CPPGM_WASM_SIMD_OPERATIONS

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
	case ValueType::ExnRef:
		return "exnref";
	case ValueType::Void:
		return "void";
	}
	return "";
}

} // namespace wasm_model
