namespace std { class type_info; }
template<class T> struct Identity { typedef T type; };
struct Outer { typedef int type; };
template<class T> void query() { (void)typeid(T::type); }
int main() { query<Outer>(); }
