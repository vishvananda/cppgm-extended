typedef unsigned ZmmWords __attribute__((vector_size(64)));
extern "C" __attribute__((target("avx512f"))) void zmm_source(const ZmmWords* left,const ZmmWords* right,ZmmWords* destination) {
    ZmmWords a=*left,b=*right,result;
    asm("vpaddd %2,%1,%0" : "=&x"(result) : "x"(a),"x"(b));
    *destination=result;
}
extern "C" __attribute__((target("avx512f"))) void zmm_source_update(ZmmWords* value,const ZmmWords* addend) {
    ZmmWords local=*value,other=*addend;
    asm("vpaddd %1,%0,%0" : "+x"(local) : "x"(other));
    *value=local;
}
extern "C" __attribute__((target("avx512f"))) unsigned zmm_source_aligned(const ZmmWords* left,const ZmmWords* right,ZmmWords* destination) {
    alignas(64) ZmmWords locals[2];
    locals[0]=*left;locals[1]=*right;
    ZmmWords result;
    asm volatile("vpaddd %2,%1,%0\nxor %%r12,%%r12" : "=&x"(result) : "x"(locals[0]),"x"(locals[1]) : "r12","cc","memory");
    *destination=result;
    return locals[0][3]+locals[1][5];
}
