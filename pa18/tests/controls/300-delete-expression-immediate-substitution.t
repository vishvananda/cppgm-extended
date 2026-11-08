// expect exit 0
// I13: N3485 5.3.5/14.8.2: immediate delete-expression validates the selected destructor without requiring a variable definition.
struct Bad { ~Bad() = delete; }; struct Good { ~Good() {} };
template<class T> struct Holder { static T* pointer; };
template<class T,class=decltype(delete T::pointer)> char choose(int); template<class> long choose(...);
static_assert(sizeof(choose<Holder<Bad>>(0))==sizeof(long),"deleted destructor");
static_assert(sizeof(choose<Holder<Good>>(0))==1,"valid delete");
int main() { return 0; }
