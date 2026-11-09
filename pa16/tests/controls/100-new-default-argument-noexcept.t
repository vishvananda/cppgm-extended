// Allocation is noexcept only when its default arguments are nonthrowing.
// expect exit 0
typedef decltype(sizeof(0)) size_t;
int throwing_default();
struct Item {
  static void* operator new(size_t, int extra = throwing_default()) noexcept;
};
static_assert(!noexcept(new Item), "default argument can throw");
static_assert(noexcept(new (1) Item), "explicit argument is nonthrowing");
int main() { return 0; }
