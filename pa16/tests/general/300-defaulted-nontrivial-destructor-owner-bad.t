struct Owner {
  int number{3};
  Owner() = default;
  ~Owner() {}
  constexpr int get() const { return number; }
};
constexpr Owner invalid{};
int main() { return 0; }
