static_assert(__builtin_elementwise_max(1,2)==2,"integer constant");
static_assert(__builtin_elementwise_min((unsigned char)255,(unsigned char)3)==3,"unpromoted constant");
constexpr __int128 a=(__int128)1<<100;
static_assert(__builtin_elementwise_max(-a,a)==a,"wide constant");
int main(){return 0;}
static_assert(__is_same(decltype(__builtin_elementwise_max((short)1,(short)2)),short),"no scalar promotion");
