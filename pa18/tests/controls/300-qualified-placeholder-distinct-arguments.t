// N3485 14.5.6.1: dependent expression identity follows the bound class template;
// expect exit 0
template<bool, class T> struct enable_if {};
template<class T> struct enable_if<true,T> { typedef T type; };
namespace n { template<bool B> struct flag { enum { value=B }; }; }
template<class T> typename enable_if<n::flag<(sizeof(T)==1)>::value,int>::type f(T) { return 3; }
template<class T> typename enable_if<n::flag<(sizeof(T)!=1)>::value,int>::type f(T) { return 7; }
int main(){return f(char())!=3 || f(int())!=7;}
