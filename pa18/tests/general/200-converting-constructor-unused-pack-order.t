// A return conversion uses the same template ordering as a direct call.
struct Source {};
struct Target {
  int selected;
  template<class T, class = void> Target(T&&) : selected(1) {}
  template<class T, class... Rest> Target(T&&, Rest&&...) : selected(2) {}
};
Target make() { return Source{}; }
int take(Target value) { return value.selected; }
int main() {
  Target direct(Source{});
  Target copy = Source{};
  const Target& reference = Source{};
  Target&& rvalue = Source{};
  Target conditional = true ? Source{} : Target(Source{});
  return direct.selected != 1 || copy.selected != 1 || reference.selected != 1
      || rvalue.selected != 1 || conditional.selected != 1
      || take(Source{}) != 1 || make().selected != 1;
}
