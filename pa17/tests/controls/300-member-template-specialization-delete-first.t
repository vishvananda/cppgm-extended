// CXX11-033; N3485 8.4.3p4 and 14.7.3. expect exit 0
// The primary member template does not prohibit a first deleted explicit specialization.
struct Caller { template<class T> int action(T) { return 3; } };
template<> int Caller::action<int>(int) = delete;
int main() { Caller object; return object.action<char>('x') - 3; }
