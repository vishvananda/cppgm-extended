// N3485 10.2p6, 11.3: an unqualified friend class name is ambiguous
// when different base classes declare different types of that name.
struct Left { struct Token {}; };
struct Right { struct Token {}; };
struct Derived: Left, Right { friend struct Token; };
int main() { return 0; }
