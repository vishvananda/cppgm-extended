struct Base {
  Base() {}
  Base(const Base&) = delete;
  Base(Base&&) = delete;
};
Base make() { return Base(); }
struct Derived : Base { Derived() : Base(make()) {} };
