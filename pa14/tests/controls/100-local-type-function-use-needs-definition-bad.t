// expect error
template<class T> void consume(T);
int main() { struct Local { int value; }; Local v = { 7 }; consume(v); }
