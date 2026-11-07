// expect exit 0
// N3485 9.5p6 permits a block-scope anonymous union's allowed variable storage specifier.
int main() { register union { int value; }; value = 3; return value != 3; }
