// Defined unsigned arithmetic, checked against independent GCC and Clang results.

using U=unsigned long;
volatile U observed=0;
#define FN(name,ty,mask) __attribute__((noinline)) U name(ty const*p,U seed){if((*p&(mask))!=0){observed+=seed;return seed*3+7;}return seed*5+11;}
FN(low,U,127UL)
FN(byte,U,255UL)
FN(half,U,65535UL)
FN(word,U,0xffffffffUL)
FN(high,U,0x8000000000000000UL)
FN(window,U,0x810000000000UL)
FN(two,U,0xa58000000000UL)
FN(holes,U,0x10001UL)
FN(signed_high,int,int(0x80000000U))
FN(no_widen_byte,signed char,0x100)
FN(no_widen_half,short,0x100000)
FN(vol,volatile U,0x810000000000UL)
__attribute__((noinline)) U atomic(const U*p,U s){if((__atomic_load_n(p,__ATOMIC_RELAXED)&0x810000000000UL)!=0){observed+=s;return s*3+7;}return s*5+11;}
__attribute__((noinline)) U reg(U x,U s){if(x&127UL){observed+=s;return s*3+7;}return s*5+11;}
__attribute__((noinline)) void overwrite(U*p,U x){*p=x;}
__attribute__((noinline)) U call(U*p,U s){U x=*p;overwrite(p,s);if(x&0x810000000000UL){observed+=s;return s*3+7;}return s*5+11;}
int main(){U s=1,h=0;int w=0;signed char c[2]={0,0};short a[2]={0,0};for(unsigned i=0;i<65536;++i){s=s*6364136223846793005UL+1;U x=s;w=int(s);c[0]=static_cast<signed char>(s);a[0]=short(s);h=h*31+low(&x,s);h^=byte(&x,s);h+=half(&x,s);h^=word(&x,s);h+=high(&x,s);h^=window(&x,s);h+=two(&x,s);h^=holes(&x,s);h+=signed_high(&w,s);h^=no_widen_byte(c,s);h+=no_widen_half(a,s);h^=vol(&x,s);h+=atomic(&x,s);h^=reg(x,s);h+=call(&x,s+3);}return h != 9443161961738622544UL || U(observed) != 15298369649592224568UL;}
