template<class,class>struct same {static const bool value=false;};template<class T>struct same<T,T>{static const bool value=true;};
struct A {
 int n;
 constexpr A(int x):n(x){}
 constexpr A(const A&o):n(o.n+1){}
 constexpr int f() const {return [*this] {return n;}();}
 constexpr int g(){auto lambda=[*this](auto value) mutable {++n;return n+value;};return lambda(2);}
};
static_assert(A(7).f()==8,"copy construction");
static_assert(A(7).g()==11,"generic mutable copied this");
int main(){return A(7).g()==11?0:1;}
