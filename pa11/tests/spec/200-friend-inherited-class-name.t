// N3485 3.4.1p8, 9p2 and 11.3: an inherited injected-class-name is
// found before the innermost namespace introduces a new friend class.
namespace inner { struct Derived; }
struct Base {
  const inner::Derived* down() const;
  int read(const inner::Derived&) const;
};
namespace inner {
struct Derived: private Base {
  friend struct Base;
  const Base* base() const { return this; }
private:
  int value = 7;
};
}
const inner::Derived* Base::down() const {
  return static_cast<const inner::Derived*>(this);
}
int Base::read(const inner::Derived& object) const { return object.value; }
int main() {
  inner::Derived object;
  return object.base()->down() == &object && object.base()->read(object) == 7 ? 0 : 1;
}
