// Adopted CWG2907 and2140: nullptr_t conversion fetches no stored value.
// expect exit 0
decltype(nullptr) n; decltype(nullptr)&r=n; constexpr int *p=r; static_assert(p==nullptr,""); int main(){return 0;}
