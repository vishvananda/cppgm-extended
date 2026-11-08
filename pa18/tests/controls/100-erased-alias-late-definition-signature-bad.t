// expect reject
// CWG 1558: an erased alias retains definition-signature substitution obligations.
template<class> using Void = void;
template<class T> void call(void*);
int main() { call<int>(0); }
template<class T> void call(Void<typename T::type>*) {}
