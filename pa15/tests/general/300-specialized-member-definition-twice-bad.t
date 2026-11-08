template<class T> struct Manager { int run() { return 7; } };
typedef Manager<int> Proxy;
template<> int Proxy::run();
template<> int Proxy::run() { return 42; }
template<> int Proxy::run() { return 43; }
