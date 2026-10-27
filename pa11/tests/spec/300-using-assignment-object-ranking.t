// N3485 13.3.1p4: using makes Base's assignment a member of Value for
// implicit-object ranking; its exact int argument beats conversion to long.
struct Base { int operator=(int) { return 1; } };
struct Value : Base {
  using Base::operator=;
  int operator=(long) { return 2; }
};
int main() {
  Value value;
  return (value = 0) == 1 && value.operator=(0) == 1 ? 0 : 1;
}
