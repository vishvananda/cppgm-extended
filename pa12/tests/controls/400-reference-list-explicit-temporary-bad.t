// expect reject
// I13: Adopted CWG2267: a reference-list temporary is copy-list-initialized and cannot use an explicit constructor.
struct E { explicit E(int) {} }; int main() { const E& object{1}; }
