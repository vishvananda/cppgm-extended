// expect exit 0
// CWG1360 resolved by P2448R2: defaulted constexpr suitability is distinct from actual initialized values.
struct Object { int n; Object()=default; constexpr Object(int):Object(){} }; static_assert(Object(0).n==0,"delegated value initialization"); int main(){return Object(0).n;}
