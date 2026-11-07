// N3485 3.7.2/5.1.2: nonautomatic TLS lookup through lambda bodies; expect exit 0
typedef int(*Read)();
template<int Tag>Read make(){thread_local int n=Tag;return []{return ++n;};}
int main(){Read a=make<10>();Read b=make<20>();return a()!=11||b()!=21||a()!=12||b()!=22;}
