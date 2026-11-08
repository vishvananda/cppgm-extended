// expect reject
// N3485 14.5.3p5: a parameter pack must be expanded at its definition boundary.
template<void (*... F)()> struct Calls { Calls() { F(); } };
void target() {} int main() { Calls<target> value; }
