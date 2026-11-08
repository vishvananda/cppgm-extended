struct Nonliteral { int value; Nonliteral(int n): value(n) {} };
constexpr auto closure = [object = Nonliteral(1)]() { return object.value; };
