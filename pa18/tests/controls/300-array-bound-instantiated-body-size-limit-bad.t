// expect reject
// The same implementation array size limit applies outside immediate context.
template<class T> void body() { typedef T Invalid[sizeof(T) - 5]; }
int main() { body<int>(); }
