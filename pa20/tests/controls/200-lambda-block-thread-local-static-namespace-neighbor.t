// N3485 3.7.2/5.1.2: nonautomatic TLS lookup through lambda bodies; expect exit 0
thread_local int global=20;
typedef int(*Read)();
Read make(){static int n=7;return []{return ++n;};}
int main(){Read a=make();auto b=[]{return ++global;};return a()!=8||b()!=21||a()!=9||b()!=22;}
