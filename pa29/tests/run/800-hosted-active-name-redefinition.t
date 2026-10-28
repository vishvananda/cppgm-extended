// GNU directives within macro arguments: the captured old definition expands,
// but its name stays disabled through replacement even after redefinition.
// Clang handles this extension differently; the hosted contract follows GCC.
int SELF(int x) { return x + 3; }
#define SELF(x) SELF x
int main() {
    int active = SELF(
#undef SELF
#define SELF(x) x
    (7));
    int after = SELF(11);
    return active != 10 || after != 11;
}
