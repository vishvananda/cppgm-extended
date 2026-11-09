extern "C" unsigned host_mix(unsigned);
extern "C" __attribute__((noinline)) unsigned add32(unsigned a, unsigned b) { return a+b; }
extern "C" __attribute__((noinline)) unsigned long long add64(unsigned long long a, unsigned long long b) { return a+b; }
extern "C" __attribute__((noinline)) int add_signed(int a, int b) { return a+b; }
extern "C" __attribute__((noinline)) unsigned shifted(unsigned a, unsigned n) { return (a+3u)<<(n&31u); }
extern "C" __attribute__((noinline)) unsigned divided(unsigned a, unsigned n) { return a/n+a%n; }
extern "C" __attribute__((noinline)) unsigned called(unsigned a, unsigned b) { return host_mix(a)+b; }
extern "C" __attribute__((noinline)) int* advanced(int* a, long n) { return a+n; }

extern "C" unsigned add32(unsigned,unsigned), shifted(unsigned,unsigned), divided(unsigned,unsigned), called(unsigned,unsigned);
extern "C" unsigned long long add64(unsigned long long,unsigned long long);
extern "C" int add_signed(int,int);
extern "C" int* advanced(int*,long);
extern "C" unsigned host_mix(unsigned a) { return (a*33u)^(a>>7); }
int main() {
  unsigned values[]={0u,1u,7u,0x7fffffffu,0x80000000u,0xffffffffu};
  for(unsigned a:values) for(unsigned b:values) {
    if(add32(a,b)!=a+b || add64(a,0xffffffff00000000ULL+b)!=a+0xffffffff00000000ULL+b) return 1;
    for(unsigned n=0;n<33;++n) if(shifted(a,n)!=((a+3u)<<(n&31u))) return 2;
    for(unsigned n=1;n<32;++n) if(divided(a,n)!=a/n+a%n) return 3;
    if(called(a,b)!=host_mix(a)+b) return 4;
  }
  for(int a=-100;a<=100;++a) for(int b=-100;b<=100;++b) if(add_signed(a,b)!=a+b) return 5;
  int array[33]; for(long n=0;n<33;++n) if(advanced(array,n)!=array+n || advanced(array+32,-n)!=array+32-n) return 6;
  return 0;
}
