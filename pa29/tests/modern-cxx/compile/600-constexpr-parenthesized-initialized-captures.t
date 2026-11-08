constexpr auto first = [Captured(7)]() constexpr { return Captured; };
constexpr int check() {
  int value = 3;
  auto next = [&Reference(value), Copy(value)]() mutable constexpr {
    Reference += Copy; return Reference;
  };
  return next() + next();
}
static_assert(first() == 7 && check() == 15, "parenthesized capture constants");
int main() { return 0; }
