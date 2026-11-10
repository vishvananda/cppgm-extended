typedef unsigned UpperWords __attribute__((vector_size(32)));
extern "C" __attribute__((target("avx512f,avx512vl"))) void upper_vector_source(const UpperWords* left,const UpperWords* right,UpperWords* out) {
    UpperWords a=*left,b=*right,result;
    asm("vpaddd %2,%1,%0" : "=&v"(result) : "v"(a),"v"(b) : "xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15");
    *out=result;
}
extern "C" __attribute__((target("avx512f,avx512vl"))) void upper_scalar_source(const double* left,const double* right,double* out) {
    double a=*left,b=*right,result;
    asm("vaddsd %2,%1,%0" : "=&v"(result) : "v"(a),"v"(b) : "xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15");
    *out=result;
}
