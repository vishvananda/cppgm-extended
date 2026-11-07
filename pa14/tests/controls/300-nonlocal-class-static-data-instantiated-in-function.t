// expect exit 0
// N3485 9.4.2: instantiation inside a function does not make a namespace class local.
template<class T> struct Global { static const int value = 3; struct Nested { static const int value = 4; }; };
int main() { return Global<int>::value + Global<int>::Nested::value != 7; }
