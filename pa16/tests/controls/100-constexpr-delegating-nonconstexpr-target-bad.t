// expect reject
// Adopted CWG1595: a user-provided non-constexpr target cannot be delegated to by a constexpr constructor.
struct Object { int n; Object():n(7) {} constexpr Object(int):Object() {} }; constexpr Object object(0); int main() { return 0; }
