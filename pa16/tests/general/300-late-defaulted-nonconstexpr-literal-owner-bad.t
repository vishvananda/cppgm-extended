struct Late {
  int number{3};
  Late();
  constexpr int get() const { return number; }
};
Late::Late() = default;
constexpr Late invalid{};
int main() { return 0; }
