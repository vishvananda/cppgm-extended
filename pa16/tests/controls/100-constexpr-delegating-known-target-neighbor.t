// expect exit 0
// Adopted CWG1595: delegation to an actual constexpr target is valid.
struct Object { int n; constexpr Object():n(7) {} constexpr Object(int):Object() {} }; static_assert(Object(0).n==7,"target value"); int main() { return Object(0).n!=7; }
