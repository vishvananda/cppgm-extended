int main() {
  auto plain = [](int value) -> auto { return value + 1; };
  auto generic = [](auto value) -> auto { return value + 2; };
  int value = 7;
  auto reference = [&]() -> decltype(auto) { return (value); };
  reference() = 9;
  return plain(3) != 4 || generic(4) != 6 || generic(1.5) != 3.5 || value != 9;
}
