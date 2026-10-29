// AUDIT: member-only operator ranking uses the lookup class; the selected call retains its declaring base.
struct Any{Any(int){}};
struct Base{int value;int& operator[](int){return value;}const int& operator[](int)const{return value;}};
template<class T>struct Wrapper:T{using T::operator[];Any operator[](Any const&)const volatile{return Any(0);}};
int main(){Wrapper<Base> w;w.value=42;const Wrapper<Base>& c=w;return w[1]!=42||c[1]!=42;}
