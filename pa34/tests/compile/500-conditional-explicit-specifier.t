// A conditional explicit-specifier decides whether the constructor takes part
// in copy initialization, and an attribute may sit between it and the rest of
// the declaration, which is how libc++ writes pair's constructors.
struct convertible {
  explicit(false) convertible(int) {}
};

struct guarded {
  explicit(1 == 1) guarded(int) {}
  guarded(long) {}
};

struct spaced {
  explicit(false) __attribute__((__visibility__("hidden"))) spaced(int) {}
};

convertible from_int() { return 7; }

guarded from_long() { return 7L; }

spaced from_attributed() { return 7; }

int main() { return 0; }
