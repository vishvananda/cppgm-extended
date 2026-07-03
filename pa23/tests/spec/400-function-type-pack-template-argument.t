// VALIDATION: compile-pass
// N3485 focus: 14.5.3 [temp.variadic], 14.8.2.5 [temp.deduct.type]
// A function type argument formed from a bound result type and a bound
// parameter pack must be resolved structurally when matching a partial
// specialization.

#include "../support.h"

template<class T>
struct function_traits;

template<class R, class... P>
struct function_traits<R(P...)>
{
  typedef R result_type;
  static const int arity = sizeof...(P);
};

template<class R, class... P>
struct make_function_type
{
  typedef typename function_traits<R(P...)>::result_type result_type;
  static const int arity = function_traits<R(P...)>::arity;
};

int main()
{
  typedef make_function_type<int, char, long> traits;
  return is_same<traits::result_type, int>::value && traits::arity == 2 ? 0 : 1;
}
