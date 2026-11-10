typedef unsigned long long V __attribute__((vector_size(16)));
V f(V a,V b){asm("pand %1,%0":"+x"(a):"x"(b));return a;}
V g(V a,V b){V c;asm("pand %[src],%[dst]" : [dst]"=x"(c):[initial]"[dst]"(a),[src]"x"(b));return c;}
int main(){V a={3,12},b={5,10};V c=f(a,b),d=g(a,b);return c[0]!=1||c[1]!=8||d[0]!=1||d[1]!=8;}
