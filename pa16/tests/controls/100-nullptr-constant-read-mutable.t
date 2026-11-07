// Adopted CWG2907 and2140: nullptr_t conversion fetches no stored value.
// expect exit 0
struct A{mutable decltype(nullptr)n;};A a;constexpr int*p=a.n;static_assert(p==nullptr,"");int main(){return 0;}
