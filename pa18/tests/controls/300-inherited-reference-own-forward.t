// expect exit 0
struct base{int value;base(int x):value(x){}};
struct derived:base{using base::base;template<class U>derived(U&&):base(88){}};
int main(){derived a(7);derived b(a);return b.value==88?0:1;}
