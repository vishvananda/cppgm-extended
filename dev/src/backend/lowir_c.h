#pragma once

#include <stdint.h>

/* Versioned C ABI for foreign LowIR producers. No C++ containers cross this
 * boundary. All arrays and strings are borrowed for the duration of a call;
 * the module owns copies. IDs are dense, zero based. CPPGM_NO_ID is absent.
 * Functions return 1 on success (IDs use CPPGM_NO_ID on failure). A failure
 * poisons that module; destroy it after reading cppgm_lowir_error(). Separate
 * modules own independent input; calls on one module must be serialized.
 * Compilation calls are serialized internally (shared backend scratch pools);
 * separate modules can construct/validate concurrently.
 *
 * Types: kind in bits 0..7, alignment log2 in 8..15, size in 16..63.
 * Scalars have zero alignment/size; obj carries both. ptr size 0 means 8.
 * Value kind: low byte CppgmValueKind, next byte floating TypeKind; index is
 * a temporary, slot, symbol, block, or index into this body's literal pool.
 * op/sub/flags/order/immediate have the LowIR vocabulary's meanings. Convert
 * immediate holds the source type. Call immediate holds a signature ID when
 * flag 2 is set. extra references the body's operand pool (phi/call/switch).
 * No textual names or number parsing occur on the instruction path.
 */
#if defined(__GNUC__)
#define CPPGM_LOWIR_API __attribute__((visibility("default")))
#else
#define CPPGM_LOWIR_API
#endif

#define CPPGM_LOWIR_ABI_VERSION 1u
#define CPPGM_NO_ID UINT32_MAX

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CppgmLowir CppgmLowir;
typedef uint64_t CppgmType;

enum CppgmTypeKind
{
	CPPGM_TYPE_VOID = 0,
	CPPGM_TYPE_I1 = 1,
	CPPGM_TYPE_I8 = 2,
	CPPGM_TYPE_U8 = 3,
	CPPGM_TYPE_I16 = 4,
	CPPGM_TYPE_U16 = 5,
	CPPGM_TYPE_I32 = 6,
	CPPGM_TYPE_U32 = 7,
	CPPGM_TYPE_I64 = 8,
	CPPGM_TYPE_I128 = 9,
	CPPGM_TYPE_F32 = 10,
	CPPGM_TYPE_F64 = 11,
	CPPGM_TYPE_F80 = 12,
	CPPGM_TYPE_PTR = 13,
	CPPGM_TYPE_V128 = 14,
	CPPGM_TYPE_OBJ = 15
};

enum CppgmOpcode
{
	CPPGM_OP_CONST = 0,
	CPPGM_OP_COPY = 1,
	CPPGM_OP_PHI = 2,
	CPPGM_OP_ADDR = 3,
	CPPGM_OP_LOAD = 4,
	CPPGM_OP_ATOMIC_LOAD = 5,
	CPPGM_OP_INDEX = 6,
	CPPGM_OP_UNARY = 7,
	CPPGM_OP_BINARY = 8,
	CPPGM_OP_COMPARE = 9,
	CPPGM_OP_SELECT = 10,
	CPPGM_OP_CONVERT = 11,
	CPPGM_OP_VECTOR = 12,
	CPPGM_OP_ATOMIC_ADD_FETCH = 13,
	CPPGM_OP_ATOMIC_EXCHANGE = 14,
	CPPGM_OP_ATOMIC_COMPARE_EXCHANGE = 15,
	CPPGM_OP_VA_ARG = 16,
	CPPGM_OP_STACK_ALLOC = 17,
	CPPGM_OP_STACK_SAVE = 18,
	CPPGM_OP_FRAME_ADDRESS = 19,
	CPPGM_OP_READ_MXCSR = 20,
	CPPGM_OP_CALL = 21,
	CPPGM_OP_EXCEPTION = 22,
	CPPGM_OP_EXCEPTION_SELECTOR = 23,
	CPPGM_OP_ASM = 24,
	CPPGM_OP_STORE = 25,
	CPPGM_OP_ATOMIC_STORE = 26,
	CPPGM_OP_ATOMIC_THREAD_FENCE = 27,
	CPPGM_OP_ATOMIC_SIGNAL_FENCE = 28,
	CPPGM_OP_VA_START = 29,
	CPPGM_OP_STACK_RESTORE = 30,
	CPPGM_OP_WRITE_MXCSR = 31,
	CPPGM_OP_COPY_OBJECT = 32,
	CPPGM_OP_ZERO_INIT = 33,
	CPPGM_OP_EH_TRY = 34,
	CPPGM_OP_EH_CLEANUP = 35,
	CPPGM_OP_EH_CATCH = 36,
	CPPGM_OP_EH_FILTER = 37,
	CPPGM_OP_EH_CATCH_ALL = 38,
	CPPGM_OP_EH_END = 39,
	CPPGM_OP_THROW = 40,
	CPPGM_OP_RESUME = 41,
	CPPGM_OP_JUMP = 42,
	CPPGM_OP_BRANCH = 43,
	CPPGM_OP_SWITCH = 44,
	CPPGM_OP_INDIRECT_JUMP = 45,
	CPPGM_OP_RETURN = 46,
	CPPGM_OP_UNREACHABLE = 47
};

enum CppgmUnaryOp
{
	CPPGM_UNARY_NEG = 0,
	CPPGM_UNARY_NOT = 1,
	CPPGM_UNARY_BIT_NOT = 2,
	CPPGM_UNARY_BYTE_SWAP = 3,
	CPPGM_UNARY_COUNT_TRAILING_ZEROS = 4,
	CPPGM_UNARY_COUNT_LEADING_ZEROS = 5,
	CPPGM_UNARY_SQRT = 6,
	CPPGM_UNARY_COUNT_ONES = 7
};

enum CppgmBinaryOp
{
	CPPGM_BINARY_ADD = 0,
	CPPGM_BINARY_SUB = 1,
	CPPGM_BINARY_MUL = 2,
	CPPGM_BINARY_DIV = 3,
	CPPGM_BINARY_MOD = 4,
	CPPGM_BINARY_U_DIV = 5,
	CPPGM_BINARY_U_MOD = 6,
	CPPGM_BINARY_AND = 7,
	CPPGM_BINARY_OR = 8,
	CPPGM_BINARY_XOR = 9,
	CPPGM_BINARY_SHL = 10,
	CPPGM_BINARY_SHR = 11,
	CPPGM_BINARY_U_SHR = 12
};

enum CppgmComparePredicate
{
	CPPGM_CMP_EQ = 0,
	CPPGM_CMP_NE = 1,
	CPPGM_CMP_LT = 2,
	CPPGM_CMP_LE = 3,
	CPPGM_CMP_GT = 4,
	CPPGM_CMP_GE = 5,
	CPPGM_CMP_ULT = 6,
	CPPGM_CMP_ULE = 7,
	CPPGM_CMP_UGT = 8,
	CPPGM_CMP_UGE = 9
};

enum CppgmConvertOp
{
	CPPGM_CONVERT_S_EXT = 0,
	CPPGM_CONVERT_Z_EXT = 1,
	CPPGM_CONVERT_TRUNC = 2,
	CPPGM_CONVERT_SI_TO_FP = 3,
	CPPGM_CONVERT_UI_TO_FP = 4,
	CPPGM_CONVERT_FP_TO_SI = 5,
	CPPGM_CONVERT_FP_TO_UI = 6,
	CPPGM_CONVERT_FP_EXT = 7,
	CPPGM_CONVERT_FP_TRUNC = 8
};

enum CppgmValueKind
{
	CPPGM_VALUE_NONE = 0,
	CPPGM_VALUE_TEMP = 1,
	CPPGM_VALUE_SLOT = 2,
	CPPGM_VALUE_SYMBOL = 3,
	CPPGM_VALUE_BLOCK = 4,
	CPPGM_VALUE_INTEGER = 5,
	CPPGM_VALUE_FLOAT = 6
};

/* Targets supported by this native object ABI: Linux ELF=0, Darwin Mach-O=1. */
/* Binding: default=0, internal=1, strong=2, weak=3. Storage: default=0,
 * readonly=1, thread_local=2. Functions use C language linkage. */
typedef struct CppgmValue
{
	uint32_t kind;
	uint32_t index;
} CppgmValue;

typedef struct CppgmLiteral
{
	uint64_t low;
	uint64_t high;
} CppgmLiteral;

typedef struct CppgmInstruction
{
	uint8_t op, sub, flags, order;
	uint32_t result;
	CppgmType type;
	CppgmValue operands[3];
	uint32_t extra_first, extra_count;
	int64_t immediate;
	uint32_t block;
	uint32_t reserved; /* must be zero */
} CppgmInstruction;

typedef struct CppgmBody
{
	uint32_t temp_count, slot_count, block_count, instruction_count;
	uint32_t operand_count, literal_count;
	const CppgmType* slots;
	const uint32_t* layout; /* permutation of 0..block_count-1 */
	const CppgmInstruction* instructions;
	const CppgmValue* operands;
	const CppgmLiteral* literals;
} CppgmBody;

typedef struct CppgmDataItem
{
	CppgmType type;
	CppgmLiteral bits;
	int64_t amount; /* address addend or zero byte count */
	uint32_t symbol;
	uint32_t kind; /* scalar=0, address=1, zero=2; float uses type.kind */
} CppgmDataItem;

CPPGM_LOWIR_API CppgmLowir* cppgm_lowir_create(uint32_t abi_version, uint32_t target);
CPPGM_LOWIR_API void cppgm_lowir_destroy(CppgmLowir* module);
CPPGM_LOWIR_API const char* cppgm_lowir_error(const CppgmLowir* module);
CPPGM_LOWIR_API uint32_t cppgm_lowir_symbol(CppgmLowir* module, const char* name, uint32_t length);
CPPGM_LOWIR_API uint32_t cppgm_lowir_signature(
	CppgmLowir* module, CppgmType result, const CppgmType* parameters, uint32_t count, uint32_t variadic);
CPPGM_LOWIR_API int cppgm_lowir_function(
	CppgmLowir* module, uint32_t symbol, uint32_t signature, uint32_t binding, uint32_t definition);
CPPGM_LOWIR_API int cppgm_lowir_global(CppgmLowir* module, uint32_t symbol, CppgmType type, uint32_t binding,
	uint32_t storage, const CppgmDataItem* items, uint32_t count, uint32_t declaration);
CPPGM_LOWIR_API int cppgm_lowir_body(CppgmLowir* module, uint32_t symbol, const CppgmBody* body);
/* Validate once after construction, before optimization or serialization. */
CPPGM_LOWIR_API int cppgm_lowir_validate(CppgmLowir* module);
CPPGM_LOWIR_API int cppgm_lowir_write(CppgmLowir* module, const char* path);
/* Optimizes and consumes the module, returning a host-linkable object in
 * module-owned memory. Valid until destroy; compile at most once. No host
 * toolchain, subprocess or temporary file is involved. level is 0..3. */
CPPGM_LOWIR_API int cppgm_lowir_compile(CppgmLowir* module, uint32_t level, uint32_t pic);
CPPGM_LOWIR_API const uint8_t* cppgm_lowir_object_data(const CppgmLowir* module);
CPPGM_LOWIR_API uint64_t cppgm_lowir_object_size(const CppgmLowir* module);

#ifdef __cplusplus
}
#endif
