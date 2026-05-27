#include <memory>

#include "600-allocator-deallocate-included-class-layout.h"

void use_allocator(included_layout_type * ptr, unsigned long n)
{
  std::allocator<included_layout_type> alloc;
  alloc.deallocate(ptr, n);
}

int main()
{
  return alignof(included_layout_type) > 0 ? 0 : 1;
}
