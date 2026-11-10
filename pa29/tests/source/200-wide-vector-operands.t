typedef unsigned WideWords __attribute__((vector_size(32)));
extern "C" __attribute__((target("avx2"))) void wide_source(const WideWords* left,const WideWords* right,WideWords* destination) {
    WideWords a=*left,b=*right,result;
    asm("vpaddd %2,%1,%0" : "=&x"(result) : "x"(a),"x"(b));
    *destination=result;
}
extern "C" __attribute__((target("avx2"))) void wide_source_update(WideWords* value,const WideWords* addend) {
    WideWords local=*value,other=*addend;
    asm("vpaddd %1,%0,%0" : "+x"(local) : "x"(other));
    *value=local;
}
extern "C" __attribute__((target("avx2"))) unsigned wide_source_aligned(const WideWords* left,const WideWords* right,WideWords* destination) {
    alignas(64) WideWords locals[2];
    locals[0]=*left;locals[1]=*right;
    WideWords result;
    asm volatile("vpaddd %2,%1,%0\nxor %%r12,%%r12" : "=&x"(result) : "x"(locals[0]),"x"(locals[1]) : "r12","cc","memory");
    *destination=result;
    return locals[0][3]+locals[1][5];
}
