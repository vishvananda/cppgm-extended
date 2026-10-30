#if !__has_builtin(__builtin_elementwise_min) || !__has_builtin(__builtin_elementwise_max)
#error missing elementwise min/max
#endif
using U=unsigned char __attribute__((vector_size(16)));
using S=short __attribute__((vector_size(16)));
using F=float __attribute__((vector_size(16)));
static_assert(__is_same(decltype(__builtin_elementwise_min(U{},U{})),U),"unsigned byte lanes");
static_assert(__is_same(decltype(__builtin_elementwise_max(S{},S{})),S),"signed word lanes");
static_assert(__is_same(decltype(__builtin_elementwise_min(F{},F{})),F),"floating lanes");
static_assert(__is_same(decltype(__builtin_elementwise_max(1.L,2.L)),long double),"extended scalar");
int main(){return 0;}
