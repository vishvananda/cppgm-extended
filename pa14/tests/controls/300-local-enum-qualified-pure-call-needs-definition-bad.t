// expect reject
template<class T> struct Base { virtual int action(const T&) = 0; };
template<class T> struct Derived : Base<T> { int action(const T&) { return 7; } };
int main() { enum Local {}; Derived<Local> object; return object.Base<Local>::action(Local()); }
