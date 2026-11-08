constexpr auto closure = []() { static int value = 1; return value; };
static_assert(closure() == 1, "static storage is not a constant invocation");
