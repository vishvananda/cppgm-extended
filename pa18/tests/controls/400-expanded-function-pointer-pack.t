// expect exit 0
// N3485 14.5.3p5: a parameter pack must be expanded at its definition boundary.
int count = 0; void first() { count += 1; } void second() { count += 2; }
template<void (*... F)()> struct Calls { Calls() { int effects[] = {0, (F(), 0)...}; } };
int main() { Calls<first, second> value; Calls<> empty; return count == 3 ? 0 : 1; }
