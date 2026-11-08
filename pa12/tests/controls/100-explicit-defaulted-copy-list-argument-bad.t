// expect reject
// I13: Adopted CWG1518: a braced function argument rejects the selected explicit default constructor.
struct B { B() = default; }; struct E : B { explicit E() = default; }; void take(E) {} int main() { take({}); }
