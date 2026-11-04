template<class A, class B> struct Same { static const bool value = false; };
template<class A> struct Same<A, A> { static const bool value = true; };
__fp16 a, b;
static_assert(sizeof(a) == 2 && alignof(__fp16) == 2, "binary16 storage");
static_assert(!Same<__fp16, _Float16>::value, "distinct half types");
static_assert(Same<decltype(a + b), float>::value, "storage promotes before arithmetic");
static_assert(Same<decltype(a + (_Float16)1), float>::value, "mixed half arithmetic");
static_assert(Same<decltype(+a), float>::value, "unary promotion");
static_assert(Same<decltype(-a), float>::value, "unary promotion");
static_assert(Same<decltype(true ? a : b), __fp16&>::value, "lvalue identity");
static_assert(Same<decltype(a = b), __fp16&>::value, "assignment identity");
char choose(float);
long choose(double);
static_assert(Same<decltype(choose(a)), char>::value, "floating promotion ranks above conversion");
using H4 = __fp16 __attribute__((vector_size(8)));
using F4 = float __attribute__((vector_size(16)));
static_assert(Same<decltype(__builtin_convertvector(H4{}, F4)), F4>::value, "vector conversion");
float decode(unsigned short bits) { return (float)__builtin_bit_cast(__fp16, bits); }
void inspect(const __fp16&) {}
