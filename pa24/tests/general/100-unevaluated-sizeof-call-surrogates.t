// Reduced from Boost.Xpressive formatter_arity.
// A class object with no operator() can still be called through a conversion
// function to function pointer. The resulting call can appear inside sizeof
// while evaluating a non-type template argument. The same unevaluated path
// must also accept variadic constructors used as "any argument" sinks.

typedef char (&one)[1];

template<unsigned long N>
struct box {
  enum { value = N };
};

struct callable {
  typedef one (*fun)(int);
  operator fun();
};

box<sizeof(callable()(0))> surrogate_call_use;

struct any_type {
  any_type(...);
};

int arg;
box<sizeof(any_type(arg))> variadic_ctor_use;

int main()
{
  (void)surrogate_call_use;
  (void)variadic_ctor_use;
  return box<sizeof(callable()(0))>::value == 1 &&
         box<sizeof(any_type(arg))>::value == 1 ? 0 : 1;
}
