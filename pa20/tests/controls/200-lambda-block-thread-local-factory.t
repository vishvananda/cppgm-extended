// N3485 3.7.2/5.1.2: nonautomatic TLS lookup through lambda bodies; expect exit 0
typedef int(*Read)();
Read make(){thread_local int n=7;return []{return ++n;};}
int main(){Read a=make();Read b=make();return a()!=8||b()!=9||a()!=10;}
