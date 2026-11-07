// N3485 3.5/3.7.2: every declaration retains namespace TLS duration; expect exit 0
typedef int(*Read)();Read make(){extern thread_local int n;return []{return ++n;};}thread_local int n=7;int main(){Read a=make();Read b=make();return a()!=8||b()!=9||n!=9;}
