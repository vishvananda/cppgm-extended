// expect exit 0
struct Literal { constexpr operator int() const { return 4; } };
constexpr Literal object = Literal();
template<int N> int read() { return N; }
int main() { return read<object>() == 4 ? 0 : 1; }
