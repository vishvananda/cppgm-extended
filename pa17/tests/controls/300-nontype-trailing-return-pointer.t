// expect exit 0
// N3485 8.3.5p2: this auto belongs to a function return, not a deduced non-type parameter.
template<auto (*Fn)() -> int> struct PointerParameter {};
int main() { return 0; }
