// expect exit 0
// I13: N3485 8.5.4/adopted CWG2894: reference T{} forms and binds a temporary in an immediate context.
template<class T,class=decltype(T{})> char choose(int); template<class> long choose(...);
static_assert(sizeof(choose<const int&>(0))==1,"const lvalue reference temporary");
static_assert(sizeof(choose<const int&&>(0))==1,"const rvalue reference temporary");
static_assert(sizeof(choose<int&>(0))==sizeof(long),"nonconst lvalue reference rejects temporary");
static_assert(sizeof(choose<const volatile int&>(0))==sizeof(long),"volatile reference rejects temporary");
int main() { return 0; }
