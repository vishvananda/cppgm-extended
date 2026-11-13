#pragma once

#include <stdint.h>
#include "lowir_records.h"
#include "lowir_c_vocabulary.h"

/* Versioned C ABI for foreign LowIR producers. No C++ containers cross this
 * boundary. Symbol strings, parameter arrays and global data descriptors are borrowed
 * for one call. Body buffers and compiled object bytes are module-owned. IDs are dense, zero based. CPPGM_NO_ID is absent.
 * Functions return 1 on success (IDs use CPPGM_NO_ID on failure). A failure
 * poisons that module; destroy it after reading cppgm_lowir_error(). Separate
 * modules own independent input; calls on one module must be serialized.
 * Compilation calls are serialized internally (shared backend scratch pools);
 * separate modules can construct/validate concurrently.
 *
 * Types: kind in bits 0..7, alignment log2 in 8..15, size in 16..63.
 * Scalars have zero alignment/size; obj carries both. ptr size 0 means 8.
 * Value: kind byte, floating TypeKind byte, zero reserved halfword; index is
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

#define CPPGM_LOWIR_ABI_VERSION 2u
#define CPPGM_NO_ID UINT32_MAX

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CppgmLowir CppgmLowir;
typedef uint64_t CppgmType;

/* Targets supported by this native object ABI: Linux ELF=0, Darwin Mach-O=1. */
/* Binding: default=0, internal=1, strong=2, weak=3. Storage: default=0,
 * readonly=1, thread_local=2. Functions use C language linkage. */
typedef LowirLiteral CppgmLiteral;
typedef LowirValue CppgmDirectValue;
typedef LowirInstruction CppgmDirectInstruction;
typedef LowirSlot CppgmDirectSlot;
typedef LowirBodyCounts CppgmBodyCounts;
typedef LowirBodyBuffers CppgmBodyBuffers;

typedef struct CppgmDataItem
{
	CppgmType type;
	CppgmLiteral bits;
	int64_t amount; /* address addend or zero byte count */
	uint32_t symbol;
	uint32_t kind; /* scalar=0, address=1, zero=2; float uses type.kind */
} CppgmDataItem;

/* Writable records come from lowir_records.h; the reserve/commit contract
 * is shared with BodyConstruction below.
 * The C API owns modules and contains exceptions at the foreign boundary. */
CPPGM_LOWIR_API int cppgm_lowir_body_reserve(
	CppgmLowir* module, uint32_t symbol, const CppgmBodyCounts* minimum, CppgmBodyBuffers* buffers);
CPPGM_LOWIR_API int cppgm_lowir_body_commit(
	CppgmLowir* module, uint32_t symbol, const CppgmBodyCounts* counts, uint32_t temp_count);

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

#ifdef __cplusplus
namespace lowir_model {

// One function under construction. reserve exposes writable prefixes of
// implementation-owned pools. Pointers expire on another reserve, commit,
// compilation or destruction; IDs survive growth. Parameters are the first
// temporaries. A construction instruction's location is its insertion block;
// next is ignored before commit. layout is a permutation of block IDs; plain
// foreign slots have zero name, kind, reserved and index fields. A second
// commit, or a count outside the reservation, fails. commit binds block links and definitions and clears construction locations.
// No text parsing, allocation per instruction, or per-instruction dispatch is
// part of this interface. Reserve once using existing size facts when possible.
class BodyConstruction
{
public:
	virtual ~BodyConstruction() {}
	virtual bool reserve(const BodyCounts& minimum, BodyBuffers& buffers) = 0;
	virtual bool commit(const BodyCounts& counts, uint32_t temp_count) = 0;
	virtual const char* construction_error() const = 0;
};

} // namespace lowir_model
#endif
