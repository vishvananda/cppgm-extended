// AUDIT-ID: MEMBER
// AUDIT-EXPECT: run
struct impl{};template<class T>struct trampoline:impl{};struct inner:trampoline<long>{};struct outer:inner,trampoline<int>{};int main(){outer o;impl*a=static_cast<inner*>(&o);impl*b=static_cast<trampoline<int>*>(&o);return a==b;}