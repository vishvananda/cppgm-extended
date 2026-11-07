// N3485 3.5/3.7.2: block extern retains namespace TLS identity; expect exit 0
thread_local int n=7;int f(){extern thread_local int n;return ++n;}int main(){return f()!=8||n!=8;}
