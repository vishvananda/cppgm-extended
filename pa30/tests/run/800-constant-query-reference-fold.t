struct Storage {};
struct Callable { constexpr int operator()() const { return 7; } };
constexpr Storage storage{};
#define VIEW reinterpret_cast<const Callable&>(storage)
constexpr const Callable& callable = __builtin_constant_p(VIEW) ? VIEW : VIEW;
constexpr const Callable& parenthesized = ((__builtin_constant_p(VIEW))) ? VIEW : VIEW;
constexpr const Callable& converted = bool(__builtin_constant_p(VIEW)) ? VIEW : VIEW;
constexpr const Callable& nested = __builtin_constant_p(0) ? (false ? VIEW : VIEW) : VIEW;
constexpr const Callable* address = &callable;
static_assert(address == &callable, "folded reference preserves the storage address");
struct Holder {
    const Callable& ref;
    constexpr Holder(const Callable& value) : ref(value) {}
};
constexpr Holder holder = __builtin_constant_p(0) ? Holder(VIEW) : Holder(VIEW);
constexpr int discarded() {
    return (__builtin_constant_p(0) ? (void)VIEW : (void)VIEW), 7;
}
static_assert(discarded() == 7, "void conditionals preserve the same folding context");
int main() {
    return callable() != 7 || parenthesized() != 7 || converted() != 7 || nested() != 7 || address != &callable || holder.ref() != 7;
}
