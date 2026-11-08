// expect exit 0
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
template<class T> struct Outer { template<class U> struct Inner; }; template<class T> template<class U> struct Outer<T>::Inner { int value; }; int main() { Outer<int>::Inner<char> x = {3}; return x.value == 3 ? 0 : 1; }
