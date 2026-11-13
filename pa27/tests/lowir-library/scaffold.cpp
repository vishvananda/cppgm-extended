#include "lowir_interface.h"
#include <cstring>
template<class A, class B> struct Same { enum { value = 0 }; };
template<class A> struct Same<A, A> { enum { value = 1 }; };
static_assert(Same<LowirInstruction, lowir_model::Instruction>::value, "one record type");
static_assert(Same<LowirValue, lowir_model::Value>::value, "one operand type");
// The scaffold contract is usable without the solution's builder or containers.
bool construct(lowir_model::BodyConstruction& builder)
{
    lowir_model::BodyCounts counts = { 1, 0, 0, 0, 1 };
    lowir_model::BodyBuffers buffers;
    if (!builder.reserve(counts, buffers)) return false;
    lowir_model::Instruction ret = {};
    ret.op = lowir_model::Opcode::Return;
    ret.type = lowir_model::Type::scalar(lowir_model::TypeKind::Void);
    ret.result = lowir_model::kNoId;
    buffers.instructions[0] = ret;
    buffers.layout[0] = 0;
    return builder.commit(counts, 0);
}

int main()
{
    lowir_model::Type type = lowir_model::Type::object(0x12345, 3);
    uint64_t bits = 0;
    std::memcpy(&bits, &type, sizeof bits);
    if (bits != (CPPGM_TYPE_OBJ | (uint64_t(3) << 8) | (uint64_t(0x12345) << 16))) return 1;
    return 0;
}
