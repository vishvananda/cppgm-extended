// N3485 5.2.2p4, 14.8.2p8: the same incomplete argument in an
// instantiated function body is a hard error, not substitution failure.
// expect reject
struct Incomplete;
template <class T> T&& source();
template <class T> void consume(T);
template <class T> void body() { consume<T>(source<T>()); }
int main() { body<Incomplete>(); }
