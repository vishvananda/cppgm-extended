// N3485 3.5/3.7.2: block extern retains namespace TLS identity; expect exit 0
int f(){extern thread_local int n;return ++n;}int g(){extern thread_local int n;return ++n;}thread_local int n=7;int main(){return f()!=8||g()!=9||n!=9;}
