typedef unsigned long long V __attribute__((vector_size(16)));
V f(V a,V b){asm("pand %1,%0,%1":"+x"(a):"x"(b));return a;}
