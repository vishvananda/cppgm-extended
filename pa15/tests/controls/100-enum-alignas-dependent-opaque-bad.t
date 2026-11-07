// expect reject
// CWG2354 also applies when the opaque member enum has dependent alignment.
template<int N> struct A { enum alignas(N) E : int; };
A<8> value;
int main() { return 0; }
