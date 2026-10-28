struct Source { int value; };
struct Target { int value; };
constexpr Source source{7};
#define VIEW reinterpret_cast<const Target&>(source)
constexpr const Target& saved = __builtin_constant_p(VIEW) ? VIEW : VIEW;
constexpr const Target* pointer = &saved;
constexpr int value = pointer->value;
