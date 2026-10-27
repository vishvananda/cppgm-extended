// N3485 13.3.3: a deleted exact copy assignment still beats conversion.
struct Base { int operator=(unsigned) { return 1; } };
struct Value : Base {
  Value(unsigned) {}
  operator unsigned() const { return 0; }
  Value& operator=(const Value&) = delete;
  using Base::operator=;
};
int main() { Value left(0), right(1); left = right; }
