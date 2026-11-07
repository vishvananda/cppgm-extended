// N3485 3.5/3.7.2: block extern retains namespace TLS identity; expect exit 0
int calls;int init(){++calls;return 7;}int f(){extern thread_local int n;return ++n;}thread_local int n=init();int main(){return f()!=8||n!=8||calls!=1;}
