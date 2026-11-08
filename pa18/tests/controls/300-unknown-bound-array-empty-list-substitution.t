// expect exit 0
// N3485 8.5.1p4: empty lists cannot supply an unknown array bound, including an immediate context.
template<class T,class=decltype(T{})> char empty(int); template<class> long empty(...);
template<class T,class=decltype(T{1,2})> char nonempty(int); template<class> long nonempty(...);
static_assert(sizeof(empty<int[]>(0))==sizeof(long),"unknown bound needs an element");
static_assert(sizeof(empty<int[2]>(0))==1,"known bound value initializes its elements");
static_assert(sizeof(nonempty<int[]>(0))==1,"nonempty list completes the unknown bound");
int main() { return 0; }
