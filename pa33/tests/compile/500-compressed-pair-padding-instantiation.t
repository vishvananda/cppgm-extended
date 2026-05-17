#if __has_include(<__memory/compressed_pair.h>)
#include <__memory/compressed_pair.h>

struct Alloc {};

#if defined(_LIBCPP_VERSION) && _LIBCPP_VERSION < 210000
::std::__compressed_pair<Alloc, int> pad;
#else
::std::__compressed_pair_padding<Alloc> pad;
#endif
#elif __has_include(<bits/shared_ptr_base.h>)
#include <bits/shared_ptr_base.h>

struct Alloc {};

::std::_Sp_ebo_helper<0, Alloc> pad{Alloc()};
#else
#error missing stdlib compressed-pair helper
#endif

int main() {
  return 0;
}
