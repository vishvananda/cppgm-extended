// CXX11-033; N3485 8.4.3p4 and 14.7.3p16. expect exit 0
// A member declaration in the primary class template does not prohibit its first explicit specialization.
template<class T> struct Holder { int action(); };
template<> int Holder<int>::action() = delete;
template<> int Holder<char>::action() { return 9; }
int main() { Holder<char> object; return object.action() - 9; }
