#include <new>

void* operator new(__SIZE_TYPE__) throw(std::bad_alloc)
{
  for (;;) {}
}
