// N3485 7.1.1p1: ordinary declarations agree; CWG1793 instantiation directives are separate. expect exit 0
extern thread_local int n;thread_local int n=7;extern thread_local int n;int f(){extern thread_local int n;return ++n;}int main(){return f()!=8||n!=8;}
