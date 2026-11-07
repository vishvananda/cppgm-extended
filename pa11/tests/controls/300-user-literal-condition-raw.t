// N3485 2.14.8 and 4.12: a user-defined literal is a call, whose result decides the condition.
// expect exit 0
int calls;
int operator "" _raw(const char* p) { ++calls; return p[0] == '0'; }
int main() { if (1_raw) return 1; if (!0_raw) return 2; return calls != 2; }
