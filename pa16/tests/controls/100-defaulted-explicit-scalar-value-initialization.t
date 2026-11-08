// expect exit 0
// CWG1360 resolved by P2448R2: defaulted constexpr suitability is distinct from actual initialized values.
struct Object { int n; explicit Object()=default; }; constexpr Object object=Object(); static_assert(object.n==0,"value initialization zeroes before default construction"); int main(){return object.n;}
