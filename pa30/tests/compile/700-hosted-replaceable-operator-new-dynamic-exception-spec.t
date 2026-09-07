// VALIDATION: compile-pass
// A replacement operator new may repeat the hosted declaration's dynamic
// exception specification.

#include <new>

void* operator new(__SIZE_TYPE__) throw(std::bad_alloc)
{
  for (;;) {}
}
