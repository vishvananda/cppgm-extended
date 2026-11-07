// N3485 3.5/3.7.2: block extern retains namespace TLS identity; expect exit 0
extern thread_local int n;int f(){return ++n;}thread_local int n=7;int main(){return f()!=8||n!=8;}
