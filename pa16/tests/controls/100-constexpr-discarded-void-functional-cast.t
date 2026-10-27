// expect exit 0
// N3485 focus: 5.2.3 [expr.type.conv], 5.19 [expr.const].
// A discarded void functional cast still evaluates its operand, if present.
typedef void Nothing;
int runtime() { return 10; }
constexpr int empty() { return (void(), 7); }
constexpr int named() { return (Nothing(), 9); }
constexpr int converted() { return (void(3), 11); }
constexpr int nested() { return (static_cast<void>(Nothing()), 13); }
constexpr int selected(bool good) { return (good ? void() : void(runtime())), 15; }
static_assert(empty() == 7, "empty void value initialization");
static_assert(named() == 9, "named void value initialization");
static_assert(converted() == 11, "void conversion");
static_assert(nested() == 13, "nested discarded void cast");
static_assert(selected(true) == 15, "only the selected operand is evaluated");
int main() {
    return empty() != 7 || named() != 9 || converted() != 11 ||
           nested() != 13 || selected(true) != 15;
}
