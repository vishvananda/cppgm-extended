// expect exit 0
// I13: Adopted CWG1518: direct-list initialization may select the explicit defaulted constructor.
struct B { B() = default; }; struct E : B { explicit E() = default; }; int main() { E object{}; return 0; }
