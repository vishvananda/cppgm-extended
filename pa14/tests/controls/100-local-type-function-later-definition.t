// expect exit 0
template<class T> int consume(T);
int main() { struct Local { int value; }; Local v = { 7 }; return consume(v) == 7 ? 0 : 1; }
template<class T> int consume(T object) { return object.value; }
