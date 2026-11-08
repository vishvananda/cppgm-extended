// expect reject
// Adopted CWG1595: a user-provided non-constexpr target cannot be delegated to by a constexpr constructor.
struct Object { int n; Object():n(7) {} constexpr Object(int):Object() {} }; int main() { return 0; }
