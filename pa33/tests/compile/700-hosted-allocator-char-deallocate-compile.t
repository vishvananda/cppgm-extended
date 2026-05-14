#include <memory>

void use_allocator(char * ptr, unsigned long n)
{
  std::allocator<char> alloc;
  alloc.deallocate(ptr, n);
}

int main()
{
  return 0;
}
