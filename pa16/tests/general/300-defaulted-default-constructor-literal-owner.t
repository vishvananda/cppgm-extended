struct Scalar {
  unsigned long mantissa{0};
  int power{0};
  Scalar() = default;
  constexpr bool equal(const Scalar& rhs) const {
    return mantissa == rhs.mantissa && power == rhs.power;
  }
};
struct Nested {
  Scalar scalar{};
  int number{7};
  Nested() = default;
  constexpr int get() const { return number; }
};
struct Derived : Nested {
  Derived() = default;
  constexpr int value() const { return get(); }
};
class Private {
  int number{9};
public:
  Private() = default;
  constexpr int get() const { return number; }
};
constexpr Scalar first{};
constexpr Scalar second;
constexpr Nested nested{};
constexpr Derived derived;
constexpr Private hidden;
static_assert(first.equal(second), "defaulted constructor is implicitly constexpr");
static_assert(nested.get() == 7 && derived.value() == 7 && hidden.get() == 9, "subobjects and private member");
int main() { Scalar runtime; return !runtime.equal(first) || nested.get() != 7; }
