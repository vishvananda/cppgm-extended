// expect reject
class Hidden { int value; } hidden;
template<class T> struct Body { typedef decltype(hidden.value) type; };
template<class T> decltype(typename Body<T>::type()) select(int);
template<class T> int select(...);
int main() { select<int>(0); }
