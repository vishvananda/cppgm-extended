#include <deque>

struct CalcToken {};

int main()
{
#ifdef _LIBCPP_VERSION
#if _LIBCPP_VERSION >= 220000
  typedef std::__1::__split_buffer<
      CalcToken *,
      std::__1::allocator<CalcToken *>,
      std::__1::__split_buffer_pointer_layout> Buffer;
#else
  typedef std::__1::__split_buffer<
      CalcToken *,
      std::__1::allocator<CalcToken *> > Buffer;
#endif
  Buffer from;
  Buffer to(std::move(from));
#endif
  return 0;
}
