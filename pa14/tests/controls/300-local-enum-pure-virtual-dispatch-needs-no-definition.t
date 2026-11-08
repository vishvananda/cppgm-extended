// expect exit 0
template<class T> struct Base {
 virtual int action(const T&) = 0;
 int call(const T& value) { return action(value); }
};
template<class T> struct Derived : Base<T> { int action(const T&) { return 7; } };
int main() { enum Local {}; return Derived<Local>().call(Local()) == 7 ? 0 : 1; }
