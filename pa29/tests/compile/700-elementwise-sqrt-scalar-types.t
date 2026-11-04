#if __has_builtin(__builtin_elementwise_sqrt)
static_assert(__is_same(decltype(__builtin_elementwise_sqrt(1.0f)), float), "float result");
static_assert(__is_same(decltype(__builtin_elementwise_sqrt(1.0)), double), "double result");
template<class T> auto root(T x) -> decltype(__builtin_elementwise_sqrt(x)) {
  return __builtin_elementwise_sqrt(x);
}
float f(float x) { return root(x); }
double d(double x) { return root(x); }
#endif
