// N3485 2.14.8 and 4.12: a user-defined literal is a call, whose result decides the condition.
// expect exit 0
int calls;
template<char... C> int operator "" _count() { ++calls; return sizeof...(C) != 3; }
int main() { if (123_count) return 1; if (!12_count) return 2; return calls != 2; }
