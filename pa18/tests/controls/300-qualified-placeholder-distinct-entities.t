// N3485 14.5.6.1: dependent expression identity follows the bound class template;
// expect exit 0
template<bool, class T> struct enable_if {};
template<class T> struct enable_if<true,T> { typedef T type; };
namespace a { template<class T> struct flag { enum { value = sizeof(T)==1 }; }; }
namespace b { template<class T> struct flag { enum { value = sizeof(T)!=1 }; }; }
template<class T> typename enable_if<a::flag<T>::value,int>::type f(T) { return 3; }
template<class T> typename enable_if<b::flag<T>::value,int>::type f(T) { return 7; }
int main(){return f(char())!=3 || f(int())!=7;}
