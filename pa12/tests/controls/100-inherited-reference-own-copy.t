// expect exit 0
struct derived;
struct base {int value;base(int x):value(x){}base(derived&);};
struct derived:base{using base::base;derived(derived&):base(8){}};
base::base(derived&):value(99){}
int main(){derived a(7);derived b(a);return b.value==8?0:1;}
