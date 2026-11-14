struct Arch { int value; };
constexpr Arch arch{7};
struct Small {
  const char* name;
  const Arch& target;
  int value;
};
constexpr Small small[] = {{"generic", arch, 19}, {"other", arch, 23}};
static_assert(small[0].value == 19, "member following a reference");
static_assert(small[1].target.value == 7, "reference in an array element");

struct Large {
  int a, b, c, d;
  const Arch& target;
  int value;
  int e, f, g, h;
};
constexpr Large large{1, 2, 3, 4, arch, 29, 5, 6, 7, 8};
static_assert(large.value == 29, "indexed member spans use target layout");
static_assert(large.target.value == 7, "indexed reference member");

extern "C" int observe() {
  return small[0].value + small[1].value + large.value + large.target.value;
}
