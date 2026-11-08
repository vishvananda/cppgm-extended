// expect reject
struct Names { static void f(); static void f(int); };
template<class T> void body() { typedef decltype(T::f) Invalid; }
int main() { body<Names>(); }
