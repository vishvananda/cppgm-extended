template<class T> struct Left { static int value; };
template<class T> struct Right { static int value; };
template<class T> struct Derived : Left<T>, Right<T> { static int get(); };
template<> int Derived<int>::get() { return value; }
int main() { return 0; }
