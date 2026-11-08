// N3485 14.3p1/14.3.3p1: a type argument cannot fill a template-template slot; expect reject
struct symbol_set {};
template<class...> struct items {};
struct Term { void multiply(items<int>&, const symbol_set&) {} };
template<class T, template<class...> class TT, class... Args>
using bar = void (T::*)(TT<Args...>&, const symbol_set&);
bar<Term, int> invalid;
int main() {}
