// __builtin_nondeterministic_value has its operand's type.
#if !__has_builtin(__builtin_nondeterministic_value)
#error missing __builtin_nondeterministic_value
#endif
typedef long long Q2 __attribute__((vector_size(16)));
typedef float F4 __attribute__((ext_vector_type(4)));
Q2 q = {1, 2};
F4 f;
int i;
static_assert(__is_same(decltype(__builtin_nondeterministic_value(q)), Q2), "a GNU vector");
static_assert(__is_same(decltype(__builtin_nondeterministic_value(f)), F4), "an extended vector");
static_assert(__is_same(decltype(__builtin_nondeterministic_value(i)), int), "an integer");
static_assert(__is_same(decltype(__builtin_nondeterministic_value(1.5)), double), "a floating value");
static_assert(__is_same(decltype(__builtin_nondeterministic_value(true)), bool), "a bool");
template<class T> T any(T x) { return __builtin_nondeterministic_value(x); }
Q2 vector(Q2 x) { return any(x); }
int scalar(int x) { return any(x); }
int main() { return 0; }
