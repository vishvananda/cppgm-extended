template<class T> class Base { static int value; };
template<class T> struct Derived : Base<T> { static int get(); };
template<> int Derived<int>::get() { return value; }
int main() { return 0; }
