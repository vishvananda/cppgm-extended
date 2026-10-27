struct Nonliteral { Nonliteral() {} };
struct Owner {
  Nonliteral member;
  Owner() = default;
  constexpr int get() const { return 1; }
};
constexpr Owner invalid{};
int main() { return 0; }
