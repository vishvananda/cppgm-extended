struct Storage {};
struct Callable { constexpr int operator()() const { return 7; } };
template<class T> struct Static { static constexpr T value{}; };
template<class T> constexpr T Static<T>::value;
struct Deduce {
    template<class F> constexpr operator const F&() const {
        return __builtin_constant_p(reinterpret_cast<const F&>(Static<Storage>::value))
            ? reinterpret_cast<const F&>(Static<Storage>::value)
            : reinterpret_cast<const F&>(Static<Storage>::value);
    }
};
constexpr const Callable& callable = Deduce();
static_assert(&callable != nullptr, "folded reference has a constant storage address");
