struct Large { long a, b, c; };
struct Small { long a, b; };
extern "C" {
__attribute__((noinline,pure)) Large pure_large(long x) { return Large{x,x+1,x+2}; }
__attribute__((noinline,const)) Large const_large(long x) { return Large{x+3,x+4,x+5}; }
__attribute__((noinline,pure)) Small pure_small(long x) { return Small{x,x+1}; }
}
int main() {
  for (long i=-100;i!=101;++i) {
    Large a=pure_large(i), b=const_large(i);
    Small c=pure_small(i);
    if (a.a!=i || a.b!=i+1 || a.c!=i+2) return 1;
    if (b.a!=i+3 || b.b!=i+4 || b.c!=i+5) return 2;
    if (c.a!=i || c.b!=i+1) return 3;
  }
  return 0;
}
