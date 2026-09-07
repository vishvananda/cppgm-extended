// VALIDATION: compile-fail
// A replacement operator new may not contradict the hosted declaration's
// dynamic exception specification.

#include <new>

void* operator new(__SIZE_TYPE__) throw(int)
{
  for (;;) {}
}
