// N3485 focus: 7.1.5 [dcl.constexpr]; CWG 1684 removes the owner-type restriction.
struct nonliteral
{
  int n;
  ~nonliteral() {}
  constexpr int value() const { return n; }
  constexpr int fixed() const { return 7; }
  static constexpr int static_value() { return 11; }
};
static_assert(nonliteral::static_value() == 11, "static constexpr member");
int main()
{
  nonliteral object = {5};
  return object.value() + object.fixed() - 12;
}
