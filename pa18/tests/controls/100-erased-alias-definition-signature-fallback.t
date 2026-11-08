// expect exit 0
// CWG 1558: an erased alias retains definition-signature substitution obligations.
template<class> using Void = void;
template<class T> int call(void*);
template<class T> int call(Void<typename T::type>*) { return 1; }
template<class T> int call(...) { return 2; }
struct Good { typedef int type; };
int main() { return call<int>(0) == 2 && call<Good>(0) == 1 ? 0 : 1; }
