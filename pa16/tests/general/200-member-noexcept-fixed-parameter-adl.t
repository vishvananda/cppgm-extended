namespace lib {
struct Fixed {};
template<class T> int probe(T) noexcept(false) { return 3; }
template<class T> struct Wrapper {
  static int test(Fixed fixed, T value) noexcept(noexcept(probe(fixed))) { return probe(fixed) + value; }
};
int probe(Fixed) noexcept { return 9; }
}
static_assert(!noexcept(lib::Wrapper<int>::test(lib::Fixed{}, 0)), "fixed lookup stays at definition");
int main() { return lib::Wrapper<int>::test(lib::Fixed{}, 0) != 3; }
