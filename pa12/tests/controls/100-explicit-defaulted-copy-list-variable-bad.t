// expect reject
// I13: Adopted CWG1518: an explicit defaulted constructor is not an aggregate copy-list bypass.
struct E { explicit E() = default; }; int main() { E object = {}; }
