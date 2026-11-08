// expect exit 0
struct Item { int run(int x) const & { return x + 2; } };
template<class R> int select(auto (Item::*p)(int) const & -> R) {
 Item i; return (i.*p)(5);
}
int main() { return select(&Item::run) == 7 ? 0 : 1; }
