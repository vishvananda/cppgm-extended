// expect reject
// I13: N3922: direct-list auto requires exactly one initializer-clause.
int main() { auto item{1,2}; return 0; }
