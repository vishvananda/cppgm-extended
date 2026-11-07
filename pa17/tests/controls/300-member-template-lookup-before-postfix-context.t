// N3485 3.4.5p1: class member-template lookup precedes the postfix context. expect exit 0
// RUN: %clang_cc1 -std=c++11 -fsyntax-only -verify %s

template<typename T>
struct set{};
struct Value {
  template<typename T>
  void set(T value) {}

  void resolves_to_same() {
    Value v;
    v.set<double>(3.2);
  }
};
void resolves_to_different() {
  {
    Value v;
    // The fact that the next line is a warning rather than an error is an
    // extension.
    v.set<double>(3.2);
  }
  {
    int set;  // Non-template.
    Value v;
    v.set<double>(3.2);
  }
}

int main() { Value v; v.resolves_to_same(); resolves_to_different(); return 0; }
