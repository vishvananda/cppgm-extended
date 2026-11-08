// expect reject
// N3485 14.8.2: an instantiated body is outside the immediate context.
struct E { explicit E(int); }; template<class T> void sink(T);
template<class T> void body() { sink<T>({1}); }
int main() { body<E>(); }
