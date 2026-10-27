// N3485 13.3.1p4, 13.3.3: using does not break tied argument conversions.
struct Base { int operator=(long) { return 1; } };
struct Value : Base {
  using Base::operator=;
  int operator=(double) { return 2; }
};
int main() { Value value; value = 1; }
