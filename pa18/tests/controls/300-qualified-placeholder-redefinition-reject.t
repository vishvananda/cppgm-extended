// N3485 14.5.6.1: dependent expression identity follows the bound class template;
// expect reject
template<bool, class T> struct enable_if {};
template<class T> struct enable_if<true,T> { typedef T type; };
namespace n {
template<class T> struct ref { typedef T type; };
template<class T> struct flag { enum { value = sizeof(T) > 0 }; };
template<class T> typename enable_if<flag<T>::value,int>::type f(T) { return 3; }
template<class T> typename enable_if<n::flag<T>::value,int>::type f(T) { return 7; }
}
int main(){return 0;}
