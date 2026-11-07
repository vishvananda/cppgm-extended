// N3485 2.14.8 and 4.12: a user-defined literal is a call, whose result decides the condition.
// expect exit 0
int calls;
int* operator "" _pointer(unsigned long long) { ++calls; return nullptr; }
int main() { if (1_pointer) return 1; return calls != 1; }
