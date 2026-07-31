#include <new>

void* operator new(__SIZE_TYPE__) throw(int)
{
  for (;;) {}
}
