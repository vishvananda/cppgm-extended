// Adopted CWG2907 and2140: nullptr_t conversion fetches no stored value.
// expect exit 0
void f(){decltype(nullptr) n; constexpr int *p=n; static_assert(p==nullptr,"");} int main(){f();return 0;}
