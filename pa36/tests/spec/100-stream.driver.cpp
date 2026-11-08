#include <cstdint>
#include <cstdio>
#include <cstring>
#include <xmmintrin.h>
using Vector = float __attribute__((vector_size(16)));
extern "C" void stream_ps(void*,Vector);
struct alignas(16) Storage { uint64_t before[2]; uint32_t bits[4]; uint64_t after[2]; };
int main() {
 const uint32_t patterns[][4]={{0x80000000,0x7fc12345,0x00000001,0xff800000},{0x11223344,0x55667788,0x99aabbcc,0xddeeff00}};
 for (const auto& pattern:patterns) {
  Storage out={{0x0123456789abcdefULL,0xfedcba9876543210ULL},{0,0,0,0},{0x55aa55aa55aa55aaULL,0xaa55aa55aa55aa55ULL}};
  Vector value;std::memcpy(&value,pattern,16);
  stream_ps(out.bits,value);_mm_sfence();
  if(std::memcmp(out.bits,pattern,16)||out.before[0]!=0x0123456789abcdefULL||out.before[1]!=0xfedcba9876543210ULL||out.after[0]!=0x55aa55aa55aa55aaULL||out.after[1]!=0xaa55aa55aa55aa55ULL)return 1;
 }
 std::puts("stream_ps: all 128 bits and guard bytes passed");
}
