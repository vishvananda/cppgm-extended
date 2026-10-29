namespace std { class type_info; }
template<class T> struct Identity { typedef T type; };
template<class T> struct Outer { template<unsigned> struct Arg { typedef T type; }; };
typedef Identity<Outer<int> >::type Ints;
typedef Identity<Outer<long> >::type Longs;
int main() { return &typeid(Ints::Arg<0>::type)!=&typeid(int) || &typeid(Longs::Arg<1>::type)!=&typeid(long); }
