// N3485 2.14.8 and 4.12: a user-defined literal is a call, whose result decides the condition.
// expect exit 0
int calls;
int operator "" _text(const char*, decltype(sizeof(0))) { ++calls; return 0; }
int main() { if ("x"_text) return 1; return calls != 1; }
