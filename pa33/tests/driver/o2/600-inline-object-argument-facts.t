extern "C" long late_value(const long*);
struct Node {
  long value;
  __attribute__((always_inline)) inline long readable_call() const {
    return late_value(&value);
  }
};
extern "C" {
long late_value(const long* p) { return p ? *p : -99; }
__attribute__((noinline)) long path(const Node* p, bool active) {
  return active ? p->readable_call() : p == nullptr;
}
__attribute__((noinline)) long path_try(const Node* p, bool active, bool fail) {
  try {
    if (fail) throw 7;
    return active ? p->readable_call() : p == nullptr;
  } catch (int) {
    return p == nullptr;
  }
}
}
int main() {
  for (long i = -512; i != 512; ++i) {
    Node n = {i};
    if (path(&n, true) != i || path(&n, false) != 0 || path(nullptr, false) != 1) return 1;
    if (path_try(nullptr, true, true) != 1 || path_try(&n, true, true) != 0) return 2;
    if (path_try(&n, true, false) != i || path_try(nullptr, false, false) != 1) return 3;
  }
  return 0;
}
