// expect exit 0
struct Public { int value; } object;
template<class T> decltype(object.value) select(T);
template<class U> int select(U) { return 9; }
int main() { return select(0) == 9 ? 0 : 1; }
