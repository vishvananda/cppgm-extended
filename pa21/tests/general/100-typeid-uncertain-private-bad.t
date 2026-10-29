namespace std { class type_info; }
template<class T> struct Identity { typedef T type; };
template<class T> struct Outer { template<unsigned> class Arg { typedef T type; }; };
typedef Identity<Outer<int> >::type Alias;
int main() { (void)typeid(Alias::Arg<0>::type); }
