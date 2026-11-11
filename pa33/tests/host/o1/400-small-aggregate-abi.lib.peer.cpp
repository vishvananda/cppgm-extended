#include "400-small-aggregate-abi.h"
#include <new>
#include <sys/mman.h>
#include <unistd.h>

static unsigned char* boundary()
{
  static unsigned char* end = nullptr;
  if (!end) {
    long page = sysconf(_SC_PAGESIZE);
    if (page <= 0) return nullptr;
    void* area = mmap(nullptr, 2 * page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (area == MAP_FAILED) return nullptr;
    unsigned char* next = static_cast<unsigned char*>(area) + page;
    if (mprotect(next, page, PROT_NONE)) { munmap(area, 2 * page); return nullptr; }
    end = next;
  }
  return end;
}

#define HOST(N) \
  extern "C" Bytes<N> host##N(Bytes<N> value) { \
    value.data[0] ^= 0x35; value.data[N - 1] ^= 0xa7; return value; \
  } \
  extern "C" const Bytes<N>* source##N(unsigned trial) { \
    unsigned char* end = boundary(); if (!end) return nullptr; \
    Bytes<N>* value = new (end - N) Bytes<N>; \
    for (unsigned k = 0; k != N; ++k) value->data[k] = static_cast<unsigned char>(trial * 31 + k * 17); \
    return value; \
  } \
  extern "C" Bytes<N> (*choose##N())(Bytes<N>) { return course##N; }
EACH_SIZE(HOST)
#undef HOST

extern "C" Bytes<16> host_stack(STACK_ARGS)
{
  if (a != 11 || b != 22 || c != 33 || d != 44 || e != 55 || f != 66 || g != 77 || tail != 99)
    value.data[8] ^= 1;
  value.data[0] ^= 0x35;
  value.data[15] ^= 0xa7;
  return value;
}

extern "C" int host_check(unsigned trial)
{
#define CHECK(N) { \
    Bytes<N> input; \
    for (unsigned k = 0; k != N; ++k) input.data[k] = static_cast<unsigned char>(trial * 13 + k * 29); \
    Bytes<N> result = course##N(input); \
    for (unsigned k = 0; k != N; ++k) { \
      unsigned char expected = input.data[k]; \
      if (k == 0) expected ^= 0x35; \
      if (k == N - 1) expected ^= 0xa7; \
      if (result.data[k] != expected) return N; \
    } \
  }
  EACH_SIZE(CHECK)
#undef CHECK
  Bytes<16> input;
  for (unsigned k = 0; k != 16; ++k) input.data[k] = static_cast<unsigned char>(trial * 13 + k * 29);
  Bytes<16> result = course_stack(11, 22, 33, 44, 55, 66, 77, input, 99);
  for (unsigned k = 0; k != 16; ++k) {
    unsigned char expected = input.data[k];
    if (k == 0) expected ^= 0x35;
    if (k == 15) expected ^= 0xa7;
    if (result.data[k] != expected) return 98;
  }
  return 0;
}
