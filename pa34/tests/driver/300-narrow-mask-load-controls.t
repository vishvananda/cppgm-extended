// Defined unsigned arithmetic, checked against independent GCC and Clang results.

using U=unsigned long;
__attribute__((noinline)) U byte(const U*p){return *p&255UL;}
__attribute__((noinline)) U half(const U*p){return *p&65535UL;}
__attribute__((noinline)) U word(const U*p){return *p&0xffffffffUL;}
__attribute__((noinline)) U signed_word(const int*p){return U(*p)&0xffffffffUL;}
__attribute__((noinline)) U narrow_byte(const signed char*p){return U(*p)&255UL;}
__attribute__((noinline)) U no_widen_byte(const signed char*p){return U(*p)&65535UL;}
__attribute__((noinline)) U no_widen_half(const short*p){return U(*p)&0xffffffffUL;}
__attribute__((noinline)) U vol(const volatile U*p){return *p&65535UL;}
__attribute__((noinline)) U atomic(const U*p){return __atomic_load_n(p,__ATOMIC_RELAXED)&255UL;}
__attribute__((noinline)) U shared(const U*p){U x=*p;return (x&255UL)^(x>>32);}
__attribute__((noinline)) void overwrite(U*p,U x){*p=x;}
__attribute__((noinline)) U call(U*p,U x){U y=*p&65535UL;overwrite(p,x);return y^*p;}
int main(){U s=1,h=0;int w=0;signed char c[2]={0,103};short a[2]={0,23901};for(unsigned i=0;i<65536;++i){s=s*6364136223846793005UL+1;U x=s;w=int(s);c[0]=static_cast<signed char>(s);a[0]=short(s);h=h*31+byte(&x);h^=half(&x);h+=word(&x);h^=signed_word(&w);h+=narrow_byte(c);h^=no_widen_byte(c);h+=no_widen_half(a);h^=vol(&x);h+=atomic(&x);h^=shared(&x);h+=call(&x,s+7);}return h != 2566950391680681992UL;}
