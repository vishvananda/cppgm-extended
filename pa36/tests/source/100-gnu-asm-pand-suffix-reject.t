typedef unsigned long long V __attribute__((vector_size(16)));
V f(V a,V b){asm("pandq %1,%0":"+x"(a):"x"(b));return a;}
