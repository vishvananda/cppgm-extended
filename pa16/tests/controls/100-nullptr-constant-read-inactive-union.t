// Adopted CWG2907 and2140: nullptr_t conversion fetches no stored value.
// expect exit 0
union U{int x;decltype(nullptr) n;}; constexpr U u={42}; constexpr int *p=u.n; static_assert(p==nullptr,""); int main(){return 0;}
