#define ATTR __attribute__((__visibility__("default")))

namespace base {
int f();
}

namespace alias ATTR = base;

int main() {
  return 0;
}
