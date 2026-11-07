// N3485 2.14.8 and 4.12: a user-defined literal is a call, whose result decides the condition.
// expect exit 0
int calls, conversions;
struct Flag { explicit operator bool() const { ++conversions; return false; } };
Flag operator "" _flag(unsigned long long) { ++calls; return Flag{}; }
int main() { if (1_flag) return 1; return calls != 1 || conversions != 1; }
