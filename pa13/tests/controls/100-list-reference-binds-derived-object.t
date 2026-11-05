// N3485 8.5.4p3, 8.5.3p5: a single reference-related element binds the
// reference directly. Preserve identity and dynamic type; construct no base
// temporary, including for a default argument. expect exit 0
struct Abstract {
  static int copies;
  Abstract() {}
  Abstract(const Abstract&) { ++copies; }
  virtual int value() const = 0;
};
int Abstract::copies = 0;
struct Concrete : Abstract { int value() const { return 7; } };
Concrete global;
int read(const Abstract& value) { return &value == &global ? value.value() : 0; }
int defaulted(const Abstract& value = { global }) { return read(value); }
struct Holder {
  const Abstract& reference;
  Holder(const Abstract& value = { global }) : reference(value) {}
};
int main() {
  const Abstract& ordinary = global;
  if (read(ordinary) != 7 || read({global}) != 7 || defaulted() != 7) return 1;
  Holder object;
  if (&object.reference != &global || object.reference.value() != 7) return 2;
  return Abstract::copies == 0 ? 0 : 3;
}
