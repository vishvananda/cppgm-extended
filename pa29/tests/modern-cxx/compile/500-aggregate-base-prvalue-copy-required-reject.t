struct Base {
  Base() {}
  Base(const Base&) = delete;
  Base(Base&&) = delete;
};
Base make() { return Base(); }
struct Derived : Base { int value; };
Derived value{make(), 9};
