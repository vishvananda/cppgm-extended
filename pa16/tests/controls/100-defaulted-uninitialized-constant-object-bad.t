// expect reject
// CWG1360 resolved by P2448R2: defaulted constexpr suitability is distinct from actual initialized values.
struct Object { int n; Object()=default; }; constexpr Object object; int main(){return 0;}
