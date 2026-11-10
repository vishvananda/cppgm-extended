struct Source {};
struct Target {};
constexpr Source source{};
struct Bind {
    const Target& ref;
    constexpr Bind() : ref(reinterpret_cast<const Target&>(source)) {}
};
// The constructor's member initializer is in its own strict context.
constexpr const Target& ref = __builtin_constant_p(0) ? Bind().ref : Bind().ref;
