// Defined unsigned arithmetic, checked against independent GCC and Clang results.

#include <cstdarg>
using U=unsigned long;
__attribute__((noinline)) U many(U a,U b,U c,U d,U e,U f,U g,U h,U i){volatile U x=a;return (((x*3+b)*5+c)*7+d)^(e*11+f*13+g*17+h*19+i*23);}
__attribute__((noinline)) U live(U seed,unsigned count){U a=seed,b=seed+1,c=seed+2,d=seed+3,e=seed+4,f=seed+5;for(unsigned i=0;i<count;++i){U x=many(a,b,c,d,e,f,seed,i,count);a+=x;b^=x*3;c+=x*7;d^=x*11;e+=x*17;f^=x*23;}return a^b^c^d^e^f;}
__attribute__((noinline)) double fp(double a,double b,double c,double d,double e,double f,double g,double h,double i,double j){volatile double x=i;return (a+b+c+d)+(e+f+g+h)+(x+j);}
__attribute__((noinline)) U floating(U x){double y=fp(1,2,3,4,5,6,7,8,double(x&127),double((x>>7)&127));return U(y);}
struct alignas(32) Wide{U a,b,c,d;};
__attribute__((noinline)) U aligned(Wide x,U n){volatile U y=x.d;return x.a+x.b*3+x.c*5+y*7+n;}
__attribute__((noinline)) U alignment(U x){Wide w={x,x+1,x+2,x+3};return aligned(w,x);}
__attribute__((noinline)) U varargs(unsigned n,...){va_list a;va_start(a,n);U x=0;for(unsigned i=0;i<n;++i)x=x*31+va_arg(a,U);va_end(a);return x;}
__attribute__((noinline)) U dynamic(U x,unsigned n){U*p=(U*)__builtin_alloca(n*sizeof(U));for(unsigned i=0;i<n;++i)p[i]=x+i;U a=many(x,x+1,x+2,x+3,x+4,x+5,x+6,x+7,x+8);for(unsigned i=0;i<n;++i)a^=p[i];return a;}
__attribute__((noinline)) U exception(U x){try{U v=many(x,1,2,3,4,5,6,7,8);if(x&1)throw v;return v;}catch(U v){return v^x;}}
__attribute__((noinline)) void* address(U*p,unsigned i,bool arm){void*q;if(arm)q=p+i;else q=p+i+3;return q;}
__attribute__((noinline)) __int128 pair(U x,U y){__int128 a=x,b=y;return (a<<65)^b;}

__attribute__((noinline)) U lookup(U*p,U key){unsigned bucket=0,amount=1;U*tomb=nullptr;while(true){U*at=p+bucket;if(*at==~0UL)return tomb?U(tomb-p):bucket;if(*at==key)return bucket+64;if(*at==~1UL&&!tomb)tomb=at;bucket=(bucket+amount++)&15;}}
__attribute__((noinline)) U early(U x,unsigned count){U a=x,s=0;for(unsigned i=0;i<count;++i){U old=a++;if(old&1)s^=old*3;else s+=old*7;s+=a&31;}return s^a;}
__attribute__((noinline)) U pointer_step(U*p,unsigned count){U*p0=p;U h=0;for(unsigned i=0;i<count;++i){U*old=p0++;if(i&1)h^=*old;else h+=*old;h+=U(p0-p);}return h;}
__attribute__((noinline)) U extra_use(U x,unsigned count){U a=x,s=0;for(unsigned i=0;i<count;++i){U old=a++;s+=many(a,old,1,2,3,4,5,6,7);s^=old;}return s;}
int main(){U s=1,h=0,buf[32]={};for(unsigned i=0;i<8192;++i){s=s*6364136223846793005UL+1;for(unsigned j=0;j<32;++j)buf[j]=~0UL;buf[0]=~1UL;buf[1]=7;h^=lookup(buf,7);h+=lookup(buf,9);h^=early(s,(i&15)+1);h+=pointer_step(buf,i&15);h^=extra_use(s,i&7);h=h*31+live(s,(i&7)+1);h^=floating(s);h+=alignment(s);h^=varargs(9,s,s+1,s+2,s+3,s+4,s+5,s+6,s+7,s+8);h+=dynamic(s,(i&15)+1);h^=exception(s);h+=U((U*)address(buf,i&15,bool(i&1))-buf);__int128 p=pair(s,h);h^=U(p)^U(p>>64);}return h != 3793556959080464386UL;}
