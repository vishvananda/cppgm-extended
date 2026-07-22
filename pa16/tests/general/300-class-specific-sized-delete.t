// VALIDATION: compile-pass
// N3485 focus: 5.3.5 [expr.delete]

struct object {
  virtual ~object() {}
  static void operator delete(void *, decltype(sizeof(0))) noexcept;
};

int main() { delete (object *)0; }
