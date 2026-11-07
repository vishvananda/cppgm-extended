// expect exit 0
// N3485 9.5p6,6.7p4: a block-scope anonymous static union is one persistent object.
int next() { static union { int value; }; return ++value; }
int main() { return next() != 1 || next() != 2; }
