constexpr auto make() { int value = 2; return [&value]() { return value; }; }
constexpr auto closure = make();
static_assert(closure() == 2, "a reference cannot outlive its captured object");
