// N3485 14.5.6.1: dependent expression identity follows the bound class template;
// expect exit 0
template<bool, class T> struct enable_if {};
template<class T> struct enable_if<true,T> { typedef T type; };
namespace n {
template<class T> struct ref { typedef T type; };
template<class T> struct flag { enum { value = sizeof(T) > 0 }; };
template<class T> struct zero { enum { value = false }; };
template<class T> typename enable_if<!zero<T>::value && flag<T>::value && n::flag<typename ref<T>::type>::value,int>::type f(T);
template<class T> typename enable_if<!n::zero<T>::value && n::flag<T>::value && n::flag<typename n::ref<T>::type>::value,int>::type f(T) { return 13; }
}
int main(){return n::f(0)!=13;}
