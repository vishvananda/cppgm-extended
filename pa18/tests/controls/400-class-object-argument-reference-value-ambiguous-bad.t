// expect error
struct Literal { constexpr operator int() const { return 4; } };
constexpr Literal object = Literal();
template<const Literal&> int select();
template<int> char select();
int main() { select<object>(); }
