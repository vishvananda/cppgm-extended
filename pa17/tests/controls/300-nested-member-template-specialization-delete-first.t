// CXX11-033; N3485 8.4.3p4 and 14.7.3p16. expect exit 0
// Each fully specialized member retains independent deletedness.
template<class T> struct Caller { template<class U> int action(U); };
template<> template<> int Caller<int>::action<double>(double) = delete;
template<> template<> int Caller<int>::action<char>(char) { return 4; }
int main() { Caller<int> object; return object.action<char>('x') - 4; }
