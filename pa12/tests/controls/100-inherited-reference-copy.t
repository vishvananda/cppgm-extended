// expect exit 0
struct derived;
struct base { int value; base(int x):value(x){} base(derived&); };
struct derived:base {using base::base;};
base::base(derived&):value(99){}
int main(){derived a(7);derived b(a);return b.value==7?0:1;}
