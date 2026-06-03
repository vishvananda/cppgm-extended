int main() {
  int x = 1;
  auto f = [&]() -> int { return x; };
  return f() - 1;
}
