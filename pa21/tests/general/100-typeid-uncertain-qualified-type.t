namespace std { class type_info; }
template<class T> struct Identity { typedef T type; };
struct Incomplete;
template<class T> struct Lazy { T value; };
template<class T> struct Outer { template<unsigned> struct Arg { typedef T type; }; };
typedef const int& Reference;
typedef const long Array[3];
typedef Identity<Outer<Reference> >::type Refs;
typedef Identity<Outer<Array> >::type Arrays;
typedef Identity<Outer<Lazy<int[]>*> >::type Pointers;
int main() { return &typeid(Refs::Arg<0>::type)!=&typeid(int) || &typeid(Arrays::Arg<0>::type)!=&typeid(long[3]) || &typeid(Pointers::Arg<0>::type)!=&typeid(Lazy<int[]>*); }
