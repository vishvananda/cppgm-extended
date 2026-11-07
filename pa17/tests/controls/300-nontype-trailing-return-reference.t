// expect exit 0
// N3485 8.3.5p2,14.1p4: a reference to a function with its resolved trailing return type.
template<auto (&Fn)() -> int> struct ReferenceParameter {};
int main() { return 0; }
