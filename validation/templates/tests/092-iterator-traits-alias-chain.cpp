// VALIDATION: compile-pass
// N3485 focus: hosted integration sentinel

#include <iterator>

template<typename It>
struct wrapper
{
  typedef typename std::iterator_traits<It>::value_type value_type;
};

int main()
{
  wrapper<int *> value;
  (void)sizeof(typename wrapper<int *>::value_type);
  (void)value;
  return 0;
}
