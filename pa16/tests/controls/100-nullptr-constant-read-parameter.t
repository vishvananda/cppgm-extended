// Adopted CWG2907 and2140: nullptr_t conversion fetches no stored value.
// expect exit 0
constexpr int *f(decltype(nullptr)&r){return r;} decltype(nullptr)n; constexpr int*p=f(n);static_assert(p==nullptr,"");int main(){return 0;}
