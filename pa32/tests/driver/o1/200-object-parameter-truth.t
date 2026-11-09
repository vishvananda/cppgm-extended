struct Node {
  long value;
  __attribute__((noinline)) int has_this() const { return this != nullptr; }
  __attribute__((noinline)) long branch_this() const {
    if (this) return value;
    return -99;
  }
  __attribute__((noinline)) const Node* select_this(const Node* other) const {
    return this ? this : other;
  }
};
extern "C" {
__attribute__((noinline)) bool reference_missing(const Node& value) {
  return &value == nullptr;
}
__attribute__((noinline)) bool pointer_missing(const Node* value) {
  return value == nullptr;
}
}
int main() {
  for (long i = -512; i != 512; ++i) {
    Node n = {i};
    if (n.has_this() != 1 || n.branch_this() != i || n.select_this(nullptr) != &n) return 1;
    if (reference_missing(n) || pointer_missing(&n) || !pointer_missing(nullptr)) return 2;
  }
  return 0;
}
