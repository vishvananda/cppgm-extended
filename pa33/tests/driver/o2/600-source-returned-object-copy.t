struct Pair { unsigned begin, end; };
struct Four { unsigned a,b,c,d; };
extern "C" __attribute__((noinline)) Pair adjacent(const unsigned* p) {
  return Pair{p[0],p[1]};
}
extern "C" __attribute__((noinline)) Pair repeated(const unsigned* p) {
  return Pair{p[0],p[0]};
}
extern "C" __attribute__((noinline)) Pair reversed(const unsigned* p) {
  return Pair{p[1],p[0]};
}
extern "C" __attribute__((noinline)) Pair volatile_pair(const volatile unsigned* p) {
  return Pair{p[0],p[1]};
}
__attribute__((noinline)) void expose(Four* out, unsigned** from, unsigned seed) {
  out->a=seed;out->b=seed+3;out->c=seed+5;out->d=seed+7;
  *from=&out->a;
}
__attribute__((noinline)) Four captured(unsigned seed) {
  Four out;
  unsigned* from;
  expose(&out,&from,seed);
  out.b=from[0];
  out.c=from[1];
  out.d=from[2];
  return out;
}
int main() {
  unsigned state=0;
  for (unsigned i=0;i!=4096;++i) {
    state=state*1664525u+1013904223u;
    unsigned values[4]={state,state^0xffffffffu,state+1,state>>3};
    for (unsigned j=0;j!=3;++j) {
      Pair a=adjacent(values+j),b=repeated(values+j),c=reversed(values+j);
      if (a.begin!=values[j] || a.end!=values[j+1]) return 1;
      if (b.begin!=values[j] || b.end!=values[j]) return 2;
      if (c.begin!=values[j+1] || c.end!=values[j]) return 3;
    }
    volatile unsigned source[2]={state,state+17};
    Pair v=volatile_pair(source);
    if (v.begin!=state || v.end!=state+17) return 4;
    Four alias=captured(state);
    if (alias.a!=state || alias.b!=state || alias.c!=state || alias.d!=state) return 5;
  }
  return 0;
}
