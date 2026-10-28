// N3485 focus: 5.19 [expr.const] p2; designation need not read a parameter.
enum class Mode { precise, none };
struct Tag {
  int unknown;
  constexpr operator Mode() const { return Mode::precise; }
  constexpr int get() const { return 42; }
  constexpr bool same() const { return this == this; }
};
struct Derived : Tag {};
int classify(Tag mode) {
  constexpr bool precise = mode == Mode::precise;
  constexpr bool none = mode == Mode::none;
  constexpr int value = mode.get();
  constexpr bool same = mode.same();
  return precise && !none && value == 42 && same;
}
int classify_derived(Derived mode) {
  constexpr bool precise = mode == Mode::precise;
  return precise;
}
struct Read {
  int value;
  constexpr int get() const { return value; }
};
constexpr int known(Read mode) { return mode.get(); }
static_assert(known(Read{7}) == 7, "a parameter bound inside the evaluation is readable");
int main() { return !classify(Tag{9}) || !classify_derived(Derived{}); }
