// expect reject
class Hidden { int value; } hidden;
template<class T> int select() { decltype(hidden.value) value = 0; return value; }
int main() { return select<int>(); }
