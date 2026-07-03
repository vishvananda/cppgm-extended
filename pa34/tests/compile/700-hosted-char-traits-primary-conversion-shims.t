// VALIDATION: compile-pass
// Hosted compatibility: Boost.Xpressive uses std::char_traits<T> for a
// user character type even when the hosted libc++ primary is undefined.

namespace std
{
  template<class T>
  struct char_traits;
}

struct UChar
{
  UChar(unsigned int code = 0) : code_(code) {}

  operator unsigned int() const
  {
    return code_;
  }

  unsigned int code_;
};

template<class Char>
struct probe
{
  typedef typename std::char_traits<Char>::int_type int_type;

  static Char to_char(int_type value)
  {
    return std::char_traits<Char>::to_char_type(value);
  }

  static int_type to_int(Char value)
  {
    return std::char_traits<Char>::to_int_type(value);
  }
};

int main()
{
  UChar ch = probe<UChar>::to_char(7);
  return probe<UChar>::to_int(ch) == 7 ? 0 : 1;
}
