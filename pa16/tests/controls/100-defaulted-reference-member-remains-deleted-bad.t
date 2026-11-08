// expect reject
// CWG1360 resolved by P2448R2: defaulted constexpr suitability is distinct from actual initialized values.
struct Object { int& reference; Object()=default; }; constexpr Object object=Object(); int main(){return 0;}
