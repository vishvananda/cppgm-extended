// GNU constant initialization does not add const or require a literal class.
struct Category {
  int value;
  constexpr Category(int n = 7) : value(n) {}
  virtual ~Category() {}
};
__constinit Category category;
__constinit int number = 3;
__constinit int zero;
__constinit int* pointer;
__constinit const int constant = 4;
static_assert(constant == 4, "const integral value remains usable");
void update() { ++number; }
