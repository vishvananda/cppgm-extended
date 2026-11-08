// expect exit 0
struct Literal { constexpr operator int() const { return 4; } };
constexpr Literal object = Literal();
template<const Literal&> int select(int) { return 1; }
template<int> int select(...) { return 2; }
int main() { return select<object>(0) == 1 ? 0 : 1; }
