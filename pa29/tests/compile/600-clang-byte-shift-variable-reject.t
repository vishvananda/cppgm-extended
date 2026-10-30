using V=long long __attribute__((vector_size(16)));
__attribute__((always_inline)) inline V bad(V a,int n){return __builtin_ia32_pslldqi128_byteshift(a,n);}
int main(){V a{};return (int)bad(a,1)[0];}
