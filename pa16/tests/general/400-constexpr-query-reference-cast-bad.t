struct Source {};
struct Target {};
constexpr Source source{};
// The course's query does not enable hosted GNU reference folding.
constexpr const Target& target = __builtin_constant_p(0)
    ? reinterpret_cast<const Target&>(source) : reinterpret_cast<const Target&>(source);
