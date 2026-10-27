// expect reject
// N3485 focus: 5.19 [expr.const]. Discarding a value does not skip a call.
int runtime() { return 10; }
constexpr int selected(bool good) { return (good ? void() : void(runtime())), 15; }
static_assert(selected(false) == 15, "a non-constexpr call is still evaluated");
int main() { return 0; }
