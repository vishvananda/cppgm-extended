template<class T> struct Manager { int run() { return 7; } };
typedef Manager<int> Proxy;
template<> int Proxy::run();
template<> int Proxy::run() { return 42; }
int main() { Proxy value; Manager<char> other; return value.run() == 42 && other.run() == 7 ? 0 : 1; }
